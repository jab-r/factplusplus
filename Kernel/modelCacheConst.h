/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2007-2015 Dmitry Tsarkov and The University of Manchester
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

#ifndef MODELCACHECONST_H
#define MODELCACHECONST_H

#include "modelCacheInterface.h"
#include "BiPointer.h"
#include "fpp_assert.h"

///	Model caching implementation for TOP/BOTTOM nodes.
class modelCacheConst: public modelCacheInterface
{
protected:	// members
		/// the const itself
	bool isTop;

protected:	// methods
		/// log a particular implementation of a cache entry
	virtual void logCacheEntryImpl ( void ) const
		{ LL << "\nConst cache: element " << (isTop ? "TOP" : "BOTTOM"); }

public:
		/// c'tor: no nominals can be here
	modelCacheConst ( bool top )
		: modelCacheInterface(/*flagNominals=*/false)
		, isTop(top)
		{}
		/// copy c'tor
	modelCacheConst ( const modelCacheConst& m )
		: modelCacheInterface(m.hasNominalNode)
		, isTop(m.isTop)
		{}
		/// empty d'tor
	virtual ~modelCacheConst ( void ) {}

		/// Check if the model contains clash
	virtual modelCacheState getState ( void ) const { return isTop ? csValid : csInvalid; }
		/// get the value of the constant
	bool getConst ( void ) const { return isTop; }

	// mergable part

		/// check whether two caches can be merged; @return state of "merged" model
	modelCacheState canMerge ( const modelCacheInterface* p ) const
	{
		if ( p->getCacheType() == mctConst )
			return isTop && static_cast<const modelCacheConst*>(p)->isTop ?
				   csValid : csInvalid;
		else
			return p->canMerge(this);
	}
		/// Get the tag identifying the cache type
	virtual modelCacheType getCacheType ( void ) const { return mctConst; }
}; // modelCacheConst

// create const cache by BP; BP should be either bpTOP or bpBOTTOM
inline
modelCacheConst* createConstCache ( BipolarPointer bp )
{
	fpp_assert ( bp == bpTOP || bp == bpBOTTOM );
	return new modelCacheConst(bp==bpTOP);
}

#endif
