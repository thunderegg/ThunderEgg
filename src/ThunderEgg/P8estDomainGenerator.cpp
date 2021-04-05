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

#include <p4est_iterate.h>

using namespace std;
using namespace ThunderEgg;

std::shared_ptr<Domain<3>> P8estDomainGenerator::getFinestDomain()
{
	return nullptr;
}
bool P8estDomainGenerator::hasCoarserDomain()
{
	return false;
}
P8estDomainGenerator::P8estDomainGenerator(p8est_t *p8est, const std::array<int, 3> &ns,
                                           int num_ghost_cells, BlockMapFunc bmf)
{
}
P8estDomainGenerator::~P8estDomainGenerator() {}

std::shared_ptr<Domain<3>> P8estDomainGenerator::getCoarserDomain()
{
	return nullptr;
}