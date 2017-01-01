/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2006-2015 Dmitry Tsarkov and The University of Manchester
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

#ifndef JNIACTOR_H
#define JNIACTOR_H

#include "tJNICache.h"

/// class for acting with concept taxonomy
template<class AccessPolicy>
class JTaxonomyActor: public WalkerInterface
{
protected:	// types
		/// array of TNEs
	typedef std::vector<TExpr*> SynVector;
		/// array for a set of taxonomy verteces
	typedef std::vector<SynVector> SetOfNodes;

protected:	// members
		/// JNI environment
	TJNICache* J;
		/// 2D array to return
	SetOfNodes acc;
		/// 1D array to return
	SynVector plain;
		/// temporary vector to keep synonyms
	SynVector syn;

protected:	// methods
		/// create vector of Java objects by given SynVector
	jobjectArray getArray ( const SynVector& vec ) const
		{ return J->buildArray ( vec, AccessPolicy::getIDs(J) ); }
		/// try current entry
	void tryEntry ( const ClassifiableEntry* p )
	{
		if ( p->isSystem() )
			return;
		if ( AccessPolicy::applicable(p) )
			syn.push_back(AccessPolicy::buildTree(J,p));
	}

public:		// interface
		/// c'tor
	JTaxonomyActor ( TJNICache* cache ) : J(cache) {}
		/// d'tor
	virtual ~JTaxonomyActor ( void ) {}

	void clear ( void ) { acc.clear(); plain.clear(); }
	// return values

		/// get single vector of synonyms (necessary for Equivalents, for example)
	jobjectArray getSynonyms ( void ) const { return getArray ( acc.empty() ? SynVector() : acc[0] ); }
		/// get 2D array of all required elements of the taxonomy
	jobjectArray getElements ( void ) const
	{
		if ( AccessPolicy::needPlain() )
			return getArray(plain);
		jobjectArray ret = J->env->NewObjectArray ( (jsize)acc.size(), AccessPolicy::getIDs(J).ArrayClassID, NULL );
		for ( unsigned int i = 0; i < acc.size(); ++i )
			J->env->SetObjectArrayElement ( ret, (jsize)i, getArray(acc[i]) );
		return ret;
	}

		/// taxonomy walking method.
		/// @return true if node was processed, and there is no need to go further
		/// @return false if node can not be processed in current settings
	virtual bool apply ( const TaxonomyVertex& v )
	{
		syn.clear();
		tryEntry(v.getPrimer());

		for ( TaxonomyVertex::syn_iterator p = v.begin_syn(), p_end=v.end_syn(); p != p_end; ++p )
			tryEntry(*p);

		/// no applicable elements were found
		if ( syn.empty() )
			return false;

		if ( AccessPolicy::needPlain() )
			plain.insert ( plain.end(), syn.begin(), syn.end() );
		else
			acc.push_back(syn);
		return true;
	}
}; // JTaxonomyActor

// policy elements

/// policy for concepts
class ClassPolicy
{
public:
	static const TClassFieldMethodIDs& getIDs ( const TJNICache* J ) { return J->ClassPointer; }
	static bool applicable ( const ClassifiableEntry* p )
		{ return !static_cast<const TConcept*>(p)->isSingleton(); }
	static bool needPlain ( void ) { return false; }
	static TExpr* buildTree ( TJNICache* J, const ClassifiableEntry* p )
	{
		if ( p->getId() >= 0 )
			return J->getCName(p->getName());

		// top or bottom
		std::string name(p->getName());

		if ( name == std::string("TOP") )
			return J->EM->Top();
		else if ( name == std::string("BOTTOM") )
			return J->EM->Bottom();
		else	// error
			return NULL;
	}
}; // ClassPolicy

/// policy for individuals
template<bool plain>
class IndividualPolicy
{
public:
	static const TClassFieldMethodIDs& getIDs ( const TJNICache* J ) { return J->IndividualPointer; }
	static bool applicable ( const ClassifiableEntry* p )
		{ return static_cast<const TConcept*>(p)->isSingleton(); }
	static bool needPlain ( void ) { return plain; }
	static TExpr* buildTree ( TJNICache* J, const ClassifiableEntry* p )
		{ return J->getIName(p->getName()); }
}; // IndividualPolicy

/// policy for object properties
class ObjectPropertyPolicy
{
public:
	static const TClassFieldMethodIDs& getIDs ( const TJNICache* J ) { return J->ObjectPropertyPointer; }
	static bool applicable ( const ClassifiableEntry* ) { return true; }
	static bool needPlain ( void ) { return false; }
	static TExpr* buildTree ( TJNICache* J, const ClassifiableEntry* p )
	{
		return p->getId() >= 0 ?
			J->getOName(p->getName()) :
			J->EM->Inverse(J->getOName(static_cast<const TRole*>(p)->realInverse()->getName()));
	}
}; // ObjectPropertyPolicy

/// policy for data properties
class DataPropertyPolicy
{
public:
	static const TClassFieldMethodIDs& getIDs ( const TJNICache* J ) { return J->DataPropertyPointer; }
	static bool applicable ( const ClassifiableEntry* p ) { return p->getId() >= 0; }
	static bool needPlain ( void ) { return false; }
	static TExpr* buildTree ( TJNICache* J, const ClassifiableEntry* p )
		{ return J->getDName(p->getName()); }
}; // DataPropertyPolicy

#endif
