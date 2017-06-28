/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2016
===================================================================================*/

// File:  differential.h
// Date:  11/6/2007
// Author:  K. Loux
// Desc:  Contains class declaration for DIFFERENTIAL class.

#ifndef DIFFERENTIAL_H_
#define DIFFERENTIAL_H_

namespace VVASE
{

// Local forward declarations
class BinaryReader;
class BinaryWriter;

class Differential
{
public:
	Differential();
	Differential(const double& biasRatio);
	Differential(const Differential &differential);
	~Differential();

	// File read/write functions
	void Write(BinaryWriter& file) const;
	void Read(BinaryReader& file, const int& fileVersion);

	// Overloaded operators
	Differential& operator=(const Differential &differential);

	double biasRatio;
};

}// namespace VVASE

#endif// DIFFERENTIAL_H_
