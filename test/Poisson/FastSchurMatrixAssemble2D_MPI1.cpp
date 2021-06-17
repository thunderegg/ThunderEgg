/***************************************************************************
 *  ThunderEgg, a library for solving Poisson's equation on adaptively
 *  refined block-structured Cartesian grids
 *
 *  Copyright (C) 2019  ThunderEgg Developers. See AUTHORS.md file at the
 *  top-level directory.
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

#include "../utils/DomainReader.h"
#include <ThunderEgg/BiLinearGhostFiller.h>
#include <ThunderEgg/BiQuadraticGhostFiller.h>
#include <ThunderEgg/DomainTools.h>
#include <ThunderEgg/GMG/LinearRestrictor.h>
#include <ThunderEgg/PETSc/MatWrapper.h>
#include <ThunderEgg/Poisson/FFTWPatchSolver.h>
#include <ThunderEgg/Poisson/FastSchurMatrixAssemble2D.h>
#include <ThunderEgg/Poisson/StarPatchOperator.h>
#include <ThunderEgg/Schur/PatchSolverWrapper.h>
#include <ThunderEgg/Schur/ValVectorGenerator.h>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace std;
using namespace ThunderEgg;

#define MESHES \
	"mesh_inputs/2d_uniform_2x2_mpi1.json", "mesh_inputs/2d_uniform_8x8_refined_cross_mpi1.json"
const string mesh_file = "mesh_inputs/2d_uniform_4x4_mpi1.json";

TEST_CASE("Poisson::FastSchurMatrixAssemble2D throws exception for non-square patches",
          "[Poisson::FastSchurMatrixAssemble2D]")
{
	auto mesh_file = GENERATE(as<std::string>{}, MESHES);
	INFO("MESH FILE " << mesh_file);
	int                   nx        = GENERATE(5, 8);
	int                   ny        = GENERATE(7, 10);
	int                   num_ghost = 1;
	bitset<4>             neumann;
	DomainReader<2>       domain_reader(mesh_file, {nx, ny}, num_ghost);
	shared_ptr<Domain<2>> d_fine       = domain_reader.getFinerDomain();
	auto                  iface_domain = make_shared<Schur::InterfaceDomain<2>>(d_fine);

	auto gf         = make_shared<BiQuadraticGhostFiller>(d_fine, GhostFillingType::Faces);
	auto p_operator = make_shared<Poisson::StarPatchOperator<2>>(d_fine, gf);
	auto p_solver   = make_shared<Poisson::FFTWPatchSolver<2>>(p_operator, neumann);

	CHECK_THROWS_AS(Poisson::FastSchurMatrixAssemble2D(iface_domain, p_solver), RuntimeError);
}
namespace ThunderEgg
{
namespace
{
template <int D>
class MockGhostFiller : public GhostFiller<D>
{
	public:
	void fillGhost(const Vector<D> &u) const override {}
};
} // namespace
} // namespace ThunderEgg
TEST_CASE("Poisson::FastSchurMatrixAssemble2D throws with unsupported ghost filler",
          "[Poisson::FastSchurMatrixAssemble2D]")
{
	auto mesh_file = GENERATE(as<std::string>{}, MESHES);
	INFO("MESH FILE " << mesh_file);
	int                   num_ghost = 1;
	bitset<4>             neumann;
	DomainReader<2>       domain_reader(mesh_file, {10, 10}, num_ghost);
	shared_ptr<Domain<2>> d_fine       = domain_reader.getFinerDomain();
	auto                  iface_domain = make_shared<Schur::InterfaceDomain<2>>(d_fine);

	auto gf         = make_shared<MockGhostFiller<2>>();
	auto p_operator = make_shared<Poisson::StarPatchOperator<2>>(d_fine, gf);
	auto p_solver   = make_shared<Poisson::FFTWPatchSolver<2>>(p_operator, neumann);

	CHECK_THROWS_AS(Poisson::FastSchurMatrixAssemble2D(iface_domain, p_solver), RuntimeError);
}
TEST_CASE(
"Poisson::FastSchurMatrixAssemble2D gives equivalent operator to Poisson::StarPatchOperator bilinear ghost filler",
"[Poisson::FastSchurMatrixAssemble2D]")
{
	auto mesh_file = GENERATE(as<std::string>{}, MESHES);
	INFO("MESH FILE " << mesh_file);
	int                   n         = 32;
	int                   num_ghost = 1;
	bitset<4>             neumann;
	DomainReader<2>       domain_reader(mesh_file, {n, n}, num_ghost);
	shared_ptr<Domain<2>> d_fine       = domain_reader.getFinerDomain();
	auto                  iface_domain = make_shared<Schur::InterfaceDomain<2>>(d_fine);

	Schur::ValVectorGenerator<1> vg(iface_domain);
	auto                         f_vec          = vg.getNewVector();
	auto                         f_vec_expected = vg.getNewVector();

	auto g_vec = vg.getNewVector();

	int index = 0;
	for (auto iface_info : iface_domain->getInterfaces()) {
		View<double, 1> view = g_vec->getComponentView(0, iface_info->local_index);
		for (int i = 0; i < n; i++) {
			double x = (index + 0.5) / g_vec->getNumLocalCells();
			view(i)  = sin(M_PI * x);
			index++;
		}
	}

	auto gf               = make_shared<BiLinearGhostFiller>(d_fine, GhostFillingType::Faces);
	auto p_operator       = make_shared<Poisson::StarPatchOperator<2>>(d_fine, gf);
	auto p_solver         = make_shared<Poisson::FFTWPatchSolver<2>>(p_operator, neumann);
	auto p_solver_wrapper = make_shared<Schur::PatchSolverWrapper<2>>(iface_domain, p_solver);
	p_solver_wrapper->apply(*g_vec, *f_vec_expected);

	// generate matrix with matrix_helper
	Mat  A          = Poisson::FastSchurMatrixAssemble2D(iface_domain, p_solver);
	auto m_operator = make_shared<PETSc::MatWrapper<1>>(A);
	m_operator->apply(*g_vec, *f_vec);

	CHECK(f_vec->infNorm() == Catch::Approx(f_vec_expected->infNorm()));
	CHECK(f_vec->twoNorm() == Catch::Approx(f_vec_expected->twoNorm()));
	REQUIRE(f_vec->infNorm() > 0);

	for (auto iface : iface_domain->getInterfaces()) {
		INFO("ID: " << iface->id);
		INFO("LOCAL_INDEX: " << iface->local_index);
		INFO("type: " << iface->patches.size());
		ComponentView<double, 1> f_vec_ld          = f_vec->getComponentView(0, iface->local_index);
		ComponentView<double, 1> f_vec_expected_ld = f_vec_expected->getComponentView(0, iface->local_index);
		nested_loop<1>(f_vec_ld.getStart(), f_vec_ld.getEnd(), [&](const array<int, 1> &coord) {
			INFO("xi:    " << coord[0]);
			CHECK(f_vec_ld[coord] == Catch::Approx(f_vec_expected_ld[coord]));
		});
	}
	MatDestroy(&A);
}
TEST_CASE(
"Poisson::FastSchurMatrixAssemble2D gives equivalent operator to Poisson::StarPatchOperator with Neumann BC bilinear ghost filler",
"[Poisson::FastSchurMatrixAssemble2D]")
{
	auto mesh_file = GENERATE(as<std::string>{}, MESHES);
	INFO("MESH FILE " << mesh_file);
	int                   n         = 32;
	int                   num_ghost = 1;
	bitset<4>             neumann   = 0xF;
	DomainReader<2>       domain_reader(mesh_file, {n, n}, num_ghost);
	shared_ptr<Domain<2>> d_fine       = domain_reader.getFinerDomain();
	auto                  iface_domain = make_shared<Schur::InterfaceDomain<2>>(d_fine);

	Schur::ValVectorGenerator<1> vg(iface_domain);
	auto                         f_vec          = vg.getNewVector();
	auto                         f_vec_expected = vg.getNewVector();

	auto g_vec = vg.getNewVector();

	int index = 0;
	for (auto iface_info : iface_domain->getInterfaces()) {
		View<double, 1> view = g_vec->getComponentView(0, iface_info->local_index);
		for (int i = 0; i < n; i++) {
			double x = (index + 0.5) / g_vec->getNumLocalCells();
			view(i)  = sin(M_PI * x);
			index++;
		}
	}

	auto gf               = make_shared<BiLinearGhostFiller>(d_fine, GhostFillingType::Faces);
	auto p_operator       = make_shared<Poisson::StarPatchOperator<2>>(d_fine, gf, true);
	auto p_solver         = make_shared<Poisson::FFTWPatchSolver<2>>(p_operator, neumann);
	auto p_solver_wrapper = make_shared<Schur::PatchSolverWrapper<2>>(iface_domain, p_solver);
	p_solver_wrapper->apply(*g_vec, *f_vec_expected);

	// generate matrix with matrix_helper
	Mat  A          = Poisson::FastSchurMatrixAssemble2D(iface_domain, p_solver);
	auto m_operator = make_shared<PETSc::MatWrapper<1>>(A);
	m_operator->apply(*g_vec, *f_vec);

	CHECK(f_vec->infNorm() == Catch::Approx(f_vec_expected->infNorm()));
	CHECK(f_vec->twoNorm() == Catch::Approx(f_vec_expected->twoNorm()));
	REQUIRE(f_vec->infNorm() > 0);

	for (int i = 0; i < f_vec->getNumLocalPatches(); i++) {
		ComponentView<double, 1> f_vec_ld          = f_vec->getComponentView(0, i);
		ComponentView<double, 1> f_vec_expected_ld = f_vec_expected->getComponentView(0, i);
		nested_loop<1>(f_vec_ld.getStart(), f_vec_ld.getEnd(), [&](const array<int, 1> &coord) {
			INFO("xi:    " << coord[0]);
			CHECK(f_vec_ld[coord] == Catch::Approx(f_vec_expected_ld[coord]));
		});
	}
	MatDestroy(&A);
}
TEST_CASE(
"Poisson::FastSchurMatrixAssemble2D gives equivalent operator to Poisson::StarPatchOperator Biquadratic ghost filler",
"[Poisson::FastSchurMatrixAssemble2D]")
{
	auto mesh_file = GENERATE(as<std::string>{}, MESHES);
	INFO("MESH FILE " << mesh_file);
	int                   n         = 32;
	int                   num_ghost = 1;
	bitset<4>             neumann;
	DomainReader<2>       domain_reader(mesh_file, {n, n}, num_ghost);
	shared_ptr<Domain<2>> d_fine       = domain_reader.getFinerDomain();
	auto                  iface_domain = make_shared<Schur::InterfaceDomain<2>>(d_fine);

	Schur::ValVectorGenerator<1> vg(iface_domain);
	auto                         f_vec          = vg.getNewVector();
	auto                         f_vec_expected = vg.getNewVector();

	auto g_vec = vg.getNewVector();

	int index = 0;
	for (auto iface_info : iface_domain->getInterfaces()) {
		View<double, 1> view = g_vec->getComponentView(0, iface_info->local_index);
		for (int i = 0; i < n; i++) {
			double x = (index + 0.5) / g_vec->getNumLocalCells();
			view(i)  = sin(M_PI * x);
			index++;
		}
	}

	auto gf               = make_shared<BiQuadraticGhostFiller>(d_fine, GhostFillingType::Faces);
	auto p_operator       = make_shared<Poisson::StarPatchOperator<2>>(d_fine, gf);
	auto p_solver         = make_shared<Poisson::FFTWPatchSolver<2>>(p_operator, neumann);
	auto p_solver_wrapper = make_shared<Schur::PatchSolverWrapper<2>>(iface_domain, p_solver);
	p_solver_wrapper->apply(*g_vec, *f_vec_expected);

	// generate matrix with matrix_helper
	Mat  A          = Poisson::FastSchurMatrixAssemble2D(iface_domain, p_solver);
	auto m_operator = make_shared<PETSc::MatWrapper<1>>(A);
	m_operator->apply(*g_vec, *f_vec);

	CHECK(f_vec->infNorm() == Catch::Approx(f_vec_expected->infNorm()));
	CHECK(f_vec->twoNorm() == Catch::Approx(f_vec_expected->twoNorm()));
	REQUIRE(f_vec->infNorm() > 0);

	for (int i = 0; i < f_vec->getNumLocalPatches(); i++) {
		ComponentView<double, 1> f_vec_ld          = f_vec->getComponentView(0, i);
		ComponentView<double, 1> f_vec_expected_ld = f_vec_expected->getComponentView(0, i);
		nested_loop<1>(f_vec_ld.getStart(), f_vec_ld.getEnd(), [&](const array<int, 1> &coord) {
			INFO("xi:    " << coord[0]);
			CHECK(f_vec_ld[coord] == Catch::Approx(f_vec_expected_ld[coord]));
		});
	}
	MatDestroy(&A);
}
TEST_CASE(
"Poisson::FastSchurMatrixAssemble2D gives equivalent operator to Poisson::StarPatchOperator with Neumann BC BiQuadratic ghost filler",
"[Poisson::FastSchurMatrixAssemble2D]")
{
	auto mesh_file = GENERATE(as<std::string>{}, MESHES);
	INFO("MESH FILE " << mesh_file);
	int                   n         = 32;
	int                   num_ghost = 1;
	bitset<4>             neumann   = 0xF;
	DomainReader<2>       domain_reader(mesh_file, {n, n}, num_ghost);
	shared_ptr<Domain<2>> d_fine       = domain_reader.getFinerDomain();
	auto                  iface_domain = make_shared<Schur::InterfaceDomain<2>>(d_fine);

	Schur::ValVectorGenerator<1> vg(iface_domain);
	auto                         f_vec          = vg.getNewVector();
	auto                         f_vec_expected = vg.getNewVector();

	auto g_vec = vg.getNewVector();

	int index = 0;
	for (auto iface_info : iface_domain->getInterfaces()) {
		View<double, 1> view = g_vec->getComponentView(0, iface_info->local_index);
		for (int i = 0; i < n; i++) {
			double x = (index + 0.5) / g_vec->getNumLocalCells();
			view(i)  = sin(M_PI * x);
			index++;
		}
	}

	auto gf               = make_shared<BiQuadraticGhostFiller>(d_fine, GhostFillingType::Faces);
	auto p_operator       = make_shared<Poisson::StarPatchOperator<2>>(d_fine, gf, true);
	auto p_solver         = make_shared<Poisson::FFTWPatchSolver<2>>(p_operator, neumann);
	auto p_solver_wrapper = make_shared<Schur::PatchSolverWrapper<2>>(iface_domain, p_solver);
	p_solver_wrapper->apply(*g_vec, *f_vec_expected);

	// generate matrix with matrix_helper
	Mat  A          = Poisson::FastSchurMatrixAssemble2D(iface_domain, p_solver);
	auto m_operator = make_shared<PETSc::MatWrapper<1>>(A);
	m_operator->apply(*g_vec, *f_vec);

	CHECK(f_vec->infNorm() == Catch::Approx(f_vec_expected->infNorm()));
	CHECK(f_vec->twoNorm() == Catch::Approx(f_vec_expected->twoNorm()));
	REQUIRE(f_vec->infNorm() > 0);

	for (int i = 0; i < f_vec->getNumLocalPatches(); i++) {
		ComponentView<double, 1> f_vec_ld          = f_vec->getComponentView(0, i);
		ComponentView<double, 1> f_vec_expected_ld = f_vec_expected->getComponentView(0, i);
		nested_loop<1>(f_vec_ld.getStart(), f_vec_ld.getEnd(), [&](const array<int, 1> &coord) {
			INFO("xi:    " << coord[0]);
			CHECK(f_vec_ld[coord] == Catch::Approx(f_vec_expected_ld[coord]));
		});
	}
	MatDestroy(&A);
}