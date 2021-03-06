/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2016

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  editTiresPanel.h
// Created:  2/19/2009
// Author:  K. Loux
// Description:  Contains the class declaration for the EditTiresPanel class.
// History:

#ifndef EDIT_TIRES_PANEL_H_
#define EDIT_TIRES_PANEL_H_

// wxWidgets headers
#include <wx/wx.h>

// VVASE forward declarations
class EditPanel;
class TireSet;

class EditTiresPanel : public wxScrolledWindow
{
public:
	EditTiresPanel(EditPanel &parent, wxWindowID id, const wxPoint& pos,
		const wxSize& size);
	~EditTiresPanel();

	void UpdateInformation(TireSet *tireSet);

private:
	EditPanel &parent;

	TireSet *currentTireSet;

	void CreateControls();

	static double ConvertSpringInput(const double& value);
	static double ConvertSpringOutput(const double& value);

	// Event IDs
	enum EditTiresEventIds
	{
		TextBoxRightFrontTireDiameter = 500 + wxID_HIGHEST,
		TextBoxRightFrontTireWidth,
		TextBoxRightFrontStiffness,

		TextBoxLeftFrontTireDiameter,
		TextBoxLeftFrontTireWidth,
		TextBoxLeftFrontStiffness,

		TextBoxRightRearTireDiameter,
		TextBoxRightRearTireWidth,
		TextBoxRightRearStiffness,

		TextBoxLeftRearTireDiameter,
		TextBoxLeftRearTireWidth,
		TextBoxLeftRearStiffness
	};

	// Event handlers-----------------------------------------------------
	void TextBoxChangeEvent(wxCommandEvent &event);
	// End event handlers-------------------------------------------------

	// The controls for the static-setup options
	wxTextCtrl *rightFrontTireDiameter;
	wxTextCtrl *rightFrontTireWidth;
	wxTextCtrl *rightFrontTireStiffness;
	wxStaticText *rightFrontDiameterUnitsLabel;
	wxStaticText *rightFrontWidthUnitsLabel;
	wxStaticText *rightFrontStiffnessUnitsLabel;

	wxTextCtrl *leftFrontTireDiameter;
	wxTextCtrl *leftFrontTireWidth;
	wxTextCtrl *leftFrontTireStiffness;
	wxStaticText *leftFrontDiameterUnitsLabel;
	wxStaticText *leftFrontWidthUnitsLabel;
	wxStaticText *leftFrontStiffnessUnitsLabel;

	wxTextCtrl *rightRearTireDiameter;
	wxTextCtrl *rightRearTireWidth;
	wxTextCtrl *rightRearTireStiffness;
	wxStaticText *rightRearDiameterUnitsLabel;
	wxStaticText *rightRearWidthUnitsLabel;
	wxStaticText *rightRearStiffnessUnitsLabel;

	wxTextCtrl *leftRearTireDiameter;
	wxTextCtrl *leftRearTireWidth;
	wxTextCtrl *leftRearTireStiffness;
	wxStaticText *leftRearDiameterUnitsLabel;
	wxStaticText *leftRearWidthUnitsLabel;
	wxStaticText *leftRearStiffnessUnitsLabel;

	DECLARE_EVENT_TABLE();
};

#endif// EDIT_TIRES_PANEL_H_