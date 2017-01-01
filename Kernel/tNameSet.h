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

#ifndef TNAMESET_H
#define TNAMESET_H

#include <string>
#include <map>

/// base class for creating Named Entries; template parameter should be derived from TNamedEntry
template<class T>
class TNameCreator
{
public:		// interface
		/// empty c'tor
	TNameCreator ( void ) {}
		/// empty d'tor
	virtual ~TNameCreator ( void ) {}

		/// create new Named Entry
	virtual T* makeEntry ( const std::string& name ) const { return new T(name); }
}; // TNameCreator


/// Implementation of NameSets by binary trees; template parameter should be derived from TNamedEntry
template<class T>
class TNameSet
{
protected:	// types
		/// base type
	typedef std::map <std::string, T*> NameTree;
		/// RO iterator
	typedef typename NameTree::const_iterator const_iterator;

public:		// types
		/// RW iterator
	typedef typename NameTree::iterator iterator;

protected:	// members
		/// Base holding all names
	NameTree Base;
		/// creator of new name
	TNameCreator<T>* Creator;

private:	// no copy
		/// no copy c'tor
	TNameSet ( const TNameSet& );
		/// no assignment
	TNameSet& operator = ( const TNameSet& );

public:		// interface
		/// c'tor (empty)
	TNameSet ( void ) : Creator(new TNameCreator<T>) {}
		/// c'tor (with given Name Creating class)
	TNameSet ( TNameCreator<T>* p ) : Creator(p) {}
		/// d'tor (delete all entries)
	virtual ~TNameSet ( void ) { clear(); delete Creator; }

		/// return pointer to existing id or NULL if no such id defined
	T* get ( const std::string& id ) const
	{
		const_iterator p = Base.find(id);
		return p == Base.end() ? NULL : p->second;
	}
		/// unconditionally add new element with name ID to the set; return new element
	T* add ( const std::string& id )
	{
		T* pne = Creator->makeEntry(id);
		Base[id] = pne;
		return pne;
	}
		/// Insert id to a nameset (if necessary); @return pointer to id structure created by external creator
	T* insert ( const std::string& id )
	{
		T* pne = get(id);
		if ( pne == NULL )	// no such Id
			pne = add(id);
		return pne;
	}
		/// remove given entry from the set
	void remove ( const std::string& id )
	{
		iterator p = Base.find(id);

		if ( p != Base.end () )	// founs such Id
		{
			delete p->second;
			Base.erase(p);
		}
	}
		/// clear name set
	void clear ( void )
	{
		for ( iterator p = Base.begin(); p != Base.end(); ++p )
			delete p->second;

		Base.clear();
	}
		/// get size of a name set
	size_t size ( void ) const { return Base.size(); }
		/// RW begin iterator
	iterator begin ( void ) { return Base.begin(); }
		/// RW end iterator
	iterator end ( void ) { return Base.end(); }
}; // TNameSet

#endif
