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

#ifndef EFACTPLUSPLUS_H
#define EFACTPLUSPLUS_H

#include <exception>

/// general FaCT++ exception
class EFaCTPlusPlus: public std::exception
{
protected:
		/// reason of the exception
	const char* reason;

public:
		/// empty c'tor
	EFaCTPlusPlus ( void )
		: exception()
		, reason("FaCT++.Kernel: General exception")
		{}
		/// init c'tor
	EFaCTPlusPlus ( const char* str )
		: exception()
		, reason(str)
		{}
		/// empty d'tor
	virtual ~EFaCTPlusPlus ( void ) throw() {}

		/// reason
	virtual const char* what ( void ) const throw() { return reason; }
}; // EFaCTPlusPlus

#endif
