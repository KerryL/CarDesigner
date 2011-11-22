/*===================================================================================
                                    CarDesigner
                           Copyright Kerry R. Loux 2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  highPassOrder1.h
// Created:  5/16/2011
// Author:  K. Loux
// Description:  First order high-pass digital filter.
// History:

#ifndef _HIGH_PASS_ORDER1_H_
#define _HIGH_PASS_ORDER1_H_

// Local headers
#include "vMath/signals/filters/filterBase.h"

class HighPassFirstOrderFilter : public FilterBase
{
public:
	// Constructor
	HighPassFirstOrderFilter(const double& cutoffFrequency,
		const double& sampleRate, const double& initialValue = 0.0);
	HighPassFirstOrderFilter(const HighPassFirstOrderFilter &f);

	// Mandatory overloads from FilterBase
	// Resets all internal variables to initialize the filter to the specified value
	virtual void Initialize(const double &initialValue);

	// Main method for filtering incoming data
	virtual double Apply(const double &_u);

	// Operators
	HighPassFirstOrderFilter& operator = (const HighPassFirstOrderFilter &f);
};

#endif// _HIGH_PASS_ORDER1_H_