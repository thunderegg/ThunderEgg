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
 *  This program is distributed in the hope that it will be u_vieweful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 ***************************************************************************/

#ifndef THUNDEREGG_ITERATIVE_PATCHSOLVER_H
#define THUNDEREGG_ITERATIVE_PATCHSOLVER_H

#include <ThunderEgg/BreakdownError.h>
#include <ThunderEgg/Domain.h>
#include <ThunderEgg/GMG/Level.h>
#include <ThunderEgg/Iterative/Solver.h>
#include <ThunderEgg/PatchOperator.h>
#include <ThunderEgg/PatchSolver.h>
#include <ThunderEgg/ValVector.h>
#include <bitset>
#include <map>

namespace ThunderEgg
{
namespace Iterative
{
/**
 * @brief Solves the patches u_viewing a Iterative iterative solver on each patch
 *
 * @tparam D the number of cartesian dimensions
 */
template <int D> class PatchSolver : public ThunderEgg::PatchSolver<D>
{
	private:
	/**
	 * @brief Generates a vector that is only the size of a patch
	 */
	class SingleVG : public VectorGenerator<D>
	{
		private:
		/**
		 * @brief number of cells along each axis
		 */
		std::array<int, D> lengths;
		/**
		 * @brief number of ghost cells
		 */
		int num_ghost_cells;
		/**
		 * @brief The number of components for each cell
		 */
		int num_components;

		public:
		/**
		 * @brief Construct a new SingleVG object for a given patch
		 *
		 * @param pinfo the PatchInfo object for the patch
		 * @param num_components the number of components for each cell
		 */
		SingleVG(const PatchInfo<D> &pinfo, int num_components)
		: lengths(pinfo.ns),
		  num_ghost_cells(pinfo.num_ghost_cells),
		  num_components(num_components)
		{
		}
		/**
		 * @brief Get a newly allocated vector
		 *
		 * @return std::shared_ptr<Vector<D>> the vector
		 */
		std::shared_ptr<Vector<D>> getNewVector() const override
		{
			return std::shared_ptr<Vector<D>>(new ValVector<D>(MPI_COMM_SELF, lengths, num_ghost_cells, num_components, 1));
		}
	};
	/**
	 * @brief Wrapper that provides a vector interface for the ComponentView of a patch
	 */
	class SinglePatchVec : public Vector<D>
	{
		private:
		/**
		 * @brief The view for the patch
		 */
		PatchView<double, D> view;

		/**
		 * @brief Get the number of local cells in the view object
		 *
		 * @param view the view
		 * @return int the number of local cells
		 */
		static int GetNumLocalCells(const PatchView<double, D> &view)
		{
			int patch_stride = 1;
			for (size_t i = 0; i < D; i++) {
				patch_stride *= view.getEnd()[i] + 1;
			}
			return patch_stride;
		}

		public:
		/**
		 * @brief Construct a new SinglePatchVec object
		 *
		 * @param view the ComponentView for the patch
		 */
		SinglePatchVec(const PatchView<double, D> &view) : Vector<D>(MPI_COMM_SELF, view.getEnd()[D] + 1, 1, GetNumLocalCells(view)), view(view) {}
		PatchView<double, D> getPatchView(int local_patch_id) override
		{
			return view;
		}
		PatchView<const double, D> getPatchView(int local_patch_id) const override
		{
			return view;
		}
	};
	/**
	 * @brief Wrapper that provides a vector interface for the ComponentView of a patch
	 */
	class SinglePatchConstVec : public Vector<D>
	{
		private:
		/**
		 * @brief The ComponentView for the patch
		 */
		PatchView<const double, D> view;

		/**
		 * @brief Get the number of local cells in the view
		 *
		 * @param view the view
		 * @return int the number of local cells
		 */
		static int GetNumLocalCells(const PatchView<const double, D> &view)
		{
			int patch_stride = 1;
			for (size_t i = 0; i < D; i++) {
				patch_stride *= view.getEnd()[i] + 1;
			}
			return patch_stride;
		}

		public:
		/**
		 * @brief Construct a new SinglePatchConstVec object
		 *
		 * @param view the view for the patch
		 */
		SinglePatchConstVec(const PatchView<const double, D> &view)
		: Vector<D>(MPI_COMM_SELF, view.getEnd()[D] + 1, 1, GetNumLocalCells(view)),
		  view(view)
		{
		}
		PatchView<double, D> getPatchView(int local_patch_id) override
		{
			throw RuntimeError("This is a read only vector");
		}
		PatchView<const double, D> getPatchView(int local_patch_id) const override
		{
			return view;
		}
	};
	/**
	 * @brief This wraps a PatchOperator object so that it only applies the operator on a specified
	 * patch
	 */
	class SinglePatchOp : public Operator<D>
	{
		private:
		/**
		 * @brief the PatchOperator that is being wrapped
		 */
		std::shared_ptr<const PatchOperator<D>> op;
		/**
		 * @brief the PatchInfo object for the patch
		 */
		const PatchInfo<D> &pinfo;

		public:
		/**
		 * @brief Construct a new SinglePatchOp object
		 *
		 * @param pinfo the patch that we want to operate on
		 * @param op the operator
		 */
		SinglePatchOp(const PatchInfo<D> &pinfo, std::shared_ptr<const PatchOperator<D>> op) : op(op), pinfo(pinfo) {}
		void apply(std::shared_ptr<const Vector<D>> x, std::shared_ptr<Vector<D>> b) const
		{
			PatchView<const double, D> x_view = x->getPatchView(0);
			PatchView<double, D>       b_view = b->getPatchView(0);
			op->enforceBoundaryConditions(pinfo, x_view);
			op->enforceZeroDirichletAtInternalBoundaries(pinfo, x_view);
			op->applySinglePatch(pinfo, x_view, b_view);
		}
	};

	/**
	 * @brief The iterative solver being u_viewed
	 */
	std::shared_ptr<const Solver<D>> solver;

	/**
	 * @brief The operator for the solve
	 */
	std::shared_ptr<const PatchOperator<D>> op;

	/**
	 * @brief whether or not to continue on BreakDownError
	 */
	bool continue_on_breakdown;

	public:
	/**
	 * @brief Construct a new IterativePatchSolver object
	 *
	 * @param op_in the PatchOperator to u_viewe
	 * @param tol_in the tolerance to u_viewe for patch solves
	 * @param max_it_in the maximum number of iterations to u_viewe for patch solves
	 * @param continue_on_breakdown continue on breakdown exception
	 */
	PatchSolver(std::shared_ptr<const Iterative::Solver<D>> solver, std::shared_ptr<const PatchOperator<D>> op_in, bool continue_on_breakdown = false)
	: ThunderEgg::PatchSolver<D>(op_in->getDomain(), op_in->getGhostFiller()),
	  solver(solver),
	  op(op_in),
	  continue_on_breakdown(continue_on_breakdown)
	{
	}
	void solveSinglePatch(const PatchInfo<D> &pinfo, const PatchView<const double, D> &f_view, const PatchView<double, D> &u_view) const override
	{
		std::shared_ptr<SinglePatchOp>      single_op(new SinglePatchOp(pinfo, op));
		std::shared_ptr<VectorGenerator<D>> vg(new SingleVG(pinfo, f_view.getEnd()[D] + 1));

		std::shared_ptr<Vector<D>> f_single(new SinglePatchConstVec(f_view));
		std::shared_ptr<Vector<D>> u_single(new SinglePatchVec(u_view));

		auto f_copy = vg->getNewVector();
		f_copy->copy(f_single);
		auto f_copy_view = f_copy->getPatchView(0);
		op->modifyRHSForZeroDirichletAtInternalBoundaries(pinfo, u_view, f_copy_view);

		int iterations = 0;
		try {
			iterations = solver->solve(vg, single_op, u_single, f_copy);
		} catch (const BreakdownError &err) {
			if (!continue_on_breakdown) {
				throw err;
			}
		}
		if (this->getDomain()->hasTimer()) {
			this->getDomain()->getTimer()->addIntInfo("Iterations", iterations);
		}
	}
};
} // namespace Iterative
} // namespace ThunderEgg
extern template class ThunderEgg::Iterative::PatchSolver<2>;
extern template class ThunderEgg::Iterative::PatchSolver<3>;
#endif
