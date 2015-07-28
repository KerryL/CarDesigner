/*===================================================================================
                                    DataPlotter
                          Copyright Kerry R. Loux 2011-2013

                   This code is licensed under the GPLv2 License
                     (http://opensource.org/licenses/GPL-2.0).

===================================================================================*/

// File:  line.h
// Created:  4/2/2015
// Author:  K. Loux
// Description:  Object representing a line, drawn with triangles faded from line
//               color to background color in order to make the lines prettier, be
//               more consistent from platform to platofrm and to support sub-pixel
//               widths.
// History:

#ifndef LINE_H_
#define LINE_H_

// Standard C++ headers
#include <vector>
#include <utility>
#include <cassert>

// Local headers
#include "vRenderer/color.h"

class Line
{
public:
	Line();

	inline void SetPretty(const bool &pretty) { this->pretty = pretty; }
	inline void SetWidth(const double &width) { assert(width > 0.0); halfWidth = 0.5 * width; }
	inline void SetLineColor(const Color &color) { lineColor = color; }
	inline void SetBackgroundColor(const Color &color) { backgroundColor = color; }
	inline void SetBackgroundColorForAlphaFade() { backgroundColor = lineColor; backgroundColor.SetAlpha(0.0); }

	void Draw(const unsigned int &x1, const unsigned int &y1, const unsigned int &x2,
		const unsigned int &y2) const;
	void Draw(const double &x1, const double &y1, const double &x2, const double &y2) const;
	void Draw(const std::vector<std::pair<unsigned int, unsigned int> > &points) const;
	void Draw(const std::vector<std::pair<double, double> > &points) const;

private:
	static const double fadeDistance;
	double halfWidth;// Due to the fading, setting the half width equal to the width seems to create a nice match for desired line width
	Color lineColor;
	Color backgroundColor;
	bool pretty;

	void ComputeOffsets(const double &x1, const double &y1, const double &x2,
		const double &y2, double& dxLine, double& dyLine, double& dxEdge, double& dyEdge) const;

	struct Offsets
	{
		double dxLine;
		double dyLine;
		double dxEdge;
		double dyEdge;
	};

	void DoUglyDraw(const double &x1, const double &y1, const double &x2, const double &y2) const;
	void DoPrettyDraw(const double &x1, const double &y1, const double &x2, const double &y2) const;
	void DoUglyDraw(const std::vector<std::pair<double, double> > &points) const;
	void DoPrettyDraw(const std::vector<std::pair<double, double> > &points) const;
};

#endif// LINE_H_
