/* This file is part of the FaCT++ DL reasoner
Copyright (C) 2013-2015 Dmitry Tsarkov and The University of Manchester
Copyright (C) 2015-2017 Dmitry Tsarkov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef PO_QUERIES_H
#define PO_QUERIES_H

#include "ConjunctiveQuerySet.h"

class POQuery: public CQSet
{
public:		// interface
	POQuery ( TExpressionManager* pEM, VariableFactory* VarFact )
		: CQSet(pEM, VarFact, true )
	{
		QRQuery * query;
		defC(PO_0004518);
		defC(PO_0009021);
		defC(PO_0009046);
		defC(PO_0025585);
		defC(PO_0009058);
		defC(PO_0009002);
		defC(PO_0025032);
		defC(PO_0009012);
		defC(PO_0009011);

		defR(RO_0002202);
		defR(has_participant);
		defR(preceded_by);
		defR(BFO_0000050);

		defV(v0);
		defV(v1);
		defV(v2);

		// query 0
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRRoleAtom(RO_0002202,v1,v0));
		query->addAtom(new QRConceptAtom(PO_0004518,v1));
 		queries.push_back(query);

		// query 1
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRRoleAtom(has_participant,v0,v1));
		query->addAtom(new QRRoleAtom(preceded_by,v0,v2));
		query->addAtom(new QRConceptAtom(PO_0009021,v0));
		query->addAtom(new QRConceptAtom(PO_0009046,v1));
		query->addAtom(new QRConceptAtom(PO_0025585,v2));
		queries.push_back(query);

		// query 2
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRRoleAtom(BFO_0000050,v0,v1));
		query->addAtom(new QRConceptAtom(PO_0009058,v1));
		queries.push_back(query);

		// query 3
		query = new QRQuery();
		query->setVarFree(v0);
		query->addAtom(new QRRoleAtom(BFO_0000050,v0,v1));
		query->addAtom(new QRConceptAtom(PO_0009002,v0));
		query->addAtom(new QRConceptAtom(PO_0025032,v1));
		queries.push_back(query);

		// query 4
		query = new QRQuery();
		query->setVarFree(v0);
		query->setVarFree(v1);
		query->addAtom(new QRRoleAtom(has_participant,v0,v1));
		query->addAtom(new QRConceptAtom(PO_0009012,v0));
		query->addAtom(new QRConceptAtom(PO_0009011,v1));
		queries.push_back(query);
	}
}; // POQuery

#endif
