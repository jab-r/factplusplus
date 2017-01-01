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

#ifndef ATOMICDECOMPOSER_H
#define ATOMICDECOMPOSER_H

#include "tOntologyAtom.h"
#include "tSignature.h"
#include "Modularity.h"

class ProgressIndicatorInterface;

/// atomical ontology structure
class AOStructure
{
public:		// types
		/// vector of atoms as a type
	typedef std::vector<TOntologyAtom*> AtomVec;
		/// RW iterator of it
	typedef AtomVec::iterator iterator;

protected:	// members
		/// all the atoms
	AtomVec Atoms;

public:		// interface
		/// empty c'tor
	AOStructure ( void ) {}
		/// d'tor: delete all atoms
	~AOStructure ( void )
	{
		for ( iterator p = Atoms.begin(), p_end = Atoms.end(); p != p_end; ++p )
			delete *p;
	}

		/// create a new atom and get a pointer to it
	TOntologyAtom* newAtom ( void )
	{
		TOntologyAtom* ret = new TOntologyAtom();
		ret->setId(Atoms.size());
		Atoms.push_back(ret);
		return ret;
	}
		/// reduce graph of the atoms in the structure
	void reduceGraph ( void )
	{
		TOntologyAtom::AtomSet checked;
		for ( iterator p = Atoms.begin(), p_end = Atoms.end(); p != p_end; ++p )
			(*p)->getAllDepAtoms(checked);
	}

		/// RW iterator begin
	iterator begin ( void ) { return Atoms.begin(); }
		/// RW iterator end
	iterator end ( void ) { return Atoms.end(); }
		/// get RW atom by its index
	TOntologyAtom* operator[] ( unsigned int index ) { return Atoms[index]; }
		/// get RO atom by its index
	const TOntologyAtom* operator[] ( unsigned int index ) const { return Atoms[index]; }
		/// size of the structure
	size_t size ( void ) const { return Atoms.size(); }
}; // AOStructure

/// atomical decomposer of the ontology
class AtomicDecomposer
{
protected:	// members
		/// atomic structure to build
	AOStructure* AOS;
		/// modularizer to build modules
	TModularizer* pModularizer;
		/// tautologies of the ontology
	AxiomVec Tautologies;
		/// progress indicator
	ProgressIndicatorInterface* PI;
		/// fake atom that represents the whole ontology
	TOntologyAtom* rootAtom;
		/// module type for current AOS creation
	ModuleType type;

protected:	// methods
		/// remove tautologies (axioms that are always local) from the ontology temporarily
	void removeTautologies ( TOntology* O );
		/// restore all tautologies back
	void restoreTautologies ( void )
	{
		for ( AxiomVec::iterator p = Tautologies.begin(), p_end = Tautologies.end(); p != p_end; ++p )
			(*p)->setUsed(true);
	}
		/// build a module for given signature SIG; use parent atom's module as a base for the module search
	TOntologyAtom* buildModule ( const TSignature& sig, TOntologyAtom* parent );
		/// create atom for given axiom AX; use parent atom's module as a base for the module search
	TOntologyAtom* createAtom ( TDLAxiom* ax, TOntologyAtom* parent );

public:		// interface
		/// init c'tor; M would NOT be deleted in d'tor
	AtomicDecomposer ( TModularizer* m ) : AOS(NULL), pModularizer(m), PI(NULL), rootAtom(NULL) {}
		/// d'tor
	~AtomicDecomposer ( void );

		/// get the atomic structure for given module type TYPE
	AOStructure* getAOS ( TOntology* O, ModuleType type );
		/// get already created atomic structure
	const AOStructure* getAOS ( void ) const { return AOS; }

		/// set progress indicator to be PI
	void setProgressIndicator ( ProgressIndicatorInterface* pi ) { PI = pi; }
		/// get number of performed locality checks
	unsigned long long getLocChekNumber ( void ) const { return pModularizer->getNChecks(); }
}; // AtomicDecomposer

#endif
