#include <ThunderEgg/Vector.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
using namespace std;
using namespace ThunderEgg;
TEST_CASE("LocalData constructor", "[LocalData]")
{
	auto nx        = GENERATE(1, 2, 10, 13);
	auto ny        = GENERATE(1, 2, 10, 13);
	auto num_ghost = GENERATE(0, 1, 2, 3, 4, 5);

	array<int, 2> lengths = {nx, ny};
	array<int, 2> strides = {1, nx + 2 * num_ghost};
	int           size    = 1;
	for (size_t i = 0; i < 2; i++) {
		size *= (lengths[i] + 2 * num_ghost);
	}
	vector<double> vec(size);
	iota(vec.begin(), vec.end(), 0);

	LocalData<2> ld(vec.data() + num_ghost * strides[0] + num_ghost * strides[1], strides, lengths,
	                num_ghost);

	CHECK(ld.getNumGhostCells() == num_ghost);
	CHECK(ld.getPtr() == vec.data() + num_ghost * strides[0] + num_ghost * strides[1]);
	for (int i = 0; i < 2; i++) {
		CHECK(ld.getLengths()[i] == lengths[i]);
		CHECK(ld.getStrides()[i] == strides[i]);
		CHECK(ld.getStart()[i] == 0);
		CHECK(ld.getEnd()[i] == lengths[i] - 1);
		CHECK(ld.getGhostStart()[i] == -num_ghost);
		CHECK(ld.getGhostEnd()[i] == lengths[i] - 1 + num_ghost);
	}
	CHECK(ld.getPtr(ld.getGhostStart()) == vec.data());
}
TEST_CASE("LocalData getEdgeSlice", "[LocalData]")
{
	auto nx        = GENERATE(1, 2, 10, 13);
	auto ny        = GENERATE(1, 2, 10, 13);
	auto nz        = GENERATE(1, 2, 10, 13);
	auto num_ghost = GENERATE(0, 1, 2, 3, 4, 5);

	array<int, 3> lengths = {nx, ny, nz};
	array<int, 3> strides = {1, nx + 2 * num_ghost, (nx + 2 * num_ghost) * (ny + 2 * num_ghost)};
	int           size    = 1;
	for (size_t i = 0; i < 3; i++) {
		size *= (lengths[i] + 2 * num_ghost);
	}
	double data[size];

	LocalData<3> ld(data + num_ghost * strides[0] + num_ghost * strides[1], strides, lengths, num_ghost);

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
			INFO("yi: " << yi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::bs(), {yi, zi});
			for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
				INFO("xi: " << xi);
				CHECK(&ld[{xi, yi, zi}] == &slice[{xi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
			INFO("yi: " << yi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::tn(), {yi, zi});
			for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
				INFO("xi: " << xi);
				CHECK(&ld[{xi, ny - 1 - yi, nz - 1 - zi}] == &slice[{xi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
			INFO("yi: " << yi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::bn(), {yi, zi});
			for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
				INFO("xi: " << xi);
				CHECK(&ld[{xi, ny - 1 - yi, zi}] == &slice[{xi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
			INFO("yi: " << yi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::ts(), {yi, zi});
			for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
				INFO("xi: " << xi);
				CHECK(&ld[{xi, yi, nz - 1 - zi}] == &slice[{xi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::bw(), {xi, zi});
			for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
				INFO("yi: " << yi);
				CHECK(&ld[{xi, yi, zi}] == &slice[{yi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::te(), {xi, zi});
			for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
				INFO("yi: " << yi);
				CHECK(&ld[{nx - 1 - xi, yi, nz - 1 - zi}] == &slice[{yi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::be(), {xi, zi});
			for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
				INFO("yi: " << yi);
				CHECK(&ld[{nx - 1 - xi, yi, zi}] == &slice[{yi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::tw(), {xi, zi});
			for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
				INFO("yi: " << yi);
				CHECK(&ld[{xi, yi, nz - 1 - zi}] == &slice[{yi}]);
			}
		}
	}

	for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
		INFO("yi: " << yi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::sw(), {xi, yi});
			for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
				INFO("zi: " << zi);
				CHECK(&ld[{xi, yi, zi}] == &slice[{zi}]);
			}
		}
	}

	for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
		INFO("yi: " << yi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::ne(), {xi, yi});
			for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
				INFO("zi: " << zi);
				CHECK(&ld[{nx - 1 - xi, ny - 1 - yi, zi}] == &slice[{zi}]);
			}
		}
	}

	for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
		INFO("yi: " << yi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::se(), {xi, yi});
			for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
				INFO("zi: " << zi);
				CHECK(&ld[{nx - 1 - xi, yi, zi}] == &slice[{zi}]);
			}
		}
	}

	for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
		INFO("yi: " << yi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::nw(), {xi, yi});
			for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
				INFO("zi: " << zi);
				CHECK(&ld[{xi, ny - 1 - yi, zi}] == &slice[{zi}]);
			}
		}
	}
}
TEST_CASE("LocalData getEdgeSlice const", "[LocalData]")
{
	auto nx        = GENERATE(1, 2, 10, 13);
	auto ny        = GENERATE(1, 2, 10, 13);
	auto nz        = GENERATE(1, 2, 10, 13);
	auto num_ghost = GENERATE(0, 1, 2, 3, 4, 5);

	array<int, 3> lengths = {nx, ny, nz};
	array<int, 3> strides = {1, nx + 2 * num_ghost, (nx + 2 * num_ghost) * (ny + 2 * num_ghost)};
	int           size    = 1;
	for (size_t i = 0; i < 3; i++) {
		size *= (lengths[i] + 2 * num_ghost);
	}
	double data[size];

	const LocalData<3> ld(data + num_ghost * strides[0] + num_ghost * strides[1], strides, lengths, num_ghost);

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
			INFO("yi: " << yi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::bs(), {yi, zi});
			for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
				INFO("xi: " << xi);
				CHECK(&ld[{xi, yi, zi}] == &slice[{xi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
			INFO("yi: " << yi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::tn(), {yi, zi});
			for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
				INFO("xi: " << xi);
				CHECK(&ld[{xi, ny - 1 - yi, nz - 1 - zi}] == &slice[{xi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
			INFO("yi: " << yi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::bn(), {yi, zi});
			for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
				INFO("xi: " << xi);
				CHECK(&ld[{xi, ny - 1 - yi, zi}] == &slice[{xi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
			INFO("yi: " << yi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::ts(), {yi, zi});
			for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
				INFO("xi: " << xi);
				CHECK(&ld[{xi, yi, nz - 1 - zi}] == &slice[{xi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::bw(), {xi, zi});
			for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
				INFO("yi: " << yi);
				CHECK(&ld[{xi, yi, zi}] == &slice[{yi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::te(), {xi, zi});
			for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
				INFO("yi: " << yi);
				CHECK(&ld[{nx - 1 - xi, yi, nz - 1 - zi}] == &slice[{yi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::be(), {xi, zi});
			for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
				INFO("yi: " << yi);
				CHECK(&ld[{nx - 1 - xi, yi, zi}] == &slice[{yi}]);
			}
		}
	}

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::tw(), {xi, zi});
			for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
				INFO("yi: " << yi);
				CHECK(&ld[{xi, yi, nz - 1 - zi}] == &slice[{yi}]);
			}
		}
	}

	for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
		INFO("yi: " << yi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::sw(), {xi, yi});
			for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
				INFO("zi: " << zi);
				CHECK(&ld[{xi, yi, zi}] == &slice[{zi}]);
			}
		}
	}

	for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
		INFO("yi: " << yi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::ne(), {xi, yi});
			for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
				INFO("zi: " << zi);
				CHECK(&ld[{nx - 1 - xi, ny - 1 - yi, zi}] == &slice[{zi}]);
			}
		}
	}

	for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
		INFO("yi: " << yi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::se(), {xi, yi});
			for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
				INFO("zi: " << zi);
				CHECK(&ld[{nx - 1 - xi, yi, zi}] == &slice[{zi}]);
			}
		}
	}

	for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
		INFO("yi: " << yi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			const LocalData<1> slice = ld.getSliceOnEdge(Edge<3>::nw(), {xi, yi});
			for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
				INFO("zi: " << zi);
				CHECK(&ld[{xi, ny - 1 - yi, zi}] == &slice[{zi}]);
			}
		}
	}
}
TEST_CASE("LocalData<3> getValueOnCorner", "[LocalData]")
{
	auto nx        = GENERATE(1, 2, 10, 13);
	auto ny        = GENERATE(1, 2, 10, 13);
	auto nz        = GENERATE(1, 2, 10, 13);
	auto num_ghost = GENERATE(0, 1, 2, 3, 4, 5);

	array<int, 3> lengths = {nx, ny, nz};
	array<int, 3> strides = {1, nx + 2 * num_ghost, (nx + 2 * num_ghost) * (ny + 2 * num_ghost)};
	int           size    = 1;
	for (size_t i = 0; i < 3; i++) {
		size *= (lengths[i] + 2 * num_ghost);
	}
	double data[size];

	LocalData<3> ld(data + num_ghost * strides[0] + num_ghost * strides[1], strides, lengths, num_ghost);

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
			INFO("yi: " << yi);
			for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
				INFO("xi: " << xi);
				CHECK(&ld[{xi, yi, zi}] == &ld.getValueOnCorner(Corner<3>::bsw(), {xi, yi, zi}));
				CHECK(&ld[{nx - 1 - xi, yi, zi}] == &ld.getValueOnCorner(Corner<3>::bse(), {xi, yi, zi}));
				CHECK(&ld[{xi, ny - 1 - yi, zi}] == &ld.getValueOnCorner(Corner<3>::bnw(), {xi, yi, zi}));
				CHECK(&ld[{nx - 1 - xi, ny - 1 - yi, zi}] == &ld.getValueOnCorner(Corner<3>::bne(), {xi, yi, zi}));
				CHECK(&ld[{xi, yi, nz - 1 - zi}] == &ld.getValueOnCorner(Corner<3>::tsw(), {xi, yi, zi}));
				CHECK(&ld[{nx - 1 - xi, yi, nz - 1 - zi}] == &ld.getValueOnCorner(Corner<3>::tse(), {xi, yi, zi}));
				CHECK(&ld[{xi, ny - 1 - yi, nz - 1 - zi}] == &ld.getValueOnCorner(Corner<3>::tnw(), {xi, yi, zi}));
				CHECK(&ld[{nx - 1 - xi, ny - 1 - yi, nz - 1 - zi}] == &ld.getValueOnCorner(Corner<3>::tne(), {xi, yi, zi}));
			}
		}
	}
}
TEST_CASE("LocalData<3> getValueOnCorner const", "[LocalData]")
{
	auto nx        = GENERATE(1, 2, 10, 13);
	auto ny        = GENERATE(1, 2, 10, 13);
	auto nz        = GENERATE(1, 2, 10, 13);
	auto num_ghost = GENERATE(0, 1, 2, 3, 4, 5);

	array<int, 3> lengths = {nx, ny, nz};
	array<int, 3> strides = {1, nx + 2 * num_ghost, (nx + 2 * num_ghost) * (ny + 2 * num_ghost)};
	int           size    = 1;
	for (size_t i = 0; i < 3; i++) {
		size *= (lengths[i] + 2 * num_ghost);
	}
	double data[size];

	const LocalData<3> ld(data + num_ghost * strides[0] + num_ghost * strides[1], strides, lengths, num_ghost);

	for (int zi = -num_ghost; zi < nz + num_ghost; zi++) {
		INFO("zi: " << zi);
		for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
			INFO("yi: " << yi);
			for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
				INFO("xi: " << xi);
				CHECK(&ld[{xi, yi, zi}] == &ld.getValueOnCorner(Corner<3>::bsw(), {xi, yi, zi}));
				CHECK(&ld[{nx - 1 - xi, yi, zi}] == &ld.getValueOnCorner(Corner<3>::bse(), {xi, yi, zi}));
				CHECK(&ld[{xi, ny - 1 - yi, zi}] == &ld.getValueOnCorner(Corner<3>::bnw(), {xi, yi, zi}));
				CHECK(&ld[{nx - 1 - xi, ny - 1 - yi, zi}] == &ld.getValueOnCorner(Corner<3>::bne(), {xi, yi, zi}));
				CHECK(&ld[{xi, yi, nz - 1 - zi}] == &ld.getValueOnCorner(Corner<3>::tsw(), {xi, yi, zi}));
				CHECK(&ld[{nx - 1 - xi, yi, nz - 1 - zi}] == &ld.getValueOnCorner(Corner<3>::tse(), {xi, yi, zi}));
				CHECK(&ld[{xi, ny - 1 - yi, nz - 1 - zi}] == &ld.getValueOnCorner(Corner<3>::tnw(), {xi, yi, zi}));
				CHECK(&ld[{nx - 1 - xi, ny - 1 - yi, nz - 1 - zi}] == &ld.getValueOnCorner(Corner<3>::tne(), {xi, yi, zi}));
			}
		}
	}
}
TEST_CASE("LocalData<2> getValueOnCorner", "[LocalData]")
{
	auto nx        = GENERATE(1, 2, 10, 13);
	auto ny        = GENERATE(1, 2, 10, 13);
	auto num_ghost = GENERATE(0, 1, 2, 3, 4, 5);

	array<int, 2> lengths = {nx, ny};
	array<int, 2> strides = {1, nx + 2 * num_ghost};
	int           size    = 1;
	for (size_t i = 0; i < 2; i++) {
		size *= (lengths[i] + 2 * num_ghost);
	}
	double data[size];

	LocalData<2> ld(data + num_ghost * strides[0] + num_ghost * strides[1], strides, lengths, num_ghost);

	for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
		INFO("yi: " << yi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			CHECK(&ld[{xi, yi}] == &ld.getValueOnCorner(Corner<2>::sw(), {xi, yi}));
			CHECK(&ld[{nx - 1 - xi, yi}] == &ld.getValueOnCorner(Corner<2>::se(), {xi, yi}));
			CHECK(&ld[{xi, ny - 1 - yi}] == &ld.getValueOnCorner(Corner<2>::nw(), {xi, yi}));
			CHECK(&ld[{nx - 1 - xi, ny - 1 - yi}] == &ld.getValueOnCorner(Corner<2>::ne(), {xi, yi}));
		}
	}
}
TEST_CASE("LocalData<2> getValueOnCorner const", "[LocalData]")
{
	auto nx        = GENERATE(1, 2, 10, 13);
	auto ny        = GENERATE(1, 2, 10, 13);
	auto num_ghost = GENERATE(0, 1, 2, 3, 4, 5);

	array<int, 2> lengths = {nx, ny};
	array<int, 2> strides = {1, nx + 2 * num_ghost};
	int           size    = 1;
	for (size_t i = 0; i < 2; i++) {
		size *= (lengths[i] + 2 * num_ghost);
	}
	double data[size];

	const LocalData<2> ld(data + num_ghost * strides[0] + num_ghost * strides[1], strides, lengths, num_ghost);

	for (int yi = -num_ghost; yi < ny + num_ghost; yi++) {
		INFO("yi: " << yi);
		for (int xi = -num_ghost; xi < nx + num_ghost; xi++) {
			INFO("xi: " << xi);
			CHECK(&ld[{xi, yi}] == &ld.getValueOnCorner(Corner<2>::sw(), {xi, yi}));
			CHECK(&ld[{nx - 1 - xi, yi}] == &ld.getValueOnCorner(Corner<2>::se(), {xi, yi}));
			CHECK(&ld[{xi, ny - 1 - yi}] == &ld.getValueOnCorner(Corner<2>::nw(), {xi, yi}));
			CHECK(&ld[{nx - 1 - xi, ny - 1 - yi}] == &ld.getValueOnCorner(Corner<2>::ne(), {xi, yi}));
		}
	}
}