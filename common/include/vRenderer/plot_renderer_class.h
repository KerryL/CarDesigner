/*===================================================================================
                                    DataPlotter
                           Copyright Kerry R. Loux 2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  plot_renderer_class.h
// Created:  5/4/2011
// Author:  K. Loux
// Description:  Derived from RenderWindow, this class is used to display plots on
//				 the screen.
// History:

#ifndef _PLOT_RENDERER_CLASS_H_
#define _PLOT_RENDERER_CLASS_H_

// Local headers
#include "renderer/render_window_class.h"

// wxWidgets forward declarations
class wxString;

// Local forward declarations
class PlotObject;
class MainFrame;
class Dataset2D;
class ZoomBox;
class PlotCursor;

class PlotRenderer : public RenderWindow
{
public:
	// Constructor
	PlotRenderer(MainFrame &_mainFrame, wxWindowID id);

	// Destructor
	~PlotRenderer();

	// Gets properties for actors
	bool GetBottomGrid(void) const;
	bool GetLeftGrid(void) const;
	bool GetRightGrid(void) const;

	Color GetGridColor(void) const;

	double GetXMin(void) const;
	double GetXMax(void) const;
	double GetLeftYMin(void) const;
	double GetLeftYMax(void) const;
	double GetRightYMin(void) const;
	double GetRightYMax(void) const;

	// Sets properties for actors
	void SetGridOn(void);
	void SetGridOff(void);
	void SetBottomGrid(const bool &grid);
	void SetLeftGrid(const bool &grid);
	void SetRightGrid(const bool &grid);

	void SetGridColor(const Color &color);

	void SetCurveProperties(const unsigned int &index, const Color &color,
		const bool &visible, const bool &rightAxis, const unsigned int &size);
	void SetXLimits(const double &min, const double &max);
	void SetLeftYLimits(const double &min, const double &max);
	void SetRightYLimits(const double &min, const double &max);

	void SetXLabel(wxString text);

	void AddCurve(const Dataset2D &data);
	void RemoveAllCurves(void);
	void RemoveCurve(const unsigned int &index);

	void AutoScale(void);
	void AutoScaleBottom(void);
	void AutoScaleLeft(void);
	void AutoScaleRight(void);

	bool GetGridOn(void);

	// Called to update the screen
	void UpdateDisplay(void);

	bool GetLeftCursorVisible(void) const;
	bool GetRightCursorVisible(void) const;
	double GetLeftCursorValue(void) const;
	double GetRightCursorValue(void) const;

	void UpdateCursors(void);

private:
	// Called from the PLOT_RENDERER constructor only in order to initialize the display
	void CreateActors(void);

	// The actors necessary to create the plot
	PlotObject *plot;

	// Parent window
	MainFrame &mainFrame;

	// Overload of size event
	void OnSize(wxSizeEvent &event);

	// Overload of interaction events
	void OnMouseWheelEvent(wxMouseEvent &event);
	void OnMouseMoveEvent(wxMouseEvent &event);
	void OnRightButtonUpEvent(wxMouseEvent &event);
	void OnLeftButtonUpEvent(wxMouseEvent &event);
	void OnLeftButtonDownEvent(wxMouseEvent &event);

	void OnMouseLeaveWindowEvent(wxMouseEvent &event);
	void OnDoubleClickEvent(wxMouseEvent &event);

	ZoomBox *zoomBox;
	PlotCursor *leftCursor;
	PlotCursor *rightCursor;

	bool draggingLeftCursor;
	bool draggingRightCursor;

	double GetCursorValue(const unsigned int &location);

protected:
	// For the event table
	DECLARE_EVENT_TABLE()
};

#endif// _PLOT_RENDERER_CLASS_H_