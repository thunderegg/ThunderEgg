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
#include "utils/DomainReader.h"
#include <ThunderEgg/DomainTools.h>
#include <ThunderEgg/RuntimeError.h>
#include <ThunderEgg/TriLinearGhostFiller.h>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

using namespace std;
using namespace ThunderEgg;

constexpr auto single_mesh_file = "mesh_inputs/3d_uniform_2x2x2_mpi1.json";
constexpr auto refined_mesh_file = "mesh_inputs/3d_refined_bnw_2x2x2_mpi1.json";

TEST_CASE("exchange various meshes 3D TriLinearGhostFiller")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 1);
          Vector<3> expected(d, 1);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };

          DomainTools::SetValues<3>(d, vec, f);
          DomainTools::SetValuesWithGhost<3>(d, expected, f);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Faces);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            Loop::Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
          }
        }
      }
    }
  }
}
TEST_CASE("exchange various meshes 3D TriLinearGhostFiller two components")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 2);
          Vector<3> expected(d, 2);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };
          auto g = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 100 + 20 * x + 9 * y + 7 * z;
          };

          DomainTools::SetValues<3>(d, vec, f, g);
          DomainTools::SetValuesWithGhost<3>(d, expected, f, g);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Faces);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> vec_ld2 = vec.getComponentView(1, pinfo.local_index);
            ComponentView<double, 3> expected_ld2 = expected.getComponentView(1, pinfo.local_index);
            Loop::Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            Loop::Loop::Nested<3>(vec_ld2.getStart(), vec_ld2.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld2[coord] == Catch::Approx(expected_ld2[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld2.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld2.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
          }
        }
      }
    }
  }
}
TEST_CASE("exchange various meshes 3D TriLinearGhostFiller ghost already set two components")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 2);
          Vector<3> expected(d, 2);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };
          auto g = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 100 + 20 * x + 9 * y + 7 * z;
          };

          DomainTools::SetValuesWithGhost<3>(d, vec, f, g);
          DomainTools::SetValuesWithGhost<3>(d, expected, f, g);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Faces);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> vec_ld2 = vec.getComponentView(1, pinfo.local_index);
            ComponentView<double, 3> expected_ld2 = expected.getComponentView(1, pinfo.local_index);
            Loop::Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            Loop::Loop::Nested<3>(vec_ld2.getStart(), vec_ld2.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld2[coord] == Catch::Approx(expected_ld2[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld2.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld2.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
          }
        }
      }
    }
  }
}
TEST_CASE("exchange various meshes 3D TriLinearGhostFiller ghost already set")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 1);
          Vector<3> expected(d, 1);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };

          DomainTools::SetValuesWithGhost<3>(d, vec, f);
          DomainTools::SetValuesWithGhost<3>(d, expected, f);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Faces);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            Loop::Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
          }
        }
      }
    }
  }
}
TEST_CASE("TriLinearGhostFiller constructor throws error with odd number of cells")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (GhostFillingType fill_type : { GhostFillingType::Faces, GhostFillingType::Edges, GhostFillingType::Corners }) {
      for (auto axis : { 0, 1, 2 }) {
        INFO("MESH: " << mesh_file);
        INFO("axis: " << axis);
        int n_even = 10;
        int n_odd = 11;
        int num_ghost = 1;

        array<int, 3> ns;
        ns.fill(n_even);
        ns[axis] = n_odd;
        DomainReader<3> domain_reader(mesh_file, ns, num_ghost);
        Domain<3> d = domain_reader.getFinerDomain();

        CHECK_THROWS_AS(TriLinearGhostFiller(d, fill_type), RuntimeError);
      }
    }
  }
}
TEST_CASE("exchange various meshes 3D TriLinearGhostFiller edges")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 1);
          Vector<3> expected(d, 1);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };

          DomainTools::SetValues<3>(d, vec, f);
          DomainTools::SetValuesWithGhost<3>(d, expected, f);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Edges);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            Loop::Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
          }
        }
      }
    }
  }
}
TEST_CASE("exchange various meshes 3D TriLinearGhostFiller two components edges")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 2);
          Vector<3> expected(d, 2);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };
          auto g = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 100 + 20 * x + 9 * y + 7 * z;
          };

          DomainTools::SetValues<3>(d, vec, f, g);
          DomainTools::SetValuesWithGhost<3>(d, expected, f, g);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Edges);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> vec_ld2 = vec.getComponentView(1, pinfo.local_index);
            ComponentView<double, 3> expected_ld2 = expected.getComponentView(1, pinfo.local_index);
            Loop::Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            Loop::Loop::Nested<3>(vec_ld2.getStart(), vec_ld2.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld2[coord] == Catch::Approx(expected_ld2[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld2.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld2.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld2.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld2.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
          }
        }
      }
    }
  }
}
TEST_CASE("exchange various meshes 3D TriLinearGhostFiller ghost already set two components edges")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 2);
          Vector<3> expected(d, 2);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };
          auto g = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 100 + 20 * x + 9 * y + 7 * z;
          };

          DomainTools::SetValuesWithGhost<3>(d, vec, f, g);
          DomainTools::SetValuesWithGhost<3>(d, expected, f, g);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Edges);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> vec_ld2 = vec.getComponentView(1, pinfo.local_index);
            ComponentView<double, 3> expected_ld2 = expected.getComponentView(1, pinfo.local_index);
            Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            Loop::Nested<3>(vec_ld2.getStart(), vec_ld2.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld2[coord] == Catch::Approx(expected_ld2[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld2.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld2.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld2.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld2.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
          }
        }
      }
    }
  }
}
TEST_CASE("exchange various meshes 3D TriLinearGhostFiller ghost already set edges")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 1);
          Vector<3> expected(d, 1);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };

          DomainTools::SetValuesWithGhost<3>(d, vec, f);
          DomainTools::SetValuesWithGhost<3>(d, expected, f);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Edges);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
          }
        }
      }
    }
  }
}
TEST_CASE("exchange various meshes 3D TriLinearGhostFiller corners")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 1);
          Vector<3> expected(d, 1);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };

          DomainTools::SetValues<3>(d, vec, f);
          DomainTools::SetValuesWithGhost<3>(d, expected, f);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Corners);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Corner<3> c : Corner<3>::getValues()) {
              View<double, 0> vec_ghost = vec_ld.getSliceOn(c, { -1, -1, -1 });
              View<double, 0> expected_ghost = expected_ld.getSliceOn(c, { -1, -1, -1 });
              if (pinfo.hasNbr(c)) {
                INFO("side:      " << c);
                INFO("nbr-type:  " << pinfo.getNbrType(c));
                CHECK(vec_ghost[{}] == Catch::Approx(expected_ghost[{}]));
              }
            }
          }
        }
      }
    }
  }
}
TEST_CASE("exchange various meshes 3D TriLinearGhostFiller two components corners")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 2);
          Vector<3> expected(d, 2);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };
          auto g = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 100 + 20 * x + 9 * y + 7 * z;
          };

          DomainTools::SetValues<3>(d, vec, f, g);
          DomainTools::SetValuesWithGhost<3>(d, expected, f, g);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Corners);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> vec_ld2 = vec.getComponentView(1, pinfo.local_index);
            ComponentView<double, 3> expected_ld2 = expected.getComponentView(1, pinfo.local_index);
            Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Corner<3> c : Corner<3>::getValues()) {
              View<double, 0> vec_ghost = vec_ld.getSliceOn(c, { -1, -1, -1 });
              View<double, 0> expected_ghost = expected_ld.getSliceOn(c, { -1, -1, -1 });
              if (pinfo.hasNbr(c)) {
                INFO("side:      " << c);
                INFO("nbr-type:  " << pinfo.getNbrType(c));
                CHECK(vec_ghost[{}] == Catch::Approx(expected_ghost[{}]));
              }
            }
            Loop::Nested<3>(vec_ld2.getStart(), vec_ld2.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld2[coord] == Catch::Approx(expected_ld2[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld2.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld2.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld2.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld2.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Corner<3> c : Corner<3>::getValues()) {
              View<double, 0> vec_ghost = vec_ld2.getSliceOn(c, { -1, -1, -1 });
              View<double, 0> expected_ghost = expected_ld2.getSliceOn(c, { -1, -1, -1 });
              if (pinfo.hasNbr(c)) {
                INFO("side:      " << c);
                INFO("nbr-type:  " << pinfo.getNbrType(c));
                CHECK(vec_ghost[{}] == Catch::Approx(expected_ghost[{}]));
              }
            }
          }
        }
      }
    }
  }
}
TEST_CASE("exchange various meshes 3D TriLinearGhostFiller ghost already set two components corners")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 2);
          Vector<3> expected(d, 2);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };
          auto g = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 100 + 20 * x + 9 * y + 7 * z;
          };

          DomainTools::SetValuesWithGhost<3>(d, vec, f, g);
          DomainTools::SetValuesWithGhost<3>(d, expected, f, g);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Corners);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> vec_ld2 = vec.getComponentView(1, pinfo.local_index);
            ComponentView<double, 3> expected_ld2 = expected.getComponentView(1, pinfo.local_index);
            Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Corner<3> c : Corner<3>::getValues()) {
              View<double, 0> vec_ghost = vec_ld.getSliceOn(c, { -1, -1, -1 });
              View<double, 0> expected_ghost = expected_ld.getSliceOn(c, { -1, -1, -1 });
              if (pinfo.hasNbr(c)) {
                INFO("side:      " << c);
                INFO("nbr-type:  " << pinfo.getNbrType(c));
                CHECK(vec_ghost[{}] == Catch::Approx(expected_ghost[{}]));
              }
            }
            Loop::Nested<3>(vec_ld2.getStart(), vec_ld2.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld2[coord] == Catch::Approx(expected_ld2[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld2.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld2.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld2.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld2.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Corner<3> c : Corner<3>::getValues()) {
              View<double, 0> vec_ghost = vec_ld2.getSliceOn(c, { -1, -1, -1 });
              View<double, 0> expected_ghost = expected_ld2.getSliceOn(c, { -1, -1, -1 });
              if (pinfo.hasNbr(c)) {
                INFO("side:      " << c);
                INFO("nbr-type:  " << pinfo.getNbrType(c));
                CHECK(vec_ghost[{}] == Catch::Approx(expected_ghost[{}]));
              }
            }
          }
        }
      }
    }
  }
}
TEST_CASE("exchange various meshes 3D TriLinearGhostFiller ghost already set corners")
{
  for (auto mesh_file : { single_mesh_file, refined_mesh_file }) {
    for (auto nx : { 4, 6 }) {
      for (auto ny : { 4, 6 }) {
        for (auto nz : { 4, 6 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;

          DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
          Domain<3> d = domain_reader.getFinerDomain();

          Vector<3> vec(d, 1);
          Vector<3> expected(d, 1);

          auto f = [&](const std::array<double, 3> coord) -> double {
            double x = coord[0];
            double y = coord[1];
            double z = coord[2];
            return 1 + 0.5 * x + y + 7 * z;
          };

          DomainTools::SetValuesWithGhost<3>(d, vec, f);
          DomainTools::SetValuesWithGhost<3>(d, expected, f);

          TriLinearGhostFiller tlgf(d, GhostFillingType::Corners);
          tlgf.fillGhost(vec);

          for (auto pinfo : d.getPatchInfoVector()) {
            INFO("Patch: " << pinfo.id);
            INFO("x:     " << pinfo.starts[0]);
            INFO("y:     " << pinfo.starts[1]);
            INFO("z:     " << pinfo.starts[2]);
            INFO("nx:    " << pinfo.ns[0]);
            INFO("ny:    " << pinfo.ns[1]);
            INFO("nz:    " << pinfo.ns[2]);
            ComponentView<double, 3> vec_ld = vec.getComponentView(0, pinfo.local_index);
            ComponentView<double, 3> expected_ld = expected.getComponentView(0, pinfo.local_index);
            Loop::Nested<3>(vec_ld.getStart(), vec_ld.getEnd(), [&](const array<int, 3>& coord) { REQUIRE(vec_ld[coord] == Catch::Approx(expected_ld[coord])); });
            for (Side<3> s : Side<3>::getValues()) {
              View<double, 2> vec_ghost = vec_ld.getSliceOn(s, { -1 });
              View<double, 2> expected_ghost = expected_ld.getSliceOn(s, { -1 });
              if (pinfo.hasNbr(s)) {
                INFO("side:      " << s);
                INFO("nbr-type:  " << pinfo.getNbrType(s));
                Loop::Nested<2>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 2>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Edge e : Edge::getValues()) {
              View<double, 1> vec_ghost = vec_ld.getSliceOn(e, { -1, -1 });
              View<double, 1> expected_ghost = expected_ld.getSliceOn(e, { -1, -1 });
              if (pinfo.hasNbr(e)) {
                INFO("side:      " << e);
                INFO("nbr-type:  " << pinfo.getNbrType(e));
                Loop::Nested<1>(vec_ghost.getStart(), vec_ghost.getEnd(), [&](const array<int, 1>& coord) {
                  INFO("coord:  " << coord[0] << ", " << coord[1]);
                  CHECK(vec_ghost[coord] == Catch::Approx(expected_ghost[coord]));
                });
              }
            }
            for (Corner<3> c : Corner<3>::getValues()) {
              View<double, 0> vec_ghost = vec_ld.getSliceOn(c, { -1, -1, -1 });
              View<double, 0> expected_ghost = expected_ld.getSliceOn(c, { -1, -1, -1 });
              if (pinfo.hasNbr(c)) {
                INFO("side:      " << c);
                INFO("nbr-type:  " << pinfo.getNbrType(c));
                CHECK(vec_ghost[{}] == Catch::Approx(expected_ghost[{}]));
              }
            }
          }
        }
      }
    }
  }
}
