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

#include "P8estDomainGenerator.h"
#include "mpi.h"
#include "p4est_base.h"

#include <p8est_extended.h>

#include <algorithm>

using namespace std;
using namespace ThunderEgg;

namespace
{
struct IterateFunctions {
	std::function<void(p8est_iter_volume_info_t *)> iter_volume;
	std::function<void(p8est_iter_face_info_t *)>   iter_face;
};

void IterVolumeWrap(p8est_iter_volume_info_t *info, void *user_data)
{
	IterateFunctions *functions = (IterateFunctions *) user_data;
	functions->iter_volume(info);
}

void IterFaceWrap(p8est_iter_face_info_t *info, void *user_data)
{
	IterateFunctions *functions = (IterateFunctions *) user_data;
	functions->iter_face(info);
}

void p8est_iterate_ext(p8est_t *                                       p8est,
                       p8est_ghost_t *                                 ghost_layer,
                       std::function<void(p8est_iter_volume_info_t *)> iter_volume,
                       std::function<void(p8est_iter_face_info_t *)>   iter_face,
                       int                                             remote)
{
	IterateFunctions functions = {iter_volume, iter_face};
	p8est_iterate_ext(
	p8est, ghost_layer, &functions, iter_volume ? IterVolumeWrap : nullptr, iter_face ? IterFaceWrap : nullptr, nullptr, nullptr, remote);
}

struct CoarsenFunctions {
	std::function<int(p4est_topidx_t, p8est_quadrant_t *[])>                                  coarsen;
	std::function<void(p4est_topidx_t, int, p8est_quadrant_t *[], int, p8est_quadrant_t *[])> replace;
};

int CoarsenWrap(p8est_t *p8est, p4est_topidx_t which_tree, p8est_quadrant_t *quadrants[])
{
	CoarsenFunctions *functions = (CoarsenFunctions *) p8est->user_pointer;
	return functions->coarsen(which_tree, quadrants);
}

void ReplaceWrap(p8est_t *         p8est,
                 p4est_topidx_t    which_tree,
                 int               num_outgoing,
                 p8est_quadrant_t *outgoing[],
                 int               num_incoming,
                 p8est_quadrant_t *incoming[])
{
	CoarsenFunctions *functions = (CoarsenFunctions *) p8est->user_pointer;
	functions->replace(which_tree, num_outgoing, outgoing, num_incoming, incoming);
}

void p8est_coarsen_ext(p8est_t *                                                                                 p8est,
                       int                                                                                       coarsen_recursive,
                       int                                                                                       callback_orphans,
                       std::function<int(p4est_topidx_t, p8est_quadrant_t *[])>                                  coarsen,
                       p8est_init_t                                                                              init_fn,
                       std::function<void(p4est_topidx_t, int, p8est_quadrant_t *[], int, p8est_quadrant_t *[])> replace)
{
	CoarsenFunctions functions = {coarsen, replace};
	p8est->user_pointer        = &functions;
	p8est_coarsen_ext(p8est, coarsen_recursive, callback_orphans, coarsen ? CoarsenWrap : nullptr, init_fn, replace ? ReplaceWrap : nullptr);
	p8est->user_pointer = nullptr;
}

int GetMaxLevel(p8est_t *p8est)
{
	int max_level = 0;

	p8est_iterate_ext(
	p8est, nullptr, [&](p8est_iter_volume_info_t *info) { max_level = max(max_level, (int) info->quad->level); }, nullptr, false);

	int global_max_level;

	MPI_Allreduce(&max_level, &global_max_level, 1, MPI_INT, MPI_MAX, p8est->mpicomm);
	return global_max_level;
}

struct Data {
	int                id;
	int                rank;
	std::array<int, 8> child_ids;
	std::array<int, 8> child_ranks;
	PatchInfo<3> *     pinfo;
};

void InitData(p8est_t *p8est, p4est_topidx_t which_tree, p8est_quadrant_t *quadrant)
{
	Data &data = *(Data *) quadrant->p.user_data;
	data.id    = -1;
	data.rank  = -1;
	data.child_ids.fill(-1);
	data.child_ranks.fill(-1);
	data.pinfo = nullptr;
}

void SetRanks(p8est_t *p8est)
{
	int rank;
	MPI_Comm_rank(p8est->mpicomm, &rank);

	p8est_iterate_ext(
	p8est,
	nullptr,
	[=](p8est_iter_volume_info_t *info) {
		Data *data = (Data *) info->quad->p.user_data;
		data->rank = rank;
	},
	nullptr,
	false);
}

void SetIds(p8est_t *p8est)
{
	int curr_global_id;
	MPI_Scan(&p8est->local_num_quadrants, &curr_global_id, 1, MPI_INT, MPI_SUM, p8est->mpicomm);
	curr_global_id -= p8est->local_num_quadrants;

	p8est_iterate_ext(
	p8est,
	nullptr,
	[&](p8est_iter_volume_info_t *info) {
		Data *data = (Data *) info->quad->p.user_data;
		data->id   = curr_global_id;
		curr_global_id++;
	},
	nullptr,
	false);
}

} // namespace

/*
 * constructor
 */
P8estDomainGenerator::P8estDomainGenerator(p8est_t *p8est, const std::array<int, 3> &ns, int num_ghost_cells, BlockMapFunc bmf)
: ns(ns),
  num_ghost_cells(num_ghost_cells),
  bmf(bmf)
{
	my_p8est = p8est_copy(p8est, false);

	p8est_reset_data(my_p8est, sizeof(Data), InitData, nullptr);

	curr_level = GetMaxLevel(my_p8est);

	SetIds(my_p8est);

	extractLevel();
}

P8estDomainGenerator::~P8estDomainGenerator()
{
	p8est_destroy(my_p8est);
}

void P8estDomainGenerator::extractLevel()
{
	if (domains.size() > 0) {
		coarsenTree();
	}

	SetRanks(my_p8est);

	createPatchInfos();

	linkNeighbors();

	std::map<int, std::shared_ptr<PatchInfo<3>>> pinfo_map = getPatchInfos();

	std::shared_ptr<Domain<3>> domain = make_shared<Domain<3>>(pinfo_map, ns, num_ghost_cells, true);

	domains.push_front(domain);

	if (domains.size() == 2) {
		updateParentRanksOfPreviousDomain();
	}

	curr_level--;
}
void P8estDomainGenerator::coarsenTree()
{
	p8est_coarsen_ext(
	my_p8est,
	false,
	true,
	[&](p4est_topidx_t which_tree, p8est_quadrant_t *quadrant[]) {
		if (quadrant[1] == nullptr) {
			// update child data to point to self
			Data *data                  = (Data *) quadrant[0]->p.user_data;
			data->child_ids[0]          = data->id;
			data->child_ranks[0]        = data->rank;
			data->pinfo->parent_id      = data->id;
			data->pinfo->orth_on_parent = Orthant<3>::null();
			return false;
		} else if (quadrant[0]->level > curr_level) {
			// update child datas to point to parent
			Data *bsw_data = (Data *) quadrant[0]->p.user_data;
			for (int i = 0; i < 8; i++) {
				Data *data                  = (Data *) quadrant[i]->p.user_data;
				data->child_ids[0]          = data->id;
				data->child_ranks[0]        = data->rank;
				data->pinfo->parent_id      = bsw_data->id;
				data->pinfo->orth_on_parent = Orthant<3>(i);
			}
			return true;
		} else {
			// update child datas to point to self
			for (int i = 0; i < 8; i++) {
				Data *data                  = (Data *) quadrant[i]->p.user_data;
				data->child_ids[0]          = data->id;
				data->child_ranks[0]        = data->rank;
				data->pinfo->parent_id      = data->id;
				data->pinfo->orth_on_parent = Orthant<3>::null();
			}
			return false;
		}
	},
	InitData,
	[](p4est_topidx_t which_tree, int num_outgoing, p8est_quadrant_t *outgoing[], int num_incoming, p8est_quadrant_t *incoming[]) {
		Data *data         = (Data *) incoming[0]->p.user_data;
		Data *fine_sw_data = (Data *) outgoing[0]->p.user_data;
		data->id           = fine_sw_data->id;
		for (int i = 0; i < 8; i++) {
			Data *fine_data      = (Data *) outgoing[i]->p.user_data;
			data->child_ids[i]   = fine_data->id;
			data->child_ranks[i] = fine_data->rank;
		}
	});
	p8est_partition_ext(my_p8est, true, nullptr);
}

void P8estDomainGenerator::createPatchInfos()
{
	p8est_iterate_ext(
	my_p8est,
	nullptr,
	[&](p8est_iter_volume_info_t *info) {
		Data *data  = (Data *) info->quad->p.user_data;
		data->pinfo = new PatchInfo<3>();

		data->pinfo->rank            = info->p4est->mpirank;
		data->pinfo->id              = data->id;
		data->pinfo->local_index     = info->quadid + p8est_tree_array_index(info->p4est->trees, info->treeid)->quadrants_offset;
		data->pinfo->ns              = ns;
		data->pinfo->num_ghost_cells = num_ghost_cells;
		data->pinfo->refine_level    = info->quad->level;

		data->pinfo->child_ids   = data->child_ids;
		data->pinfo->child_ranks = data->child_ranks;

		double unit_x = (double) info->quad->x / P8EST_ROOT_LEN;
		double unit_y = (double) info->quad->y / P8EST_ROOT_LEN;
		double unit_z = (double) info->quad->z / P8EST_ROOT_LEN;

		bmf(info->treeid, unit_x, unit_y, unit_z, data->pinfo->starts[0], data->pinfo->starts[1], data->pinfo->starts[2]);

		double upper_unit_x = (double) (info->quad->x + P8EST_QUADRANT_LEN(info->quad->level)) / P8EST_ROOT_LEN;
		double upper_unit_y = (double) (info->quad->y + P8EST_QUADRANT_LEN(info->quad->level)) / P8EST_ROOT_LEN;
		double upper_unit_z = (double) (info->quad->z + P8EST_QUADRANT_LEN(info->quad->level)) / P8EST_ROOT_LEN;

		bmf(info->treeid, upper_unit_x, upper_unit_y, upper_unit_z, data->pinfo->spacings[0], data->pinfo->spacings[1], data->pinfo->spacings[2]);

		for (int i = 0; i < 3; i++) {
			data->pinfo->spacings[i] -= data->pinfo->starts[i];
			data->pinfo->spacings[i] /= ns[i];
		}
	},
	nullptr,
	false);
}

/*
 * Functions for linkNeighbors
 */
namespace
{
/**
 * @brief set the CoarseNbrInfos on side1
 *
 * @param side1
 * @param side2
 * @param ghost_data
 */
void addCoarseNbrInfo(p8est_iter_face_side_t side1, p8est_iter_face_side_t side2, Data *ghost_data)
{
	Data *nbr_data;
	if (side2.is.full.is_ghost) {
		nbr_data = ghost_data + side2.is.full.quadid;
	} else {
		nbr_data = (Data *) side2.is.full.quad->p.user_data;
	}
	for (int i = 0; i < 4; i++) {
		if (!side1.is.hanging.is_ghost[i]) {
			Data *data = (Data *) side1.is.hanging.quad[i]->p.user_data;

			Side<3> side(side1.face);

			data->pinfo->nbr_info[side.getIndex()]             = make_shared<CoarseNbrInfo<3>>();
			data->pinfo->getCoarseNbrInfo(side).id             = nbr_data->id;
			data->pinfo->getCoarseNbrInfo(side).rank           = nbr_data->rank;
			data->pinfo->getCoarseNbrInfo(side).orth_on_coarse = Orthant<2>(i);
		}
	}
}

/**
 * @brief set the FineNbrInfo on side1
 *
 * @param side1
 * @param side2
 * @param ghost_data
 */
void addFineNbrInfo(p8est_iter_face_side_t side1, p8est_iter_face_side_t side2, Data *ghost_data)
{
	if (!side1.is.full.is_ghost) {
		Data *data = (Data *) side1.is.full.quad->p.user_data;
		Data *nbr_datas[4];
		for (int i = 0; i < 4; i++) {
			if (side2.is.hanging.is_ghost[i]) {
				nbr_datas[i] = ghost_data + side2.is.hanging.quadid[i];
			} else {
				nbr_datas[i] = (Data *) side2.is.hanging.quad[i]->p.user_data;
			}
		}

		Side<3> side(side1.face);

		data->pinfo->nbr_info[side.getIndex()] = make_shared<FineNbrInfo<3>>();
		for (int i = 0; i < 4; i++) {
			data->pinfo->getFineNbrInfo(side).ids[i]   = nbr_datas[i]->id;
			data->pinfo->getFineNbrInfo(side).ranks[i] = nbr_datas[i]->rank;
		}
	}
}

/**
 * @brief Set the NormalNbrInfo on side1
 *
 * @param side1
 * @param side2
 * @param ghost_data
 */
void addNormalNbrInfo(p8est_iter_face_side_t side1, p8est_iter_face_side_t side2, Data *ghost_data)
{
	if (!side1.is.full.is_ghost) {
		Data *data = (Data *) side1.is.full.quad->p.user_data;
		Data *nbr_data;
		if (side2.is.full.is_ghost) {
			nbr_data = ghost_data + side2.is.full.quadid;
		} else {
			nbr_data = (Data *) side2.is.full.quad->p.user_data;
		}

		Side<3> side(side1.face);

		data->pinfo->nbr_info[side.getIndex()]   = make_shared<NormalNbrInfo<3>>(nbr_data->id);
		data->pinfo->getNormalNbrInfo(side).rank = nbr_data->rank;
	}
}

/**
 * @brief Add information from side2 to NbrInfo objects on side1
 *
 * @param side1
 * @param side2
 * @param ghost_data p8est ghost data array
 */
void addNbrInfo(p8est_iter_face_side_t side1, p8est_iter_face_side_t side2, Data *ghost_data)
{
	if (side1.is_hanging) {
		addCoarseNbrInfo(side1, side2, ghost_data);
	} else if (side2.is_hanging) {
		addFineNbrInfo(side1, side2, ghost_data);
	} else {
		addNormalNbrInfo(side1, side2, ghost_data);
	}
}
} // namespace

void P8estDomainGenerator::linkNeighbors()
{
	p8est_ghost_t *ghost = p8est_ghost_new(my_p8est, P8EST_CONNECT_FACE);

	Data ghost_data[ghost->ghosts.elem_count];
	p8est_ghost_exchange_data(my_p8est, ghost, ghost_data);

	p8est_iterate_ext(
	my_p8est,
	ghost,
	nullptr,
	[&](p8est_iter_face_info_t *info) {
		if (info->sides.elem_count == 2) {
			p8est_iter_face_side_t side1 = ((p8est_iter_face_side_t *) info->sides.array)[0];
			p8est_iter_face_side_t side2 = ((p8est_iter_face_side_t *) info->sides.array)[1];
			addNbrInfo(side1, side2, ghost_data);
			addNbrInfo(side2, side1, ghost_data);
		}
	},
	false);

	p8est_ghost_destroy(ghost);
}

std::map<int, std::shared_ptr<PatchInfo<3>>> P8estDomainGenerator::getPatchInfos()
{
	map<int, shared_ptr<PatchInfo<3>>> pinfo_map;
	p8est_iterate_ext(
	my_p8est,
	nullptr,
	[&](p8est_iter_volume_info_t *info) {
		Data *data                 = (Data *) info->quad->p.user_data;
		pinfo_map[data->pinfo->id] = shared_ptr<PatchInfo<3>>(data->pinfo);
	},
	nullptr,
	false);

	return pinfo_map;
}

void P8estDomainGenerator::updateParentRanksOfPreviousDomain()
{
	auto old_level = domains.back()->getPatchInfoMap();
	auto new_level = domains.front()->getPatchInfoMap();
	// update parent ranks
	std::map<int, int> id_rank_map;
	// get outgoing information
	std::map<int, std::set<int>> out_info;
	for (auto pair : new_level) {
		id_rank_map[pair.first] = my_p8est->mpirank;
		auto &pinfo             = pair.second;
		for (int i = 0; i < 8; i++) {
			if (pinfo->child_ranks[i] != -1 && pinfo->child_ranks[i] != my_p8est->mpirank) {
				out_info[pinfo->child_ranks[i]].insert(pinfo->id);
			}
		}
	}
	// get incoming information
	std::set<int> incoming_ids;
	for (auto &pair : old_level) {
		if (!new_level.count(pair.second->parent_id)) {
			incoming_ids.insert(pair.second->parent_id);
		}
	}
	// send info
	std::deque<std::vector<int>> buffers;
	std::vector<MPI_Request>     send_requests;
	for (auto &pair : out_info) {
		int              dest = pair.first;
		std::vector<int> buffer(pair.second.begin(), pair.second.end());
		buffers.push_back(buffer);
		MPI_Request request;
		MPI_Isend(buffer.data(), buffer.size(), MPI_INT, dest, 0, MPI_COMM_WORLD, &request);
		send_requests.push_back(request);
	}
	// recv info
	while (incoming_ids.size()) {
		MPI_Status status;
		MPI_Probe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
		int size;
		MPI_Get_count(&status, MPI_INT, &size);
		int *buffer = new int[size];

		MPI_Recv(buffer, size, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);

		for (int i = 0; i < size; i++) {
			id_rank_map[buffer[i]] = status.MPI_SOURCE;
			incoming_ids.erase(buffer[i]);
		}

		delete[] buffer;
	}
	// wait for all
	MPI_Waitall(send_requests.size(), &send_requests[0], MPI_STATUSES_IGNORE);
	// update rank info
	for (auto &pair : old_level) {
		pair.second->parent_rank = id_rank_map.at(pair.second->parent_id);
	}
}

std::shared_ptr<Domain<3>> P8estDomainGenerator::getCoarserDomain()
{
	if (curr_level >= 0) {
		extractLevel();
	}
	shared_ptr<Domain<3>> domain = domains.back();
	domains.pop_back();
	return domain;
}

std::shared_ptr<Domain<3>> P8estDomainGenerator::getFinestDomain()
{
	if (curr_level >= 0) {
		extractLevel();
	}
	shared_ptr<Domain<3>> domain = domains.back();
	domains.pop_back();
	return domain;
}

bool P8estDomainGenerator::hasCoarserDomain()
{
	return curr_level > 0;
}
