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

#include "AtomicDecomposer.h"
#include "logging.h"
#include "ProgressIndicatorInterface.h"

//#define RKG_DEBUG_AD

/// d'tor
AtomicDecomposer :: ~AtomicDecomposer ( void )
{
	delete AOS;
	delete PI;
}

/// remove tautologies (axioms that are always local) from the ontology temporarily
void
AtomicDecomposer :: removeTautologies ( TOntology* O )
{
	// we might use it for another decomposition
	Tautologies.clear();
	unsigned long nAx = 0;
	for ( TOntology::iterator p = O->begin(), p_end = O->end(); p != p_end; ++p )
		if ( likely((*p)->isUsed()) )
		{
			if ( unlikely(pModularizer->isTautology(*p,type)) )
			{
				Tautologies.push_back(*p);
				(*p)->setUsed(false);
			}
			else
				++nAx;
		}
	if ( PI )
		PI->setLimit(nAx);
}

/// build a module for given axiom AX; use parent atom's module as a base for the module search
TOntologyAtom*
AtomicDecomposer :: buildModule ( const TSignature& sig, TOntologyAtom* parent )
{
	// build a module for a given signature
	pModularizer->extract ( parent->getModule().begin(), parent->getModule().end(), sig, type );
	const AxiomVec& Module = pModularizer->getModule();
	// if module is empty (empty bottom atom) -- do nothing
	if ( Module.empty() )
		return NULL;
	// here the module is created; report it
	if ( PI )
		PI->incIndicator();
	// check if the module corresponds to a PARENT one; modules are the same iff their sizes are the same
	if ( parent != rootAtom && Module.size() == parent->getModule().size() )	// same module means same atom
		return parent;
	// create new atom with that module
	TOntologyAtom* atom = AOS->newAtom();
	atom->setModule(Module);
	return atom;
}

/// create atom for given axiom AX; use parent atom's module as a base for the module search
TOntologyAtom*
AtomicDecomposer :: createAtom ( TDLAxiom* ax, TOntologyAtom* parent )
{
	// check whether axiom already has an atom
	if ( ax->getAtom() != NULL )
		return const_cast<TOntologyAtom*>(ax->getAtom());
	// build an atom: use a module to find atomic dependencies
	TOntologyAtom* atom = buildModule( ax->getSignature(), parent );
	// no empty modules should be here
	fpp_assert ( atom != NULL );
	// register axiom as a part of an atom
	atom->addAxiom(ax);
	// if atom is the same as parent -- nothing more to do
	if ( atom == parent )
		return parent;
	// not the same as parent: for all atom's axioms check their atoms and make ATOM depend on them
#ifdef RKG_DEBUG_AD
	// do cycle via set to keep the order
	typedef std::set<TDLAxiom*> AxSet;
	const AxSet M ( atom->getModule().begin(), atom->getModule().end() );
	for ( AxSet::iterator q = M.begin(); q != M.end(); ++q )
#else
	for ( TOntologyAtom::AxiomSet::const_iterator q = atom->getModule().begin(), q_end = atom->getModule().end(); q != q_end; ++q )
#endif
		if ( likely ( *q != ax ) )
			atom->addDepAtom ( createAtom ( *q, atom ) );
	return atom;
}

/// get the atomic structure for given module type T
AOStructure*
AtomicDecomposer :: getAOS ( TOntology* O, ModuleType t )
{
	// remember the type of the module
	type = t;

	// prepare a new AO structure
	delete AOS;
	AOS = new AOStructure();

	// init semantic locality checker
	pModularizer->preprocessOntology(O->getAxioms());

	// we don't need tautologies here
	removeTautologies(O);

	// init the root atom
	rootAtom = new TOntologyAtom();
	rootAtom -> setModule ( TOntologyAtom::AxiomSet ( O->begin(), O->end() ) );

	// build the "bottom" atom for an empty signature
	TOntologyAtom* BottomAtom = buildModule ( TSignature(), rootAtom );
	if ( BottomAtom )
		for ( TOntologyAtom::AxiomSet::const_iterator q = BottomAtom->getModule().begin(), q_end = BottomAtom->getModule().end(); q != q_end; ++q )
			BottomAtom->addAxiom(*q);

	// create atoms for all the axioms in the ontology
	for ( TOntology::iterator p = O->begin(), p_end = O->end(); p != p_end; ++p )
		if ( (*p)->isUsed() && (*p)->getAtom() == NULL )
			createAtom ( *p, rootAtom );

	// restore tautologies in the ontology
	restoreTautologies();

	if ( LLM.isWritable(llAlways) )
		LL << "\nThere were " << pModularizer->getNNonLocal() << " non-local axioms out of " << pModularizer->getNChecks() << " totally checked\n";

	// clear the root atom
	delete rootAtom;

	// reduce graph
	AOS->reduceGraph();

	return AOS;
}
