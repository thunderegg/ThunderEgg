/***************************************************************************
 *  Thunderegg, a library for solving Poisson's equation on adaptively
 *  refined block-structured Cartesian grids
 *
 *  Copyright (C) 2019  Thunderegg Developers. See AUTHORS.md file at the
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

#ifndef GMGCYCLEFACTORY2D_H
#define GMGCYCLEFACTORY2D_H
#include <Thunderegg/DomainCollection.h>
#include <Thunderegg/DomainCollectionGenerator.h>
#include <Thunderegg/GMG/Cycle.h>
#include <Thunderegg/SchurHelper.h>
namespace GMG
{
class CycleFactory2d
{
	public:
	static std::shared_ptr<Cycle<2>> getCycle(std::string config_file,
	                                          std::shared_ptr<DomainCollectionGenerator<2>> dcg,
	                                          std::shared_ptr<PatchSolver<2>>               solver,
	                                          std::shared_ptr<PatchOperator<2>>             op,
	                                          std::shared_ptr<IfaceInterp<2>> interpolator);
};
} // namespace GMG
#endif