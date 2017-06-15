/*===================================================================================
                                    CarDesigner
                        Copyright Kerry R. Loux 2011-2015

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  textInputDialog.cpp
// Created:  5/20/2013
// Author:  K. Loux
// Description:  Dialog box similar to ::wxGetTextFromUser() but allows differentiation between
//				 canceling and returning an empty string.
// History:

// wxWidgets headers
#include <wx/wx.h>

// Local headers
#include "gui/dialogs/textInputDialog.h"

//==========================================================================
// Class:			TextInputDialog
// Function:		TextInputDialog
//
// Description:		Constructor for TextInputDialog class.
//
// Input Arguments:
//		message		= const wxString&, propmt to display for user
//		title		= const wxString&, dialog title
//		defaultText	= const wxString&, default input text
//		parent		= const wxWindow*, pointing to owner
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
TextInputDialog::TextInputDialog(const wxString &message, const wxString &title,
	const wxString &defaultText, wxWindow *parent) : wxDialog(parent, wxID_ANY, title)
{
	CreateControls(message, defaultText);
}

//==========================================================================
// Class:			TextInputDialog
// Function:		~TextInputDialog
//
// Description:		Destructor for TextInputDialog class.
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
TextInputDialog::~TextInputDialog()
{
}

//==========================================================================
// Class:			TextInputDialog
// Function:		CreateControls
//
// Description:		Constructor for TextInputDialog class.
//
// Input Arguments:
//		message		= const wxString&, propmt to display for user
//		defaultText	= const wxString&, default input text
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void TextInputDialog::CreateControls(const wxString &message, const wxString &defaultText)
{
	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	topSizer->Add(mainSizer, 0, wxALL | wxEXPAND, 8);

	mainSizer->Add(new wxStaticText(this, wxID_ANY, message), 0, wxALL, 5);
	text = new wxTextCtrl(this, wxID_ANY, defaultText);
	mainSizer->Add(text, 1, wxGROW | wxALL, 5);
	mainSizer->AddSpacer(10);

	mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 1, wxGROW);

	SetSizerAndFit(topSizer);
	Center();

	text->SetFocus();
}