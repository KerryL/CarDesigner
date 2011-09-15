/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2010

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  complex_class.h
// Created:  3/30/2008
// Author:  K. Loux
// Description:  Contains class declaration for complex number class.
// History:
//	4/11/2009	- Changed all functions to take addresses of and use const, K. Loux.
//	6/15/2009	- Corrected function signatures for overloaded operators, K. Loux.
//	11/22/2009	- Moved to vMath.lib, K. Loux.

#ifndef _COMPLEX_CLASS_H_
#define _COMPLEX_CLASS_H_

using namespace std;

class COMPLEX
{
public:
	// Constructor
	COMPLEX();
	COMPLEX(const double &_Real, const double &_Imaginary);

	// Destructor
	~COMPLEX();

	// Prints the value to a string
	wxString Print(void) const;

	// Gets the complex conjugate of this object
	const COMPLEX GetConjugate(void) const;

	// Operators
	const COMPLEX operator + (const COMPLEX &Complex) const;
	const COMPLEX operator - (const COMPLEX &Complex) const;
	const COMPLEX operator * (const COMPLEX &Complex) const;
	const COMPLEX operator / (const COMPLEX &Complex) const;
	COMPLEX& operator += (const COMPLEX &Complex);
	COMPLEX& operator -= (const COMPLEX &Complex);
	COMPLEX& operator *= (const COMPLEX &Complex);
	COMPLEX& operator /= (const COMPLEX &Complex);
	bool operator == (const COMPLEX &Complex) const;
	bool operator != (const COMPLEX &Complex) const;
	const COMPLEX operator + (const double &Double) const;
	const COMPLEX operator - (const double &Double) const;
	const COMPLEX operator * (const double &Double) const;
	const COMPLEX operator / (const double &Double) const;

	// Raises this object to the specified power
	COMPLEX& ToPower(const double &Power);

	// For streaming the value
	friend ostream &operator << (ostream &WriteOut, const COMPLEX &Complex);

	// The actual data contents of this class
	double Real;
	double Imaginary;

	// Defining the square root of negative 1
	static const COMPLEX I;
};

#endif// _COMPLEX_CLASS_H_