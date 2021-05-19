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

#include <ThunderEgg/BiLinearGhostFiller.h>
#include <ThunderEgg/RuntimeError.h>
namespace ThunderEgg
{
BiLinearGhostFiller::BiLinearGhostFiller(std::shared_ptr<const Domain<2>> domain, GhostFillingType fill_type) : MPIGhostFiller<2>(domain, fill_type)
{
}
namespace
{
void FillGhostForNormalNbr(const std::vector<LocalData<2>> &local_datas, const std::vector<LocalData<2>> &nbr_datas, const Side<2> side)
{
	for (size_t c = 0; c < local_datas.size(); c++) {
		auto local_slice = local_datas[c].getSliceOn(side, {0});
		auto nbr_ghosts  = nbr_datas[c].getSliceOn(side.opposite(), {-1});
		nested_loop<1>(nbr_ghosts.getStart(), nbr_ghosts.getEnd(), [&](const std::array<int, 1> &coord) { nbr_ghosts[coord] = local_slice[coord]; });
	}
}
void FillGhostForCoarseNbr(const PatchInfo<2> &             pinfo,
                           const std::vector<LocalData<2>> &local_datas,
                           const std::vector<LocalData<2>> &nbr_datas,
                           const Side<2>                    side,
                           const Orthant<1>                 orthant)
{
	auto nbr_info = pinfo.getCoarseNbrInfo(side);
	int  offset   = 0;
	if (orthant == Orthant<1>::upper()) {
		offset = pinfo.ns[!side.getAxisIndex()];
	}
	for (size_t c = 0; c < local_datas.size(); c++) {
		auto local_slice = local_datas[c].getSliceOn(side, {0});
		auto nbr_ghosts  = nbr_datas[c].getSliceOn(side.opposite(), {-1});
		nested_loop<1>(nbr_ghosts.getStart(), nbr_ghosts.getEnd(), [&](const std::array<int, 1> &coord) {
			nbr_ghosts[{(coord[0] + offset) / 2}] += 2.0 / 3.0 * local_slice[coord];
		});
	}
}
void FillGhostForFineNbr(const PatchInfo<2> &             pinfo,
                         const std::vector<LocalData<2>> &local_datas,
                         const std::vector<LocalData<2>> &nbr_datas,
                         const Side<2>                    side,
                         const Orthant<1>                 orthant)
{
	auto nbr_info = pinfo.getFineNbrInfo(side);
	int  offset   = 0;
	if (orthant == Orthant<1>::upper()) {
		offset = pinfo.ns[!side.getAxisIndex()];
	}
	for (size_t c = 0; c < local_datas.size(); c++) {
		auto local_slice = local_datas[c].getSliceOn(side, {0});
		auto nbr_ghosts  = nbr_datas[c].getSliceOn(side.opposite(), {-1});
		nested_loop<1>(nbr_ghosts.getStart(), nbr_ghosts.getEnd(), [&](const std::array<int, 1> &coord) {
			nbr_ghosts[coord] += 2.0 / 3.0 * local_slice[{(coord[0] + offset) / 2}];
		});
	}
}
void FillLocalGhostsForCoarseNbr(const PatchInfo<2> &pinfo, const LocalData<2> &local_data, const Side<2> side)
{
	auto local_slice  = local_data.getSliceOn(side, {0});
	auto local_ghosts = local_data.getSliceOn(side, {-1});
	int  offset       = 0;
	if (pinfo.getCoarseNbrInfo(side).orth_on_coarse == Orthant<1>::upper()) {
		offset = pinfo.ns[!side.getAxisIndex()];
	}
	nested_loop<1>(local_ghosts.getStart(), local_ghosts.getEnd(), [&](const std::array<int, 1> &coord) {
		local_ghosts[coord] += 2.0 / 3.0 * local_slice[coord];
		if ((coord[0] + offset) % 2 == 0) {
			local_ghosts[{coord[0] + 1}] += -1.0 / 3.0 * local_slice[coord];
		} else {
			local_ghosts[{coord[0] - 1}] += -1.0 / 3.0 * local_slice[coord];
		}
	});
}
void FillLocalGhostsForFineNbr(const LocalData<2> &local_data, const Side<2> side)
{
	auto local_slice  = local_data.getSliceOn(side, {0});
	auto local_ghosts = local_data.getSliceOn(side, {-1});
	nested_loop<1>(
	local_ghosts.getStart(), local_ghosts.getEnd(), [&](const std::array<int, 1> &coord) { local_ghosts[coord] += -1.0 / 3.0 * local_slice[coord]; });
}
void FillGhostForCornerNormalNbr(const std::vector<LocalData<2>> &local_datas, const std::vector<LocalData<2>> &nbr_datas, Corner<2> corner)
{
	for (size_t c = 0; c < local_datas.size(); c++) {
		auto local_slice = local_datas[c].getSliceOn(corner, {0, 0});
		auto nbr_ghost   = nbr_datas[c].getSliceOn(corner.opposite(), {-1, -1});
		nbr_ghost[{}]    = local_slice[{}];
	}
}
void FillGhostForCornerCoarseNbr(const PatchInfo<2> &             pinfo,
                                 const std::vector<LocalData<2>> &local_datas,
                                 const std::vector<LocalData<2>> &nbr_datas,
                                 Corner<2>                        corner)
{
	for (size_t c = 0; c < local_datas.size(); c++) {
		auto nbr_ghosts = nbr_datas[c].getSliceOn(corner.opposite(), {-1, -1});
		nbr_ghosts[{}] += 4.0 * local_datas[c].getSliceOn(corner, {0, 0})[{}] / 3.0;
	}
}
void FillGhostForCornerFineNbr(const PatchInfo<2> &             pinfo,
                               const std::vector<LocalData<2>> &local_datas,
                               const std::vector<LocalData<2>> &nbr_datas,
                               Corner<2>                        corner)
{
	for (size_t c = 0; c < local_datas.size(); c++) {
		auto local_slice = local_datas[c].getSliceOn(corner, {0, 0});
		auto nbr_ghosts  = nbr_datas[c].getSliceOn(corner.opposite(), {-1, -1});
		nbr_ghosts[{}] += 2.0 * local_slice[{}] / 3.0;
	}
}
void FillLocalGhostsForCornerCoarseNbr(const PatchInfo<2> &pinfo, const LocalData<2> &local_data, Corner<2> corner)
{
	auto local_slice  = local_data.getSliceOn(corner, {0, 0});
	auto local_ghosts = local_data.getSliceOn(corner, {-1, -1});
	local_ghosts[{}] += local_slice[{}] / 3.0;
}
void FillLocalGhostsForCornerFineNbr(const LocalData<2> &local_data, Corner<2> corner)
{
	auto local_slice  = local_data.getSliceOn(corner, {0, 0});
	auto local_ghosts = local_data.getSliceOn(corner, {-1, -1});
	local_ghosts[{}] += -local_slice[{}] / 3.0;
}
void FillLocalGhostCellsOnSides(const PatchInfo<2> &pinfo, const LocalData<2> local_data)
{
	for (Side<2> side : Side<2>::getValues()) {
		if (pinfo.hasNbr(side)) {
			switch (pinfo.getNbrType(side)) {
				case NbrType::Normal:
					// nothing needs to be done
					break;
				case NbrType::Coarse:
					FillLocalGhostsForCoarseNbr(pinfo, local_data, side);
					break;
				case NbrType::Fine:
					FillLocalGhostsForFineNbr(local_data, side);
					break;
				default:
					throw RuntimeError("Unsupported Nbr Type");
			}
		}
	}
}
void FillLocalGhostCellsOnCorners(const PatchInfo<2> &pinfo, const LocalData<2> local_data)
{
	for (Corner<2> corner : Corner<2>::getValues()) {
		if (pinfo.hasNbr(corner)) {
			switch (pinfo.getNbrType(corner)) {
				case NbrType::Normal:
					// nothing needs to be done
					break;
				case NbrType::Coarse:
					FillLocalGhostsForCornerCoarseNbr(pinfo, local_data, corner);
					break;
				case NbrType::Fine:
					FillLocalGhostsForCornerFineNbr(local_data, corner);
					break;
				default:
					throw RuntimeError("Unsupported Nbr Type");
			}
		}
	}
}
} // namespace
void BiLinearGhostFiller::fillGhostCellsForNbrPatch(const PatchInfo<2> &             pinfo,
                                                    const std::vector<LocalData<2>> &local_datas,
                                                    std::vector<LocalData<2>> &      nbr_datas,
                                                    Side<2>                          side,
                                                    NbrType                          nbr_type,
                                                    Orthant<1>                       orthant_on_coarse) const
{
	switch (nbr_type) {
		case NbrType::Normal:
			FillGhostForNormalNbr(local_datas, nbr_datas, side);
			break;
		case NbrType::Coarse:
			FillGhostForCoarseNbr(pinfo, local_datas, nbr_datas, side, orthant_on_coarse);
			break;
		case NbrType::Fine:
			FillGhostForFineNbr(pinfo, local_datas, nbr_datas, side, orthant_on_coarse);
			break;
		default:
			throw RuntimeError("Unsupported Nbr Type");
	}
}
void BiLinearGhostFiller::fillGhostCellsForEdgeNbrPatch(const PatchInfo<2> &             pinfo,
                                                        const std::vector<LocalData<2>> &local_datas,
                                                        std::vector<LocalData<2>> &      nbr_datas,
                                                        Edge                             edge,
                                                        NbrType                          nbr_type,
                                                        Orthant<1>                       orthant_on_coarse) const
{
	// 2D, edges not needed
}

void BiLinearGhostFiller::fillGhostCellsForCornerNbrPatch(const PatchInfo<2> &             pinfo,
                                                          const std::vector<LocalData<2>> &local_datas,
                                                          std::vector<LocalData<2>> &      nbr_datas,
                                                          Corner<2>                        corner,
                                                          NbrType                          nbr_type) const
{
	switch (nbr_type) {
		case NbrType::Normal:
			FillGhostForCornerNormalNbr(local_datas, nbr_datas, corner);
			break;
		case NbrType::Coarse:
			FillGhostForCornerCoarseNbr(pinfo, local_datas, nbr_datas, corner);
			break;
		case NbrType::Fine:
			FillGhostForCornerFineNbr(pinfo, local_datas, nbr_datas, corner);
			break;
		default:
			throw RuntimeError("Unsupported Nbr Type");
	}
}
void BiLinearGhostFiller::fillGhostCellsForLocalPatch(const PatchInfo<2> &pinfo, std::vector<LocalData<2>> &local_datas) const
{
	for (const LocalData<2> &local_data : local_datas) {
		switch (this->getFillType()) {
			case GhostFillingType::Corners:
				FillLocalGhostCellsOnCorners(pinfo, local_data);
			case GhostFillingType::Edges:
			case GhostFillingType::Faces:
				FillLocalGhostCellsOnSides(pinfo, local_data);
				break;
			default:
				throw RuntimeError("Unsupported GhostFillingType");
		}
	}
}
} // namespace ThunderEgg