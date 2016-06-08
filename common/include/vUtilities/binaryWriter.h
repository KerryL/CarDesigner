/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2016

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  binaryWriter.h
// Created:   6/8/2016
// Author:  K. Loux
// Description:  Class for assisting writing of binary files.  This object is
//               intended to provide consistency for file I/O between 32 and 64 bit
//               platforms.

#ifndef BINARY_WRITER_H_
#define BINARY_WRITER_H_

// Standard C++ headers
#include <string>
#include <fstream>
#include <vector>

// Local headers
#include "vUtilities/wheelSetStructures.h"

// Local forward delcarations
class Vector;

class BinaryWriter
{
public:
	BinaryWriter(std::ofstream& file);

	bool Write(const std::string& v);
	bool Write(const short& v);
	bool Write(const int& v);
	bool Write(const long& v);
	bool Write(const long long& v);
	bool Write(const unsigned short& v);
	bool Write(const unsigned int& v);
	bool Write(const unsigned long& v);
	bool Write(const unsigned long long& v);
	bool Write(const float& v);
	bool Write(const double& v);
	bool Write(const long double& v);
	bool Write(const bool& v);

	bool Write(const Vector& v);

	template<typename T>
	bool Write(const CornerSet<T>& v);

	template<typename T>
	bool Write(const EndSet<T>& v);

	template<typename T>
	bool Write(const std::vector<T>& v);

private:
	bool Write8Bit(const char& v);
	bool Write16Bit(const char& v);
	bool Write32Bit(const char& v);
	bool Write64Bit(const char& v);
};

#endif// BINARY_WRITER_H_