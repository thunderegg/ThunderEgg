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
#include <ThunderEgg/DomainTools.h>
#include <ThunderEgg/GMG/LinearRestrictor.h>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace std;
using namespace ThunderEgg;

const string mesh_file = "mesh_inputs/2d_uniform_quad_mpi2.json";

TEST_CASE("Linear Test LinearRestrictor", "[GMG::LinearRestrictor]")
{
	auto            nx        = GENERATE(2, 10);
	auto            ny        = GENERATE(2, 10);
	int             num_ghost = 1;
	DomainReader<2> domain_reader(mesh_file, {nx, ny}, num_ghost);
	Domain<2>       d_fine   = domain_reader.getFinerDomain();
	Domain<2>       d_coarse = domain_reader.getCoarserDomain();

	Vector<2> fine_vec(d_fine, 1);
	Vector<2> coarse_expected(d_coarse, 1);

	auto f = [&](const std::array<double, 2> coord) -> double {
		double x = coord[0];
		double y = coord[1];
		return 1 + ((x * 0.3) + y);
	};

	DomainTools::SetValuesWithGhost<2>(d_fine, fine_vec, f);
	DomainTools::SetValuesWithGhost<2>(d_coarse, coarse_expected, f);

	GMG::LinearRestrictor<2> restrictor(d_fine, d_coarse, true);

	Vector<2> coarse_vec = restrictor.restrict(fine_vec);

	for (auto pinfo : d_coarse.getPatchInfoVector()) {
		INFO("Patch: " << pinfo.id);
		INFO("x:     " << pinfo.starts[0]);
		INFO("y:     " << pinfo.starts[1]);
		INFO("nx:    " << pinfo.ns[0]);
		INFO("ny:    " << pinfo.ns[1]);
		ComponentView<double, 2> vec_ld      = coarse_vec.getComponentView(0, pinfo.local_index);
		ComponentView<double, 2> expected_ld = coarse_expected.getComponentView(0, pinfo.local_index);
		Loop::Nested<2>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 2> &coord) {
			REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord]));
		});
		for (Side<2> s : Side<2>::getValues()) {
			View<double, 1> vec_ghost      = vec_ld.getSliceOn(s, {-1});
			View<double, 1> expected_ghost = expected_ld.getSliceOn(s, {-1});
			if (!pinfo.hasNbr(s)) {
				INFO("side:      " << s);
				Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(),
				                [&](const array<int, 1> &coord) {
					                INFO("coord:  " << coord[0]);
					                CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
				                });
			}
		}
	}
}