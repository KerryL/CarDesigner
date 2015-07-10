/*===================================================================================
                                    CarDesigner
                           Copyright Kerry R. Loux 2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  plotRenderer.cpp
// Created:  5/4/2011
// Author:  K. Loux
// Description:  Derived from RenderWindow, this class is used to display plots on
//				 the screen.
// History:

// wxWidgets headers
#include <wx/wx.h>

// Local headers
#include "gui/renderer/plotRenderer.h"
#include "gui/plotObject.h"
#include "gui/plotPanel.h"
#include "vRenderer/primitives/zoomBox.h"
#include "vRenderer/primitives/plotCursor.h"
#include "vRenderer/primitives/axis.h"

//==========================================================================
// Class:			PlotRenderer
// Function:		PlotRenderer
//
// Description:		Constructor for PlotRenderer class.
//
// Input Arguments:
//		_parent		= PlotPanel& reference to this object's parent window
//		args		= int[] NOTE: Under GTK, must contain WX_GL_DOUBLEBUFFER at minimum
//		id			= wxWindowID
//		_mainFrame	= MainFrame&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
PlotRenderer::PlotRenderer(PlotPanel &_parent, wxWindowID id, int args[],
						   MainFrame &_mainFrame)
						   : RenderWindow(_parent, id, args, wxDefaultPosition,
						   wxDefaultSize), mainFrame(_mainFrame), parent(_parent)
{
	// Create the actors
	CreateActors();

	// Set this to a 2D view by default
	SetView3D(false);

	draggingLeftCursor = false;
	draggingRightCursor = false;
}

//==========================================================================
// Class:			PlotRenderer
// Function:		~PlotRenderer
//
// Description:		Destructor for PlotRenderer class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
PlotRenderer::~PlotRenderer()
{
	// Delete the plot object
	delete plot;
	plot = NULL;
}

//==========================================================================
// Class:			PlotRenderer
// Function:		Event Tables
//
// Description:		Event table for the PlotRenderer class.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
BEGIN_EVENT_TABLE(PlotRenderer, RenderWindow)
	EVT_SIZE(PlotRenderer::OnSize)

	// Interaction events
	EVT_MOUSEWHEEL(		PlotRenderer::OnMouseWheelEvent)
	EVT_MOTION(			PlotRenderer::OnMouseMoveEvent)

	EVT_LEAVE_WINDOW(	PlotRenderer::OnMouseLeaveWindowEvent)

	// Click events
	EVT_LEFT_DCLICK(	PlotRenderer::OnDoubleClickEvent)
	EVT_RIGHT_UP(		PlotRenderer::OnRightButtonUpEvent)
	EVT_LEFT_UP(		PlotRenderer::OnLeftButtonUpEvent)
	EVT_LEFT_DOWN(		PlotRenderer::OnLeftButtonDownEvent)
END_EVENT_TABLE()

//==========================================================================
// Class:			PlotRenderer
// Function:		UpdateDisplay
//
// Description:		Updates the displayed plots to match the current data.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::UpdateDisplay(void)
{
	// Update the plot
	plot->Update();
	Refresh();
}

//==========================================================================
// Class:			PLOT_RENDERER
// Function:		CreateActors
//
// Description:		Creates the actors for this plot.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::CreateActors(void)
{
	// Create plot area
	plot = new PlotObject(*this);

	// Also create the zoom box and cursors, even though they aren't drawn yet
	zoomBox = new ZoomBox(*this);
	leftCursor = new PlotCursor(*this, *plot->GetBottomAxis());
	rightCursor = new PlotCursor(*this, *plot->GetBottomAxis());
}

//==========================================================================
// Class:			PlotRenderer
// Function:		OnSize
//
// Description:		Handles EVT_SIZE events for this class.  Required to make
//					the plot size update with the window.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		event	= wxSizeEvent&
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::OnSize(wxSizeEvent &event)
{
	// If the cursors are visible, set them visible again so they get updated
	if (leftCursor->GetIsVisible())
		leftCursor->SetVisibility(true);
	if (rightCursor->GetIsVisible())
		rightCursor->SetVisibility(true);

	// If the object exists, update the display
	UpdateDisplay();

	// Skip this event so the base class OnSize event fires, too
	event.Skip();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		OnMouseWheelEvent
//
// Description:		Event handler for the mouse wheel event.
//
// Input Arguments:
//		event	= wxMouseEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::OnMouseWheelEvent(wxMouseEvent &event)
{
	// If we are a 3D plot, let the default event handlers do their job
	if (view3D)
	{
		event.Skip();
		return;
	}

	// ZOOM in or out
	double zoomScaleX = 0.05;// [% of current scale]
	double zoomScaleY = 0.05;// [% of current scale]

	// If the CTRL key is down (and not SHIFT), only scale the X-axis
	if (event.ControlDown() && !event.ShiftDown())
		zoomScaleY = 0.0;

	// If the SHIFT key is down (and not CTRL), only scale the Y-axis
	else if (event.ShiftDown() && !event.ControlDown())
		zoomScaleX = 0.0;

	// Otherwise, scale both axes
	// FIXME:  Focus the zooming around the cursor
	// Adjust the axis limits
	double xDelta = (plot->GetXMax() - plot->GetXMin()) * zoomScaleX * event.GetWheelRotation() / 120.0;
	double yLeftDelta = (plot->GetLeftYMax() - plot->GetLeftYMin()) * zoomScaleY * event.GetWheelRotation() / 120.0;
	double yRightDelta = (plot->GetLeftYMax() - plot->GetLeftYMin()) * zoomScaleY * event.GetWheelRotation() / 120.0;

	plot->SetXMin(plot->GetXMin() + xDelta);
	plot->SetXMax(plot->GetXMax() - xDelta);
	plot->SetLeftYMin(plot->GetLeftYMin() + yLeftDelta);
	plot->SetLeftYMax(plot->GetLeftYMax() - yLeftDelta);
	plot->SetRightYMin(plot->GetRightYMin() + yRightDelta);
	plot->SetRightYMax(plot->GetRightYMax() - yRightDelta);

	// Update the plot display
	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		OnMouseMoveEvent
//
// Description:		Event handler for the mouse move event.  Only used to
//					capture drag events for rotating, panning, or dollying
//					the scene.
//
// Input Arguments:
//		event	= wxMouseEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::OnMouseMoveEvent(wxMouseEvent &event)
{
	// If we are a 3D plot, let the default event handlers do their job
	if (view3D)
	{
		event.Skip();
		return;
	}

	// Don't perform and actions if this isn't a dragging event
	if (!event.Dragging())
	{
		StoreMousePosition(event);
		return;
	}

	// Are we moving cursors?
	if (draggingLeftCursor)
		leftCursor->SetValue(GetCursorValue(event.GetX()));
	else if (draggingRightCursor)
		rightCursor->SetValue(GetCursorValue(event.GetX()));
	// ZOOM:  Left or Right mouse button + CTRL or SHIFT
	else if ((event.ControlDown() || event.ShiftDown()) && (event.RightIsDown() || event.LeftIsDown()))
	{
		// CTRL for Left Y-zoom
		// SHIFT for Right Y-zoom

		// ZOOM in or out
		double zoomXScale = 0.005 * (event.GetX() - lastMousePosition[0]);// [% of current scale]
		double zoomYScale = 0.005 * (event.GetY() - lastMousePosition[1]);// [% of current scale]

		// FIXME:  Focus the zooming around the cursor
		// Adjust the axis limits
		double xDelta = (plot->GetXMax() - plot->GetXMin()) * zoomXScale;
		double yLeftDelta = (plot->GetLeftYMax() - plot->GetLeftYMin()) * zoomYScale * (int)event.ControlDown();
		double yRightDelta = (plot->GetRightYMax() - plot->GetRightYMin()) * zoomYScale * (int)event.ShiftDown();

		// Left mouse button fixes left and bottom corner, right button fixes right and top corner
		if (event.LeftIsDown())
		{
			plot->SetXMax(plot->GetXMax() - xDelta);
			plot->SetLeftYMax(plot->GetLeftYMax() + yLeftDelta);
			plot->SetRightYMax(plot->GetRightYMax() + yRightDelta);
		}
		else
		{
			plot->SetXMin(plot->GetXMin() - xDelta);
			plot->SetLeftYMin(plot->GetLeftYMin() + yLeftDelta);
			plot->SetRightYMin(plot->GetRightYMin() + yRightDelta);
		}
	}
	// ZOOM WITH BOX: Right mouse button
	else if (event.RightIsDown())
	{
		// If we're not already visible, set the anchor and make us visible
		if (!zoomBox->GetIsVisible())
		{
			zoomBox->SetVisibility(true);
			zoomBox->SetAnchorCorner(lastMousePosition[0], GetSize().GetHeight() - lastMousePosition[1]);
		}

		unsigned int x = event.GetX();
		unsigned int y = event.GetY();

		// Make sure we're still over the plot area - if we're not, draw the box as if we were
		if (x < plot->GetLeftYAxis()->GetOffsetFromWindowEdge())
			x = plot->GetLeftYAxis()->GetOffsetFromWindowEdge();
		else if (x > GetSize().GetWidth() - plot->GetRightYAxis()->GetOffsetFromWindowEdge())
			x = GetSize().GetWidth() - plot->GetRightYAxis()->GetOffsetFromWindowEdge();

		if (y < plot->GetTopAxis()->GetOffsetFromWindowEdge())
			y = plot->GetTopAxis()->GetOffsetFromWindowEdge();
		else if (y > GetSize().GetHeight() - plot->GetBottomAxis()->GetOffsetFromWindowEdge())
			y = GetSize().GetHeight() - plot->GetBottomAxis()->GetOffsetFromWindowEdge();

		// Tell the zoom box where to draw the floaing corner
		zoomBox->SetFloatingCorner(x, GetSize().GetHeight() - y);
	}
	// PAN:  Left mouse button (includes with any buttons not caught above)
	else if (event.LeftIsDown())
	{
		// Determine size of plot within window and scale actions according to the scale of the plot window
		int height = GetSize().GetHeight() - plot->GetBottomAxis()->GetOffsetFromWindowEdge() - plot->GetTopAxis()->GetOffsetFromWindowEdge();
		int width = GetSize().GetWidth() - plot->GetLeftYAxis()->GetOffsetFromWindowEdge() - plot->GetRightYAxis()->GetOffsetFromWindowEdge();

		// Adjust the axis limits
		double xDelta = (plot->GetXMax() - plot->GetXMin()) * (event.GetX() - lastMousePosition[0]) / width;
		double yLeftDelta = (plot->GetLeftYMax() - plot->GetLeftYMin()) * (event.GetY() - lastMousePosition[1]) / height;
		double yRightDelta = (plot->GetRightYMax() - plot->GetRightYMin()) * (event.GetY() - lastMousePosition[1]) / height;

		// Adjust the deltas so we can't zoom by scrolling (could occur if only one side was against a limit)
		/*if (plot->GetXMin() - xDelta < plot->GetXMinOriginal())
			xDelta = plot->GetXMin() - plot->GetXMinOriginal();
		if (plot->GetXMax() - xDelta > plot->GetXMaxOriginal())
			xDelta = plot->GetXMax() - plot->GetXMaxOriginal();
		if (plot->GetLeftYMin() + yLeftDelta < plot->GetLeftYMinOriginal())
			yLeftDelta = plot->GetLeftYMinOriginal() - plot->GetLeftYMin();
		if (plot->GetLeftYMax() + yLeftDelta > plot->GetLeftYMaxOriginal())
			yLeftDelta = plot->GetLeftYMaxOriginal() - plot->GetLeftYMax();
		if (plot->GetRightYMin() + yRightDelta < plot->GetRightYMinOriginal())
			yRightDelta = plot->GetRightYMinOriginal() - plot->GetRightYMin();
		if (plot->GetRightYMax() + yRightDelta > plot->GetRightYMaxOriginal())
			yRightDelta = plot->GetRightYMaxOriginal() - plot->GetRightYMax();*/
		// FIXME:  Is this supposed to be commented out, or do we want to uncomment it?

		plot->SetXMin(plot->GetXMin() - xDelta);
		plot->SetXMax(plot->GetXMax() - xDelta);
		plot->SetLeftYMin(plot->GetLeftYMin() + yLeftDelta);
		plot->SetLeftYMax(plot->GetLeftYMax() + yLeftDelta);
		plot->SetRightYMin(plot->GetRightYMin() + yRightDelta);
		plot->SetRightYMax(plot->GetRightYMax() + yRightDelta);
	}
	else// Not recognized
	{
		StoreMousePosition(event);
		return;
	}

	// Store the last mouse position
	StoreMousePosition(event);

	// Update the display
	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		OnRightButtonUpEvent
//
// Description:		Handles end of zoom-by-box events.
//
// Input Arguments:
//		event	= wxMouseEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::OnRightButtonUpEvent(wxMouseEvent &event)
{
	// If the zoom box is not visible, process this like a right click event
	if (!zoomBox->GetIsVisible())
	{
		// Determine the context
		PlotPanel::PlotContext context;
		unsigned int x = event.GetX();
		unsigned int y = event.GetY();
		if (x < plot->GetLeftYAxis()->GetOffsetFromWindowEdge() &&
			y > plot->GetTopAxis()->GetOffsetFromWindowEdge() &&
			y < GetSize().GetHeight() - plot->GetBottomAxis()->GetOffsetFromWindowEdge())
			context = PlotPanel::plotContextLeftYAxis;
		else if (x > GetSize().GetWidth() - plot->GetRightYAxis()->GetOffsetFromWindowEdge() &&
			y > plot->GetTopAxis()->GetOffsetFromWindowEdge() &&
			y < GetSize().GetHeight() - plot->GetBottomAxis()->GetOffsetFromWindowEdge())
			context = PlotPanel::plotContextRightYAxis;
		else if (y > GetSize().GetHeight() - plot->GetBottomAxis()->GetOffsetFromWindowEdge() &&
			x > plot->GetLeftYAxis()->GetOffsetFromWindowEdge() &&
			x < GetSize().GetWidth() - plot->GetRightYAxis()->GetOffsetFromWindowEdge())
			context = PlotPanel::plotContextXAxis;
		else
			context = PlotPanel::plotContextPlotArea;

		// Display the context menu (further events handled by MainFrame)
		parent.CreatePlotContextMenu(GetPosition() + event.GetPosition(), context);

		return;
	}

	// Hide the zoom box
	zoomBox->SetVisibility(false);

	// Make sure the box isn't too small
	int limit = 5;// [pixels]
	if (abs(int(zoomBox->GetXAnchor() - zoomBox->GetXFloat())) > limit &&
		abs(int(zoomBox->GetYAnchor() - zoomBox->GetYFloat())) > limit)
	{
		// Determine the new zoom range by interpolation
		// Remember: OpenGL uses Bottom Left as origin, normal windows use Top Left as origin
		int xCoordLeft = plot->GetLeftYAxis()->GetOffsetFromWindowEdge();
		int xCoordRight = GetSize().GetWidth() - plot->GetRightYAxis()->GetOffsetFromWindowEdge();
		int yCoordBottom = plot->GetBottomAxis()->GetOffsetFromWindowEdge();
		int yCoordTop = GetSize().GetHeight() - plot->GetTopAxis()->GetOffsetFromWindowEdge();

		int leftX;
		int rightX;
		int bottomY;
		int topY;
		if (zoomBox->GetXAnchor() > zoomBox->GetXFloat())
		{
			leftX = zoomBox->GetXFloat();
			rightX = zoomBox->GetXAnchor();
		}
		else
		{
			leftX = zoomBox->GetXAnchor();
			rightX = zoomBox->GetXFloat();
		}

		if (zoomBox->GetYAnchor() > zoomBox->GetYFloat())
		{
			bottomY = zoomBox->GetYFloat();
			topY = zoomBox->GetYAnchor();
		}
		else
		{
			bottomY = zoomBox->GetYAnchor();
			topY = zoomBox->GetYFloat();
		}

		// Do the interpolation
		double xMin = plot->GetXMin()
			+ double(leftX - xCoordLeft) / double(xCoordRight - xCoordLeft)
			* (plot->GetXMax() - plot->GetXMin());
		double xMax = plot->GetXMin()
			+ double(rightX - xCoordLeft) / double(xCoordRight - xCoordLeft)
			* (plot->GetXMax() - plot->GetXMin());
		double yLeftMin = plot->GetLeftYMin()
			+ double(bottomY - yCoordBottom) / double(yCoordTop - yCoordBottom)
			* (plot->GetLeftYMax() - plot->GetLeftYMin());
		double yLeftMax = plot->GetLeftYMin()
			+ double(topY - yCoordBottom) / double(yCoordTop - yCoordBottom)
			* (plot->GetLeftYMax() - plot->GetLeftYMin());
		double yRightMin = plot->GetRightYMin()
			+ double(bottomY - yCoordBottom) / double(yCoordTop - yCoordBottom)
			* (plot->GetRightYMax() - plot->GetRightYMin());
		double yRightMax = plot->GetRightYMin()
			+ double(topY - yCoordBottom) / double(yCoordTop - yCoordBottom)
			* (plot->GetRightYMax() - plot->GetRightYMin());

		// Assign the results
		SetXLimits(xMin, xMax);
		SetLeftYLimits(yLeftMin, yLeftMax);
		SetRightYLimits(yRightMin, yRightMax);
	}

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetGridOn
//
// Description:		Returns status of the grid lines.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for visible, false for hidden
//
//==========================================================================
bool PlotRenderer::GetGridOn(void)
{
	return plot->GetGrid();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetGridOn
//
// Description:		Turns on plot grid.
//
// Input Arguments:
//		grid	= const bool&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetGridOn(const bool &grid)
{
	plot->SetGrid(grid);
	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetGridOn
//
// Description:		Turns off plot grid.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetGridOff()
{
	plot->SetGrid(false);
	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetBottomGrid
//
// Description:		Returns the status of the bottom grid.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool indicating status of bottom grid
//
//==========================================================================
bool PlotRenderer::GetBottomGrid(void) const
{
	return plot->GetBottomAxis()->GetGrid();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetLeftGrid
//
// Description:		Returns the status of the left grid.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool indicating status of left grid
//
//==========================================================================
bool PlotRenderer::GetLeftGrid(void) const
{
	return plot->GetLeftYAxis()->GetGrid();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetRightGrid
//
// Description:		Returns the status of the right grid.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool indicating status of right grid
//
//==========================================================================
bool PlotRenderer::GetRightGrid(void) const
{
	return plot->GetRightYAxis()->GetGrid();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetBottomGrid
//
// Description:		Sets the status of the bottom axis' grid.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetBottomGrid(const bool &grid)
{
	plot->SetXGrid(grid);

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetLeftGrid
//
// Description:		Sets the status of the left axis' grid.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetLeftGrid(const bool &grid)
{
	plot->SetLeftYGrid(grid);

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetRightGrid
//
// Description:		Sets the status of the right axis' grid.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetRightGrid(const bool &grid)
{
	plot->SetRightYGrid(grid);

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetCurveProperties
//
// Description:		Sets properties for the specified curve object.
//
// Input Arguments:
//		index		= const unsigned int& specifying the curve
//		color		= const Color& of the curve
//		visible		= const bool& indiciating whether or not the curve is to
//					  be drawn
//		rightAxis	= const bool& indicating whether the curve should be tied
//					  to the left or right axis
//		size		= const unsigned int&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetCurveProperties(const unsigned int &index, const Color &color,
									  const bool &visible, const bool &rightAxis,
									  const unsigned int &size)
{
	plot->SetCurveProperties(index, color, visible, rightAxis, size);
	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetXLimits
//
// Description:		Sets the axis limits for the X axis.
//
// Input Arguments:
//		min	= const double&
//		max = const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetXLimits(const double &min, const double &max)
{
	if (max > min)
	{
		plot->SetXMax(max);
		plot->SetXMin(min);
	}
	else
	{
		plot->SetXMax(min);
		plot->SetXMin(max);
	}

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetLeftYLimits
//
// Description:		Sets the axis limits for the left Y axis.
//
// Input Arguments:
//		min	= const double&
//		max = const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetLeftYLimits(const double &min, const double &max)
{
	if (max > min)
	{
		plot->SetLeftYMax(max);
		plot->SetLeftYMin(min);
	}
	else
	{
		plot->SetLeftYMax(min);
		plot->SetLeftYMin(max);
	}

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetRightYLimits
//
// Description:		Sets the axis limits for the right Y axis.
//
// Input Arguments:
//		min	= const double&
//		max = const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetRightYLimits(const double &min, const double &max)
{
	if (max > min)
	{
		plot->SetRightYMax(max);
		plot->SetRightYMin(min);
	}
	else
	{
		plot->SetRightYMax(min);
		plot->SetRightYMin(max);
	}

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		AddCurve
//
// Description:		Sets properties for the specified curve object.
//
// Input Arguments:
//		data	= const Dataset2D& to be plotted
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::AddCurve(const Dataset2D &data)
{
	plot->AddCurve(data);
}

//==========================================================================
// Class:			PlotRenderer
// Function:		RemoveAllCurves
//
// Description:		Removes all curves from the plot.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::RemoveAllCurves()
{
	plot->RemoveExistingPlots();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		RemoveCurve
//
// Description:		Removes all curves from the plot.
//
// Input Arguments:
//		index	= const unsigned int& specifying the curve to be removed
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::RemoveCurve(const unsigned int& index)
{
	plot->RemovePlot(index);
}

//==========================================================================
// Class:			PlotRenderer
// Function:		AutoScale
//
// Description:		Enables auto-scaling of the axes.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::AutoScale()
{
	plot->ResetAutoScaling();

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		AutoScaleBottom
//
// Description:		Enables auto-scaling of the bottom axis.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::AutoScaleBottom()
{
	plot->SetAutoScaleBottom();

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		AutoScaleLeft
//
// Description:		Enables auto-scaling of the left axis.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::AutoScaleLeft()
{
	plot->SetAutoScaleLeft();

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		AutoScaleRight
//
// Description:		Enables auto-scaling of the right axis.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::AutoScaleRight()
{
	plot->SetAutoScaleRight();

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetXLabel
//
// Description:		Sets the text for the x-axis label.
//
// Input Arguments:
//		text	= wxString
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetXLabel(wxString text)
{
	plot->SetXLabel(text);

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetLeftYLabel
//
// Description:		Sets the text for the y-axis label.
//
// Input Arguments:
//		text	= wxString
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetLeftYLabel(wxString text)
{
	plot->SetLeftYLabel(text);

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetRightYLabel
//
// Description:		Sets the text for the y-axis label.
//
// Input Arguments:
//		text	= wxString
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetRightYLabel(wxString text)
{
	plot->SetRightYLabel(text);

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetTitle
//
// Description:		Sets the plot title text.
//
// Input Arguments:
//		text	= wxString
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetTitle(wxString text)
{
	plot->SetTitle(text);

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		OnMouseLeaveWindowEvent
//
// Description:		Cleans up some zoom box and cursor items.
//
// Input Arguments:
//		event	= wxMouseEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::OnMouseLeaveWindowEvent(wxMouseEvent& WXUNUSED(event))
{
	// Hide the zoom box (but only if it's not already hidden!)
	if (zoomBox->GetIsVisible())
		zoomBox->SetVisibility(false);

	draggingLeftCursor = false;
	draggingRightCursor = false;

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		OnDoubleClickEvent
//
// Description:		Handles double click events.  Allows user to change axis
//					limits or create a cursor.
//
// Input Arguments:
//		event	= wxMouseEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::OnDoubleClickEvent(wxMouseEvent &event)
{
	unsigned int x = event.GetX();
	unsigned int y = event.GetY();

	// If the click is within the plot area, move a cursor there and make it visible
	if (x > plot->GetLeftYAxis()->GetOffsetFromWindowEdge() &&
		x < GetSize().GetWidth() - plot->GetRightYAxis()->GetOffsetFromWindowEdge() &&
		y > plot->GetTopAxis()->GetOffsetFromWindowEdge() &&
		y < GetSize().GetHeight() - plot->GetBottomAxis()->GetOffsetFromWindowEdge())
	{
		double value = GetCursorValue(x);

		if (!leftCursor->GetIsVisible())
		{
			leftCursor->SetVisibility(true);
			leftCursor->SetValue(value);
		}
		else if (!rightCursor->GetIsVisible())
		{
			rightCursor->SetVisibility(true);
			rightCursor->SetValue(value);
		}
		else
		{
			// Both cursors are visible - move the closer one to the click spot
			// FIXME:  Another option is to always alternate which one was moved?
			if (fabs(leftCursor->GetValue() - value) < fabs(rightCursor->GetValue() - value))
				leftCursor->SetValue(value);
			else
				rightCursor->SetValue(value);
		}
	}
	else
	{
		// Determine the context
		PlotPanel::PlotContext context;
		if (x < plot->GetLeftYAxis()->GetOffsetFromWindowEdge() &&
			y > plot->GetTopAxis()->GetOffsetFromWindowEdge() &&
			y < GetSize().GetHeight() - plot->GetBottomAxis()->GetOffsetFromWindowEdge())
			context = PlotPanel::plotContextLeftYAxis;
		else if (x > GetSize().GetWidth() - plot->GetRightYAxis()->GetOffsetFromWindowEdge() &&
			y > plot->GetTopAxis()->GetOffsetFromWindowEdge() &&
			y < GetSize().GetHeight() - plot->GetBottomAxis()->GetOffsetFromWindowEdge())
			context = PlotPanel::plotContextRightYAxis;
		else if (y > GetSize().GetHeight() - plot->GetBottomAxis()->GetOffsetFromWindowEdge() &&
			x > plot->GetLeftYAxis()->GetOffsetFromWindowEdge() &&
			x < GetSize().GetWidth() - plot->GetRightYAxis()->GetOffsetFromWindowEdge())
			context = PlotPanel::plotContextXAxis;
		else
			context = PlotPanel::plotContextPlotArea;

		// Display the dialog
		parent.DisplayAxisRangeDialog(context);
	}

	UpdateDisplay();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetCursorValue
//
// Description:		Gets the cursor value (plot units) given the position of
//					the cursor (screen units).
//
// Input Arguments:
//		location	= const unsigned int&
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double PlotRenderer::GetCursorValue(const unsigned int &location)
{
	unsigned int width = GetSize().GetWidth() - plot->GetLeftYAxis()->GetOffsetFromWindowEdge() - plot->GetRightYAxis()->GetOffsetFromWindowEdge();
	return double(location - plot->GetLeftYAxis()->GetOffsetFromWindowEdge()) / (double)width *
		(plot->GetBottomAxis()->GetMaximum() - plot->GetBottomAxis()->GetMinimum()) + plot->GetBottomAxis()->GetMinimum();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		OnMouseLeftDownEvent
//
// Description:		Checks to see if the user is dragging a cursor.
//
// Input Arguments:
//		event	= wxMouseEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::OnLeftButtonDownEvent(wxMouseEvent &event)
{
	// Check to see if we're on a cursor
	if (leftCursor->IsUnder(event.GetX()))
		draggingLeftCursor = true;
	else if (rightCursor->IsUnder(event.GetX()))
		draggingRightCursor = true;
}

//==========================================================================
// Class:			PlotRenderer
// Function:		OnLeftButtonUpEvent
//
// Description:		Makes sure we stop dragging when we stop clicking.
//
// Input Arguments:
//		event	= wxMouseEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::OnLeftButtonUpEvent(wxMouseEvent& WXUNUSED(event))
{
	draggingLeftCursor = false;
	draggingRightCursor = false;
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetLeftCursorVisible
//
// Description:		Returns status of left cursor visibility flag.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool indicating status of left cursor visibility flag
//
//==========================================================================
bool PlotRenderer::GetLeftCursorVisible(void) const
{
	return leftCursor->GetIsVisible();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetRightCursorVisible
//
// Description:		Returns status of right cursor visibility flag.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool indicating status of right cursor visibility flag
//
//==========================================================================
bool PlotRenderer::GetRightCursorVisible(void) const
{
	return rightCursor->GetIsVisible();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetLeftCursorValue
//
// Description:		Returns x-value of left cursor.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double PlotRenderer::GetLeftCursorValue(void) const
{
	return leftCursor->GetValue();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetRightCursorValue
//
// Description:		Makes sure we stop dragging when we stop clicking.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double PlotRenderer::GetRightCursorValue(void) const
{
	return rightCursor->GetValue();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		UpdateCursors
//
// Description:		Updates the cursor calculations.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::UpdateCursors(void)
{
	// Tell the cursors they need to recalculate
	leftCursor->SetModified();
	rightCursor->SetModified();

	// Calculations are performed on Draw
	leftCursor->Draw();
	rightCursor->Draw();

	Refresh();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetXMin
//
// Description:		Returns the minimum value of the X-axis.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		double indicating the minimum value of the X-axis
//
//==========================================================================
double PlotRenderer::GetXMin(void) const
{
	return plot->GetBottomAxis()->GetMinimum();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetXMin
//
// Description:		Returns the minimum value of the X-axis.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		double indicating the minimum value of the X-axis
//
//==========================================================================
double PlotRenderer::GetXMax(void) const
{
	return plot->GetBottomAxis()->GetMaximum();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetXMin
//
// Description:		Returns the minimum value of the X-axis.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		double indicating the minimum value of the X-axis
//
//==========================================================================
double PlotRenderer::GetLeftYMin(void) const
{
	return plot->GetLeftYAxis()->GetMinimum();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetXMin
//
// Description:		Returns the minimum value of the X-axis.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		double indicating the minimum value of the X-axis
//
//==========================================================================
double PlotRenderer::GetLeftYMax(void) const
{
	return plot->GetLeftYAxis()->GetMaximum();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetXMin
//
// Description:		Returns the minimum value of the X-axis.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		double indicating the minimum value of the X-axis
//
//==========================================================================
double PlotRenderer::GetRightYMin(void) const
{
	return plot->GetRightYAxis()->GetMinimum();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetXMin
//
// Description:		Returns the minimum value of the X-axis.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		double indicating the minimum value of the X-axis
//
//==========================================================================
double PlotRenderer::GetRightYMax(void) const
{
	return plot->GetRightYAxis()->GetMaximum();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		GetGridColor
//
// Description:		Returns the color of the gridlines for this plot.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		Color
//
//==========================================================================
Color PlotRenderer::GetGridColor(void) const
{
	return plot->GetGridColor();
}

//==========================================================================
// Class:			PlotRenderer
// Function:		SetGridColor
//
// Description:		Sets the color of the gridlines for this plot.
//
// Input Arguments:
//		color	= const Color&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void PlotRenderer::SetGridColor(const Color &color)
{
	plot->SetGridColor(color);
}