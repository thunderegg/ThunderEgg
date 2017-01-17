/*
   Example 3

   Interface:    Semi-Structured interface with two parts

*/

#include <stdio.h>

/* SStruct linear solvers headers */
#include "HYPRE_sstruct_ls.h"

#include "HYPRE_parcsr_ls.h"
#include "HYPRE_parcsr_mv.h"

#include "vis.c"
#include "mpi.h"

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

int main (int argc, char *argv[])
{
    int myid, num_procs;

    int vis = 0;

    HYPRE_SStructGrid     grid;
    HYPRE_SStructGraph    graph;
    HYPRE_SStructStencil  stencil_5pt;
    HYPRE_SStructMatrix   A;
    HYPRE_SStructVector   b;
    HYPRE_SStructVector   x;
    HYPRE_SStructSolver   solver;
    HYPRE_SStructSolver   precond;

    HYPRE_ParCSRMatrix parcsr_A;
    HYPRE_ParVector par_b;
    HYPRE_ParVector par_x;



    /* Initialize MPI */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    /* ----------------------------------------------------
       User defined parameters (mesh size and solver type)
       ---------------------------------------------------- */

    int nx = 200;  /* nx should equal ny */
    int ny = 200;

    /*
      0 : PCG (no preconditioning)
      1 : PCG (BoomerAMG preconditioning);
      2 : BoomerAMG (no preconditioning)
    */
    int solver_id = 0;

    /*
      Print levels for solvers.
      0   : No printing
      >0  : Print iterations (depends on solver)
    */
    int solver_print_level = 0;

    /* -------------------------------------------------
       Things the user shouldn't change
       ------------------------------------------------- */

    if (nx != ny)
    {
        printf("nx should equal ny for this problem\n");
        MPI_Finalize();
        return(0);
    }

    if (solver_id < 0 || solver_id > 2)
    {
        printf("Solver_id must be 0, 1, or 2\n");
        MPI_Finalize();
        return(0);
    }

    double h = 1.0/nx;   /* Assume that nx == ny */
    double h2 = h*h;

    int ndim = 2;       /* Dimension of the problem */
    int nparts = 2;     /* Number of parts (fixed)  */
    int var = 0;        /* Variable index */
    int nvars = 1;      /* Number of variables */

    int object_type;
    switch(solver_id)
    {
    case 0:
        object_type = HYPRE_SSTRUCT;
        break;
    case 1:
    case 2:
        object_type = HYPRE_PARCSR;    /* Doesn't yet work */
    }


    /* Index space for each part */
    int ll[2], lr[2], ul[2], ur[2];
    ll[0] = 1;
    ll[1] = 1;
    lr[0] = nx;
    lr[1] = 1;
    ul[0] = 1;
    ul[1] = ny;
    ur[0] = nx;
    ur[1] = ny;

    /* Ghost indices, needed to set neighbor connectivity */
    int ll_g[2], lr_g[2], ul_g[2], ur_g[2];
    ll_g[0] = ll[0] - 1;
    ll_g[1] = ll[1];
    lr_g[0] = lr[0] + 1;
    lr_g[1] = lr[1];
    ul_g[0] = ul[0] - 1;
    ul_g[1] = ul[1];
    ur_g[0] = ur[0] + 1;
    ur_g[1] = ur[1];

    /* Miscellaneous variables */
    int i, j, part;


    /* -------------------------------------------------------------
       For this code we assume that all parts are local to a single
       processor.
       ------------------------------------------------------------- */

    if (num_procs != 1)
    {
        printf("Must run with 1 processor!\n");
        MPI_Finalize();

        return(0);
    }

    /* -------------------------------------------------------------
       0. Parse the command line
       ------------------------------------------------------------- */
    {
        int arg_index = 0;
        int print_usage = 0;

        while (arg_index < argc)
        {
            if ( strcmp(argv[arg_index], "-vis") == 0 )
            {
                arg_index++;
                vis = 1;
            }
            else if ( strcmp(argv[arg_index], "-help") == 0 )
            {
                print_usage = 1;
                break;
            }
            else
            {
                arg_index++;
            }
        }

        if ((print_usage) && (myid == 0))
        {
            printf("\n");
            printf("Usage: %s [<options>]\n", argv[0]);
            printf("\n");
            printf("  -vis : save the solution for GLVis visualization\n");
            printf("\n");
        }

        if (print_usage)
        {
            MPI_Finalize();
            return (0);
        }
    }

    /* -------------------------------------------------------------
       1. Set up the 2D grid.
       ------------------------------------------------------------- */

    {
        /* Create an empty 2D grid object */
        HYPRE_SStructGridCreate(MPI_COMM_WORLD, ndim, nparts, &grid);

        /* Set the extents of the grid - each processor sets its grid
           boxes.  Each part has its own relative index space numbering. */

        part = 0;
        HYPRE_SStructGridSetExtents(grid, part, ll, ur);

        part = 1;
        HYPRE_SStructGridSetExtents(grid, part, ll, ur);

        /* Set cell centered data */
        HYPRE_SStructVariable vartypes[1] = {HYPRE_SSTRUCT_VARIABLE_CELL};

        part = 0;
        HYPRE_SStructGridSetVariables(grid, part, nvars, vartypes);

        part = 1;
        HYPRE_SStructGridSetVariables(grid, part, nvars, vartypes);

        /* Describe part connectivity */
        {
            int index_map[2] = {0,1};
            int index_dir[2] = {1,1};
            int nbor_part;

            /* connectivity described in terms of both part and neighbor
               coordinates */
            part = 0;
            nbor_part = 1;
            HYPRE_SStructGridSetNeighborPart(grid, part, lr_g, ur_g,
                                             nbor_part, ll, ul,
                                             index_map, index_dir);

            part = 1;
            nbor_part = 0;
            HYPRE_SStructGridSetNeighborPart(grid, part, ll_g, ul_g,
                                             nbor_part, lr, ur,
                                             index_map, index_dir);
        }

        /* Now the grid is ready to use */
        HYPRE_SStructGridAssemble(grid);
    }


    /* -------------------------------------------
       2. Define the discretization stencils
       ------------------------------------------- */
    {
        int entry;

        int offsets[5][2] = {{0,0}, {-1,0}, {1,0}, {0,-1}, {0,1}};
        int stencil_size = 5;

        HYPRE_SStructStencilCreate(ndim, stencil_size, &stencil_5pt);

        for (entry = 0; entry < stencil_size; entry++)
            HYPRE_SStructStencilSetEntry(stencil_5pt, entry, offsets[entry], var);

    }


    /* -------------------------------------------------------------
       3. Set up the graph.
       ------------------------------------------------------------- */
    {
        /* Create the graph object */
        HYPRE_SStructGraphCreate(MPI_COMM_WORLD, grid, &graph);

        /* See MatrixSetObjectType below */
        HYPRE_SStructGraphSetObjectType(graph, object_type);

        /* Use the 5-pt stencil on each part  */
        part = 0;
        HYPRE_SStructGraphSetStencil(graph, part, var, stencil_5pt);

        part = 1;
        HYPRE_SStructGraphSetStencil(graph, part, var, stencil_5pt);

        /* Assemble the graph */
        HYPRE_SStructGraphAssemble(graph);
    }

    /* -------------------------------------------------------------
       4. Set up a SStruct Matrix
       ------------------------------------------------------------- */
    {
        /* Create the empty matrix object */
        HYPRE_SStructMatrixCreate(MPI_COMM_WORLD, graph, &A);

        /* Set the object type (by default HYPRE_SSTRUCT). This determines the
           data structure used to store the matrix.  If you want to use unstructured
           solvers, e.g. BoomerAMG, the object type should be HYPRE_PARCSR.
           If the problem is purely structured (with one part), you may want to use
           HYPRE_STRUCT to access the structured solvers.  Since we have two parts
           with different stencils, we set the object type to HYPRE_SSTRUCT. */
        HYPRE_SStructMatrixSetObjectType(A, object_type);

        /* Get ready to set values */
        HYPRE_SStructMatrixInitialize(A);

        /* Set the matrix coefficients */
        {
            int nentries = 5;
            int nvalues  = nx*ny*nentries; /* nx*ny grid points */
            double *values = calloc(nvalues, sizeof(double));

            int stencil_indices[5];
            for (j = 0; j < nentries; j++)
            {
                stencil_indices[j] = j;
            }

            for (i = 0; i < nvalues; i += nentries)
            {
                values[i] = 4.0/h2;
                for (j = 1; j < nentries; j++)
                {
                    values[i+j] = -1.0/h2;
                }
            }

            part = 0;
            HYPRE_SStructMatrixSetBoxValues(A, part, ll, ur,
                                            var, nentries,
                                            stencil_indices, values);
            part = 1;
            HYPRE_SStructMatrixSetBoxValues(A, part, ll, ur,
                                            var, nentries,
                                            stencil_indices, values);

            free(values);
        }

        /* Set boundary stencils to use homogenous Neumann conditions. */
        {
            int nentries = 2;
            int maxnvalues = nentries*MAX(nx,ny);
            double *values = calloc(maxnvalues,sizeof(double));
            int stencil_indices[2];

            for (i = 0; i < maxnvalues; i += nentries)
            {
                values[i] = 3.0/h2;
                values[i+1] = 0;
            }

            /* Left edge - part 0 */
            part = 0;
            stencil_indices[0] = 0;
            stencil_indices[1] = 1;
            HYPRE_SStructMatrixSetBoxValues(A, part, ll, ul,
                                            var, nentries,
                                            stencil_indices, values);

            /* Right edge  - part 1 */
            part = 1;
            stencil_indices[0] = 0;
            stencil_indices[1] = 2;
            HYPRE_SStructMatrixSetBoxValues(A, part, lr, ur,
                                            var, nentries,
                                            stencil_indices, values);

            /* Bottom edge - part 0 */
            part = 0;
            stencil_indices[0] = 0;
            stencil_indices[1] = 3;
            HYPRE_SStructMatrixSetBoxValues(A, part, ll, lr,
                                            var, nentries,
                                            stencil_indices, values);

            /* Bottom edge - part 1 */
            part = 1;
            stencil_indices[0] = 0;
            stencil_indices[1] = 3;
            HYPRE_SStructMatrixSetBoxValues(A, part, ll, lr,
                                            var, nentries,
                                            stencil_indices, values);

            /* Top edge - part 0 */
            part = 0;
            stencil_indices[0] = 0;
            stencil_indices[1] = 4;
            HYPRE_SStructMatrixSetBoxValues(A, part, ul, ur,
                                            var, nentries,
                                            stencil_indices, values);

            /* Top edge - part 1 */
            part = 1;
            stencil_indices[0] = 0;
            stencil_indices[1] = 4;
            HYPRE_SStructMatrixSetBoxValues(A, part, ul, ur,
                                            var, nentries,
                                            stencil_indices, values);

            free(values);
        }

        /* Fix corner stencils */
        {
            int nentries = 3;                 /* Number of stencil weights to set */
            double values[3] = {2/h2,0,0};    /* setting only 1 stencil per call */

            int stencil_indices[3];
            stencil_indices[0] = 0;

            /* Lower left corner */
            part = 0;
            stencil_indices[1] = 1;
            stencil_indices[2] = 3;
            HYPRE_SStructMatrixSetBoxValues(A, part, ll, ll,
                                            var, nentries,
                                            stencil_indices, values);

            /* Lower right corner */
            part = 1;
            stencil_indices[1] = 2;
            stencil_indices[2] = 3;
            HYPRE_SStructMatrixSetBoxValues(A, part, lr, lr,
                                            var, nentries,
                                            stencil_indices, values);

            /* Upper left corner */
            part = 0;
            stencil_indices[1] = 1;
            stencil_indices[2] = 4;
            HYPRE_SStructMatrixSetBoxValues(A, part, ul, ul,
                                            var, nentries,
                                            stencil_indices, values);

            /* Upper right corner */
            part = 1;
            stencil_indices[1] = 2;
            stencil_indices[2] = 4;
            HYPRE_SStructMatrixSetBoxValues(A, part, ur, ur,
                                            var, nentries,
                                            stencil_indices, values);
        }

        /* Set one stencil to Dirichlet */
        {
            int nentries = 3;                 /* Number of stencil weights to set */
            double values[3] = {6/h2,0,0};    /* setting only 1 stencil per call */

            int stencil_indices[3];
            stencil_indices[0] = 0;
            stencil_indices[1] = 1;  /* Bottom edge */
            stencil_indices[2] = 3;  /* Bottom edge */

            /* Lower left corner set to Dirichlet */
            part = 0;
            HYPRE_SStructMatrixSetBoxValues(A, part, ll, ll,
                                            var, nentries,
                                            stencil_indices, values);
        }


        HYPRE_SStructMatrixAssemble(A);
        HYPRE_SStructMatrixPrint("matrix.ex3",A,0);

    }


    /* -------------------------------------------------------------
       5. Set up SStruct Vectors for b and x
       ------------------------------------------------------------- */
    {
        /* Create an empty vector object */
        HYPRE_SStructVectorCreate(MPI_COMM_WORLD, grid, &b);
        HYPRE_SStructVectorCreate(MPI_COMM_WORLD, grid, &x);

        /* As with the matrix,  set the object type for the vectors
           to be the sstruct type */
        HYPRE_SStructVectorSetObjectType(b, object_type);
        HYPRE_SStructVectorSetObjectType(x, object_type);

        /* Indicate that the vector coefficients are ready to be set */
        HYPRE_SStructVectorInitialize(b);
        HYPRE_SStructVectorInitialize(x);

        /* Set the vector coefficients over the gridpoints in my first box */
        int nvalues = nx*ny;  /* 6 grid points */
        double *values = calloc(nvalues,sizeof(double));

        {
            /* Set b (RHS) */
            for (i = 0; i < nvalues; i++)
                values[i] = 1.0;

            part = 0;
            HYPRE_SStructVectorSetBoxValues(b, part, ll, ur, var, values);

            part = 1;
            HYPRE_SStructVectorSetBoxValues(b, part, ll, ur, var, values);

            /* Set x (initial guess) */
            for (i = 0; i < nvalues; i++)
                values[i] = 0.0;

            part = 0;
            HYPRE_SStructVectorSetBoxValues(x, part, ll,ur, var, values);

            part = 1;
            HYPRE_SStructVectorSetBoxValues(x, part, ll,ur, var, values);
        }
        free(values);

        HYPRE_SStructVectorAssemble(b);
        HYPRE_SStructVectorAssemble(x);

    }


    /* -------------------------------------------------------------
       6. Set up the solver and solve problem
       ------------------------------------------------------------- */
    {
        double t0, t1;
        t0 = MPI_Wtime();
        if (solver_id == 0)
        {
            /* This seems to work */

            /* Create an empty PCG Struct solver */
            HYPRE_SStructPCGCreate(MPI_COMM_WORLD, &solver);

            /* Set PCG parameters */
            HYPRE_SStructPCGSetTol(solver, 1.0e-12 );
            HYPRE_SStructPCGSetPrintLevel(solver, solver_print_level);
            HYPRE_SStructPCGSetMaxIter(solver, 2000);

            /* Create a split SStruct solver for use as a preconditioner */
            HYPRE_SStructSplitCreate(MPI_COMM_WORLD, &precond);
            HYPRE_SStructSplitSetMaxIter(precond, 1);
            HYPRE_SStructSplitSetTol(precond, 0.0);
            HYPRE_SStructSplitSetZeroGuess(precond);

            /* Set the preconditioner type to split-SMG */
#if 1
            HYPRE_SStructSplitSetStructSolver(precond, HYPRE_SMG);

            /* Set preconditioner and solve */
            HYPRE_SStructPCGSetPrecond(solver, HYPRE_SStructSplitSolve,
                                       HYPRE_SStructSplitSetup, precond);
#endif

            HYPRE_SStructPCGSetup(solver, A, b, x);
            HYPRE_SStructPCGSolve(solver, A, b, x);

            HYPRE_SStructPCGDestroy(solver);
            HYPRE_SStructSplitDestroy(precond);
        }
        else if (solver_id == 1)
        {
            HYPRE_Solver solver;
            HYPRE_Solver precond;

            /* Set the PCG solvers parameters */
            HYPRE_ParCSRPCGCreate(MPI_COMM_WORLD, &solver);
            HYPRE_ParCSRPCGSetTol(solver, 1.0e-12);
            HYPRE_ParCSRPCGSetPrintLevel(solver, solver_print_level);
            HYPRE_ParCSRPCGSetMaxIter(solver, 500);

#if 0
            /* Not sure what these do */
            HYPRE_ParCSRPCGSetTwoNorm(solver, 1);
            HYPRE_ParCSRPCGSetLogging(solver, 3);
#endif

            /* Set the AMG preconditioner parameters */
            HYPRE_BoomerAMGCreate(&precond);
            HYPRE_BoomerAMGSetStrongThreshold(precond, .25);
            HYPRE_BoomerAMGSetCoarsenType(precond, 6);
            HYPRE_BoomerAMGSetTol(precond, 0.0);
            HYPRE_BoomerAMGSetMaxIter(precond, 1);
            HYPRE_BoomerAMGSetPrintLevel(precond, 0);

            /* Set the preconditioner */
            HYPRE_ParCSRPCGSetPrecond(solver,
                                      HYPRE_BoomerAMGSolve,
                                      HYPRE_BoomerAMGSetup,
                                      precond);

            /* Get matrix and vectors in the sparse matrix format */
            HYPRE_SStructMatrixGetObject(A, (void **) &parcsr_A);
            HYPRE_SStructVectorGetObject(b, (void **) &par_b);
            HYPRE_SStructVectorGetObject(x, (void **) &par_x);

            /* Solve the system */
            HYPRE_ParCSRPCGSetup(solver, parcsr_A, par_b, par_x);
            HYPRE_ParCSRPCGSolve(solver, parcsr_A, par_b, par_x);

            HYPRE_BoomerAMGDestroy(precond);
            HYPRE_ParCSRPCGDestroy(solver);
        }
        else if (solver_id == 2)
        {
            HYPRE_Solver solver;

            double final_res_norm;
            int num_iterations;

            HYPRE_BoomerAMGCreate(&solver);
            HYPRE_BoomerAMGSetStrongThreshold(solver, .15);
            HYPRE_BoomerAMGSetCoarsenType(solver, 6);
            HYPRE_BoomerAMGSetTol(solver, 1e-12);
            HYPRE_BoomerAMGSetMaxIter(solver, 500);
            HYPRE_BoomerAMGSetPrintLevel(solver, solver_print_level);

            HYPRE_BoomerAMGSetRelaxType(solver, 6);
            HYPRE_BoomerAMGSetNumSweeps(solver, 1);
            HYPRE_BoomerAMGSetMaxLevels(solver, 20);
            HYPRE_BoomerAMGSetInterpType(solver, 14);
            HYPRE_BoomerAMGSetAggInterpType(solver, 2);

            HYPRE_SStructMatrixGetObject(A, (void **) &parcsr_A);
            HYPRE_SStructVectorGetObject(b, (void **) &par_b);
            HYPRE_SStructVectorGetObject(x, (void **) &par_x);


            HYPRE_BoomerAMGSetup(solver, parcsr_A, par_b, par_x);
            HYPRE_BoomerAMGSolve(solver, parcsr_A, par_b, par_x);

            //Get Run Information
            HYPRE_BoomerAMGGetNumIterations(solver, &num_iterations);
            HYPRE_BoomerAMGGetFinalRelativeResidualNorm(solver, &final_res_norm);

            HYPRE_BoomerAMGDestroy(solver);
        }
        t1 = MPI_Wtime();
        printf("Elapsed time : %12.4e\n",t1-t0);

    }

    /* Save the solution for GLVis visualization, see vis/glvis-ex8.sh */
    if (vis)
    {
        /* Scale indices by h */
        double T[8] = {h,0,0,h,h,0,0,h};

        /* Shift second part by (1,0) */
        double O[4] = {0,0,nx*h,0};

        GLVis_PrintSStructGrid(grid, "vis/ex3.mesh", myid, T, O);
        if (solver_id == 0)
        {
            GLVis_PrintSStructVector(x, 0, "vis/ex3.sol", myid);
        }
        else
        {
            /* Needed if solution is a ParVector */
            FILE *file;
            char filename[255];

            int nvalues = nparts*nx*ny;
            double *values;

            /* get the local solution */
            values = hypre_VectorData(hypre_ParVectorLocalVector(par_x));

            sprintf(filename, "%s.%06d", "vis/ex3.sol", myid);
            if ((file = fopen(filename, "w")) == NULL)
            {
                printf("Error: can't open output file %s\n", filename);
                MPI_Finalize();
                exit(1);
            }

            /* From vis.c */
            fprintf(file, "FiniteElementSpace\n");
            fprintf(file, "FiniteElementCollection: Local_L2_2D_P0\n");  /* cell-centered */
            fprintf(file, "VDim: 1\n");
            fprintf(file, "Ordering: 0\n\n");

            /* save solution */
            for (i = 0; i < nvalues; i++)
                fprintf(file, "%.14e\n", values[i]);

            fflush(file);
            fclose(file);
        }


        GLVis_PrintData("vis/ex3.data", myid, num_procs);
    }

    /* Free memory */
    HYPRE_SStructGridDestroy(grid);
    HYPRE_SStructStencilDestroy(stencil_5pt);
    HYPRE_SStructGraphDestroy(graph);
    HYPRE_SStructMatrixDestroy(A);
    HYPRE_SStructVectorDestroy(b);
    HYPRE_SStructVectorDestroy(x);

    /* Finalize MPI */
    MPI_Finalize();

    return (0);
}
