#include <ThunderEgg/PatchInfo.h>

#include <catch2/catch_test_macros.hpp>

using namespace std;
using namespace ThunderEgg;

TEST_CASE("PatchInfo Serialization/Deserialization", "[PatchInfo]")
{
	PatchInfo<3> *d_ptr = new PatchInfo<3>;
	PatchInfo<3> &d     = *d_ptr;
	d.id                = 0;
	d.nbr_info[Side<3>::north().getIndex()].reset(new NormalNbrInfo<3>(1));
	d.nbr_info[Side<3>::east().getIndex()].reset(new CoarseNbrInfo<3>(2, Orthant<2>::nw()));
	d.nbr_info[Side<3>::south().getIndex()].reset(new FineNbrInfo<3>({3, 4, 5, 6}));
	d.corner_nbr_info[Corner<3>::bsw().getIndex()].reset(new NormalNbrInfo<1>(1));
	d.corner_nbr_info[Corner<3>::tse().getIndex()].reset(new CoarseNbrInfo<1>(2, Orthant<0>::null()));
	d.corner_nbr_info[Corner<3>::bnw().getIndex()].reset(new FineNbrInfo<1>({1}));
	d.edge_nbr_info[Edge<3>::sw().getIndex()].reset(new NormalNbrInfo<2>(1));
	d.edge_nbr_info[Edge<3>::bn().getIndex()].reset(new CoarseNbrInfo<2>(2, Orthant<1>::lower()));
	d.edge_nbr_info[Edge<3>::tw().getIndex()].reset(new FineNbrInfo<2>({1, 2}));

	// serialize and then deserialize
	char *buff = new char[d.serialize(nullptr)];
	d.serialize(buff);
	delete d_ptr;
	PatchInfo<3> out;
	out.deserialize(buff);
	delete[] buff;

	// check that deserialized version has the same information
	REQUIRE(out.id == 0);

	REQUIRE(!out.hasNbr(Side<3>::west()));

	REQUIRE(out.hasNbr(Side<3>::east()));
	REQUIRE(out.getNbrType(Side<3>::east()) == NbrType::Coarse);
	REQUIRE(out.getCoarseNbrInfo(Side<3>::east()).id == 2);
	REQUIRE(out.getCoarseNbrInfo(Side<3>::east()).orth_on_coarse == Orthant<2>::nw());

	REQUIRE(out.hasNbr(Side<3>::south()));
	REQUIRE(out.getNbrType(Side<3>::south()) == NbrType::Fine);
	REQUIRE(out.getFineNbrInfo(Side<3>::south()).ids[0] == 3);
	REQUIRE(out.getFineNbrInfo(Side<3>::south()).ids[1] == 4);
	REQUIRE(out.getFineNbrInfo(Side<3>::south()).ids[2] == 5);
	REQUIRE(out.getFineNbrInfo(Side<3>::south()).ids[3] == 6);

	REQUIRE(out.hasNbr(Side<3>::north()));
	REQUIRE(out.getNbrType(Side<3>::north()) == NbrType::Normal);
	REQUIRE(out.getNormalNbrInfo(Side<3>::north()).id == 1);

	REQUIRE(!out.hasNbr(Side<3>::bottom()));
	REQUIRE(!out.hasNbr(Side<3>::top()));

	// Corners

	REQUIRE(out.hasCornerNbr(Corner<3>::bsw()));
	REQUIRE(out.getCornerNbrType(Corner<3>::bsw()) == NbrType::Normal);
	REQUIRE(out.getCornerNormalNbrInfo(Corner<3>::bsw()).id == 1);

	REQUIRE(!out.hasCornerNbr(Corner<3>::bse()));

	REQUIRE(out.hasCornerNbr(Corner<3>::bnw()));
	REQUIRE(out.getCornerNbrType(Corner<3>::bnw()) == NbrType::Fine);
	REQUIRE(out.getCornerFineNbrInfo(Corner<3>::bnw()).ids[0] == 1);

	REQUIRE(!out.hasCornerNbr(Corner<3>::bne()));
	REQUIRE(!out.hasCornerNbr(Corner<3>::tsw()));

	REQUIRE(out.hasCornerNbr(Corner<3>::tse()));
	REQUIRE(out.getCornerNbrType(Corner<3>::tse()) == NbrType::Coarse);
	REQUIRE(out.getCornerCoarseNbrInfo(Corner<3>::tse()).id == 2);
	REQUIRE(out.getCornerCoarseNbrInfo(Corner<3>::tse()).orth_on_coarse == Orthant<0>::null());

	REQUIRE(!out.hasCornerNbr(Corner<3>::tnw()));
	REQUIRE(!out.hasCornerNbr(Corner<3>::tne()));

	// Edges

	REQUIRE(!out.hasEdgeNbr(Edge<3>::bs()));
	REQUIRE(!out.hasEdgeNbr(Edge<3>::tn()));

	REQUIRE(out.hasEdgeNbr(Edge<3>::bn()));
	REQUIRE(out.getEdgeNbrType(Edge<3>::bn()) == NbrType::Coarse);
	REQUIRE(out.getEdgeCoarseNbrInfo(Edge<3>::bn()).id == 2);
	REQUIRE(out.getEdgeCoarseNbrInfo(Edge<3>::bn()).orth_on_coarse == Orthant<1>::lower());

	REQUIRE(!out.hasEdgeNbr(Edge<3>::ts()));
	REQUIRE(!out.hasEdgeNbr(Edge<3>::bw()));
	REQUIRE(!out.hasEdgeNbr(Edge<3>::te()));
	REQUIRE(!out.hasEdgeNbr(Edge<3>::be()));

	REQUIRE(out.hasEdgeNbr(Edge<3>::tw()));
	REQUIRE(out.getEdgeNbrType(Edge<3>::tw()) == NbrType::Fine);
	REQUIRE(out.getEdgeFineNbrInfo(Edge<3>::tw()).ids[0] == 1);
	REQUIRE(out.getEdgeFineNbrInfo(Edge<3>::tw()).ids[1] == 2);

	REQUIRE(out.hasEdgeNbr(Edge<3>::sw()));
	REQUIRE(out.getEdgeNbrType(Edge<3>::sw()) == NbrType::Normal);
	REQUIRE(out.getEdgeNormalNbrInfo(Edge<3>::sw()).id == 1);

	REQUIRE(!out.hasEdgeNbr(Edge<3>::ne()));
	REQUIRE(!out.hasEdgeNbr(Edge<3>::se()));
	REQUIRE(!out.hasEdgeNbr(Edge<3>::nw()));
}
TEST_CASE("PatchInfo Default Values", "[PatchInfo]")
{
	PatchInfo<3> pinfo;
	CHECK(pinfo.id == 0);
	CHECK(pinfo.local_index == 0);
	CHECK(pinfo.global_index == 0);
	CHECK(pinfo.refine_level == -1);
	CHECK(pinfo.parent_id == -1);
	CHECK(pinfo.parent_rank == -1);
	for (int child_id : pinfo.child_ids) {
		CHECK(child_id == -1);
	}
	for (int child_rank : pinfo.child_ids) {
		CHECK(child_rank == -1);
	}
	CHECK(pinfo.num_ghost_cells == 0);
	CHECK(pinfo.rank == -1);
	CHECK(pinfo.orth_on_parent == Orthant<3>::null());
	for (int n : pinfo.ns) {
		CHECK(n == 1);
	}
	for (double start : pinfo.starts) {
		CHECK(start == 0);
	}
	for (double spacing : pinfo.spacings) {
		CHECK(spacing == 1);
	}
	for (auto &nbr_info : pinfo.nbr_info) {
		CHECK(nbr_info == nullptr);
	}
	for (auto &nbr_info : pinfo.corner_nbr_info) {
		CHECK(nbr_info == nullptr);
	}
	for (auto &nbr_info : pinfo.edge_nbr_info) {
		CHECK(nbr_info == nullptr);
	}
}
TEST_CASE("PatchInfo to_json no children", "[PatchInfo]")
{
	PatchInfo<3> d;
	d.id             = 9;
	d.rank           = 0;
	d.parent_id      = 2;
	d.parent_rank    = 3;
	d.orth_on_parent = Orthant<3>::tnw();
	d.starts         = {1, 2, 3};
	d.spacings       = {0.1, 0.2, 0.3};
	d.ns             = {10, 20, 30};
	d.nbr_info[Side<3>::north().getIndex()].reset(new NormalNbrInfo<3>(1));
	d.nbr_info[Side<3>::east().getIndex()].reset(new CoarseNbrInfo<3>(2, Orthant<2>::nw()));
	d.nbr_info[Side<3>::south().getIndex()].reset(new FineNbrInfo<3>({3, 4, 5, 6}));
	d.corner_nbr_info[Corner<3>::bsw().getIndex()].reset(new NormalNbrInfo<1>(1));
	d.corner_nbr_info[Corner<3>::tse().getIndex()].reset(new CoarseNbrInfo<1>(2, Orthant<0>(0)));
	d.corner_nbr_info[Corner<3>::bnw().getIndex()].reset(new FineNbrInfo<1>({1}));
	d.edge_nbr_info[Edge<3>::sw().getIndex()].reset(new NormalNbrInfo<2>(1));
	d.edge_nbr_info[Edge<3>::bn().getIndex()].reset(new CoarseNbrInfo<2>(2, Orthant<1>::lower()));
	d.edge_nbr_info[Edge<3>::tw().getIndex()].reset(new FineNbrInfo<2>({1, 2}));

	nlohmann::json j = d;

	CHECK(j["id"] == d.id);
	CHECK(j["parent_id"] == d.parent_id);
	CHECK(j["parent_rank"] == d.parent_rank);
	CHECK(j["orth_on_parent"] == "TNW");
	CHECK(j["rank"] == d.rank);
	CHECK(j["child_ids"] == nullptr);
	CHECK(j["child_ranks"] == nullptr);

	REQUIRE(j["starts"].is_array());
	REQUIRE(j["starts"].size() == 3);
	CHECK(j["starts"][0] == d.starts[0]);
	CHECK(j["starts"][1] == d.starts[1]);
	CHECK(j["starts"][2] == d.starts[2]);

	REQUIRE(j["lengths"].is_array());
	REQUIRE(j["lengths"].size() == 3);
	CHECK(j["lengths"][0] == d.spacings[0] * d.ns[0]);
	CHECK(j["lengths"][1] == d.spacings[1] * d.ns[1]);
	CHECK(j["lengths"][2] == d.spacings[2] * d.ns[2]);

	REQUIRE(j["nbrs"].is_array());
	REQUIRE(j["nbrs"].size() == 3);

	CHECK(j["nbrs"][0]["type"] == "COARSE");
	CHECK(j["nbrs"][0]["side"] == "EAST");

	CHECK(j["nbrs"][1]["type"] == "FINE");
	CHECK(j["nbrs"][1]["side"] == "SOUTH");

	CHECK(j["nbrs"][2]["type"] == "NORMAL");
	CHECK(j["nbrs"][2]["side"] == "NORTH");

	REQUIRE(j["corner_nbrs"].is_array());
	REQUIRE(j["corner_nbrs"].size() == 3);

	CHECK(j["corner_nbrs"][0]["type"] == "NORMAL");
	CHECK(j["corner_nbrs"][0]["corner"] == "BSW");

	CHECK(j["corner_nbrs"][1]["type"] == "FINE");
	CHECK(j["corner_nbrs"][1]["corner"] == "BNW");

	CHECK(j["corner_nbrs"][2]["type"] == "COARSE");
	CHECK(j["corner_nbrs"][2]["corner"] == "TSE");

	REQUIRE(j["edge_nbrs"].is_array());
	REQUIRE(j["edge_nbrs"].size() == 3);

	CHECK(j["edge_nbrs"][0]["type"] == "COARSE");
	CHECK(j["edge_nbrs"][0]["edge"] == "BN");

	CHECK(j["edge_nbrs"][1]["type"] == "FINE");
	CHECK(j["edge_nbrs"][1]["edge"] == "TW");

	CHECK(j["edge_nbrs"][2]["type"] == "NORMAL");
	CHECK(j["edge_nbrs"][2]["edge"] == "SW");
}
TEST_CASE("PatchInfo to_json no children no neighbors", "[PatchInfo]")
{
	PatchInfo<3> d;
	d.id           = 9;
	d.rank         = 0;
	d.parent_id    = 2;
	d.parent_rank  = 3;
	d.refine_level = 329;
	d.starts       = {1, 2, 3};
	d.spacings     = {0.1, 0.2, 0.3};
	d.ns           = {10, 20, 30};

	nlohmann::json j = d;

	CHECK(j["id"] == d.id);
	CHECK(j["parent_id"] == d.parent_id);
	CHECK(j["parent_rank"] == d.parent_rank);
	CHECK(j["rank"] == d.rank);
	CHECK(j["refine_level"] == 329);
	CHECK(j["child_ids"] == nullptr);
	CHECK(j["child_ranks"] == nullptr);
	CHECK(j["orth_on_parent"] == nullptr);

	REQUIRE(j["starts"].is_array());
	REQUIRE(j["starts"].size() == 3);
	CHECK(j["starts"][0] == d.starts[0]);
	CHECK(j["starts"][1] == d.starts[1]);
	CHECK(j["starts"][2] == d.starts[2]);

	REQUIRE(j["lengths"].is_array());
	REQUIRE(j["lengths"].size() == 3);
	CHECK(j["lengths"][0] == d.spacings[0] * d.ns[0]);
	CHECK(j["lengths"][1] == d.spacings[1] * d.ns[1]);
	CHECK(j["lengths"][2] == d.spacings[2] * d.ns[2]);

	REQUIRE(j["nbrs"].is_array());
	REQUIRE(j["nbrs"].size() == 0);
}
TEST_CASE("PatchInfo to_json with children", "[PatchInfo]")
{
	PatchInfo<3> d;
	d.id           = 9;
	d.rank         = 0;
	d.parent_id    = 2;
	d.parent_rank  = 3;
	d.refine_level = 329;
	d.starts       = {1, 2, 3};
	d.spacings     = {0.1, 0.2, 0.3};
	d.ns           = {10, 20, 30};
	d.child_ids    = {3, 4, 5, 6, 7, 8, 9, 10};
	d.child_ranks  = {1, 2, 3, 4, 5, 6, 7, 8};
	d.nbr_info[Side<3>::north().getIndex()].reset(new NormalNbrInfo<3>(1));
	d.nbr_info[Side<3>::east().getIndex()].reset(new CoarseNbrInfo<3>(2, Orthant<2>::nw()));
	d.nbr_info[Side<3>::south().getIndex()].reset(new FineNbrInfo<3>({3, 4, 5, 6}));

	nlohmann::json j = d;

	CHECK(j["id"] == d.id);
	CHECK(j["parent_id"] == d.parent_id);
	CHECK(j["parent_rank"] == d.parent_rank);
	CHECK(j["rank"] == d.rank);
	CHECK(j["refine_level"] == 329);

	REQUIRE(j["child_ids"].is_array());
	REQUIRE(j["child_ids"].size() == 8);
	CHECK(j["child_ids"][0] == d.child_ids[0]);
	CHECK(j["child_ids"][1] == d.child_ids[1]);
	CHECK(j["child_ids"][2] == d.child_ids[2]);
	CHECK(j["child_ids"][3] == d.child_ids[3]);
	CHECK(j["child_ids"][4] == d.child_ids[4]);
	CHECK(j["child_ids"][5] == d.child_ids[5]);
	CHECK(j["child_ids"][6] == d.child_ids[6]);
	CHECK(j["child_ids"][7] == d.child_ids[7]);

	REQUIRE(j["child_ranks"].is_array());
	REQUIRE(j["child_ranks"].size() == 8);
	CHECK(j["child_ranks"][0] == d.child_ranks[0]);
	CHECK(j["child_ranks"][1] == d.child_ranks[1]);
	CHECK(j["child_ranks"][2] == d.child_ranks[2]);
	CHECK(j["child_ranks"][3] == d.child_ranks[3]);
	CHECK(j["child_ranks"][4] == d.child_ranks[4]);
	CHECK(j["child_ranks"][5] == d.child_ranks[5]);
	CHECK(j["child_ranks"][6] == d.child_ranks[6]);
	CHECK(j["child_ranks"][7] == d.child_ranks[7]);

	REQUIRE(j["starts"].is_array());
	REQUIRE(j["starts"].size() == 3);
	CHECK(j["starts"][0] == d.starts[0]);
	CHECK(j["starts"][1] == d.starts[1]);
	CHECK(j["starts"][2] == d.starts[2]);

	REQUIRE(j["lengths"].is_array());
	REQUIRE(j["lengths"].size() == 3);
	CHECK(j["lengths"][0] == d.spacings[0] * d.ns[0]);
	CHECK(j["lengths"][1] == d.spacings[1] * d.ns[1]);
	CHECK(j["lengths"][2] == d.spacings[2] * d.ns[2]);

	REQUIRE(j["nbrs"].is_array());
	REQUIRE(j["nbrs"].size() == 3);

	CHECK(j["nbrs"][0]["type"] == "COARSE");
	CHECK(j["nbrs"][0]["side"] == "EAST");

	CHECK(j["nbrs"][1]["type"] == "FINE");
	CHECK(j["nbrs"][1]["side"] == "SOUTH");

	CHECK(j["nbrs"][2]["type"] == "NORMAL");
	CHECK(j["nbrs"][2]["side"] == "NORTH");
}
TEST_CASE("PatchInfo from_json no children", "[PatchInfo]")
{
	nlohmann::json j;
	j["id"]                       = 9;
	j["rank"]                     = 3;
	j["refine_level"]             = 329;
	j["parent_id"]                = 2;
	j["parent_rank"]              = 3;
	j["starts"]                   = {1, 2, 3};
	j["lengths"]                  = {10, 20, 30};
	j["nbrs"]                     = {NormalNbrInfo<3>(1), CoarseNbrInfo<3>(2, Orthant<2>::nw()), FineNbrInfo<3>({3, 4, 5, 6})};
	j["nbrs"][0]["side"]          = "NORTH";
	j["nbrs"][1]["side"]          = "EAST";
	j["nbrs"][2]["side"]          = "SOUTH";
	j["corner_nbrs"]              = {NormalNbrInfo<1>(1), CoarseNbrInfo<1>(2, Orthant<0>(0)), FineNbrInfo<1>({1})};
	j["corner_nbrs"][0]["corner"] = "BSW";
	j["corner_nbrs"][1]["corner"] = "TSE";
	j["corner_nbrs"][2]["corner"] = "BNW";
	j["edge_nbrs"]                = {NormalNbrInfo<2>(1), CoarseNbrInfo<2>(2, Orthant<1>::lower()), FineNbrInfo<2>({1, 2})};
	j["edge_nbrs"][0]["edge"]     = "SW";
	j["edge_nbrs"][1]["edge"]     = "BN";
	j["edge_nbrs"][2]["edge"]     = "TW";

	PatchInfo<3>
	d = j.get<PatchInfo<3>>();
	CHECK(d.id == 9);
	CHECK(d.rank == 3);
	CHECK(d.refine_level == 329);
	CHECK(d.parent_id == 2);
	CHECK(d.parent_rank == 3);
	CHECK(d.orth_on_parent == Orthant<3>::null());
	CHECK(d.starts[0] == 1);
	CHECK(d.starts[1] == 2);
	CHECK(d.starts[2] == 3);
	CHECK(d.spacings[0] == 10);
	CHECK(d.spacings[1] == 20);
	CHECK(d.spacings[2] == 30);
	CHECK(d.ns[0] == 1);
	CHECK(d.ns[1] == 1);
	CHECK(d.ns[2] == 1);
	CHECK_FALSE(d.hasNbr(Side<3>::west()));
	CHECK(d.hasNbr(Side<3>::east()));
	CHECK(d.getNbrType(Side<3>::east()) == NbrType::Coarse);
	CHECK(d.hasNbr(Side<3>::south()));
	CHECK(d.getNbrType(Side<3>::south()) == NbrType::Fine);
	CHECK(d.hasNbr(Side<3>::north()));
	CHECK(d.getNbrType(Side<3>::north()) == NbrType::Normal);
	CHECK_FALSE(d.hasNbr(Side<3>::bottom()));
	CHECK_FALSE(d.hasNbr(Side<3>::top()));

	CHECK(d.hasCornerNbr(Corner<3>::bsw()));
	CHECK(d.getCornerNbrType(Corner<3>::bsw()) == NbrType::Normal);
	CHECK_FALSE(d.hasCornerNbr(Corner<3>::bse()));
	CHECK(d.hasCornerNbr(Corner<3>::bnw()));
	CHECK(d.getCornerNbrType(Corner<3>::bnw()) == NbrType::Fine);
	CHECK_FALSE(d.hasCornerNbr(Corner<3>::bne()));
	CHECK_FALSE(d.hasCornerNbr(Corner<3>::tsw()));
	CHECK(d.hasCornerNbr(Corner<3>::tse()));
	CHECK(d.getCornerNbrType(Corner<3>::tse()) == NbrType::Coarse);
	CHECK_FALSE(d.hasCornerNbr(Corner<3>::tnw()));
	CHECK_FALSE(d.hasCornerNbr(Corner<3>::tne()));

	CHECK_FALSE(d.hasEdgeNbr(Edge<3>::bs()));
	CHECK_FALSE(d.hasEdgeNbr(Edge<3>::tn()));
	CHECK(d.hasEdgeNbr(Edge<3>::bn()));
	CHECK(d.getEdgeNbrType(Edge<3>::bn()) == NbrType::Coarse);
	CHECK_FALSE(d.hasEdgeNbr(Edge<3>::ts()));
	CHECK_FALSE(d.hasEdgeNbr(Edge<3>::bw()));
	CHECK_FALSE(d.hasEdgeNbr(Edge<3>::te()));
	CHECK_FALSE(d.hasEdgeNbr(Edge<3>::be()));
	CHECK(d.hasEdgeNbr(Edge<3>::tw()));
	CHECK(d.getEdgeNbrType(Edge<3>::tw()) == NbrType::Fine);
	CHECK(d.hasEdgeNbr(Edge<3>::sw()));
	CHECK(d.getEdgeNbrType(Edge<3>::sw()) == NbrType::Normal);
	CHECK_FALSE(d.hasEdgeNbr(Edge<3>::ne()));
	CHECK_FALSE(d.hasEdgeNbr(Edge<3>::se()));
	CHECK_FALSE(d.hasEdgeNbr(Edge<3>::nw()));
}
TEST_CASE("PatchInfo from_json with children", "[PatchInfo]")
{
	nlohmann::json j;
	j["id"]             = 9;
	j["rank"]           = 3;
	j["refine_level"]   = 329;
	j["parent_id"]      = 2;
	j["parent_rank"]    = 3;
	j["orth_on_parent"] = "TNW";
	j["starts"]         = {1, 2, 3};
	j["lengths"]        = {10, 20, 30};
	j["child_ids"]      = {1, 2, 3, 4, 5, 6, 7, 8};
	j["child_ranks"]    = {0, 1, 2, 3, 4, 5, 6, 7};
	j["nbrs"]
	= {NormalNbrInfo<3>(1), CoarseNbrInfo<3>(2, Orthant<2>::nw()), FineNbrInfo<3>({3, 4, 5, 6})};
	j["nbrs"][0]["side"] = "NORTH";
	j["nbrs"][1]["side"] = "EAST";
	j["nbrs"][2]["side"] = "SOUTH";

	PatchInfo<3> d = j.get<PatchInfo<3>>();
	CHECK(d.id == 9);
	CHECK(d.rank == 3);
	CHECK(d.refine_level == 329);
	CHECK(d.parent_id == 2);
	CHECK(d.parent_rank == 3);
	CHECK(d.orth_on_parent == Orthant<3>::tnw());
	CHECK(d.starts[0] == 1);
	CHECK(d.starts[1] == 2);
	CHECK(d.starts[2] == 3);
	CHECK(d.spacings[0] == 10);
	CHECK(d.spacings[1] == 20);
	CHECK(d.spacings[2] == 30);
	CHECK(d.ns[0] == 1);
	CHECK(d.ns[1] == 1);
	CHECK(d.ns[2] == 1);
	CHECK(d.child_ids[0] == 1);
	CHECK(d.child_ids[1] == 2);
	CHECK(d.child_ids[2] == 3);
	CHECK(d.child_ids[3] == 4);
	CHECK(d.child_ids[4] == 5);
	CHECK(d.child_ids[5] == 6);
	CHECK(d.child_ids[6] == 7);
	CHECK(d.child_ids[7] == 8);
	CHECK(d.child_ranks[0] == 0);
	CHECK(d.child_ranks[1] == 1);
	CHECK(d.child_ranks[2] == 2);
	CHECK(d.child_ranks[3] == 3);
	CHECK(d.child_ranks[4] == 4);
	CHECK(d.child_ranks[5] == 5);
	CHECK(d.child_ranks[6] == 6);
	CHECK(d.child_ranks[7] == 7);
	CHECK_FALSE(d.hasNbr(Side<3>::west()));
	CHECK(d.hasNbr(Side<3>::east()));
	CHECK(d.getNbrType(Side<3>::east()) == NbrType::Coarse);
	CHECK(d.hasNbr(Side<3>::south()));
	CHECK(d.getNbrType(Side<3>::south()) == NbrType::Fine);
	CHECK(d.hasNbr(Side<3>::north()));
	CHECK(d.getNbrType(Side<3>::north()) == NbrType::Normal);
	CHECK_FALSE(d.hasNbr(Side<3>::bottom()));
	CHECK_FALSE(d.hasNbr(Side<3>::top()));
}
TEST_CASE("PatchInfo copy constructor", "[PatchInfo]")
{
	PatchInfo<3> d;
	d.id              = 9;
	d.local_index     = 10;
	d.global_index    = 10;
	d.rank            = 0;
	d.parent_id       = 2;
	d.parent_rank     = 3;
	d.num_ghost_cells = 239;
	d.refine_level    = 329;
	d.starts          = {1, 2, 3};
	d.spacings        = {0.1, 0.2, 0.3};
	d.ns              = {10, 20, 30};
	d.child_ids       = {3, 4, 5, 6, 7, 8, 9, 10};
	d.child_ranks     = {1, 2, 3, 4, 5, 6, 7, 8};
	d.nbr_info[Side<3>::north().getIndex()].reset(new NormalNbrInfo<3>(1));
	d.nbr_info[Side<3>::east().getIndex()].reset(new CoarseNbrInfo<3>(2, Orthant<2>::nw()));
	d.nbr_info[Side<3>::south().getIndex()].reset(new FineNbrInfo<3>({3, 4, 5, 6}));
	d.corner_nbr_info[Corner<3>::bsw().getIndex()].reset(new NormalNbrInfo<1>(1));
	d.corner_nbr_info[Corner<3>::tse().getIndex()].reset(new CoarseNbrInfo<1>(2, Orthant<0>(0)));
	d.corner_nbr_info[Corner<3>::bnw().getIndex()].reset(new FineNbrInfo<1>({1}));
	d.edge_nbr_info[Edge<3>::sw().getIndex()].reset(new NormalNbrInfo<2>(1));
	d.edge_nbr_info[Edge<3>::bn().getIndex()].reset(new CoarseNbrInfo<2>(2, Orthant<1>::lower()));
	d.edge_nbr_info[Edge<3>::tw().getIndex()].reset(new FineNbrInfo<2>({1, 2}));

	PatchInfo<3> d2(d);

	CHECK(d.id == d2.id);
	CHECK(d.local_index == d2.global_index);
	CHECK(d.rank == d2.rank);
	CHECK(d.parent_id == d2.parent_id);
	CHECK(d.parent_rank == d2.parent_rank);
	CHECK(d.num_ghost_cells == d2.num_ghost_cells);
	CHECK(d.refine_level == d2.refine_level);
	CHECK(d.starts == d2.starts);
	CHECK(d.spacings == d2.spacings);
	CHECK(d.ns == d2.ns);
	CHECK(d.child_ids == d2.child_ids);
	CHECK(d.child_ranks == d2.child_ranks);

	for (Side<3> s : Side<3>::getValues()) {
		REQUIRE(d.hasNbr(s) == d2.hasNbr(s));
		if (d.hasNbr(s)) {
			CHECK(d.nbr_info[s.getIndex()] != d2.nbr_info[s.getIndex()]);
			switch (d.getNbrType(s)) {
				case NbrType::Normal:
					CHECK(d.getNormalNbrInfo(s).id == d2.getNormalNbrInfo(s).id);
					break;
				case NbrType::Fine:
					CHECK(d.getFineNbrInfo(s).ids[0] == d2.getFineNbrInfo(s).ids[0]);
					break;
				case NbrType::Coarse:
					CHECK(d.getCoarseNbrInfo(s).id == d2.getCoarseNbrInfo(s).id);
					break;
			}
		}
	}
	for (Corner<3> c : Corner<3>::getValues()) {
		REQUIRE(d.hasCornerNbr(c) == d2.hasCornerNbr(c));
		if (d.hasCornerNbr(c)) {
			CHECK(d.corner_nbr_info[c.getIndex()] != d2.corner_nbr_info[c.getIndex()]);
			switch (d.getCornerNbrType(c)) {
				case NbrType::Normal:
					CHECK(d.getCornerNormalNbrInfo(c).id == d2.getCornerNormalNbrInfo(c).id);
					break;
				case NbrType::Fine:
					CHECK(d.getCornerFineNbrInfo(c).ids[0] == d2.getCornerFineNbrInfo(c).ids[0]);
					break;
				case NbrType::Coarse:
					CHECK(d.getCornerCoarseNbrInfo(c).id == d2.getCornerCoarseNbrInfo(c).id);
					break;
			}
		}
	}
	for (Edge<3> c : Edge<3>::getValues()) {
		REQUIRE(d.hasEdgeNbr(c) == d2.hasEdgeNbr(c));
		if (d.hasEdgeNbr(c)) {
			CHECK(d.edge_nbr_info[c.getIndex()] != d2.edge_nbr_info[c.getIndex()]);
			switch (d.getEdgeNbrType(c)) {
				case NbrType::Normal:
					CHECK(d.getEdgeNormalNbrInfo(c).id == d2.getEdgeNormalNbrInfo(c).id);
					break;
				case NbrType::Fine:
					CHECK(d.getEdgeFineNbrInfo(c).ids[0] == d2.getEdgeFineNbrInfo(c).ids[0]);
					break;
				case NbrType::Coarse:
					CHECK(d.getEdgeCoarseNbrInfo(c).id == d2.getEdgeCoarseNbrInfo(c).id);
					break;
			}
		}
	}
}
TEST_CASE("PatchInfo copy assignment", "[PatchInfo]")
{
	PatchInfo<3> d;
	d.id              = 9;
	d.local_index     = 10;
	d.global_index    = 10;
	d.rank            = 0;
	d.parent_id       = 2;
	d.parent_rank     = 3;
	d.num_ghost_cells = 239;
	d.refine_level    = 329;
	d.starts          = {1, 2, 3};
	d.spacings        = {0.1, 0.2, 0.3};
	d.ns              = {10, 20, 30};
	d.child_ids       = {3, 4, 5, 6, 7, 8, 9, 10};
	d.child_ranks     = {1, 2, 3, 4, 5, 6, 7, 8};
	d.nbr_info[Side<3>::north().getIndex()].reset(new NormalNbrInfo<3>(1));
	d.nbr_info[Side<3>::east().getIndex()].reset(new CoarseNbrInfo<3>(2, Orthant<2>::nw()));
	d.nbr_info[Side<3>::south().getIndex()].reset(new FineNbrInfo<3>({3, 4, 5, 6}));
	d.corner_nbr_info[Corner<3>::bsw().getIndex()].reset(new NormalNbrInfo<1>(1));
	d.corner_nbr_info[Corner<3>::tse().getIndex()].reset(new CoarseNbrInfo<1>(2, Orthant<0>(0)));
	d.corner_nbr_info[Corner<3>::bnw().getIndex()].reset(new FineNbrInfo<1>({1}));
	d.edge_nbr_info[Edge<3>::sw().getIndex()].reset(new NormalNbrInfo<2>(1));
	d.edge_nbr_info[Edge<3>::bn().getIndex()].reset(new CoarseNbrInfo<2>(2, Orthant<1>::lower()));
	d.edge_nbr_info[Edge<3>::tw().getIndex()].reset(new FineNbrInfo<2>({1, 2}));

	PatchInfo<3> d2;
	d2 = d;

	CHECK(d.id == d2.id);
	CHECK(d.local_index == d2.global_index);
	CHECK(d.rank == d2.rank);
	CHECK(d.parent_id == d2.parent_id);
	CHECK(d.parent_rank == d2.parent_rank);
	CHECK(d.num_ghost_cells == d2.num_ghost_cells);
	CHECK(d.refine_level == d2.refine_level);
	CHECK(d.starts == d2.starts);
	CHECK(d.spacings == d2.spacings);
	CHECK(d.ns == d2.ns);
	CHECK(d.child_ids == d2.child_ids);
	CHECK(d.child_ranks == d2.child_ranks);

	for (Side<3> s : Side<3>::getValues()) {
		REQUIRE(d.hasNbr(s) == d2.hasNbr(s));
		if (d.hasNbr(s)) {
			CHECK(d.nbr_info[s.getIndex()] != d2.nbr_info[s.getIndex()]);
			switch (d.getNbrType(s)) {
				case NbrType::Normal:
					CHECK(d.getNormalNbrInfo(s).id == d2.getNormalNbrInfo(s).id);
					break;
				case NbrType::Fine:
					CHECK(d.getFineNbrInfo(s).ids[0] == d2.getFineNbrInfo(s).ids[0]);
					break;
				case NbrType::Coarse:
					CHECK(d.getCoarseNbrInfo(s).id == d2.getCoarseNbrInfo(s).id);
					break;
			}
		}
	}
	for (Corner<3> c : Corner<3>::getValues()) {
		REQUIRE(d.hasCornerNbr(c) == d2.hasCornerNbr(c));
		if (d.hasCornerNbr(c)) {
			CHECK(d.corner_nbr_info[c.getIndex()] != d2.corner_nbr_info[c.getIndex()]);
			switch (d.getCornerNbrType(c)) {
				case NbrType::Normal:
					CHECK(d.getCornerNormalNbrInfo(c).id == d2.getCornerNormalNbrInfo(c).id);
					break;
				case NbrType::Fine:
					CHECK(d.getCornerFineNbrInfo(c).ids[0] == d2.getCornerFineNbrInfo(c).ids[0]);
					break;
				case NbrType::Coarse:
					CHECK(d.getCornerCoarseNbrInfo(c).id == d2.getCornerCoarseNbrInfo(c).id);
					break;
			}
		}
	}
	for (Edge<3> c : Edge<3>::getValues()) {
		REQUIRE(d.hasEdgeNbr(c) == d2.hasEdgeNbr(c));
		if (d.hasEdgeNbr(c)) {
			CHECK(d.edge_nbr_info[c.getIndex()] != d2.edge_nbr_info[c.getIndex()]);
			switch (d.getEdgeNbrType(c)) {
				case NbrType::Normal:
					CHECK(d.getEdgeNormalNbrInfo(c).id == d2.getEdgeNormalNbrInfo(c).id);
					break;
				case NbrType::Fine:
					CHECK(d.getEdgeFineNbrInfo(c).ids[0] == d2.getEdgeFineNbrInfo(c).ids[0]);
					break;
				case NbrType::Coarse:
					CHECK(d.getEdgeCoarseNbrInfo(c).id == d2.getEdgeCoarseNbrInfo(c).id);
					break;
			}
		}
	}
}