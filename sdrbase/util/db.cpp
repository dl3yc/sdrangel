///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2015 Edouard Griffiths, F4EXB                                   //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
// (at your option) any later version.                                           //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include "util/db.h"
#include <cmath>
#include <cassert>

double CalcDb::dbPower(double magsq, double floor)
{
    assert(floor > 0.0);

	if (magsq > floor)
	{
	    return 10.0 * log10(magsq);
	}
	else
	{
		return 10.0 * log10(floor);
	}
}

double CalcDb::powerFromdB(double powerdB)
{
    return pow(10.0, powerdB / 10.0);
}
