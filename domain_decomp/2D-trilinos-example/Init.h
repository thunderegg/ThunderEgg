#include "DomainCollection.h"
#include <functional>
class Init
{
	public:
	static void initNeumann(DomainCollection &dc, int n, double *f_vec, double *exact_vec,
	                        std::function<double(double, double)> ffun,
	                        std::function<double(double, double)> efun,
	                        std::function<double(double, double)> nfunx,
	                        std::function<double(double, double)> nfuny);
	static void initDirichlet(DomainCollection &dc, int n, double *f_vec, double *exact_vec,
	                          std::function<double(double, double)> ffun,
	                          std::function<double(double, double)> efun);
};