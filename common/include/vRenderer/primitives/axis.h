/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  axis.h
// Created:  5/2/2011
// Author:  K. Loux
// Description:  Derived from PRIMITVE, this class is used to draw plot axis.
// History:

#ifndef _AXIS_H_
#define _AXIS_H_

// wxWidgets headers
#include <wx/wx.h>

// Local headers
#include "vRenderer/primitives/primitive.h"

// FTGL forward declarations
class FTFont;

class Axis : public Primitive
{
public:
	// Constructor
	Axis(RenderWindow &_renderWindow);

	// Destructor
	~Axis();

	// Mandatory overloads from PRIMITIVE - for creating geometry and testing the
	// validity of this object's parameters
	void GenerateGeometry(void);
	bool HasValidParameters(void);

	// Enumeration for the axis orientations
	enum AxisOrientation
	{
		OrientationBottom,
		OrientationTop,
		OrientationLeft,
		OrientationRight
	};

	// Enumeration for the tick styles
	enum TickStyle
	{
		TickStyleThrough,
		TickStyleInside,
		TickStyleOutside,
		TickStyleNone
	};

	// Set option methods
	void SetOrientation(const AxisOrientation &_orientation) { orientation = _orientation; modified = true; };
	void SetMinimum(const double &_minimum) { minimum = _minimum; modified = true; };
	void SetMaximum(const double &_maximum) { maximum = _maximum; modified = true; };
	void SetMajorResolution(const double &_majorResolution) { majorResolution = _majorResolution; modified = true; };
	void SetMinorResolution(const double &_minorResolution) { minorResolution = _minorResolution; modified = true; };
	void SetGrid(const bool &_grid) { grid = _grid; modified = true; };
	void SetLabel(wxString _label) { label = _label; modified = true; };
	void SetFont(FTFont *_font) { font = _font; modified = true; };
	void SetGridColor(const Color &_gridColor) { gridColor = _gridColor; modified = true; };
	void SetTickStyle(const TickStyle &_tickStyle) { tickStyle = _tickStyle; modified = true; };
	void SetTickSize(const int &_tickSize) { tickSize = _tickSize; modified = true; };
	void SetOffsetFromWindowEdge(const unsigned int &offset) { offsetFromWindowEdge = offset; modified = true; };
	
	void SetAxisAtMinEnd(const Axis *min) { minAxis = min; modified = true; };
	void SetAxisAtMaxEnd(const Axis *max) { maxAxis = max; modified = true; };
	void SetOppositeAxis(const Axis *opposite) { oppositeAxis = opposite; modified = true; };

	// Get option methods
	inline double GetMinimum(void) const { return minimum; };
	inline double GetMaximum(void) const { return maximum; };
	bool IsHorizontal(void) const;
	inline unsigned int GetOffsetFromWindowEdge(void) const { return offsetFromWindowEdge; };
	inline bool GetGrid(void) const { return grid; };
	inline Color GetGridColor(void) const { return gridColor; };
	
	inline const Axis* GetAxisAtMinEnd(void) const { return minAxis; };
	inline const Axis* GetAxisAtMaxEnd(void) const { return maxAxis; };
	inline const Axis* GetOppositeAxis(void) const { return oppositeAxis; };
	
	inline wxString GetLabel(void) const { return label; };

private:
	// This object's orientation
	AxisOrientation orientation;

	// This object's range
	double minimum;
	double maximum;

	// For the tick and grid spacing
	double majorResolution;
	double minorResolution;

	// The tick options
	TickStyle tickStyle;
	bool grid;
	int tickSize;

	// Color of the grid
	Color gridColor;

	// Distance for edge of plot render window to the axis
	unsigned int offsetFromWindowEdge;// [pixles]
	
	// Pointers to the axes at either end of this axis
	const Axis *minAxis;
	const Axis *maxAxis;
	const Axis *oppositeAxis;

	// The axis label and font
	wxString label;
	FTFont *font;
};

#endif// _AXIS_H_