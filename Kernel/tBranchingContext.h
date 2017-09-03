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

#ifndef TBRANCHINGCONTEXT_H
#define TBRANCHINGCONTEXT_H

#include <iostream>

#include "ConceptWithDep.h"

class DlCompletionTree;

	/// class for saving branching context of a Reasoner
class BranchingContext
{
public:		// members
		/// currently processed node
	DlCompletionTree* curNode;
		/// currently processed concept
	ConceptWDep curConcept;
		/// positions of the Used members
	size_t pUsedIndex, nUsedIndex;
		/// size of a session GCIs vector
	size_t SGsize;
		/// dependencies for branching clashes
	DepSet branchDep;

public:		// interface
		/// empty c'tor
	BranchingContext ( void )
		: curNode{nullptr}
		, curConcept{bpINVALID}
		, pUsedIndex{0}
		, nUsedIndex{0}
		, SGsize{0}
		, branchDep{}
		{}
		/// no copy c'tor
	BranchingContext ( const BranchingContext& ) = delete;
		/// no assignment
	BranchingContext& operator = ( const BranchingContext& ) = delete;
		/// empty d'tor
	virtual ~BranchingContext() = default;

		/// init indices (if necessary)
	virtual void init ( void ) {}
		/// give the next branching alternative
	virtual void nextOption ( void ) {}
}; // BranchingContext

		/// branching context for the OR operations
class BCOr: public BranchingContext
{
public:		// types
		/// single OR argument
	class OrArg
	{
	public:	// struct for now
			/// argument itself
		BipolarPointer C = bpINVALID;
			/// argument negation
		BipolarPointer NotC = bpINVALID;
			/// dep-set representing clash reason
		DepSet clashReason;
			/// true iff this one is currently chosen
		bool chosen = false;
			/// true iff currently available
		bool free = true;
			/// true iff was chosen before and ruled out now
		bool tried = false;
	public:		// interface
			/// mark argument as tried
		void setTried ( const DepSet& ds )
		{
			fpp_assert(!tried);
			chosen = false;
			free = false;
			tried = true;
			clashReason = ds;
			std::cerr << "Not free option " << C << "\n";
		}

			/// init free argument
		void initFree ( BipolarPointer c )
		{
			C = c;
			NotC = inverse(c);
			clashReason = DepSet();
			chosen = false;
			free = true;
			tried = false;
		}
			/// init argument with clash
		void initClash ( BipolarPointer c, const DepSet& ds )
		{
			initFree(c);
			setTried(ds);
		}
	}; // OrArg
public:		// types
		/// short OR indexes
	typedef std::vector<OrArg> OrVector;
		/// short OR index iterator
	typedef OrVector::iterator or_iterator;
		/// short OR index const iterator
	typedef OrVector::const_iterator or_const_iterator;

private:	// members
		/// relevant disjuncts (ready to add)
	OrVector orEntries;
		/// level (global place in the stack)
	unsigned int level = 0;
		/// current branching index
	size_t branchIndex = 0;
		/// number of available options
	unsigned int freeChoices = 0;

public:		// interface
		/// empty c'tor
	BCOr ( void ) : BranchingContext{} {}
		/// init branch index
	void init ( void ) override
	{
		orEntries.clear();
		level = 0;
		branchIndex = 0;
		freeChoices = 0;
	}
		/// give the next branching alternative
	void nextOption ( void ) override { ++branchIndex; }

		// init the options
	void setOrIndex ( OrVector& index )
	{
		orEntries.swap(index);
		freeChoices = 0;
		for ( size_t i = 0; i < orEntries.size(); i++ )
			if ( orEntries[i].free )
				++freeChoices;
	}

	bool noMoreOptions ( void ) const { return freeChoices == 0; }
	void gatherClashSet ( void )
	{
		for ( size_t i = 0; i < orEntries.size(); i++ )
			branchDep.add(orEntries[i].clashReason);
	}
	BipolarPointer chooseFreeOption ( void )
	{
		// try to return chosen one
		or_iterator p, p_beg = orEntries.begin(), p_end = orEntries.end();
		for ( p = p_beg; p != p_end; ++p )
			if ( p->chosen )
				return p->C;
		for ( p = p_beg; p != p_end; ++p )
			if ( p->free )
			{
				p->free = false;
				p->chosen = true;
				return p->C;
			}
		return bpINVALID;
	}
	void failCurOption ( const DepSet& dep, unsigned int curLevel )
	{
		fpp_assert ( curLevel == level );
		for ( or_iterator p = orEntries.begin(), p_end = orEntries.end(); p != p_end; ++p )
			if ( p->chosen )
			{
				--freeChoices;
				p->chosen = false;
				p->tried = true;
				p->clashReason = dep;
				p->clashReason.restrict(curLevel);
				std::cerr << "BC-" << level << ", add ";
				p->clashReason.print(std::cerr);
				std::cerr << " (from ";
				dep.print(std::cerr);
				std::cerr << ") for alternative " << p->C << "\n";
				return;
			}
	}
	bool clearDep ( unsigned int curLevel )
	{
		bool changeSelected = false;
//		std::cerr << "Clear " << curLevel << " for BC-" << level << "\n";
		for ( or_iterator p = orEntries.begin(), p_end = orEntries.end(); p != p_end; ++p )
			if ( p->clashReason.contains(curLevel) )
			{
				std::cerr << "BC-" << level << ", clear ";
				p->clashReason.print(std::cerr);
				std::cerr << " for alternative " << p->C << "\n";
				p->clashReason.clear();
				if ( p->tried )
				{
					p->free = true;
					p->tried = false;
					++freeChoices;
				}
				else if ( p->chosen )
				{
					std::cerr << "BC-" << level << ", clear selection";
					std::cerr << " for alternative " << p->C << "\n";
					changeSelected = true;
					p->free = true;
					p->chosen = false;
				}
			}
		return changeSelected;
	}
	void setChosenDep ( const DepSet& ds )
	{
		for ( or_iterator p = orEntries.begin(), p_end = orEntries.end(); p != p_end; ++p )
			if ( p->chosen )
			{
				std::cerr << "BC-" << level << ", selection dependent on ";
				ds.print(std::cerr);
				std::cerr << " for alternative " << p->C << "\n";
				p->clashReason = ds;
			}
	}
	// access to the fields

		/// check if the current processing OR entry is the last one
	bool isLastOrEntry ( void ) const { return orEntries.size() == branchIndex+1; }
		/// 1st element of OrVector
	or_const_iterator orBeg ( void ) const { return orEntries.begin(); }
		/// current element of OrVector
	or_const_iterator orCur ( void ) const { return orBeg() + branchIndex; }
		/// last (completely or used) element of OrVector
	or_const_iterator orEnd ( void ) const { return RKG_USE_DYNAMIC_BACKJUMPING ? orEntries.end() : orCur(); }

	void setLevel ( unsigned int l ) { level = l; }
	unsigned int getLevel ( void ) const { return level; }
	void print ( std::ostream& o ) const
	{
		o << "BC-" << level << " (" << freeChoices << "/" << orEntries.size() << "): [";
		for ( or_const_iterator p = orBeg(), p_end = orEnd(); p != p_end; ++p )
		{
			if ( p->free )
				o << ' ';
			if ( p->tried )
				o << 'x';
			if ( p->chosen )
				o << '*';
//			o << "arg " << p->C << ", fct=" << p->free << p->chosen << p->tried << ", clash set ";
			o << p->C;
			if ( p->tried )
			{
				if ( p->clashReason.empty() )
					o << "{}";
				else
					p->clashReason.print(o);
			}
			else if ( p->chosen )
				p->clashReason.print(o);

			o << " ";
		}
		o << "]\n";
	}
}; // BCOr

	/// branching context for the Choose-rule
class BCChoose: public BranchingContext
{
public:		// interface
		/// empty c'tor
	BCChoose ( void ) : BranchingContext{} {}
}; // BCChoose

	/// branching context for the NN-rule
class BCNN: public BranchingContext
{
public:		// members
		/// the value of M used in the NN rule
	unsigned int value;

public:		// interface
		/// empty c'tor
	BCNN ( void ) : BranchingContext{}, value{0} {}
		/// init value
	void init ( void ) override { value = 1; }
		/// give the next branching alternative
	void nextOption ( void ) override { ++value; }

	// access to the fields

		/// check if the NN has no option to process
	bool noMoreNNOptions ( unsigned int n ) const { return value > n; }
}; // BCNN

	/// branching context for the LE operations
template <typename T>
class BCLE: public BranchingContext
{
public:		// types
		/// Cardinality Restriction index type
		// TODO: make it 8-bit or so
	typedef unsigned short int CRIndex;
		/// vector of edges
	typedef std::vector<T*> EdgeVector;

public:		// members
		/// vector of edges to be merged
	EdgeVector ItemsToMerge;
		/// index of a edge into which the merge is performing
	CRIndex toIndex;
		/// index of a merge candidate
	CRIndex fromIndex;

public:		// interface
		/// empty c'tor
	BCLE ( void ) : BranchingContext{}, toIndex{0}, fromIndex{0} {}
		/// init indices
	void init ( void ) override
	{
		toIndex = 0;
		fromIndex = 0;
	}
		/// correct fromIndex after changing
	void resetMCI ( void ) { fromIndex = static_cast<CRIndex>(ItemsToMerge.size() - 1); }
		/// give the next branching alternative
	void nextOption ( void ) override
	{
		--fromIndex;	// get new merge candidate
		if ( fromIndex == toIndex )	// nothing more can be mergeable to BI node
		{
			++toIndex;	// change the candidate to merge to
			resetMCI();
		}
	}

	// access to the fields

		/// get FROM pointer to merge
	T* getFrom ( void ) const { return ItemsToMerge[fromIndex]; }
		/// get FROM pointer to merge
	T* getTo ( void ) const { return ItemsToMerge[toIndex]; }
		/// check if the LE has no option to process
	bool noMoreLEOptions ( void ) const { return fromIndex <= toIndex; }
}; // BCLE

	/// branching context for the barrier
class BCBarrier: public BranchingContext
{
public:		// interface
		/// empty c'tor
	BCBarrier ( void ) : BranchingContext() {}
}; // BCBarrier

#endif
