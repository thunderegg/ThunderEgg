/***************************************************************************
 *  ThunderEgg, a library for solvers on adaptively refined block-structured 
 *  Cartesian grids.
 *
 *  Copyright (c) 2020-2021 Scott Aiton
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 ***************************************************************************/
#include "PatchSolver_MOCKS.h"
#include "utils/DomainReader.h"
#include <ThunderEgg/DomainTools.h>
#include <ThunderEgg/MPIGhostFiller.h>

#include <list>
#include <sstream>

#include <catch2/generators/catch_generators.hpp>

using namespace std;
using namespace ThunderEgg;

constexpr auto single_mesh_file  = "mesh_inputs/2d_uniform_2x2_mpi1.json";
constexpr auto refined_mesh_file = "mesh_inputs/2d_uniform_2x2_refined_nw_mpi1.json";
constexpr auto cross_mesh_file   = "mesh_inputs/2d_uniform_8x8_refined_cross_mpi1.json";

TEST_CASE("PatchSolver apply for various domains", "[PatchSolver]")
{
	auto mesh_file
	= GENERATE(as<std::string>{}, single_mesh_file, refined_mesh_file, cross_mesh_file);
	INFO("MESH: " << mesh_file);
	auto            nx        = GENERATE(2, 5);
	auto            ny        = GENERATE(2, 5);
	int             num_ghost = 1;
	DomainReader<2> domain_reader(mesh_file, {nx, ny}, num_ghost);
	Domain<2>       d_fine = domain_reader.getFinerDomain();

	auto      u_num_components = GENERATE(1, 2, 3);
	Vector<2> u(d_fine, u_num_components);
	auto      f_num_components = GENERATE(1, 2, 3);
	Vector<2> f(d_fine, f_num_components);

	MockGhostFiller<2> mgf;
	MockPatchSolver<2> mps(d_fine, mgf);

	u.setWithGhost(1);
	mps.apply(f, u);

	for (int i = 0; i < u.getNumLocalPatches(); i++) {
		for (int c = 0; c < u.getNumComponents(); c++) {
			auto ld = u.getComponentView(c, i);
			Loop::Nested<2>(ld.getStart(), ld.getEnd(),
			                [&](const std::array<int, 2> &coord) { CHECK(ld[coord] == 0); });
		}
	}
	CHECK_FALSE(mgf.wasCalled());
	CHECK(mps.allPatchesCalled());
}
TEST_CASE("PatchSolver apply for various domains with timer", "[PatchSolver]")
{
	Communicator comm(MPI_COMM_WORLD);
	auto         mesh_file
	= GENERATE(as<std::string>{}, single_mesh_file, refined_mesh_file, cross_mesh_file);
	INFO("MESH: " << mesh_file);
	auto            nx        = GENERATE(2, 5);
	auto            ny        = GENERATE(2, 5);
	int             num_ghost = 1;
	DomainReader<2> domain_reader(mesh_file, {nx, ny}, num_ghost);
	Domain<2>       d_fine = domain_reader.getFinerDomain();
	d_fine.setTimer(make_shared<Timer>(comm));

	auto      u_num_components = GENERATE(1, 2, 3);
	Vector<2> u(d_fine, u_num_components);
	auto      f_num_components = GENERATE(1, 2, 3);
	Vector<2> f(d_fine, f_num_components);

	MockGhostFiller<2> mgf;
	MockPatchSolver<2> mps(d_fine, mgf);

	u.setWithGhost(1);
	mps.apply(f, u);

	for (int i = 0; i < u.getNumLocalPatches(); i++) {
		for (int c = 0; c < u.getNumComponents(); c++) {
			auto ld = u.getComponentView(c, i);
			Loop::Nested<2>(ld.getStart(), ld.getEnd(),
			                [&](const std::array<int, 2> &coord) { CHECK(ld[coord] == 0); });
		}
	}
	CHECK_FALSE(mgf.wasCalled());
	CHECK(mps.allPatchesCalled());
	stringstream ss;
	ss << *d_fine.getTimer();
	CHECK(ss.str().find("Total Patch") != string::npos);
	CHECK(ss.str().find("Single Patch") != string::npos);
}
TEST_CASE("PatchSolver smooth for various domains", "[PatchSolver]")
{
	Communicator comm(MPI_COMM_WORLD);
	auto         mesh_file
	= GENERATE(as<std::string>{}, single_mesh_file, refined_mesh_file, cross_mesh_file);
	INFO("MESH: " << mesh_file);
	auto            nx        = GENERATE(2, 5);
	auto            ny        = GENERATE(2, 5);
	int             num_ghost = 1;
	DomainReader<2> domain_reader(mesh_file, {nx, ny}, num_ghost);
	Domain<2>       d_fine = domain_reader.getFinerDomain();
	d_fine.setTimer(make_shared<Timer>(comm));

	auto      u_num_components = GENERATE(1, 2, 3);
	Vector<2> u(d_fine, u_num_components);
	auto      f_num_components = GENERATE(1, 2, 3);
	Vector<2> f(d_fine, f_num_components);

	MockGhostFiller<2> mgf;
	MockPatchSolver<2> mps(d_fine, mgf);

	u.setWithGhost(1);
	mps.smooth(f, u);

	for (int i = 0; i < u.getNumLocalPatches(); i++) {
		for (int c = 0; c < u.getNumComponents(); c++) {
			auto ld = u.getComponentView(c, i);
			Loop::Nested<2>(ld.getStart(), ld.getEnd(),
			                [&](const std::array<int, 2> &coord) { CHECK(ld[coord] == 1); });
		}
	}
	CHECK(mgf.wasCalled());
	CHECK(mps.allPatchesCalled());
	stringstream ss;
	ss << *d_fine.getTimer();
	CHECK(ss.str().find("Total Patch") != string::npos);
	CHECK(ss.str().find("Single Patch") != string::npos);
}
TEST_CASE("PatchSolver smooth for various domains with timer", "[PatchSolver]")
{
	auto mesh_file
	= GENERATE(as<std::string>{}, single_mesh_file, refined_mesh_file, cross_mesh_file);
	INFO("MESH: " << mesh_file);
	auto            nx        = GENERATE(2, 5);
	auto            ny        = GENERATE(2, 5);
	int             num_ghost = 1;
	DomainReader<2> domain_reader(mesh_file, {nx, ny}, num_ghost);
	Domain<2>       d_fine = domain_reader.getFinerDomain();

	auto      u_num_components = GENERATE(1, 2, 3);
	Vector<2> u(d_fine, u_num_components);
	auto      f_num_components = GENERATE(1, 2, 3);
	Vector<2> f(d_fine, f_num_components);

	MockGhostFiller<2> mgf;
	MockPatchSolver<2> mps(d_fine, mgf);

	u.setWithGhost(1);
	mps.smooth(f, u);

	for (int i = 0; i < u.getNumLocalPatches(); i++) {
		for (int c = 0; c < u.getNumComponents(); c++) {
			auto ld = u.getComponentView(c, i);
			Loop::Nested<2>(ld.getStart(), ld.getEnd(),
			                [&](const std::array<int, 2> &coord) { CHECK(ld[coord] == 1); });
		}
	}
	CHECK(mgf.wasCalled());
	CHECK(mps.allPatchesCalled());
}
TEST_CASE("PatchSolver getDomain", "[PatchSolver]")
{
	auto mesh_file
	= GENERATE(as<std::string>{}, single_mesh_file, refined_mesh_file, cross_mesh_file);
	INFO("MESH: " << mesh_file);
	auto            nx        = GENERATE(2, 5);
	auto            ny        = GENERATE(2, 5);
	int             num_ghost = 1;
	DomainReader<2> domain_reader(mesh_file, {nx, ny}, num_ghost);
	Domain<2>       d_fine = domain_reader.getFinerDomain();

	Vector<2> u(d_fine, 1);
	Vector<2> f(d_fine, 1);

	MockGhostFiller<2> mgf;
	MockPatchSolver<2> mps(d_fine, mgf);

	CHECK(mps.getDomain().getNumLocalPatches() == d_fine.getNumLocalPatches());
}
TEST_CASE("PatchSolver getGhostFiller", "[PatchSolver]")
{
	auto mesh_file
	= GENERATE(as<std::string>{}, single_mesh_file, refined_mesh_file, cross_mesh_file);
	INFO("MESH: " << mesh_file);
	auto            nx        = GENERATE(2, 5);
	auto            ny        = GENERATE(2, 5);
	int             num_ghost = 1;
	DomainReader<2> domain_reader(mesh_file, {nx, ny}, num_ghost);
	Domain<2>       d_fine = domain_reader.getFinerDomain();

	Vector<2> u(d_fine, 1);
	Vector<2> f(d_fine, 1);

	MockGhostFiller<2> mgf;
	MockPatchSolver<2> mps(d_fine, mgf);

	const GhostFiller<2> &mps_mgf = mps.getGhostFiller();
	CHECK(typeid(mps_mgf) == typeid(mgf));
}