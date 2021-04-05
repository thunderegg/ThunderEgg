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

#ifndef THUNDEREGG_P8ESTDOMAINGENERATOR_H
#define THUNDEREGG_P8ESTDOMAINGENERATOR_H
#include <ThunderEgg/DomainGenerator.h>
#include <functional>
#include <list>
#include <p8est.h>
namespace ThunderEgg
{
/**
 * @brief Generates Domain objects form a given p4est object
 */
class P8estDomainGenerator : public DomainGenerator<3>
{
	public:
	/**
	 * @brief Maps coordinate in a block to a coordinate in the domain.
	 *
	 * Each block is treated as a unit square. The input wil be the block number and a coordinate
	 * withing that unit square.
	 *
	 * @param block_no the block number
	 * @param unit_x the x coordinate in the block
	 * @param unit_y the y coordinate in the block
	 * @param unit_z the z coordinate in the block
	 * @param x the resulting x coordinate of the mapping function
	 * @param y the resulting y coordinate of the mapping function
	 * @param z the resulting z coordinate of the mapping function
	 */
	using BlockMapFunc = std::function<void(int block_no, double unit_x, double unit_y,
	                                        double unit_z, double &x, double &y, double &z)>;

	private:
	/**
	 * @brief copy of p8est tree
	 */
	p8est_t *my_p8est;
	/**
	 * @brief List of the domains
	 *
	 * Finest domain is stored in front
	 */
	std::list<std::shared_ptr<Domain<2>>> domain_list;

	/**
	 * @brief the number of ghost cells on each side of the patch
	 */
	int num_ghost_cells;
	/**
	 * @brief The current level that has been generated.
	 *
	 * Will start with num_levels-1
	 */
	int curr_level;
	/**
	 * @brief The number of levels the p4est quad-tree
	 */
	int num_levels;
	/**
	 * @brief The dimensions of each patch
	 */
	std::array<int, 2> ns;
	/**
	 * @brief The length of a block on the x-axis
	 */
	double x_scale;
	/**
	 * @brief The length of a block on the y-axis
	 */
	double y_scale;
	/**
	 * @brief The block Mapping function being used.
	 */
	BlockMapFunc bmf;
	/**
	 * @brief The function used to set neumann boundary conditions
	 */
	IsNeumannFunc<3> inf;

	/**
	 * @brief Get a new coarser level and add it to the end of domain_list
	 */
	void extractLevel();

	public:
	/**
	 * @brief Construct a new P8estDomainGenerator object
	 *
	 * @param p8est the p8est object
	 * @param ns the number of cells in each direction
	 * @param num_ghost_cells the number of ghost cells on each side of the patch
	 * @param inf the function used to set neumann boundary conditions
	 * @param bmf the function used to map the blocks to the domain
	 */
	P8estDomainGenerator(p8est_t *p8est, const std::array<int, 3> &ns, int num_ghost_cells,
	                     BlockMapFunc bmf);
	~P8estDomainGenerator();
	std::shared_ptr<Domain<3>> getFinestDomain();
	bool                       hasCoarserDomain();
	std::shared_ptr<Domain<3>> getCoarserDomain();
};
} // namespace ThunderEgg
#endif