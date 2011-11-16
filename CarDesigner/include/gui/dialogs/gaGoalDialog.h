/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  gaGoalDialog.h
// Created:  7/30/2009
// Author:  K. Loux
// Description:  Dialog for editing goal properties.
// History:

// wxWidgets headers
#include <wx/wx.h>

// CarDesigner headers
#include "vSolver/physics/kinematicOutputs.h"
#include "vSolver/physics/kinematics.h"
#include "vUtilities/convert.h"

// CarDesigner forward declarations
class MAIN_FRAME;

class GA_GOAL_DIALOG : public wxDialog
{
public:
	// Constructor
	GA_GOAL_DIALOG(wxWindow *Parent, const Convert &_Converter, const KinematicOutputs::OutputsComplete &_Output,
		const double &_DesiredValue, const double &_ExpectedDeviation, const double &_Importance,
		const Kinematics::Inputs &_BeforeInputs, const Kinematics::Inputs &_AfterInputs,
		wxWindowID Id, const wxPoint &Position, long Style = wxDEFAULT_DIALOG_STYLE);

	// Destructor
	~GA_GOAL_DIALOG();

	// Private member accessors
	KinematicOutputs::OutputsComplete GetOutput(void) const { return Output; };
	double GetDesiredValue(void) const { return DesiredValue; };
	double GetExpectedDeviation(void) const { return ExpectedDeviation; };
	double GetImportance(void) const { return Importance; };
	Kinematics::Inputs GetBeforeInputs(void) const { return BeforeInputs; };
	Kinematics::Inputs GetAfterInputs(void) const { return AfterInputs; };

private:
	// The object that handles the unit conversions between the input and output
	const Convert &Converter;

	// Method for creating controls
	void CreateControls(void);

	// Updates some controls when user clicks the checkbox
	void FormatDialogDifference(void);

	// Controls
	wxComboBox *OutputCombo;

	wxTextCtrl *BeforePitchText;
	wxTextCtrl *BeforeRollText;
	wxTextCtrl *BeforeHeaveText;
	wxTextCtrl *BeforeSteerText;

	wxTextCtrl *AfterPitchText;
	wxTextCtrl *AfterRollText;
	wxTextCtrl *AfterHeaveText;
	wxTextCtrl *AfterSteerText;

	wxTextCtrl *DesiredValueText;
	wxTextCtrl *DeviationText;
	wxTextCtrl *ImportanceText;

	wxCheckBox *Difference;

	wxStaticText *DesiredValueLabel;
	wxStaticText *DesiredValueUnitsLabel;
	wxStaticText *DeviationUnitsLabel;
	wxStaticText *BeforeLabel;
	wxStaticText *AfterLabel;

	// Values (populated when OK is clicked)
	KinematicOutputs::OutputsComplete Output;

	double DesiredValue;
	double ExpectedDeviation;
	double Importance;

	Kinematics::Inputs BeforeInputs;
	Kinematics::Inputs AfterInputs;

	// Event handlers
	virtual void OKClickEvent(wxCommandEvent &event);
	virtual void CancelClickEvent(wxCommandEvent &event);
	virtual void OnCheckEvent(wxCommandEvent &event);
	virtual void OnOutputChangeEvent(wxCommandEvent &event);

	// For the event table
	DECLARE_EVENT_TABLE();
};