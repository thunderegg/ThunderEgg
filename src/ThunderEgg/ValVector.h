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

#ifndef THUNDEREGG_VALVECTOR_H
#define THUNDEREGG_VALVECTOR_H
#include <ThunderEgg/Vector.h>
#include <valarray>
namespace ThunderEgg
{
/**
 * @brief Vector class the uses std::valarray for data storage
 *
 * @tparam D the number of Cartesian dimensions
 */
template <size_t D> class ValVector : public Vector<D>

{
	private:
	/**
	 * @brief the number of cells in each patch
	 */
	int patch_stride;
	/**
	 * @brief the number of non-ghost cells in each direction of the patch
	 */
	std::array<int, D> lengths;
	/**
	 * @brief the strides for each axis of the patch
	 */
	std::array<int, D> strides;
	/**
	 * @brief the number of ghost cells on each side of the patch
	 */
	int num_ghost_cells;
	/**
	 * @brief the offset of the first element in each patch
	 */
	int first_offset;

	public:
	/**
	 * @brief the underlying vector
	 */
	std::valarray<double> vec;
	std::valarray<double> ghost_data;
	ValVector(MPI_Comm comm_in, const std::array<int, D> &lengths, int num_ghost_cells,
	          int num_patches)
	: Vector<D>(comm_in)
	{
		this->num_local_patches = num_patches;
		this->num_ghost_cells   = num_ghost_cells;
		int size                = 1;
		this->lengths           = lengths;
		first_offset            = 0;
		for (size_t i = 0; i < D; i++) {
			strides[i] = size;
			size *= (this->lengths[i] + 2 * num_ghost_cells);
			first_offset += strides[i] * num_ghost_cells;
		}
		patch_stride = size;
		size *= num_patches;
		vec.resize(size);

		int num_cells_in_patch = 1;
		for (size_t i = 0; i < D; i++) {
			num_cells_in_patch *= this->lengths[i];
		}
		this->num_local_cells = this->num_local_patches * num_cells_in_patch;
	}
	/**
	 * @brief Get a new ValVector object for a given Domain
	 *
	 * @param domain the Domain
	 * @return std::shared_ptr<ValVector<D>> the new Vector
	 */
	static std::shared_ptr<ValVector<D>> GetNewVector(std::shared_ptr<const Domain<D>> domain)
	{
		return std::shared_ptr<ValVector<D>>(new ValVector<D>(
		MPI_COMM_WORLD, domain->getNs(), domain->getNumGhostCells(), domain->getNumLocalPatches()));
	}
	LocalData<D> getLocalData(int local_patch_id)
	{
		double *data = &vec[patch_stride * local_patch_id + first_offset];
		return LocalData<D>(data, strides, lengths, num_ghost_cells, nullptr);
	}
	const LocalData<D> getLocalData(int local_patch_id) const
	{
		double *data = const_cast<double *>(&vec[patch_stride * local_patch_id + first_offset]);
		return LocalData<D>(data, strides, lengths, num_ghost_cells, nullptr);
	}
	void setNumGhostPatches(int num_ghost_patches)
	{
		ghost_data.resize(patch_stride * num_ghost_patches);
	}
};
} // namespace ThunderEgg
#endif