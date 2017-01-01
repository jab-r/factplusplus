/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2003-2015 Dmitry Tsarkov and The University of Manchester
Copyright (C) 2015-2017 Dmitry Tsarkov

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef TONTOLOGYLOADER_H
#define TONTOLOGYLOADER_H

#include "tOntology.h"
#include "tExpressionTranslator.h"
#include "dlTBox.h"

class TOntologyLoader: public DLAxiomVisitor
{
protected:	// members
		/// KB to load the ontology
	TBox& kb;
		/// Transforms TDLExpression hierarchy to the DLTree*
	TExpressionTranslator ETrans;
		/// temporary vector for arguments of TBox n-ary axioms
	std::vector<DLTree*> ArgList;

protected:	// methods
		/// get DLTree corresponding to an expression EXPR
	DLTree* e ( const TDLExpression* expr )
	{
		expr->accept(ETrans);
		return ETrans;
	}
		/// get role by the DLTree; throw exception if unable
	TRole* getRole ( const TDLRoleExpression* r, const char* reason )
	{
		try
		{
			return resolveRole(TreeDeleter(e(r)));
		}
		catch ( const EFaCTPlusPlus& e ) { throw EFaCTPlusPlus(reason); }
	}
		/// get an individual be the DLTree; throw exception if unable
	TIndividual* getIndividual ( const TDLIndividualExpression* I, const char* reason )
	{
		try
		{
			TreeDeleter i = e(I);
			if ( i == NULL )
				throw EFaCTPlusPlus(reason);
			return static_cast<TIndividual*>(kb.getCI(i));
		}
		catch(...)
		{
			throw EFaCTPlusPlus(reason);
		}
	}
		/// ensure that the expression EXPR has its named entities linked to the KB ones
	void ensureNames ( const TDLExpression* Expr )
	{
		fpp_assert ( Expr != NULL );	// FORNOW
	}
		/// prepare arguments for the [begin,end) interval
	template<class Iterator>
	void prepareArgList ( Iterator begin, Iterator end )
	{
		ArgList.clear();
		for ( ; begin != end; ++begin )
		{
			ensureNames(*begin);
			ArgList.push_back(e(*begin));
		}
	}

public:		// visitor interface
	virtual void visit ( const TDLAxiomDeclaration& axiom )
	{
		ensureNames(axiom.getDeclaration());
		TreeDeleter(e(axiom.getDeclaration()));	// register names in the KB
	}

	// n-ary axioms

	virtual void visit ( const TDLAxiomEquivalentConcepts& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processEquivalentC(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( const TDLAxiomDisjointConcepts& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processDisjointC(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( const TDLAxiomDisjointUnion& axiom )
	{
		// first make a disjoint axiom
		prepareArgList(axiom.begin(),axiom.end());
		kb.processDisjointC(ArgList.begin(),ArgList.end());
		// now define C as a union-of axiom
		ArgList.clear();
		ensureNames(axiom.getC());
		ArgList.push_back(e(axiom.getC()));
		DLTree* acc = createBottom();
		for ( TDLAxiomDisjointUnion::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			acc = createSNFOr ( acc, e(*p) );
		ArgList.push_back(acc);
		kb.processEquivalentC(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( const TDLAxiomEquivalentORoles& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processEquivalentR(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( const TDLAxiomEquivalentDRoles& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processEquivalentR(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( const TDLAxiomDisjointORoles& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processDisjointR(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( const TDLAxiomDisjointDRoles& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processDisjointR(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( const TDLAxiomSameIndividuals& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processSame(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( const TDLAxiomDifferentIndividuals& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.processDifferent(ArgList.begin(),ArgList.end());
	}
	virtual void visit ( const TDLAxiomFairnessConstraint& axiom )
	{
		prepareArgList(axiom.begin(),axiom.end());
		kb.setFairnessConstraint(ArgList.begin(),ArgList.end());
	}

	// role axioms

	virtual void visit ( const TDLAxiomRoleInverse& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getInvRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role Inverse axiom" );
		TRole* iR = getRole ( axiom.getInvRole(), "Role expression expected in Role Inverse axiom" );
		kb.getRM(R)->addRoleSynonym ( iR->inverse(), R );
	}
	virtual void visit ( const TDLAxiomORoleSubsumption& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getSubRole());
		DLTree* Sub = e(axiom.getSubRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Object Roles Subsumption axiom" );
		kb.getRM(R)->addRoleParent ( Sub, R );
	}
	virtual void visit ( const TDLAxiomDRoleSubsumption& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getSubRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Data Roles Subsumption axiom" );
		TRole* S = getRole ( axiom.getSubRole(), "Role expression expected in Data Roles Subsumption axiom" );
		kb.getDRM()->addRoleParent ( S, R );
	}
	virtual void visit ( const TDLAxiomORoleDomain& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getDomain());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Object Role Domain axiom" );
		DLTree* C = e(axiom.getDomain());

		if ( unlikely(R->isTop()) )	// add GCI
			kb.addSubsumeAxiom ( createTop(), C );
		else if ( likely(!R->isBottom()) )	// nothing to do for bottom
			R->setDomain(C);
	}
	virtual void visit ( const TDLAxiomDRoleDomain& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getDomain());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Data Role Domain axiom" );
		DLTree* C = e(axiom.getDomain());

		if ( unlikely(R->isTop()) )	// add GCI
			kb.addSubsumeAxiom ( createTop(), C );
		else if ( likely(!R->isBottom()) )	// nothing to do for bottom
			R->setDomain(C);
	}
	virtual void visit ( const TDLAxiomORoleRange& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getRange());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Object Role Range axiom" );
		DLTree* C = e(axiom.getRange());

		if ( unlikely(R->isTop()) )	// add GCI
			kb.addSubsumeAxiom ( createTop(), C );
		else if ( likely(!R->isBottom()) )	// nothing to do for bottom
			R->setRange(C);
	}
	virtual void visit ( const TDLAxiomDRoleRange& axiom )
	{
		ensureNames(axiom.getRole());
		ensureNames(axiom.getRange());
		getRole ( axiom.getRole(), "Role expression expected in Data Role Range axiom" )->setRange(e(axiom.getRange()));
	}
	virtual void visit ( const TDLAxiomRoleTransitive& axiom )
	{
		ensureNames(axiom.getRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role Transitivity axiom" );
		if ( !R->isTop() && !R->isBottom() )	// top/bottom roles are always transitive
			R->setTransitive();
	}
	virtual void visit ( const TDLAxiomRoleReflexive& axiom )
	{
		ensureNames(axiom.getRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role Reflexivity axiom" );
		if ( R->isBottom() )	// empty role can't be reflexive
			throw EFPPInconsistentKB();
		if ( !R->isTop() )	// universal role always reflexive
			R->setReflexive(true);
	}
	virtual void visit ( const TDLAxiomRoleIrreflexive& axiom )
	{
		ensureNames(axiom.getRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role Irreflexivity axiom" );
		if ( R->isTop() )	// KB became inconsistent
			throw EFPPInconsistentKB();
		if ( !R->isBottom() )	// empty role already irreflexive
		{
			R->setDomain(createSNFNot(createSNFSelf(e(axiom.getRole()))));
			R->setIrreflexive(true);
		}
	}
	virtual void visit ( const TDLAxiomRoleSymmetric& axiom )
	{
		ensureNames(axiom.getRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role Symmetry axiom" );
		if ( !R->isTop() && !R->isBottom() )	// both are symmetric
		{
			R->setSymmetric(true);
			kb.getORM()->addRoleParent ( R, R->inverse() );
		}
	}
	virtual void visit ( const TDLAxiomRoleAsymmetric& axiom )
	{
		ensureNames(axiom.getRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role Asymmetry axiom" );
		if ( R->isTop() )	// KB became inconsistent
			throw EFPPInconsistentKB();
		if ( !R->isBottom() )
		{
			R->setAsymmetric(true);
			kb.getORM()->addDisjointRoles ( R, R->inverse() );
		}
	}
	virtual void visit ( const TDLAxiomORoleFunctional& axiom )
	{
		ensureNames(axiom.getRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Object Role Functionality axiom" );
		if ( R->isTop() )	// NOTE!! not really the case (1-point models are fine here), but OWL 2 forbid this
			throw EFPPInconsistentKB();
		if ( !R->isBottom() )
			R->setFunctional();
	}
	virtual void visit ( const TDLAxiomDRoleFunctional& axiom )
	{
		ensureNames(axiom.getRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Data Role Functionality axiom" );
		if ( R->isTop() )	// KB became inconsistent
			throw EFPPInconsistentKB();
		if ( !R->isBottom() )
			R->setFunctional();
	}
	virtual void visit ( const TDLAxiomRoleInverseFunctional& axiom )
	{
		ensureNames(axiom.getRole());
		TRole* R = getRole ( axiom.getRole(), "Role expression expected in Role Inverse Functionality axiom" );
		if ( R->isTop() )	// NOTE!! not really the case (1-point models are fine here), but OWL 2 forbid this
			throw EFPPInconsistentKB();
		if ( !R->isBottom() )
			R->inverse()->setFunctional();
	}

	// concept/individual axioms

	virtual void visit ( const TDLAxiomConceptInclusion& axiom )
	{
		ensureNames(axiom.getSubC());
		ensureNames(axiom.getSupC());
		DLTree* C = e(axiom.getSubC());
		DLTree* D = e(axiom.getSupC());
		kb.addSubsumeAxiom ( C, D );
	}
	virtual void visit ( const TDLAxiomInstanceOf& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getC());
		getIndividual ( axiom.getIndividual(), "Individual expected in Instance axiom" );
		DLTree* I = e(axiom.getIndividual());
		DLTree* C = e(axiom.getC());
		kb.addSubsumeAxiom ( I, C );
	}
	virtual void visit ( const TDLAxiomRelatedTo& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getRelation());
		ensureNames(axiom.getRelatedIndividual());
		TRole* R = getRole ( axiom.getRelation(), "Role expression expected in Related To axiom" );
		if ( R->isBottom() )
			throw EFPPInconsistentKB();
		if ( !R->isTop() )	// nothing to do for universal role
		{
			TIndividual* I = getIndividual ( axiom.getIndividual(), "Individual expected in Related To axiom" );
			TIndividual* J = getIndividual ( axiom.getRelatedIndividual(), "Individual expected in Related To axiom" );
			kb.RegisterIndividualRelation ( I, R, J );
		}
	}
	virtual void visit ( const TDLAxiomRelatedToNot& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getRelation());
		ensureNames(axiom.getRelatedIndividual());
		TRole* R = getRole ( axiom.getRelation(), "Role expression expected in Related To Not axiom" );
		if ( R->isTop() )	// inconsistent ontology
			throw EFPPInconsistentKB();
		// make sure everything is consistent
		getIndividual ( axiom.getIndividual(), "Individual expected in Related To Not axiom" ),
		getIndividual ( axiom.getRelatedIndividual(), "Individual expected in Related To Not axiom" );
		if ( !R->isBottom() )	// nothing to do
			kb.addSubsumeAxiom (	// make an axiom i:AR.\neg{j}
				e(axiom.getIndividual()),
				createSNFForall ( e(axiom.getRelation()), createSNFNot(e(axiom.getRelatedIndividual())) ) );
	}
	virtual void visit ( const TDLAxiomValueOf& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getAttribute());
		getIndividual ( axiom.getIndividual(), "Individual expected in Value Of axiom" );
		// FIXME!! think about ensuring the value
		TRole* R = getRole ( axiom.getAttribute(), "Role expression expected in Value Of axiom" );
		if ( R->isBottom() )
			throw EFPPInconsistentKB();
		if ( !R->isTop() )	// nothing to do for universal role
			kb.addSubsumeAxiom (	// make an axiom i:EA.V
				e(axiom.getIndividual()),
				createSNFExists ( e(axiom.getAttribute()), e(axiom.getValue())) );
	}
	virtual void visit ( const TDLAxiomValueOfNot& axiom )
	{
		ensureNames(axiom.getIndividual());
		ensureNames(axiom.getAttribute());
		getIndividual ( axiom.getIndividual(), "Individual expected in Value Of Not axiom" );
		// FIXME!! think about ensuring the value
		TRole* R = getRole ( axiom.getAttribute(), "Role expression expected in Value Of Not axiom" );
		if ( R->isTop() )
			throw EFPPInconsistentKB();
		if ( !R->isBottom() )	// nothing to do for empty role
			kb.addSubsumeAxiom (	// make an axiom i:AA.\neg V
				e(axiom.getIndividual()),
				createSNFForall ( e(axiom.getAttribute()), createSNFNot(e(axiom.getValue()))) );
	}

public:		// interface
		/// init c'tor
	TOntologyLoader ( TBox& KB ) : kb(KB), ETrans(KB) {}
		/// empty d'tor
	virtual ~TOntologyLoader ( void ) {}

		/// load ontology to a given KB
	void visitOntology ( TOntology& ontology )
	{
		ontology.visitOntology(*this);
		kb.finishLoading();
	}
}; // TOntologyLoader

#endif
