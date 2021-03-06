///////////////////////////////////////////////////////////////////////////////
///
///	\file    SimpleGrid.h
///	\author  Paul Ullrich
///	\version November 18, 2015
///
///	<remarks>
///		Copyright 2000-2014 Paul Ullrich
///
///		This file is distributed as part of the Tempest source code package.
///		Permission is granted to use, copy, modify and distribute this
///		source code and its documentation under the terms of the GNU General
///		Public License.  This software is provided "as is" without express
///		or implied warranty.
///	</remarks>

#ifndef _SIMPLEGRID_H_
#define _SIMPLEGRID_H_

#include "DataVector.h"

#include <cstdlib>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>

#include "netcdfcpp.h"

///////////////////////////////////////////////////////////////////////////////

///	<summary>
///		A data structure describing the grid, including coordinates of
///		each data point and graph connectivity of elements.
///	</summary>
class SimpleGrid {

public:
	///	<summary>
	///		Generate the unstructured grid information for a
	///		longitude-latitude grid.
	///	</summary>
	void GenerateLatitudeLongitude(
		const DataVector<double> & vecLat,
		const DataVector<double> & vecLon,
		bool fRegional
	) {
		int nLat = vecLat.GetRows();
		int nLon = vecLon.GetRows();

		m_dLat.Initialize(nLon * nLat);
		m_dLon.Initialize(nLon * nLat);
		m_vecConnectivity.resize(nLon * nLat);

		m_nGridDim.resize(2);
		m_nGridDim[0] = nLat;
		m_nGridDim[1] = nLon;

		// Verify units of latitude and longitude
		for (int j = 0; j < nLat; j++) {
			if (fabs(vecLat[j]) > 0.5 * M_PI + 1.0e-12) {
				_EXCEPTIONT("In SimpleGrid, latitude array must be given in radians");
			}
		}

		int ixs = 0;
		for (int j = 0; j < nLat; j++) {
		for (int i = 0; i < nLon; i++) {

			// Vectorize coordinates
			m_dLat[ixs] = vecLat[j];
			m_dLon[ixs] = vecLon[i];

			// Connectivity in each compass direction
			if (j != 0) {
				m_vecConnectivity[ixs].push_back((j-1) * nLon + i);
			}
			if (j != nLat-1) {
				m_vecConnectivity[ixs].push_back((j+1) * nLon + i);
			}

			if ((!fRegional) ||
			    ((i != 0) && (i != nLon-1))
			) {
				m_vecConnectivity[ixs].push_back(
					j * nLon + ((i + 1) % nLon));
				m_vecConnectivity[ixs].push_back(
					j * nLon + ((i + nLon - 1) % nLon));
			}

			ixs++;
		}
		}

	}

	///	<summary>
	///		Try to automatically generate the SimpleGrid from a NetCDF
	///		file with latitude/longitude coordinates.
	///	</summary>
	void GenerateLatitudeLongitude(
		NcFile * ncFile,
		bool fRegional
	) {
		NcDim * dimLat = ncFile->get_dim("lat");
		if (dimLat == NULL) {
			_EXCEPTIONT("No dimension \"lat\" found in input file");
		}

		NcDim * dimLon = ncFile->get_dim("lon");
		if (dimLon == NULL) {
			_EXCEPTIONT("No dimension \"lon\" found in input file");
		}

		NcVar * varLat = ncFile->get_var("lat");
		if (varLat == NULL) {
			_EXCEPTIONT("No variable \"lat\" found in input file");
		}

		NcVar * varLon = ncFile->get_var("lon");
		if (varLon == NULL) {
			_EXCEPTIONT("No variable \"lon\" found in input file");
		}

		int nLat = dimLat->size();
		int nLon = dimLon->size();

		DataVector<double> vecLat(nLat);
		varLat->get(vecLat, nLat);

		for (int j = 0; j < nLat; j++) {
			vecLat[j] *= M_PI / 180.0;
		}

		DataVector<double> vecLon(nLon);
		varLon->get(vecLon, nLon);

		for (int i = 0; i < nLon; i++) {
			vecLon[i] *= M_PI / 180.0;
		}

		// Generate the SimpleGrid
		GenerateLatitudeLongitude(vecLat, vecLon, fRegional);
	}

	///	<summary>
	///		Load the grid information from a file.
	///	</summary>
	void FromFile(
		const std::string & strGridInfoFile
	) {
		std::ifstream fsGrid(strGridInfoFile.c_str());
		if (!fsGrid.is_open()) {
			_EXCEPTION1("Unable to open file \"%s\"",
				strGridInfoFile.c_str());
		}

		size_t nFaces;
		fsGrid >> nFaces;

		m_nGridDim.resize(1);
		m_nGridDim[0] = nFaces;

		m_dLon.Initialize(nFaces);
		m_dLat.Initialize(nFaces);
		m_vecConnectivity.resize(nFaces);

		for (size_t f = 0; f < nFaces; f++) {
			size_t sNeighbors;
			char cComma;
			fsGrid >> m_dLon[f];
			fsGrid >> cComma;
			fsGrid >> m_dLat[f];
			fsGrid >> cComma;
			fsGrid >> sNeighbors;
			fsGrid >> cComma;

			// Convert to radians
			m_dLon[f] *= M_PI / 180.0;
			m_dLat[f] *= M_PI / 180.0;

			// Load connectivity
			m_vecConnectivity[f].resize(sNeighbors);
			for (size_t n = 0; n < sNeighbors; n++) {
				fsGrid >> m_vecConnectivity[f][n];
				if (n != sNeighbors-1) {
					fsGrid >> cComma;
				}
				m_vecConnectivity[f][n]--;
			}
			if (fsGrid.eof()) {
				if (f != nFaces-1) {
					_EXCEPTIONT("Premature end of file");
				}
			}
		}
	}

	///	<summary>
	///		Get the size of the SimpleGrid (number of points).
	///	</summary>
	size_t GetSize() const {
		return (m_vecConnectivity.size());
	}

	///	<summary>
	///		Convert a coordinate to an index.
	///	</summary>
	int CoordinateVectorToIndex(
		const std::vector<int> & coordvec
	) {
		if (m_nGridDim.size() == 0) {
			_EXCEPTIONT("Invalid SimpleGrid");
		}

		if (coordvec.size() != m_nGridDim.size()) {
			_EXCEPTIONT("Invalid coordinate vector");
		}
		if (coordvec.size() == 1) {
			if (coordvec[0] >= m_nGridDim[0]) {
				_EXCEPTIONT("Coordinate vector out of range");
			}
			return coordvec[0];
		}
		if (coordvec.size() == 2) {
			if (coordvec[0] >= m_nGridDim[0]) {
				_EXCEPTIONT("Coordinate vector out of range");
			}
			if (coordvec[1] >= m_nGridDim[1]) {
				_EXCEPTIONT("Coordinate vector out of range");
			}
		}

		int ix = 0;
		int d = 1;
		for (int i = 0; i < coordvec.size(); i++) {
			if (coordvec[i] >= m_nGridDim[i]) {
				_EXCEPTIONT("Coordinate vector out of range");
			}
			ix = ix + i * d;
			d = d * m_nGridDim[i];
		}

		return ix;
	}

public:
	///	<summary>
	///		Longitude of each grid point (in radians).
	///	</summary>
	DataVector<double> m_dLon;

	///	<summary>
	///		Latitude of each grid point (in radians).
	///	</summary>
	DataVector<double> m_dLat;

	///	<summary>
	///		Connectivity of each grid point.
	///	</summary>
	std::vector< std::vector<int> > m_vecConnectivity;

	///	<summary>
	///		Grid dimensions.
	///	</summary>
	std::vector<size_t> m_nGridDim;
};

///////////////////////////////////////////////////////////////////////////////

#endif

