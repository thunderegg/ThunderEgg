#ifndef OPATA_H
#define OPATA_H
#include "DomainCollection.h"
#include "MyTypeDefs.h"
#include "RBMatrix.h"
#include <BelosLinearProblem.hpp>
#include <BelosTpetraAdapter.hpp>
#include <Teuchos_RCP.hpp>
class OpATA : public Tpetra::Operator<scalar_type>
{
	public:
	Teuchos::RCP<RBMatrix> A;
	OpATA(Teuchos::RCP<RBMatrix> A)
	{
		this->A  = A;
	}
	void apply(const vector_type &x, vector_type &y, Teuchos::ETransp mode = Teuchos::NO_TRANS,
	           scalar_type alpha = Teuchos::ScalarTraits<scalar_type>::one(),
	           scalar_type beta  = Teuchos::ScalarTraits<scalar_type>::zero()) const
	{
		vector_type x2(x.getMap(), 1);
        A->apply(x,x2);
        A->apply(x2,y);
	}
	Teuchos::RCP<const map_type> getDomainMap() const { return A->getDomainMap(); }
	Teuchos::RCP<const map_type> getRangeMap() const { return A->getRangeMap(); }
};
#endif