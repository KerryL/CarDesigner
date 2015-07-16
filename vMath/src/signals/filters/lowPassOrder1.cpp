/*===================================================================================
                                    CarDesigner
                           Copyright Kerry R. Loux 2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  lowPassOrder1.cpp
// Created:  5/16/2011
// Author:  K. Loux
// Description:  First order low-pass digital filter.
// History:

// Local headers
#include "vMath/signals/filters/lowPassOrder1.h"
#include "vMath/carMath.h"

//==========================================================================
// Class:			LowPassFirstOrderFilter
// Function:		LowPassFirstOrderFilter
//
// Description:		Constructor for the LowPassFirstOrderFilter class.
//
// Input Arguments:
//		cutoffFrequency	= const double& specifying the cutoff frequency [Hz]
//		_sampleRate		= const double& specifying the sampling rate [Hz]
//		initialValue	= const double& specifying initial conditions for this filter
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
LowPassFirstOrderFilter::LowPassFirstOrderFilter(const double& cutoffFrequency,
	const double& sampleRate, const double& initialValue) : FilterBase(sampleRate)
{
	// Allocate and determine the coefficients for the filter
	u = new double[2];
	a = new double[1];

	y = new double[2];
	b = new double[1];

	double sampleTime = 1.0 / sampleRate;// [sec]
	double cutoffRadians = 2.0 * VVASEMath::Pi * cutoffFrequency;// [rad/sec]

	a[0] = sampleTime * cutoffRadians;
	// a1 = a0, so we don't store it

	double b0 = a[0] + 2.0;
	b[0] = a[0] - 2.0;// Actually b1, but we're not storing b0

	// Scale the coefficients so b0 = 1;
	a[0] /= b0;
	b[0] /= b0;

	// Initialize the filter
	Initialize(initialValue);
}

//==========================================================================
// Class:			LowPassFirstOrderFilter
// Function:		LowPassFirstOrderFilter
//
// Description:		Copy constructor for the LowPassFirstOrderFilter class.
//
// Input Arguments:
//		f	= const LowPassFirstOrderFilter& to be copied
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
LowPassFirstOrderFilter::LowPassFirstOrderFilter(const LowPassFirstOrderFilter &f) : FilterBase(sampleRate)
{
	// Copy from the argument to this
	*this = f;
}

//==========================================================================
// Class:			LowPassFirstOrderFilter
// Function:		Initialize
//
// Description:		Initialized (or re-initializes) the filter to the specified value.
//
// Input Arguments:
//		initialValue	= const double& specifying initial conditions for this filter
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void LowPassFirstOrderFilter::Initialize(const double &initialValue)
{
	y[0] = initialValue;
	y[1] = initialValue;

	u[0] = initialValue;
	u[1] = initialValue;
}

//==========================================================================
// Class:			LowPassFirstOrderFilter
// Function:		Apply
//
// Description:		Applies the filter.
//
// Input Arguments:
//		in	= const double& specifying the raw data input to the filter
//
// Output Arguments:
//		None
//
// Return Value:
//		double indicating the newly filtered data (same units as input data)
//
//==========================================================================
double LowPassFirstOrderFilter::Apply(const double &in)
{
	// Shift the inputs one space
	u[1] = u[0];
	u[0] = in;

	y[1] = y[0];
	y[0] = (u[0] + u[1]) * a[0] - y[1] * b[0];

	return y[0];
}

//==========================================================================
// Class:			LowPassFirstOrderFilter
// Function:		operator=
//
// Description:		Assignment operator.
//
// Input Arguments:
//		f	=	const LowPassFirstOrderFilter&
//
// Output Arguments:
//		None
//
// Return Value:
//		LowPassFirstOrderFilter&, reference to this
//
//==========================================================================
LowPassFirstOrderFilter& LowPassFirstOrderFilter::operator = (const LowPassFirstOrderFilter &f)
{
	// Check for self assignment
	if (this == &f)
		return *this;

	// Assign member elements
	u[0] = f.u[0];
	u[1] = f.u[1];

	a[0] = f.a[0];

	y[0] = f.y[0];
	y[1] = f.y[1];

	b[0] = f.b[0];

	return *this;
}