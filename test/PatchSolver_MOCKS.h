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

#include <ThunderEgg/GhostFiller.h>
#include <ThunderEgg/PatchSolver.h>

#include <set>

#include <catch2/catch_test_macros.hpp>

namespace ThunderEgg
{
namespace
{
template <int D>
class MockGhostFiller : public GhostFiller<D>
{
	private:
	std::shared_ptr<bool> called = std::make_shared<bool>(false);

	public:
	MockGhostFiller<D> *clone() const override
	{
		return new MockGhostFiller<D>(*this);
	}
	void fillGhost(const Vector<D> &u) const override
	{
		*called = true;
	}
	bool wasCalled()
	{
		return *called;
	}
};
template <int D>
class MockPatchSolver : public PatchSolver<D>
{
	private:
	mutable std::set<const PatchInfo<D> *> patches_to_be_called;

	public:
	MockPatchSolver(std::shared_ptr<const Domain<D>> domain_in,
	                const GhostFiller<D> &           ghost_filler_in)
	: PatchSolver<D>(domain_in, ghost_filler_in)
	{
		for (const PatchInfo<D> &pinfo : domain_in->getPatchInfoVector()) {
			patches_to_be_called.insert(&pinfo);
		}
	}
	MockPatchSolver<D> *clone() const override
	{
		return new MockPatchSolver<D>(*this);
	}
	void solveSinglePatch(const PatchInfo<D> &              pinfo,
	                      const PatchView<const double, D> &fs,
	                      const PatchView<double, D> &      us) const override
	{
		CHECK(patches_to_be_called.count(&pinfo) == 1);
		patches_to_be_called.erase(&pinfo);
		std::array<int, D + 1> zero;
		zero.fill(0);
	}
	bool allPatchesCalled()
	{
		return patches_to_be_called.empty();
	}
};
} // namespace
} // namespace ThunderEgg