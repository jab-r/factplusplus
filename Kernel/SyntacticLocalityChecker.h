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

#ifndef SYNLOCCHECKER_H
#define SYNLOCCHECKER_H

#include "GeneralSyntacticLocalityChecker.h"

// forward declarations
class BotEquivalenceEvaluator;
class TopEquivalenceEvaluator;

/// check whether class expressions are equivalent to bottom wrt given locality class
class BotEquivalenceEvaluator: protected SigAccessor, public DLExpressionVisitorEmpty
{
protected:	// members
		/// corresponding top evaluator
	TopEquivalenceEvaluator* TopEval;
		/// keep the value here
	bool isBotEq;

protected:	// methods
		/// check whether the expression is top-equivalent
	bool isTopEquivalent ( const TDLExpression& expr );
		/// convenience helper
	bool isTopEquivalent ( const TDLExpression* expr ) { return isTopEquivalent(*expr); }

	// non-empty Concept/Data expression

		/// @return true iff C^I is non-empty
	bool isBotDistinct ( const TDLExpression* C )
	{
		// TOP is non-empty
		if ( isTopEquivalent(C) )
			return true;
		// built-in DT are non-empty
		if ( dynamic_cast<const TDLDataTypeName*>(C) )
			return true;
		// FIXME!! that's it for now
		return false;
	}

	// cardinality of a concept/data expression interpretation

		/// @return true if #C^I > n
	bool isCardLargerThan ( const TDLExpression* C, unsigned int n )
	{
		if ( n == 0 )	// non-empty is enough
			return isBotDistinct(C);
		// data top is infinite
		if ( dynamic_cast<const TDLDataExpression*>(C) && isTopEquivalent(C) )
			return true;
		if ( const TDLDataTypeName* namedDT = dynamic_cast<const TDLDataTypeName*>(C) )
		{	// string/time are infinite DT
			std::string name = namedDT->getName();
			if ( name == TDataTypeManager::getStrTypeName() || name == TDataTypeManager::getTimeTypeName() )
				return true;
		}
		// FIXME!! try to be more precise
		return false;
	}

	// QCRs

		/// @return true iff (>= n R.C) is botEq
	bool isMinBotEquivalent ( unsigned int n, const TDLRoleExpression* R, const TDLExpression* C )
		{ return (n > 0) && (isBotEquivalent(R) || isBotEquivalent(C)); }
		/// @return true iff (<= n R.C) is botEq
	bool isMaxBotEquivalent ( unsigned int n, const TDLRoleExpression* R, const TDLExpression* C )
		{ return isTopEquivalent(R) && isCardLargerThan ( C, n ); }

public:		// interface
		/// init c'tor
	BotEquivalenceEvaluator ( const TSignature* s ) : SigAccessor(s), isBotEq(false) {}
		/// empty d'tor
	virtual ~BotEquivalenceEvaluator ( void ) {}

	// set fields

		/// set the corresponding top evaluator
	void setTopEval ( TopEquivalenceEvaluator* eval ) { TopEval = eval; }
		/// @return true iff an EXPRession is equivalent to bottom wrt defined policy
	bool isBotEquivalent ( const TDLExpression& expr )
	{
		expr.accept(*this);
		return isBotEq;
	}
		/// @return true iff an EXPRession is equivalent to bottom wrt defined policy
	bool isBotEquivalent ( const TDLExpression* expr ) { return isBotEquivalent(*expr); }

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { isBotEq = false; }
	virtual void visit ( const TDLConceptBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLConceptName& expr ) { isBotEq = !topCLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLConceptNot& expr ) { isBotEq = isTopEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr )
	{
		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )	// here isBotEq is true, so just return
				return;
		isBotEq = false;
	}
	virtual void visit ( const TDLConceptOr& expr )
	{
		for ( TDLConceptOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )	// here isBotEq is false, so just return
				return;
		isBotEq = true;
	}
	virtual void visit ( const TDLConceptOneOf& expr ) { isBotEq = expr.empty(); }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { isBotEq = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { isBotEq = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectForall& expr )
		{ isBotEq = isTopEquivalent(expr.getOR()) && isBotEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ isBotEq = isMinBotEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ isBotEq = isMaxBotEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLObjectRoleExpression* R = expr.getOR();
		const TDLConceptExpression* C = expr.getC();
		isBotEq = isMinBotEquivalent ( n, R, C ) || isMaxBotEquivalent ( n, R, C );
	}
	virtual void visit ( const TDLConceptDataValue& expr )
		{ isBotEq = isBotEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExists& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataForall& expr )
		{ isBotEq = isTopEquivalent(expr.getDR()) && !isTopEquivalent(expr.getExpr()); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
		{ isBotEq = isMinBotEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
		{ isBotEq = isMaxBotEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLDataRoleExpression* R = expr.getDR();
		const TDLDataExpression* D = expr.getExpr();
		isBotEq = isMinBotEquivalent ( n, R, D ) || isMaxBotEquivalent ( n, R, D );
	}

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { isBotEq = false; }
	virtual void visit ( const TDLObjectRoleBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLObjectRoleName& expr ) { isBotEq = !topRLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { isBotEq = isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		isBotEq = true;
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )	// isBotEq is true here
				return;
		isBotEq = false;
	}
		// FaCT++ extension: equivalent to R(x,y) and C(x), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getOR(), expr.getC() ); }
		// FaCT++ extension: equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ isBotEq = isMinBotEquivalent ( 1, expr.getOR(), expr.getC() ); }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { isBotEq = false; }
	virtual void visit ( const TDLDataRoleBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLDataRoleName& expr ) { isBotEq = !topRLocal() && nc(expr.getEntity()); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { isBotEq = false; }
	virtual void visit ( const TDLDataBottom& ) { isBotEq = true; }
	virtual void visit ( const TDLDataTypeName& ) { isBotEq = false; }
	virtual void visit ( const TDLDataTypeRestriction& ) { isBotEq = false; }
	virtual void visit ( const TDLDataValue& ) { isBotEq = false; }
	virtual void visit ( const TDLDataNot& expr ) { isBotEq = isTopEquivalent(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr )
	{
		for ( TDLDataAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isBotEquivalent(*p) )	// here isBotEq is true, so just return
				return;
		isBotEq = false;
	}
	virtual void visit ( const TDLDataOr& expr )
	{
		for ( TDLDataOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isBotEquivalent(*p) )	// here isBotEq is false, so just return
				return;
		isBotEq = true;
	}
	virtual void visit ( const TDLDataOneOf& expr ) { isBotEq = expr.empty(); }
}; // BotEquivalenceEvaluator

/// check whether class expressions are equivalent to top wrt given locality class
class TopEquivalenceEvaluator: protected SigAccessor, public DLExpressionVisitorEmpty
{
protected:	// members
		/// corresponding bottom evaluator
	BotEquivalenceEvaluator* BotEval;
		/// keep the value here
	bool isTopEq;

protected:	// methods
		/// check whether the expression is top-equivalent
	bool isBotEquivalent ( const TDLExpression& expr ) { return BotEval->isBotEquivalent(expr); }
		/// convenience helper
	bool isBotEquivalent ( const TDLExpression* expr ) { return isBotEquivalent(*expr); }

	// non-empty Concept/Data expression

		/// @return true iff C^I is non-empty
	bool isBotDistinct ( const TDLExpression* C )
	{
		// TOP is non-empty
		if ( isTopEquivalent(C) )
			return true;
		// built-in DT are non-empty
		if ( dynamic_cast<const TDLDataTypeName*>(C) )
			return true;
		// FIXME!! that's it for now
		return false;
	}

	// cardinality of a concept/data expression interpretation

		/// @return true if #C^I > n
	bool isCardLargerThan ( const TDLExpression* C, unsigned int n )
	{
		if ( n == 0 )	// non-empty is enough
			return isBotDistinct(C);
		// data top is infinite
		if ( dynamic_cast<const TDLDataExpression*>(C) && isTopEquivalent(C) )
			return true;
		if ( const TDLDataTypeName* namedDT = dynamic_cast<const TDLDataTypeName*>(C) )
		{	// string/time are infinite DT
			std::string name = namedDT->getName();
			if ( name == TDataTypeManager::getStrTypeName() || name == TDataTypeManager::getTimeTypeName() )
				return true;
		}
		// FIXME!! try to be more precise
		return false;
	}

	// QCRs

		/// @return true iff (>= n R.C) is topEq
	bool isMinTopEquivalent ( unsigned int n, const TDLRoleExpression* R, const TDLExpression* C )
		{ return (n == 0) || ( isTopEquivalent(R) && isCardLargerThan ( C, n-1 ) ); }
		/// @return true iff (<= n R.C) is topEq
	bool isMaxTopEquivalent ( unsigned int, const TDLRoleExpression* R, const TDLExpression* C )
		{ return isBotEquivalent(R) || isBotEquivalent(C); }

public:		// interface
		/// init c'tor
	TopEquivalenceEvaluator ( const TSignature* s ) : SigAccessor(s), isTopEq(false) {}
		/// empty d'tor
	virtual ~TopEquivalenceEvaluator ( void ) {}

	// set fields

		/// set the corresponding bottom evaluator
	void setBotEval ( BotEquivalenceEvaluator* eval ) { BotEval = eval; }
		/// @return true iff an EXPRession is equivalent to top wrt defined policy
	bool isTopEquivalent ( const TDLExpression& expr )
	{
		expr.accept(*this);
		return isTopEq;
	}
		/// @return true iff an EXPRession is equivalent to top wrt defined policy
	bool isTopEquivalent ( const TDLExpression* expr ) { return isTopEquivalent(*expr); }

public:		// visitor interface
	// concept expressions
	virtual void visit ( const TDLConceptTop& ) { isTopEq = true; }
	virtual void visit ( const TDLConceptBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLConceptName& expr ) { isTopEq = topCLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLConceptNot& expr ) { isTopEq = isBotEquivalent(expr.getC()); }
	virtual void visit ( const TDLConceptAnd& expr )
	{
		for ( TDLConceptAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )	// here isTopEq is false, so just return
				return;
		isTopEq = true;
	}
	virtual void visit ( const TDLConceptOr& expr )
	{
		for ( TDLConceptOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isTopEquivalent(*p) )	// here isTopEq is true, so just return
				return;
		isTopEq = false;
	}
	virtual void visit ( const TDLConceptOneOf& ) { isTopEq = false; }
	virtual void visit ( const TDLConceptObjectSelf& expr ) { isTopEq = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectValue& expr ) { isTopEq = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectExists& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectForall& expr )
		{ isTopEq = isTopEquivalent(expr.getC()) || isBotEquivalent(expr.getOR()); }
	virtual void visit ( const TDLConceptObjectMinCardinality& expr )
		{ isTopEq = isMinTopEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectMaxCardinality& expr )
		{ isTopEq = isMaxTopEquivalent ( expr.getNumber(), expr.getOR(), expr.getC() ); }
	virtual void visit ( const TDLConceptObjectExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLObjectRoleExpression* R = expr.getOR();
		const TDLConceptExpression* C = expr.getC();
		isTopEq = isMinTopEquivalent ( n, R, C ) && isMaxTopEquivalent ( n, R, C );
	}
	virtual void visit ( const TDLConceptDataValue& expr )
		{ isTopEq = isTopEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataExists& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataForall& expr ) { isTopEq = isTopEquivalent(expr.getExpr()) || isBotEquivalent(expr.getDR()); }
	virtual void visit ( const TDLConceptDataMinCardinality& expr )
		{ isTopEq = isMinTopEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataMaxCardinality& expr )
		{ isTopEq = isMaxTopEquivalent ( expr.getNumber(), expr.getDR(), expr.getExpr() ); }
	virtual void visit ( const TDLConceptDataExactCardinality& expr )
	{
		unsigned int n = expr.getNumber();
		const TDLDataRoleExpression* R = expr.getDR();
		const TDLDataExpression* D = expr.getExpr();
		isTopEq = isMinTopEquivalent ( n, R, D ) && isMaxTopEquivalent ( n, R, D );
	}

	// object role expressions
	virtual void visit ( const TDLObjectRoleTop& ) { isTopEq = true; }
	virtual void visit ( const TDLObjectRoleBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLObjectRoleName& expr ) { isTopEq = topRLocal() && nc(expr.getEntity()); }
	virtual void visit ( const TDLObjectRoleInverse& expr ) { isTopEq = isTopEquivalent(expr.getOR()); }
	virtual void visit ( const TDLObjectRoleChain& expr )
	{
		isTopEq = false;
		for ( TDLObjectRoleChain::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )	// isTopEq is false here
				return;
		isTopEq = true;
	}
		// FaCT++ extension: equivalent to R(x,y) and C(x), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionFrom& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getOR(), expr.getC() ); }
		// FaCT++ extension: equivalent to R(x,y) and C(y), so copy behaviour from ER.X
	virtual void visit ( const TDLObjectRoleProjectionInto& expr )
		{ isTopEq = isMinTopEquivalent ( 1, expr.getOR(), expr.getC() ); }

	// data role expressions
	virtual void visit ( const TDLDataRoleTop& ) { isTopEq = true; }
	virtual void visit ( const TDLDataRoleBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLDataRoleName& expr ) { isTopEq = topRLocal() && nc(expr.getEntity()); }

	// data expressions
	virtual void visit ( const TDLDataTop& ) { isTopEq = true; }
	virtual void visit ( const TDLDataBottom& ) { isTopEq = false; }
	virtual void visit ( const TDLDataTypeName& ) { isTopEq = false; }
	virtual void visit ( const TDLDataTypeRestriction& ) { isTopEq = false; }
	virtual void visit ( const TDLDataValue& ) { isTopEq = false; }
	virtual void visit ( const TDLDataNot& expr ) { isTopEq = isBotEquivalent(expr.getExpr()); }
	virtual void visit ( const TDLDataAnd& expr )
	{
		for ( TDLDataAnd::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( !isTopEquivalent(*p) )	// here isTopEq is false, so just return
				return;
		isTopEq = true;
	}
	virtual void visit ( const TDLDataOr& expr )
	{
		for ( TDLDataOr::iterator p = expr.begin(), p_end = expr.end(); p != p_end; ++p )
			if ( isTopEquivalent(*p) )	// here isTopEq is true, so just return
				return;
		isTopEq = false;
	}
	virtual void visit ( const TDLDataOneOf& ) { isTopEq = false; }
}; // TopEquivalenceEvaluator

inline bool
BotEquivalenceEvaluator :: isTopEquivalent ( const TDLExpression& expr )
{
	return TopEval->isTopEquivalent(expr);
}


/// syntactic locality checker for DL axioms
class SyntacticLocalityChecker: public GeneralSyntacticLocalityChecker
{
protected:	// members
		/// top evaluator
	TopEquivalenceEvaluator TopEval;
		/// bottom evaluator
	BotEquivalenceEvaluator BotEval;

protected:	// methods
		/// @return true iff EXPR is top equivalent
	virtual bool isTopEquivalent ( const TDLExpression* expr ) { return TopEval.isTopEquivalent(*expr); }
		/// @return true iff EXPR is bottom equivalent
	virtual bool isBotEquivalent ( const TDLExpression* expr ) { return BotEval.isBotEquivalent(*expr); }

public:		// interface
		/// init c'tor
	SyntacticLocalityChecker ( const TSignature* s )
		: GeneralSyntacticLocalityChecker(s)
		, TopEval(s)
		, BotEval(s)
	{
		TopEval.setBotEval(&BotEval);
		BotEval.setTopEval(&TopEval);
	}
		/// empty d'tor
	virtual ~SyntacticLocalityChecker ( void ) {}
}; // SyntacticLocalityChecker

#endif
