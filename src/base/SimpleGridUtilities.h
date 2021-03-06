///////////////////////////////////////////////////////////////////////////////
///
///	\file    SimpleGridUtilities.h
///	\author  Paul Ullrich
///	\version August 14, 2018
///
///	<remarks>
///		Copyright 2000-2018 Paul Ullrich
///
///		This file is distributed as part of the Tempest source code package.
///		Permission is granted to use, copy, modify and distribute this
///		source code and its documentation under the terms of the GNU General
///		Public License.  This software is provided "as is" without express
///		or implied warranty.
///	</remarks>

#ifndef _SIMPLEGRIDUTILITIES_H_
#define _SIMPLEGRIDUTILITIES_H_

#include "SimpleGrid.h"
#include "DataVector.h"

#include <set>

///////////////////////////////////////////////////////////////////////////////

///	<summary>
///		Find the minimum/maximum value of a field near the given point.
///	</summary>
///	<param name="dMaxDist">
///		Maximum distance from the initial point in degrees.
///	</param>
///	<param name="ixExtremum">
///		Output node index at which the extremum occurs.
///	</param>
///	<param name="dMaxValue">
///		Output value of the field taken at the extremum point.
///	</param>
///	<param name="dRMax">
///		Output distance from the centerpoint at which the extremum occurs
///		in great circle distance (degrees).
///	</param>
template <typename real>
void FindLocalMinMax(
	const SimpleGrid & grid,
	bool fMinimum,
	const DataVector<real> & data,
	int ix0,
	double dMaxDist,
	int & ixExtremum,
	real & dMaxValue,
	float & dRMax
);

///	<summary>
///		Find the locations of all minima in the given DataVector.
///	</summary>
template <typename real>
void FindAllLocalMinima(
	const SimpleGrid & grid,
	const DataVector<real> & data,
	std::set<int> & setMinima
);

///	<summary>
///		Find the locations of all maxima in the given DataVector.
///	</summary>
template <typename real>
void FindAllLocalMaxima(
	const SimpleGrid & grid,
	const DataVector<real> & data,
	std::set<int> & setMaxima
);

///	<summary>
///		Find the local average of a field near the given point.
///	</summary>
///	<param name="dMaxDist">
///		Maximum distance from the initial point in degrees.
///	</param>
template <typename real>
void FindLocalAverage(
	const SimpleGrid & grid,
	const DataVector<real> & data,
	int ix0,
	double dMaxDist,
	real & dAverage
);

///////////////////////////////////////////////////////////////////////////////

#endif // _SIMPLEGRIDUTILITIES_H_

