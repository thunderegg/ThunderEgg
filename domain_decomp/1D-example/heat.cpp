#include <Eigen/Dense>
#include <Eigen/LU>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <valarray>
#include <vector>

#define PI M_PI

using namespace std;
using namespace Eigen;
#include "Domain.h"
#include "TriDiagSolver.h"

// lapack function signatures
extern "C" void dgesv_(int *n, int *nrhs, double *a, int *lda, int *ipiv, double *b, int *ldb,
                       int *info);
extern "C" void dgecon_(char *norm, int *n, double *A, int *lda, double *Anorm, double *rcond,
                        double *work, int *iwork, int *info);

// double uxx_init(double x) { return -PI * PI * sin(PI * x); }
// double exact_solution(double x) { return sin(PI * x); }
double uxx_init(double x) { return -0.5 + abs(x - 0.5); }
double exact_solution(double x)
{
	return 1.0 / 24.0 - (x - 0.5) * (x - 0.5) / 4.0
	       + abs(x - 0.5) * abs(x - 0.5) * abs(x - 0.5) / 6.0;
}
double error(vector<Domain> &dmns)
{
	double l2norm     = 0;
	double exact_norm = 0;
	for (Domain &d : dmns) {
		int    m       = d.size();
		double d_begin = d.domainBegin();
		double d_end   = d.domainEnd();
		for (int i = 0; i < m; i++) {
			double x     = d_begin + (i + 0.5) / m * (d_end - d_begin);
			double exact = exact_solution(x);
			double diff  = exact - d.u_curr[i];
			l2norm += diff * diff;
			exact_norm += exact * exact;
		}
	}
	return sqrt(l2norm) / sqrt(exact_norm);
}

/**
 * @brief solve over each of the domains
 *
 * @param tds the solver to use
 * @param dmns the domains to over
 *
 * @return the difference between the gamma values and the computed value at the domain
 */
VectorXd solveOnAllDomains(TriDiagSolver &tds, vector<Domain> &dmns)
{
	// solve over the domains
	for (Domain &d : dmns) {
		tds.solve(d);
	}

	// get the difference between the gamma value and computed solution at the interface
	VectorXd z(dmns.size() - 1);
	for (int i = 0; i < z.size(); i++) {
		Domain &left_dmn  = dmns[i];
		Domain &right_dmn = dmns[i + 1];
		double  gamma     = left_dmn.rightGamma();

		double left_val  = left_dmn.u_curr[left_dmn.u_curr.size() - 1];
		double right_val = right_dmn.u_curr[0];

		z[i] = left_val + right_val - 2 * gamma;
	}
	return z;
}

void printSolution(vector<Domain> &dmns, ostream &os)
{
	for (Domain &d : dmns) {
		for (double x : d.u_curr) {
			os << x << "\t";
		}
	}
	os << '\n';
}

char *getCmdOption(char **begin, char **end, const std::string &option)
{
	char **itr = std::find(begin, end, option);
	if (itr != end && ++itr != end) {
		return *itr;
	}
	return 0;
}

bool cmdOptionExists(char **begin, char **end, const string &option)
{
	return find(begin, end, option) != end;
}

void printHelp()
{
	cout << "Usage:\n";
	cout << "./heat <num_cells> <numdomains> [options]\n\n";
	cout << "Options can be:\n";
	cout << "\t -s <file> \t save solution to file\n";
	cout << "\t -m \t print the matrix that was formed.\n";
	cout << "\t -h \t print this help message\n";
}

int main(int argc, char *argv[])
{
	string save_solution_file = "";
	bool   print_matrix       = false;
	if (argc < 2 || cmdOptionExists(argv, argv + argc, "-h")) {
		printHelp();
		return 1;
	}
	if (cmdOptionExists(argv, argv + argc, "-s")) {
		save_solution_file = getCmdOption(argv, argv + argc, "-s");
	}
	if (cmdOptionExists(argv, argv + argc, "-m")) {
		print_matrix = true;
	}

	// set cout to print full precision
	// cout.precision(numeric_limits<double>::max_digits10);
	cout.precision(9);

	// create a solver with 0 for the boundary conditions
	TriDiagSolver  tds(0.0, 0.0);
	int            m           = stoi(argv[1]);
	int            num_domains = stoi(argv[2]);
	vector<Domain> dmns(num_domains);

	// create the domains
	for (int i = 0; i < num_domains; i++) {
		double x_start = (0.0 + i) / num_domains;
		double x_end   = (1.0 + i) / num_domains;
		dmns[i]        = Domain(x_start, x_end, m / num_domains, uxx_init);
	}

	// create an array to store the gamma values for each of the interfaces
	VectorXd gammas(num_domains - 1);

	// set the gamma pointers
	if (num_domains > 1) {
		const int last_i        = num_domains - 1;
		dmns[0].right_gamma_ptr = &gammas[0];
		for (int i = 1; i < last_i; i++) {
			dmns[i].left_gamma_ptr  = &gammas[i - 1];
			dmns[i].right_gamma_ptr = &gammas[i];
		}
		dmns[last_i].left_gamma_ptr = &gammas[last_i - 1];
	}

	double condition_number;
	if (num_domains > 1) {
		// solve with gammas set to zero
        gammas = VectorXd::Zero(gammas.size());
		VectorXd b = solveOnAllDomains(tds, dmns);

		if (print_matrix) {
			cout << "b value(s):\n";
			cout << b;
			cout << "\n\n";
		}

		// build the A matrix
		int      n = gammas.size();
		MatrixXd A(n, n);
		for (int i = 0; i < n; i++) {
			gammas[i] = 1.0;
			A.col(i)  = solveOnAllDomains(tds, dmns) - b;
			gammas[i] = 0.0;
		}

		if (print_matrix) {
			cout << "The Schur complement matrix:\n";
			cout << A;
			cout << "\n\n";
		}

		// solve for the gamma values
		FullPivLU<MatrixXd> lu(A);
		VectorXd            tmp = lu.solve(b);
		gammas                  = -tmp;
		condition_number        = 1.0 / lu.rcond();

		if (print_matrix) {
			cout << "calculated gamma value(s):\n";
			cout << gammas;
			cout << "\n\n";
		}
	}

	/*
	 * get the final solution
	 */
	solveOnAllDomains(tds, dmns);

	if (save_solution_file != "") {
		// print out solution
		ofstream out_file(save_solution_file);
		printSolution(dmns, out_file);
		out_file.close();
	}

	cout << "error: " << scientific << error(dmns) << "\n" << defaultfloat;
	if (num_domains > 1) {
		cout << "condition number of A: " << condition_number << "\n";
	}
}