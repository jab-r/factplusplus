/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2011-2015 Dmitry Tsarkov and The University of Manchester
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

#ifndef GENERALSYNLOCCHECKER_H
#define GENERALSYNLOCCHECKER_H

#include "LocalityChecker.h"

/// syntactic locality checker for DL axioms
class GeneralSyntacticLocalityChecker: public LocalityChecker
{
protected:	// methods
		/// @return true iff EXPR is top equivalent
	virtual bool isTopEquivalent ( const TDLExpression* expr ) = 0;
		/// @return true iff EXPR is bottom equivalent
	virtual bool isBotEquivalent ( const TDLExpression* expr ) = 0;

protected:	// methods
		/// processing method for all Equivalent axioms; @return true if axiom is local
	template<class Entity>
	bool processEquivalentAxiom ( const TDLNAryExpression<Entity>& axiom )
	{
		// 1 element => local
		if ( axiom.size() <= 1 )
			return true;
		// axiom is local iff all the elements are either top- or bot-local
		typename TDLNAryExpression<Entity>::iterator p = axiom.begin(), p_end = axiom.end();
		if ( isBotEquivalent(*p) )
		{	// all should be \bot-eq
			while ( ++p != p_end )
				if ( !isBotEquivalent(*p) )
					return false;
		}
		else if ( isTopEquivalent(*p) )
		{	// all should be \top-eq
			while ( ++p != p_end )
				if ( !isTopEquivalent(*p) )
					return false;
		}
		else	// neither \bot- no \top-eq: non-local
			return false;

		// all elements have the same locality
		return true;
	}
		/// processing method for all Disjoint axioms; @return true if axiom is local
	template<class Entity>
	bool processDisjointAxiom ( const TDLNAryExpression<Entity>& axiom )
	{
		// local iff at most 1 element is not bot-equiv
		bool hasNBE = false;
		for ( typename TDLNAryExpression<Entity>::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )
			{
				if ( hasNBE )	// already seen one non-bot-eq element
					return false;		// non-local
				else	// record that 1 non-bot-eq element was found
					hasNBE = true;
			}
		return true;
	}

public:		// interface
		/// init c'tor
	GeneralSyntacticLocalityChecker ( const TSignature* s ) : LocalityChecker(s) {}
		/// empty d'tor
	virtual ~GeneralSyntacticLocalityChecker ( void ) {}

public:		// visitor interface
	virtual void visit ( const TDLAxiomDeclaration& ) { isLocal = true; }

	virtual void visit ( const TDLAxiomEquivalentConcepts& axiom ) { isLocal = processEquivalentAxiom(axiom); }
	virtual void visit ( const TDLAxiomDisjointConcepts& axiom ) { isLocal = processDisjointAxiom(axiom); }
	virtual void visit ( const TDLAxiomDisjointUnion& axiom )
	{
		// DisjointUnion(A, C1,..., Cn) is local if
		//    (1) A and all of Ci are bot-equivalent,
		// or (2) A and one Ci are top-equivalent and the remaining Cj are bot-equivalent
		isLocal = false;
		bool lhsIsTopEq;
		if ( isTopEquivalent(axiom.getC()) )
			lhsIsTopEq = true;	// need to check (2)
		else if ( isBotEquivalent(axiom.getC()) )
			lhsIsTopEq = false;	// need to check (1)
		else
			return;				// neither (1) nor (2)

		bool topEqDesc = false;
		for ( TDLAxiomDisjointUnion::iterator p = axiom.begin(), p_end = axiom.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )
			{
				if ( lhsIsTopEq && isTopEquivalent(*p) )
				{
					if ( topEqDesc )
						return;	// 2nd top in there -- violate (2) -- non-local
					else
						topEqDesc = true;
				}
				else	// either (1) or fail to have a top-eq for (2)
					return;
			}

		// check whether for (2) we found a top-eq concept
		if ( lhsIsTopEq && !topEqDesc )
			return;

		// it is local in the end!
		isLocal = true;
	}
	virtual void visit ( const TDLAxiomEquivalentORoles& axiom ) { isLocal = processEquivalentAxiom(axiom); }
	virtual void visit ( const TDLAxiomEquivalentDRoles& axiom ) { isLocal = processEquivalentAxiom(axiom); }
	virtual void visit ( const TDLAxiomDisjointORoles& axiom ) { isLocal = processDisjointAxiom(axiom); }
	virtual void visit ( const TDLAxiomDisjointDRoles& axiom ) { isLocal = processDisjointAxiom(axiom); }
	virtual void visit ( const TDLAxiomSameIndividuals& ) { isLocal = false; }
	virtual void visit ( const TDLAxiomDifferentIndividuals& ) { isLocal = false; }
		/// FaCT++ extension: there is no such axiom in OWL API, but I hope nobody would use Fairness here
	virtual void visit ( const TDLAxiomFairnessConstraint& ) { isLocal = true; }

	virtual void visit ( const TDLAxiomRoleInverse& axiom )
	{
		isLocal = ( isBotEquivalent(axiom.getRole()) && isBotEquivalent(axiom.getInvRole()) ) ||
				  ( isTopEquivalent(axiom.getRole()) && isTopEquivalent(axiom.getInvRole()) );
	}
	virtual void visit ( const TDLAxiomORoleSubsumption& axiom ) { isLocal = isTopEquivalent(axiom.getRole()) || isBotEquivalent(axiom.getSubRole()); }
	virtual void visit ( const TDLAxiomDRoleSubsumption& axiom ) { isLocal = isTopEquivalent(axiom.getRole()) || isBotEquivalent(axiom.getSubRole()); }
	virtual void visit ( const TDLAxiomORoleDomain& axiom )
		{ isLocal = isTopEquivalent(axiom.getDomain()) || isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomDRoleDomain& axiom )
		{ isLocal = isTopEquivalent(axiom.getDomain()) || isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomORoleRange& axiom )
		{ isLocal = isTopEquivalent(axiom.getRange()) || isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomDRoleRange& axiom )
		{ isLocal = isTopEquivalent(axiom.getRange()) || isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleTransitive& axiom ) { isLocal = isBotEquivalent(axiom.getRole()) || isTopEquivalent(axiom.getRole()); }
		/// as BotRole is irreflexive, the only local axiom is topEquivalent(R)
	virtual void visit ( const TDLAxiomRoleReflexive& axiom ) { isLocal = isTopEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleIrreflexive& axiom ) { isLocal = isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleSymmetric& axiom ) { isLocal = isBotEquivalent(axiom.getRole()) || isTopEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleAsymmetric& ) { isLocal = false; }
	virtual void visit ( const TDLAxiomORoleFunctional& axiom ) { isLocal = isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomDRoleFunctional& axiom ) { isLocal = isBotEquivalent(axiom.getRole()); }
	virtual void visit ( const TDLAxiomRoleInverseFunctional& axiom ) { isLocal = isBotEquivalent(axiom.getRole()); }

	virtual void visit ( const TDLAxiomConceptInclusion& axiom ) { isLocal = isBotEquivalent(axiom.getSubC()) || isTopEquivalent(axiom.getSupC()); }
	virtual void visit ( const TDLAxiomInstanceOf& axiom ) { isLocal = isTopEquivalent(axiom.getC()); }
	virtual void visit ( const TDLAxiomRelatedTo& axiom ) { isLocal = isTopEquivalent(axiom.getRelation()); }
	virtual void visit ( const TDLAxiomRelatedToNot& axiom ) { isLocal = isBotEquivalent(axiom.getRelation()); }
	virtual void visit ( const TDLAxiomValueOf& axiom ) { isLocal = isTopEquivalent(axiom.getAttribute()); }
	virtual void visit ( const TDLAxiomValueOfNot& axiom ) { isLocal = isBotEquivalent(axiom.getAttribute()); }
}; // GeneralSyntacticLocalityChecker

#endif
