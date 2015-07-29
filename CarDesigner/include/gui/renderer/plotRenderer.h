/*===================================================================================
                                    CarDesigner
                           Copyright Kerry R. Loux 2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  plotRenderer.h
// Created:  5/4/2011
// Author:  K. Loux
// Description:  Derived from RenderWindow, this class is used to display plots on
//				 the screen.
// History:

#ifndef PLOT_RENDERER_H_
#define PLOT_RENDERER_H_

// Standard C++ headers
#include <stack>
#include <vector>

// Local headers
#include "vRenderer/renderWindow.h"
#include "vRenderer/primitives/legend.h"

// wxWidgets forward declarations
class wxString;

class PlotPanel;
typedef PlotPanel PlotOwner;

// Local forward declarations
class PlotObject;
class Dataset2D;
class ZoomBox;
class PlotCursor;

class PlotRenderer : public RenderWindow
{
public:
	PlotRenderer(wxWindow &wxParent, PlotOwner &plotOwner, wxWindowID id, int args[]);
	~PlotRenderer();

	// Gets properties for actors
	bool GetBottomMajorGrid() const;
	bool GetLeftMajorGrid() const;
	bool GetRightMajorGrid() const;

	bool GetBottomMinorGrid() const;
	bool GetLeftMinorGrid() const;
	bool GetRightMinorGrid() const;

	Color GetGridColor() const;

	double GetXMin() const;
	double GetXMax() const;
	double GetLeftYMin() const;
	double GetLeftYMax() const;
	double GetRightYMin() const;
	double GetRightYMax() const;

	bool GetXLogarithmic() const;
	bool GetLeftLogarithmic() const;
	bool GetRightLogarithmic() const;

	bool GetXAxisZoomed() const;

	// Sets properties for actors
	void SetMajorGridOn();
	void SetMajorGridOff();
	void SetMinorGridOn();
	void SetMinorGridOff();

	void SetBottomMajorGrid(const bool &grid);
	void SetLeftMajorGrid(const bool &grid);
	void SetRightMajorGrid(const bool &grid);
	void SetBottomMinorGrid(const bool &grid);
	void SetLeftMinorGrid(const bool &grid);
	void SetRightMinorGrid(const bool &grid);

	void SetBottomMajorResolution(const double &resolution);
	void SetLeftMajorResolution(const double &resolution);
	void SetRightMajorResolution(const double &resolution);

	double GetBottomMajorResolution() const;
	double GetLeftMajorResolution() const;
	double GetRightMajorResolution() const;

	void SetGridColor(const Color &color);

	void SetCurveProperties(const unsigned int &index, const Color &color,
		const bool &visible, const bool &rightAxis, const double &lineSize,
		const int &markerSize);
	void SetXLimits(const double &min, const double &max);
	void SetLeftYLimits(const double &min, const double &max);
	void SetRightYLimits(const double &min, const double &max);

	void SetXLabel(wxString text);
	void SetLeftYLabel(wxString text);
	void SetRightYLabel(wxString text);
	void SetTitle(wxString text);

	wxString GetXLabel() const;
	wxString GetLeftYLabel() const;
	wxString GetRightYLabel() const;
	wxString GetTitle() const;

	void AddCurve(const Dataset2D &data);
	void RemoveAllCurves();
	void RemoveCurve(const unsigned int &index);

	void AutoScale();
	void AutoScaleBottom();
	void AutoScaleLeft();
	void AutoScaleRight();

	void SetXLogarithmic(const bool &log);
	void SetLeftLogarithmic(const bool &log);
	void SetRightLogarithmic(const bool &log);

	bool GetMajorGridOn();
	bool GetMinorGridOn();
	
	bool LegendIsVisible();
	void SetLegendOn();
	void SetLegendOff();
	void UpdateLegend(const std::vector<Legend::LegendEntryInfo> &entries);

	// Called to update the screen
	void UpdateDisplay();

	bool GetLeftCursorVisible() const;
	bool GetRightCursorVisible() const;
	double GetLeftCursorValue() const;
	double GetRightCursorValue() const;

	void UpdateCursors();

	PlotOwner *GetPlotOwner() { return &plotOwner; }

	void SaveCurrentZoom();
	void ClearZoomStack();

	static const unsigned int maxXTicks;
	static const unsigned int maxYTicks;
	static double ComputeTickSpacing(const double &min, const double &max, const int &maxTicks);

	enum CurveQuality
	{
		QualityAlwaysLow = 0,
		QualityHighWrite = 1 << 0,
		QualityHighDrag = 1 << 1,
		QualityHighStatic = 1 << 2,
		QualityAlwaysHigh = QualityHighWrite | QualityHighDrag | QualityHighStatic
	};

	void SetCurveQuality(const CurveQuality& curveQuality);
	CurveQuality GetCurveQuality() const { return curveQuality; }

	unsigned long long GetTotalPointCount() const;

	wxImage GetImage() const;

private:
	// Called from the PlotRenderer constructor only in order to initialize the display
	void CreateActors();

	PlotOwner &plotOwner;
	PlotObject *plot;

	// Overload of size event
	void OnSize(wxSizeEvent &event);

	// Overload of interaction events
	void OnMouseWheelEvent(wxMouseEvent &event);
	void OnMouseMoveEvent(wxMouseEvent &event);
	void OnRightButtonUpEvent(wxMouseEvent &event);
	void OnLeftButtonUpEvent(wxMouseEvent &event);
	void OnLeftButtonDownEvent(wxMouseEvent &event);
	void OnMiddleButtonUpEvent(wxMouseEvent &event);

	void OnMouseLeaveWindowEvent(wxMouseEvent &event);
	void OnDoubleClickEvent(wxMouseEvent &event);

	ZoomBox *zoomBox;
	PlotCursor *leftCursor;
	PlotCursor *rightCursor;
	Legend *legend;

	bool draggingLeftCursor;
	bool draggingRightCursor;
	bool draggingLegend;

	void ComputePrettyLimits(double &min, double &max, const unsigned int& maxTicks) const;
	void UpdateLegendAnchor();

protected:
	void ProcessZoom(wxMouseEvent &event);
	void ProcessZoomWithBox(wxMouseEvent &event);
	void ProcessPan(wxMouseEvent &event);

	void PanBottomXAxis(wxMouseEvent &event);
	void PanLeftYAxis(wxMouseEvent &event);
	void PanRightYAxis(wxMouseEvent &event);

	void ProcessPlotAreaDoubleClick(const unsigned int &x);
	void ProcessOffPlotDoubleClick(const unsigned int &x, const unsigned int &y);

	void ProcessRightClick(wxMouseEvent &event);
	void ProcessZoomBoxEnd();

	void ForcePointWithinPlotArea(unsigned int &x, unsigned int &y);

	struct Zoom
	{
		double xMin;
		double xMax;
		double xMajor;

		double leftYMin;
		double leftYMax;
		double leftYMajor;

		double rightYMin;
		double rightYMax;
		double rightYMajor;
	};

	std::stack<Zoom> zoom;
	void UndoZoom();
	bool ZoomChanged() const;

	bool ignoreNextMouseMove;
	CurveQuality curveQuality;

	// For the event table
	DECLARE_EVENT_TABLE()
};

#endif// PLOT_RENDERER_H_
