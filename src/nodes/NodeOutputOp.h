///////////////////////////////////////////////////////////////////////////////
///
///	\file    NodeOutputOp.h
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

#ifndef _NODEOUTPUTOP_H_
#define _NODEOUTPUTOP_H_

#include "SimpleGridUtilities.h"

///////////////////////////////////////////////////////////////////////////////

///	<summary>
///		A class storing an output operator.
///	</summary>
class OutputOp {

public:
	///	<summary>
	///		Possible operations.
	///	</summary>
	enum Operation {
		Max,
		Min,
		Avg,
		MaxDist,
		MinDist
	};

public:
	///	<summary>
	///		Parse a output operator string.
	///	</summary>
	void Parse(
		VariableRegistry & varreg,
		const std::string & strOp
	) {
		// Read mode
		enum {
			ReadMode_Op,
			ReadMode_Distance,
			ReadMode_Invalid
		} eReadMode = ReadMode_Op;

		// Get variable information
		Variable var;
		int iLast = var.ParseFromString(varreg, strOp) + 1;
		m_varix = varreg.FindOrRegister(var);

		// Loop through string
		for (int i = iLast; i <= strOp.length(); i++) {

			// Comma-delineated
			if ((i == strOp.length()) || (strOp[i] == ',')) {

				std::string strSubStr =
					strOp.substr(iLast, i - iLast);

				// Read in operation
				if (eReadMode == ReadMode_Op) {
					if (strSubStr == "max") {
						m_eOp = Max;
					} else if (strSubStr == "min") {
						m_eOp = Min;
					} else if (strSubStr == "avg") {
						m_eOp = Avg;
					} else if (strSubStr == "maxdist") {
						m_eOp = MaxDist;
					} else if (strSubStr == "mindist") {
						m_eOp = MinDist;
					} else {
						_EXCEPTION1("Output invalid operation \"%s\"",
							strSubStr.c_str());
					}

					iLast = i + 1;
					eReadMode = ReadMode_Distance;

				// Read in minimum count
				} else if (eReadMode == ReadMode_Distance) {
					m_dDistance = atof(strSubStr.c_str());

					iLast = i + 1;
					eReadMode = ReadMode_Invalid;

				// Invalid
				} else if (eReadMode == ReadMode_Invalid) {
					_EXCEPTION1("\nInsufficient entries in output op \"%s\""
							"\nRequired: \"<name>,<operation>,<distance>\"",
							strOp.c_str());
				}
			}
		}

		if (eReadMode != ReadMode_Invalid) {
			_EXCEPTION1("\nInsufficient entries in output op \"%s\""
					"\nRequired: \"<name>,<operation>,<distance>\"",
					strOp.c_str());
		}

		if (m_dDistance < 0.0) {
			_EXCEPTIONT("For output op, distance must be nonnegative");
		}

		// Output announcement
		std::string strDescription;

		if (m_eOp == Max) {
			strDescription += "Maximum of ";
		} else if (m_eOp == Min) {
			strDescription += "Minimum of ";
		} else if (m_eOp == Avg) {
			strDescription += "Average of ";
		} else if (m_eOp == MaxDist) {
			strDescription += "Distance to maximum of ";
		} else if (m_eOp == MinDist) {
			strDescription += "Distance to minimum of ";
		}

		char szBuffer[128];

		sprintf(szBuffer, "%s", var.ToString(varreg).c_str());
		strDescription += szBuffer;

		sprintf(szBuffer, " within %f degrees", m_dDistance);
		strDescription += szBuffer;

		Announce("%s", strDescription.c_str());
	}

public:
	///	<summary>
	///		Variable to use for output.
	///	</summary>
	VariableIndex m_varix;

	///	<summary>
	///		Operation.
	///	</summary>
	Operation m_eOp;

	///	<summary>
	///		Distance to use when applying operation.
	///	</summary>
	double m_dDistance;
};

///////////////////////////////////////////////////////////////////////////////

template <typename real>
void ApplyOutputOp(
	const OutputOp & op,
	const SimpleGrid & grid,
	VariableRegistry & varreg,
	NcFileVector & vecFiles,
	int ixTime,
	int ixCandidate,
	std::string & strResult
) {
	static const char * szFormat = "%3.6e";
	char buf[100];

	// Load the search variable data
	Variable & var = varreg.Get(op.m_varix);
	var.LoadGridData(varreg, vecFiles, grid, ixTime);
	const DataVector<float> & dataState = var.GetData();

	// Return values from the output operators
	int ixExtremum;
	float dValue;
	float dRMax;

	if (op.m_eOp == OutputOp::Max) {
		FindLocalMinMax<real>(
			grid,
			false,
			dataState,
			ixCandidate,
			op.m_dDistance,
			ixExtremum,
			dValue,
			dRMax);

		sprintf(buf, szFormat, dValue);
		strResult = buf;

	} else if (op.m_eOp == OutputOp::MaxDist) {
		FindLocalMinMax<float>(
			grid,
			false,
			dataState,
			ixCandidate,
			op.m_dDistance,
			ixExtremum,
			dValue,
			dRMax);

		sprintf(buf, szFormat, dRMax);
		strResult = buf;

	} else if (op.m_eOp == OutputOp::Min) {
		FindLocalMinMax<float>(
			grid,
			true,
			dataState,
			ixCandidate,
			op.m_dDistance,
			ixExtremum,
			dValue,
			dRMax);

		sprintf(buf, szFormat, dValue);
		strResult = buf;

	} else if (op.m_eOp == OutputOp::MinDist) {
		FindLocalMinMax<float>(
			grid,
			true,
			dataState,
			ixCandidate,
			op.m_dDistance,
			ixExtremum,
			dValue,
			dRMax);

		sprintf(buf, szFormat, dRMax);
		strResult = buf;

	} else if (op.m_eOp == OutputOp::Avg) {
		FindLocalAverage<float>(
			grid,
			dataState,
			ixCandidate,
			op.m_dDistance,
			dValue);

		sprintf(buf, szFormat, dValue);
		strResult = buf;

	} else {
		_EXCEPTIONT("Invalid Output operator");
	}
}

///////////////////////////////////////////////////////////////////////////////

#endif // _NODEOUTPUTOP_H_

