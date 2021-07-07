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
#include <ThunderEgg/Iterative/BiCGStab.h>
#include <ThunderEgg/Poisson/StarPatchOperator.h>

#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace std;
using namespace ThunderEgg;
using namespace ThunderEgg::Iterative;

TEST_CASE("BiCGStab default max iterations", "[BiCGStab]")
{
	BiCGStab<2> bcgs;
	CHECK(bcgs.getMaxIterations() == 1000);
}
TEST_CASE("BiCGStab set max iterations", "[BiCGStab]")
{
	BiCGStab<2> bcgs;
	int         iterations = GENERATE(1, 2, 3);
	bcgs.setMaxIterations(iterations);
	CHECK(bcgs.getMaxIterations() == iterations);
}
TEST_CASE("BiCGStab default tolerance", "[BiCGStab]")
{
	BiCGStab<2> bcgs;
	CHECK(bcgs.getTolerance() == 1e-12);
}
TEST_CASE("BiCGStab set tolerance", "[BiCGStab]")
{
	BiCGStab<2> bcgs;
	double      tolerance = GENERATE(1.2, 2.3, 3.4);
	bcgs.setTolerance(tolerance);
	CHECK(bcgs.getTolerance() == tolerance);
}
TEST_CASE("BiCGStab default timer", "[BiCGStab]")
{
	BiCGStab<2> bcgs;
	CHECK(bcgs.getTimer() == nullptr);
}
TEST_CASE("BiCGStab set timer", "[BiCGStab]")
{
	Communicator comm(MPI_COMM_WORLD);
	BiCGStab<2>  bcgs;
	auto         timer = make_shared<Timer>(comm);
	bcgs.setTimer(timer);
	CHECK(bcgs.getTimer() == timer);
}
TEST_CASE("BiCGStab clone", "[BiCGStab]")
{
	BiCGStab<2> bcgs;
	int         iterations = GENERATE(1, 2, 3);
	bcgs.setMaxIterations(iterations);

	double tolerance = GENERATE(1.2, 2.3, 3.4);
	bcgs.setTolerance(tolerance);

	Communicator comm(MPI_COMM_WORLD);
	auto         timer = make_shared<Timer>(comm);
	bcgs.setTimer(timer);

	unique_ptr<BiCGStab<2>> clone(bcgs.clone());
	CHECK(bcgs.getTimer() == clone->getTimer());
	CHECK(bcgs.getMaxIterations() == clone->getMaxIterations());
	CHECK(bcgs.getTolerance() == clone->getTolerance());
}
TEST_CASE("BiCGStab solves poisson problem withing given tolerance", "[BiCGStab]")
{
	string mesh_file = "mesh_inputs/2d_uniform_2x2_mpi1.json";
	INFO("MESH FILE " << mesh_file);
	DomainReader<2>       domain_reader(mesh_file, {32, 32}, 1);
	shared_ptr<Domain<2>> domain = domain_reader.getCoarserDomain();

	auto ffun = [](const std::array<double, 2> &coord) {
		double x = coord[0];
		double y = coord[1];
		return -5 * M_PI * M_PI * sin(M_PI * y) * cos(2 * M_PI * x);
	};
	auto gfun = [](const std::array<double, 2> &coord) {
		double x = coord[0];
		double y = coord[1];
		return sin(M_PI * y) * cos(2 * M_PI * x);
	};

	Vector<2> f_vec(*domain, 1);
	DomainTools::SetValues<2>(*domain, f_vec, ffun);
	Vector<2> residual(*domain, 1);

	Vector<2> g_vec(*domain, 1);

	auto gf = make_shared<BiLinearGhostFiller>(domain, GhostFillingType::Faces);

	Poisson::StarPatchOperator<2> p_operator(domain, gf);
	p_operator.addDrichletBCToRHS(f_vec, gfun);

	double tolerance = GENERATE(1e-9, 1e-7, 1e-5);

	BiCGStab<2> solver;
	solver.setMaxIterations(1000);
	solver.setTolerance(tolerance);
	solver.solve(p_operator, g_vec, f_vec);

	p_operator.apply(g_vec, residual);
	residual.addScaled(-1, f_vec);
	CHECK(residual.dot(residual) / f_vec.dot(f_vec) <= tolerance);
}
TEST_CASE("BiCGStab handles zero rhs vector", "[BiCGStab]")
{
	string mesh_file = "mesh_inputs/2d_uniform_2x2_mpi1.json";
	INFO("MESH FILE " << mesh_file);
	DomainReader<2>       domain_reader(mesh_file, {32, 32}, 1);
	shared_ptr<Domain<2>> domain = domain_reader.getCoarserDomain();

	Vector<2> f_vec(*domain, 1);

	Vector<2> g_vec(*domain, 1);

	auto gf = make_shared<BiLinearGhostFiller>(domain, GhostFillingType::Faces);

	Poisson::StarPatchOperator<2> p_operator(domain, gf);

	double tolerance = GENERATE(1e-9, 1e-7, 1e-5);

	BiCGStab<2> solver;
	solver.setMaxIterations(1000);
	solver.setTolerance(tolerance);
	solver.solve(p_operator, g_vec, f_vec);

	CHECK(g_vec.infNorm() == 0);
}
TEST_CASE("outputs iteration count and residual to output", "[BiCGStab]")
{
	string mesh_file = "mesh_inputs/2d_uniform_2x2_mpi1.json";
	INFO("MESH FILE " << mesh_file);
	DomainReader<2>       domain_reader(mesh_file, {32, 32}, 1);
	shared_ptr<Domain<2>> domain = domain_reader.getCoarserDomain();

	auto ffun = [](const std::array<double, 2> &coord) {
		double x = coord[0];
		double y = coord[1];
		return -5 * M_PI * M_PI * sin(M_PI * y) * cos(2 * M_PI * x);
	};
	auto gfun = [](const std::array<double, 2> &coord) {
		double x = coord[0];
		double y = coord[1];
		return sin(M_PI * y) * cos(2 * M_PI * x);
	};

	Vector<2> f_vec(*domain, 1);
	DomainTools::SetValues<2>(*domain, f_vec, ffun);
	Vector<2> residual(*domain, 1);

	Vector<2> g_vec(*domain, 1);

	auto gf = make_shared<BiLinearGhostFiller>(domain, GhostFillingType::Faces);

	Poisson::StarPatchOperator<2> p_operator(domain, gf);
	p_operator.addDrichletBCToRHS(f_vec, gfun);

	double tolerance = 1e-7;

	std::stringstream ss;

	BiCGStab<2> solver;
	solver.setMaxIterations(1000);
	solver.setTolerance(tolerance);
	solver.solve(p_operator, g_vec, f_vec, nullptr,
	             true, ss);

	INFO(ss.str());
	int    prev_iteration;
	double resid;
	ss >> prev_iteration >> resid;
	while (prev_iteration < 5) {
		int iteration;
		ss >> iteration >> resid;
		CHECK(iteration == prev_iteration + 1);
		prev_iteration = iteration;
	}
}
TEST_CASE("giving a good initial guess reduces the iterations", "[BiCGStab]")
{
	string mesh_file = "mesh_inputs/2d_uniform_2x2_mpi1.json";
	INFO("MESH FILE " << mesh_file);
	DomainReader<2>       domain_reader(mesh_file, {32, 32}, 1);
	shared_ptr<Domain<2>> domain = domain_reader.getCoarserDomain();

	auto ffun = [](const std::array<double, 2> &coord) {
		double x = coord[0];
		double y = coord[1];
		return -5 * M_PI * M_PI * sin(M_PI * y) * cos(2 * M_PI * x);
	};
	auto gfun = [](const std::array<double, 2> &coord) {
		double x = coord[0];
		double y = coord[1];
		return sin(M_PI * y) * cos(2 * M_PI * x);
	};

	Vector<2> f_vec(*domain, 1);
	DomainTools::SetValues<2>(*domain, f_vec, ffun);
	Vector<2> residual(*domain, 1);

	Vector<2> g_vec(*domain, 1);

	auto gf = make_shared<BiLinearGhostFiller>(domain, GhostFillingType::Faces);

	Poisson::StarPatchOperator<2> p_operator(domain, gf);
	p_operator.addDrichletBCToRHS(f_vec, gfun);

	double tolerance = 1e-5;

	BiCGStab<2> solver;
	solver.setMaxIterations(1000);
	solver.setTolerance(tolerance);
	solver.solve(p_operator, g_vec, f_vec);

	int iterations_with_solved_guess
	= solver.solve(p_operator, g_vec, f_vec);

	CHECK(iterations_with_solved_guess == 0);
}
namespace
{
class MockOperator : public Operator<2>
{
	public:
	void apply(const Vector<2> &, Vector<2> &) const override {}
};
class I2Operator : public Operator<2>
{
	public:
	void apply(const Vector<2> &x, Vector<2> &y) const override
	{
		y.copy(x);
		y.scale(2);
	}
	I2Operator *clone() const override
	{
		return new I2Operator(*this);
	}
};
} // namespace
TEST_CASE("BiCGStab solves poisson 2I problem", "[BiCGStab]")
{
	string mesh_file = "mesh_inputs/2d_uniform_2x2_mpi1.json";
	INFO("MESH FILE " << mesh_file);
	DomainReader<2>       domain_reader(mesh_file, {32, 32}, 1);
	shared_ptr<Domain<2>> domain = domain_reader.getCoarserDomain();

	auto ffun = [](const std::array<double, 2> &coord) {
		double x = coord[0];
		double y = coord[1];
		return -5 * M_PI * M_PI * sin(M_PI * y) * cos(2 * M_PI * x);
	};

	Vector<2> f_vec(*domain, 1);
	DomainTools::SetValues<2>(*domain, f_vec, ffun);
	Vector<2> residual(*domain, 1);

	Vector<2> g_vec(*domain, 1);

	auto gf = make_shared<BiLinearGhostFiller>(domain, GhostFillingType::Faces);

	I2Operator op;

	double tolerance = GENERATE(1e-9, 1e-7, 1e-5);

	BiCGStab<2> solver;
	solver.setMaxIterations(1000);
	solver.setTolerance(tolerance);
	solver.solve(op, g_vec, f_vec);

	op.apply(g_vec, residual);
	residual.addScaled(-1, f_vec);
	CHECK(residual.dot(residual) / f_vec.dot(f_vec) <= tolerance);
}