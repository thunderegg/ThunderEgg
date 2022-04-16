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
#include "MPIGhostFiller_MOCKS.h"
#include "utils/DomainReader.h"
#include <ThunderEgg/DomainTools.h>
#include <ThunderEgg/MPIGhostFiller.h>

#include <list>

#include <catch2/generators/catch_generators.hpp>

using namespace std;
using namespace ThunderEgg;

constexpr auto single_mesh_file = "mesh_inputs/2d_uniform_2x2_mpi1.json";
constexpr auto refined_mesh_file = "mesh_inputs/2d_uniform_2x2_refined_nw_mpi1.json";
constexpr auto cross_mesh_file = "mesh_inputs/2d_uniform_8x8_refined_cross_mpi1.json";

constexpr auto td_uniform = "mesh_inputs/3d_uniform_2x2x2_mpi1.json";
constexpr auto td_refined = "mesh_inputs/3d_refined_bnw_2x2x2_mpi1.json";
constexpr auto td_mid_refine = "mesh_inputs/3d_mid_refine_4x4x4_mpi1.json";

TEST_CASE("No calls for 1 patch domain")
{
  for (auto num_components : { 1, 2 }) {
    for (auto nx : { 2, 3 }) {
      for (auto ny : { 2, 3 }) {
        for (GhostFillingType fill_type : { GhostFillingType::Faces, GhostFillingType::Edges, GhostFillingType::Corners }) {
          int num_ghost = 1;
          DomainReader<2> domain_reader(single_mesh_file, { nx, ny }, num_ghost);
          Domain<2> d_coarse = domain_reader.getCoarserDomain();

          Vector<2> vec(d_coarse, num_components);

          CallMockMPIGhostFiller<2> mgf(d_coarse, num_components, fill_type);

          mgf.fillGhost(vec);

          CHECK(mgf.called == true);
          mgf.checkCalls();
        }
      }
    }
  }
}
TEST_CASE("Calls for various domains 2d face cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { single_mesh_file, refined_mesh_file, cross_mesh_file }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;
          DomainReader<2> domain_reader(mesh_file, { nx, ny }, num_ghost);
          Domain<2> d_fine = domain_reader.getFinerDomain();

          Vector<2> vec(d_fine, num_components);

          CallMockMPIGhostFiller<2> mgf(d_fine, num_components, GhostFillingType::Faces);

          mgf.fillGhost(vec);

          CHECK(mgf.called == true);

          mgf.checkCalls();
        }
      }
    }
  }
}
TEST_CASE("Calls for various domains 2d corner cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { single_mesh_file, refined_mesh_file, cross_mesh_file }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          INFO("MESH: " << mesh_file);
          int num_ghost = 1;
          DomainReader<2> domain_reader(mesh_file, { nx, ny }, num_ghost);
          Domain<2> d_fine = domain_reader.getFinerDomain();

          Vector<2> vec(d_fine, num_components);

          CallMockMPIGhostFiller<2> mgf(d_fine, num_components, GhostFillingType::Corners);

          mgf.fillGhost(vec);

          CHECK(mgf.called == true);

          mgf.checkCalls();
        }
      }
    }
  }
}
TEST_CASE("Calls for various domains 3d face cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { td_uniform, td_refined, td_mid_refine }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (auto nz : { 2, 3 }) {
            INFO("MESH: " << mesh_file);
            int num_ghost = 1;
            DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
            Domain<3> d_fine = domain_reader.getFinerDomain();

            Vector<3> vec(d_fine, num_components);

            CallMockMPIGhostFiller<3> mgf(d_fine, num_components, GhostFillingType::Faces);

            mgf.fillGhost(vec);

            CHECK(mgf.called == true);

            mgf.checkCalls();
          }
        }
      }
    }
  }
}
TEST_CASE("Calls for various domains 3d edge cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { td_uniform, td_refined, td_mid_refine }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (auto nz : { 2, 3 }) {
            INFO("MESH: " << mesh_file);
            int num_ghost = 1;
            DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
            Domain<3> d_fine = domain_reader.getFinerDomain();

            Vector<3> vec(d_fine, num_components);

            CallMockMPIGhostFiller<3> mgf(d_fine, num_components, GhostFillingType::Edges);

            mgf.fillGhost(vec);

            CHECK(mgf.called == true);

            mgf.checkCalls();
          }
        }
      }
    }
  }
}
TEST_CASE("Calls for various domains 3d corner cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { td_uniform, td_refined, td_mid_refine }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (auto nz : { 2, 3 }) {
            INFO("MESH: " << mesh_file);
            int num_ghost = 1;
            DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
            Domain<3> d_fine = domain_reader.getFinerDomain();

            Vector<3> vec(d_fine, num_components);

            CallMockMPIGhostFiller<3> mgf(d_fine, num_components, GhostFillingType::Corners);

            mgf.fillGhost(vec);

            CHECK(mgf.called == true);

            mgf.checkCalls();
          }
        }
      }
    }
  }
}
TEST_CASE("Exchange for various domains 2d face cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { single_mesh_file, refined_mesh_file, cross_mesh_file }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (int num_ghost : { 1, 2 }) {
            INFO("MESH: " << mesh_file);
            DomainReader<2> domain_reader(mesh_file, { nx, ny }, num_ghost);
            Domain<2> d_fine = domain_reader.getFinerDomain();

            Vector<2> vec(d_fine, num_components);
            for (auto pinfo : d_fine.getPatchInfoVector()) {
              for (int c = 0; c < num_components; c++) {
                auto data = vec.getComponentView(c, pinfo.local_index);
                Loop::Nested<2>(data.getStart(), data.getEnd(), [&](const std::array<int, 2>& coord) { data[coord] = pinfo.id; });
              }
            }

            ExchangeMockMPIGhostFiller<2> mgf(d_fine, GhostFillingType::Faces);

            mgf.fillGhost(vec);

            mgf.checkVector(vec);
          }
        }
      }
    }
  }
}

TEST_CASE("Exchange for various domains 2d corner cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { single_mesh_file, refined_mesh_file, cross_mesh_file }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (int num_ghost : { 1, 2 }) {
            INFO("MESH: " << mesh_file);
            DomainReader<2> domain_reader(mesh_file, { nx, ny }, num_ghost);
            Domain<2> d_fine = domain_reader.getFinerDomain();

            Vector<2> vec(d_fine, num_components);
            for (auto pinfo : d_fine.getPatchInfoVector()) {
              for (int c = 0; c < num_components; c++) {
                auto data = vec.getComponentView(c, pinfo.local_index);
                Loop::Nested<2>(data.getStart(), data.getEnd(), [&](const std::array<int, 2>& coord) { data[coord] = pinfo.id; });
              }
            }

            ExchangeMockMPIGhostFiller<2> mgf(d_fine, GhostFillingType::Corners);

            mgf.fillGhost(vec);

            mgf.checkVector(vec);
          }
        }
      }
    }
  }
}

TEST_CASE("Exchange for various domains 3d face cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { td_uniform, td_refined, td_mid_refine }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (auto nz : { 2, 3 }) {
            for (int num_ghost : { 1, 2 }) {
              INFO("MESH: " << mesh_file);
              DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
              Domain<3> d_fine = domain_reader.getFinerDomain();

              Vector<3> vec(d_fine, num_components);
              for (auto pinfo : d_fine.getPatchInfoVector()) {
                for (int c = 0; c < num_components; c++) {
                  auto data = vec.getComponentView(c, pinfo.local_index);
                  Loop::Nested<3>(data.getStart(), data.getEnd(), [&](const std::array<int, 3>& coord) { data[coord] = pinfo.id; });
                }
              }

              ExchangeMockMPIGhostFiller<3> mgf(d_fine, GhostFillingType::Faces);

              mgf.fillGhost(vec);

              mgf.checkVector(vec);
            }
          }
        }
      }
    }
  }
}
TEST_CASE("Exchange for various domains 3d edge cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { td_uniform, td_refined, td_mid_refine }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (auto nz : { 2, 3 }) {
            for (int num_ghost : { 1, 2 }) {
              INFO("MESH: " << mesh_file);
              DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
              Domain<3> d_fine = domain_reader.getFinerDomain();

              Vector<3> vec(d_fine, num_components);
              for (auto pinfo : d_fine.getPatchInfoVector()) {
                for (int c = 0; c < num_components; c++) {
                  auto data = vec.getComponentView(c, pinfo.local_index);
                  Loop::Nested<3>(data.getStart(), data.getEnd(), [&](const std::array<int, 3>& coord) { data[coord] = pinfo.id; });
                }
              }

              ExchangeMockMPIGhostFiller<3> mgf(d_fine, GhostFillingType::Edges);

              mgf.fillGhost(vec);

              mgf.checkVector(vec);
            }
          }
        }
      }
    }
  }
}
TEST_CASE("Exchange for various domains 3d corner cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { td_uniform, td_refined, td_mid_refine }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (auto nz : { 2, 3 }) {
            for (int num_ghost : { 1, 2 }) {
              INFO("MESH: " << mesh_file);
              DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
              Domain<3> d_fine = domain_reader.getFinerDomain();

              Vector<3> vec(d_fine, num_components);
              for (auto pinfo : d_fine.getPatchInfoVector()) {
                for (int c = 0; c < num_components; c++) {
                  auto data = vec.getComponentView(c, pinfo.local_index);
                  Loop::Nested<3>(data.getStart(), data.getEnd(), [&](const std::array<int, 3>& coord) { data[coord] = pinfo.id; });
                }
              }

              ExchangeMockMPIGhostFiller<3> mgf(d_fine, GhostFillingType::Corners);

              mgf.fillGhost(vec);

              mgf.checkVector(vec);
            }
          }
        }
      }
    }
  }
}

TEST_CASE("Two Exchanges for various domains 2d face cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { single_mesh_file, refined_mesh_file, cross_mesh_file }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (int num_ghost : { 1, 2 }) {
            INFO("MESH: " << mesh_file);
            DomainReader<2> domain_reader(mesh_file, { nx, ny }, num_ghost);
            Domain<2> d_fine = domain_reader.getFinerDomain();

            Vector<2> vec(d_fine, num_components);
            for (auto pinfo : d_fine.getPatchInfoVector()) {
              for (int c = 0; c < num_components; c++) {
                auto data = vec.getComponentView(c, pinfo.local_index);
                Loop::Nested<2>(data.getStart(), data.getEnd(), [&](const std::array<int, 2>& coord) { data[coord] = pinfo.id; });
              }
            }

            ExchangeMockMPIGhostFiller<2> mgf(d_fine, GhostFillingType::Faces);

            mgf.fillGhost(vec);
            mgf.fillGhost(vec);

            mgf.checkVector(vec);
          }
        }
      }
    }
  }
}
TEST_CASE("Two Exchanges for various domains 2d corner cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { single_mesh_file, refined_mesh_file, cross_mesh_file }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (int num_ghost : { 1, 2 }) {
            INFO("MESH: " << mesh_file);
            DomainReader<2> domain_reader(mesh_file, { nx, ny }, num_ghost);
            Domain<2> d_fine = domain_reader.getFinerDomain();

            Vector<2> vec(d_fine, num_components);
            for (auto pinfo : d_fine.getPatchInfoVector()) {
              for (int c = 0; c < num_components; c++) {
                auto data = vec.getComponentView(c, pinfo.local_index);
                Loop::Nested<2>(data.getStart(), data.getEnd(), [&](const std::array<int, 2>& coord) { data[coord] = pinfo.id; });
              }
            }

            ExchangeMockMPIGhostFiller<2> mgf(d_fine, GhostFillingType::Corners);

            mgf.fillGhost(vec);
            mgf.fillGhost(vec);

            mgf.checkVector(vec);
          }
        }
      }
    }
  }
}
TEST_CASE("Two Exchange for various domains 3d face cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { td_uniform, td_refined, td_mid_refine }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (auto nz : { 2, 3 }) {
            for (int num_ghost : { 1, 2 }) {
              INFO("MESH: " << mesh_file);
              DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
              Domain<3> d_fine = domain_reader.getFinerDomain();

              Vector<3> vec(d_fine, num_components);
              for (auto pinfo : d_fine.getPatchInfoVector()) {
                for (int c = 0; c < num_components; c++) {
                  auto data = vec.getComponentView(c, pinfo.local_index);
                  Loop::Nested<3>(data.getStart(), data.getEnd(), [&](const std::array<int, 3>& coord) { data[coord] = pinfo.id; });
                }
              }

              ExchangeMockMPIGhostFiller<3> mgf(d_fine, GhostFillingType::Faces);

              mgf.fillGhost(vec);
              mgf.fillGhost(vec);

              mgf.checkVector(vec);
            }
          }
        }
      }
    }
  }
}
TEST_CASE("Two Exchange for various domains 3d edge cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { td_uniform, td_refined, td_mid_refine }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (auto nz : { 2, 3 }) {
            for (int num_ghost : { 1, 2 }) {
              INFO("MESH: " << mesh_file);
              DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
              Domain<3> d_fine = domain_reader.getFinerDomain();

              Vector<3> vec(d_fine, num_components);
              for (auto pinfo : d_fine.getPatchInfoVector()) {
                for (int c = 0; c < num_components; c++) {
                  auto data = vec.getComponentView(c, pinfo.local_index);
                  Loop::Nested<3>(data.getStart(), data.getEnd(), [&](const std::array<int, 3>& coord) { data[coord] = pinfo.id; });
                }
              }

              ExchangeMockMPIGhostFiller<3> mgf(d_fine, GhostFillingType::Edges);

              mgf.fillGhost(vec);
              mgf.fillGhost(vec);

              mgf.checkVector(vec);
            }
          }
        }
      }
    }
  }
}
TEST_CASE("Two Exchange for various domains 3d corner cases")
{
  for (auto num_components : { 1, 2 }) {
    for (auto mesh_file : { td_uniform, td_refined, td_mid_refine }) {
      for (auto nx : { 2, 3 }) {
        for (auto ny : { 2, 3 }) {
          for (auto nz : { 2, 3 }) {
            for (int num_ghost : { 1, 2 }) {
              INFO("MESH: " << mesh_file);
              DomainReader<3> domain_reader(mesh_file, { nx, ny, nz }, num_ghost);
              Domain<3> d_fine = domain_reader.getFinerDomain();

              Vector<3> vec(d_fine, num_components);
              for (auto pinfo : d_fine.getPatchInfoVector()) {
                for (int c = 0; c < num_components; c++) {
                  auto data = vec.getComponentView(c, pinfo.local_index);
                  Loop::Nested<3>(data.getStart(), data.getEnd(), [&](const std::array<int, 3>& coord) { data[coord] = pinfo.id; });
                }
              }

              ExchangeMockMPIGhostFiller<3> mgf(d_fine, GhostFillingType::Corners);

              mgf.fillGhost(vec);
              mgf.fillGhost(vec);

              mgf.checkVector(vec);
            }
          }
        }
      }
    }
  }
}
