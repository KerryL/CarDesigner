/*===================================================================================
                                    CarDesigner
                           Copyright Kerry R. Loux 2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  lowPassOrder1.h
// Created:  5/16/2011
// Author:  K. Loux
// Description:  First order low-pass digital filter.
// History:

#ifndef LOW_PASS_ORDER1_H_
#define LOW_PASS_ORDER1_H_

// Local headers
#include "vMath/signals/filters/filterBase.h"

class LowPassFirstOrderFilter : public FilterBase
{
public:
	// Constructor
	LowPassFirstOrderFilter(const double& cutoffFrequency,
		const double& sampleRate, const double& initialValue = 0.0);
	LowPassFirstOrderFilter(const LowPassFirstOrderFilter &f);

	// Mandatory overloads from FilterBase
	// Resets all internal variables to initialize the filter to the specified value
	virtual void Initialize(const double &initialValue);

	// Main method for filtering incoming data
	virtual double Apply(const double &in);

	// Operators
	LowPassFirstOrderFilter& operator=(const LowPassFirstOrderFilter &f);
};

#endif// LOW_PASS_ORDER1_H_