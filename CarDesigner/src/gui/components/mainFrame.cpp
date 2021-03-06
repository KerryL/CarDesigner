/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2016

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  mainFrame.cpp
// Created:  3/7/2008
// Author:  K. Loux
// Description:  Contains the class functionality (event handlers, etc.) for the
//				 MainFrame class.  Uses wxWidgets for the GUI components.
// History:
//	1/24/2009	- Major application structure change - MainFrame uses GuiObject instead of
//				  GuiCar.  GuiObject changed to only contain either GuiCar or Iteration
//				  objects, K. Loux.
//	1/28/2009	- Changed structure of GUI components so context menu creation for all
//				  objects is handled by this class, K. Loux.
//	2/10/2009	- Added EditPanel to application, K. Loux.
//	2/17/2009	- Moved the Kinematics object for primary analysis into the GUI_CAR class.
//	6/7/2009	- Changed GetFilenameFromUser() to make use of wx functions for checking for file
//				  existence and selecting multiple files to open, K. Loux.
//	10/14/2010	- Added configuration file for storing application level options, K. Loux.
//	11/28/2010	- Added number of threads to configuration file, K. Loux.

// For difficult debugging problems, use this flag to print messages to file as well as to the output pane
//#define DEBUG_TO_FILE_

// Standard C++ headers
#ifdef DEBUG_TO_FILE_
#include <fstream>
#endif

// wxWidgets headers
#include <wx/aboutdlg.h>
#include <wx/datetime.h>
#include <wx/mimetype.h>
#include <wx/fileconf.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/docview.h>

//#include <wx/platinfo.h>

// CarDesigner headers
#include "application/vvaseConstants.h"
#include "vCar/car.h"
#include "vCar/drivetrain.h"
#include "gui/renderer/carRenderer.h"
#include "gui/renderer/plotRenderer.h"
#include "gui/components/editPanel/editPanel.h"
#include "gui/components/mainFrame.h"
#include "gui/components/mainNotebook.h"
#include "gui/components/mainTree.h"
#include "gui/components/outputPanel.h"
#include "gui/guiObject.h"
#include "gui/guiCar.h"
#include "gui/iteration.h"
#include "gui/geneticOptimization.h"
#include "gui/gaObject.h"
#include "gui/dialogs/optionsDialog.h"
#include "gui/appearanceOptions.h"
#include "gui/dropTarget.h"
#include "vSolver/physics/kinematics.h"
#include "vSolver/physics/kinematicOutputs.h"
#include "vSolver/threads/jobQueue.h"
#include "vSolver/threads/workerThread.h"
#include "vSolver/threads/threadEvent.h"
#include "vMath/vector.h"
#include "vUtilities/fontFinder.h"
#include "vUtilities/debugger.h"

// *nix Icons
#ifdef __WXGTK__
#include "../res/icons/aavase16.xpm"
#include "../res/icons/aavase32.xpm"
#include "../res/icons/aavase48.xpm"
#include "../res/icons/perspective16.xpm"
//#include "../res/icons/perspective32.xpm"
//#include "../res/icons/perspective48.xpm"
#include "../res/icons/ortho16.xpm"
//#include "../res/icons/ortho32.xpm"
//#include "../res/icons/ortho48.xpm"
#endif

//==========================================================================
// Class:			MainFrame
// Function:		MainFrame
//
// Description:		Constructor for MainFrame class.  Initializes the form
//					and creates the controls, etc.
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
MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition,
	wxDefaultSize, wxDEFAULT_FRAME_STYLE), /*maxRecentFiles(9), */undoRedo(*this)
{
	systemsTree = new MainTree(*this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_DEFAULT_STYLE | wxSUNKEN_BORDER
		| wxTR_HIDE_ROOT);

	notebook = new MainNotebook(*this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxAUI_NB_TOP | wxAUI_NB_TAB_SPLIT | wxAUI_NB_TAB_MOVE |
		wxAUI_NB_SCROLL_BUTTONS | wxAUI_NB_CLOSE_ON_ALL_TABS | wxAUI_NB_WINDOWLIST_BUTTON);

	editPanel = new EditPanel(*this);
	outputPanel = new OutputPanel(*this);

	debugPane = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
		wxDefaultSize, wxTE_PROCESS_TAB | wxTE_MULTILINE | wxHSCROLL | wxTE_READONLY
		| wxTE_RICH);

	kinematicToolbar = NULL;
	quasiStaticToolbar = NULL;
	toolbar3D = NULL;
	CreateKinematicAnalysisToolbar();
	CreateQuasiStaticAnalysisToolbar();
	Create3DToolbar();

	CreateMenuBar();

	// These need to be in this order - otherwise the centering doesn't work (resize first)
	DoLayout();
	InitializeSolver();
	SetProperties();// Includes reading configuration file

	activeIndex = -1;
	beingDeleted = false;
	applicationExiting = false;

	Debugger::GetInstance() << carDesignerName << " Initialized!" << Debugger::PriorityHigh;
}

//==========================================================================
// Class:			MainFrame
// Function:		~MainFrame
//
// Description:		Destructor for MainFrame class.  Frees memory and
//					releases GUI object managers.
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
MainFrame::~MainFrame()
{
	delete recentFileManager;
	recentFileManager = NULL;

	delete jobQueue;
	jobQueue = NULL;

	while (openObjectList.GetCount() > 0)
		RemoveObjectFromList(0);

	manager.UnInit();
}

//==========================================================================
// Class:			MainFrame
// Function:		Constant Declarations
//
// Description:		Constant declarations for the MainFrame class.
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
#ifdef __WXGTK__
const wxString MainFrame::pathToConfigFile = _T("vvase.rc");
#else
const wxString MainFrame::pathToConfigFile = _T("config.ini");
#endif

const wxSize MainFrame::minFrameSize(1024, 700);// was 768 vertical - leave some room for taskbar, etc.

const wxString MainFrame::paneNameNotebook(_T("MainNotebook"));
const wxString MainFrame::paneNameSystemsTree(_T("SystemsTree"));
const wxString MainFrame::paneNameEditPanel(_T("EditPanel"));
const wxString MainFrame::paneNameOutputPane(_T("OutputPane"));
const wxString MainFrame::paneNameOutputList(_T("OutputList"));
const wxString MainFrame::paneNameKinematicsToolbar(_T("KinematicsToolbar"));
const wxString MainFrame::paneNameQuasiStaticToolbar(_T("QuasiStaticToolbar"));
const wxString MainFrame::paneName3DToolbar(_T("3DToolbar"));

//==========================================================================
// Class:			MainFrame
// Function:		DoLayout
//
// Description:		Creates the layout for this window and positions the
//					form on the screen.
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
void MainFrame::DoLayout()
{
	manager.SetManagedWindow(this);

	// Add the panes to the manager - this order is important (see below)
	// It's necessary to use the wxAuiPaneInfo() method because we need to assign internal names
	// to make calls to SavePerspective() and LoadPerspective() work properly.
	manager.AddPane(notebook, wxAuiPaneInfo().Name(paneNameNotebook).CenterPane());
	manager.AddPane(debugPane, wxAuiPaneInfo().Name(paneNameOutputPane).Bottom().Caption(_T("Output")));
	// For some reason, these get reversed under Linux
#ifdef __WXGTK__
	manager.AddPane(editPanel, wxAuiPaneInfo().Name(paneNameEditPanel).Left().Caption(_T("Edit Sub-Systems")));
	manager.AddPane(systemsTree, wxAuiPaneInfo().Name(paneNameSystemsTree).Left().Caption(_T("Systems Tree")));
#else
	manager.AddPane(systemsTree, wxAuiPaneInfo().Name(paneNameSystemsTree).Left().Caption(_T("Systems Tree")));
	manager.AddPane(editPanel, wxAuiPaneInfo().Name(paneNameEditPanel).Left().Caption(_T("Edit Sub-Systems")));
#endif
	manager.AddPane(outputPanel, wxAuiPaneInfo().Name(paneNameOutputList).Right().Caption(_T("Output List")));

	// This layer stuff is required to get the desired initial layout (see below)
	manager.GetPane(debugPane).Layer(0);
	manager.GetPane(notebook).Layer(0);
	manager.GetPane(systemsTree).Layer(1);
	manager.GetPane(editPanel).Layer(1);
	manager.GetPane(outputPanel).Layer(0);

	// Setup default sizes
	const int minOppositeDirection(100);
	manager.GetPane(debugPane).MinSize(100, minOppositeDirection);
	manager.GetPane(outputPanel).MinSize(320, minOppositeDirection);
	manager.GetPane(editPanel).MinSize(290, minOppositeDirection);
	manager.GetPane(systemsTree).MinSize(minOppositeDirection, minOppositeDirection);

	/* Desired initial layout is as follows:
	=============================================================
	|Toolbars													|
	|-----------------------------------------------------------|
	| Systems	| Main Notebook							|		|
	| Tree		|										|Output	|
	|			|										|List	|
	|			|										|		|
	|			|										|		|
	|			|										|		|
	|			|										|		|
	|-----------|										|		|
	| Edit		|										|		|
	| Panel		|										|		|
	|			|										|		|
	|			|										|		|
	|			|-----------------------------------------------|
	|			| Output Pane									|
	|			|												|
	|			|												|
	|			|												|
	===========================================================*/

	manager.Update();
	outputPanel->FinishUpdate(0);
}

//==========================================================================
// Class:			MainFrame
// Function:		SetProperties
//
// Description:		Sets the window properties for this window.  Includes
//					title, frame size, and default font.
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
void MainFrame::SetProperties()
{
	SetTitle(carDesignerName);
	SetName(carDesignerName);
	SetMinSize(minFrameSize);

	// Add the icons
	wxIconBundle bundle;
#ifdef __WXMSW__
	bundle.AddIcon(wxIcon(_T("ICON_ID_MAIN"), wxBITMAP_TYPE_ICO_RESOURCE, 16, 16));
	bundle.AddIcon(wxIcon(_T("ICON_ID_MAIN"), wxBITMAP_TYPE_ICO_RESOURCE, 32, 32));
	bundle.AddIcon(wxIcon(_T("ICON_ID_MAIN"), wxBITMAP_TYPE_ICO_RESOURCE, 48, 48));
#elif __WXGTK__
	bundle.AddIcon(wxIcon(aavase16_xpm));
	bundle.AddIcon(wxIcon(aavase32_xpm));
	bundle.AddIcon(wxIcon(aavase48_xpm));
#endif
	SetIcons(bundle);

	Debugger::GetInstance().SetTargetOutput(debugPane);
	Debugger::GetInstance().SetDebugLevel(Debugger::PriorityHigh);

	// Add the application level entry to the SystemsTree (this one is hidden, but necessary)
	systemsTree->AddRoot(_T("Application Level"), -1, -1);

	DisableUndo();
	DisableRedo();

	// This section disables all menu items that are not yet implemented
	menuBar->FindItem(IdMenuEditCut)->Enable(false);
	menuBar->FindItem(IdMenuEditCopy)->Enable(false);
	menuBar->FindItem(IdMenuEditPaste)->Enable(false);
	/*menuBar->FindItem(IdMenuToolsDoE)->Enable(false);
	menuBar->FindItem(IdMenuToolsDynamic)->Enable(false);*/
	
	ReadConfiguration();

	lastAnalysisWasKinematic = true;

	UpdateViewMenuChecks();

	toolbar3D->ToggleTool(IdToolbar3DOrtho, useOrthoView);

	wxString fontFaceName;
	
	// Check to see if we read the output font preference from the config file
	if (outputFont.IsNull() || !outputFont.IsOk())
	{
		wxArrayString preferredFonts;
		preferredFonts.Add(_T("Monospace"));// GTK preference
		preferredFonts.Add(_T("Courier New"));// MSW preference
		bool foundPreferredFont = FontFinder::GetFontFaceName(
			wxFONTENCODING_SYSTEM, preferredFonts, true, fontFaceName);

		if (!fontFaceName.IsEmpty())
		{
			// Populate the wxFont object
			outputFont.SetPointSize(9);
			outputFont.SetFamily(wxFONTFAMILY_MODERN);
			if (!outputFont.SetFaceName(fontFaceName))
				Debugger::GetInstance() << "Error setting font face to " << fontFaceName << Debugger::PriorityHigh;
		}
		
		if (!foundPreferredFont)
		{
			Debugger::GetInstance() << "Could not find preferred fixed-width font; using " << fontFaceName << Debugger::PriorityHigh;
			Debugger::GetInstance() << "This can be changed in Tools->Options->Fonts" << Debugger::PriorityHigh;
		}
	}

	SetOutputFont(outputFont);

	if (plotFont.IsNull() || !plotFont.IsOk())
	{
		wxArrayString preferredFonts;

		preferredFonts.Add(_T("DejaVu Sans"));// GTK preference
		preferredFonts.Add(_T("Arial"));// MSW preference

		wxString fontFile;
		bool foundFont = FontFinder::GetPreferredFontFileName(wxFONTENCODING_SYSTEM,
			preferredFonts, false, fontFile);

		// Tell the user if we're unsure of the font
		if (!foundFont)
		{
			if (!fontFile.IsEmpty())
				Debugger::GetInstance() << "Could not find preferred plot font; using " << fontFile << Debugger::PriorityHigh;
			else
				Debugger::GetInstance() << "Could not find any *.ttf files - cannot generate plot fonts" << Debugger::PriorityHigh;
		}
		else
		{
			// Store what we found in the MainFrame configuration
			wxString fontName;
			if (FontFinder::GetFontName(fontFile, fontName))
			{
				if (plotFont.SetFaceName(fontName))
					SetPlotFont(plotFont);
			}
		}
	}

	// Also put the cursor at the bottom of the text, so the window scrolls automatically
	// as it updates with text
	// NOTE:  This already works as desired under GTK, issue is with MSW
	// FIXME:  This doesn't work!

	// Allow dragging-and-dropping of files onto this window to open them
	SetDropTarget(dynamic_cast<wxDropTarget*>(new DropTarget(*this)));
}

//==========================================================================
// Class:			MainFrame
// Function:		UpdateViewMenuChecks
//
// Description:		Updates the checkboxes in the View menu.
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
void MainFrame::UpdateViewMenuChecks()
{
	// Depending on whether elements are shown, check the corresponding menu item
	menuBar->Check(IdMenuViewToolbarsKinematic, manager.GetPane(kinematicToolbar).IsShown());
	//menuBar->Check(IdMenuViewToolbarsQuasiStatic, manager.GetPane(quasiStaticToolbar).IsShown());
	menuBar->Check(IdMenuViewToolbars3D, manager.GetPane(toolbar3D).IsShown());

	menuBar->Check(IdMenuViewSystemsTree, manager.GetPane(systemsTree).IsShown());
	menuBar->Check(IdMenuViewEditPanel, manager.GetPane(editPanel).IsShown());
	menuBar->Check(IdMenuViewOutputPane, manager.GetPane(debugPane).IsShown());
	menuBar->Check(IdMenuViewOutputList, manager.GetPane(outputPanel).IsShown());
}

//==========================================================================
// Class:			MainFrame
// Function:		OnPaneClose
//
// Description:		Handles pane close events.
//
// Input Arguments:
//		event	= wxAuiManagerEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OnPaneClose(wxAuiManagerEvent& event)
{
	wxString name = event.GetPane()->name;
	int id;
	if (name.IsSameAs(paneNameSystemsTree))
		id = IdMenuViewSystemsTree;
	else if (name.IsSameAs(paneNameEditPanel))
		id = IdMenuViewEditPanel;
	else if (name.IsSameAs(paneNameOutputPane))
		id = IdMenuViewOutputPane;
	else if (name.IsSameAs(paneNameOutputList))
		id = IdMenuViewOutputList;
	else if (name.IsSameAs(paneNameKinematicsToolbar))
		id = IdMenuViewToolbarsKinematic;
	else if (name.IsSameAs(paneNameQuasiStaticToolbar))
		id = IdMenuViewToolbarsQuasiStatic;
	else if (name.IsSameAs(paneName3DToolbar))
		id = IdMenuViewToolbars3D;
	else
		return;

	menuBar->Check(id, false);
}

//==========================================================================
// Class:			MainFrame
// Function:		InitializeSolver
//
// Description:		Initializes solver settings.
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
void MainFrame::InitializeSolver()
{
	jobQueue = new JobQueue(GetEventHandler());

	activeThreads = 0;
	openJobCount = 0;
	numberOfThreads = 0;

	kinematicInputs.pitch = 0.0;
	kinematicInputs.roll = 0.0;
	kinematicInputs.heave = 0.0;
	kinematicInputs.rackTravel = 0.0;
	kinematicInputs.tireDeflections.leftFront = 0.0;
	kinematicInputs.tireDeflections.rightFront = 0.0;
	kinematicInputs.tireDeflections.leftRear = 0.0;
	kinematicInputs.tireDeflections.rightRear = 0.0;

	quasiStaticInputs.gx = 0.0;
	quasiStaticInputs.gy = 0.0;
	quasiStaticInputs.rackTravel = 0.0;
}

//==========================================================================
// Class:			MainFrame
// Function:		SetNumberOfThreads
//
// Description:		Sets the number of worker threads to the specified value.
//					Handles creation or deletion of threads as necessary to
//					ensure the correct number of threads are left.
//
// Input Arguments:
//		newNumberOfThreads	= unsigned int specifying the number of
//							  threads desired
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::SetNumberOfThreads(unsigned int newNumberOfThreads)
{
	if (newNumberOfThreads < 1)
		newNumberOfThreads = 1;

	unsigned int i;
	if (newNumberOfThreads > numberOfThreads)
	{
		// Spawn threads for the thread pool
		for (i = numberOfThreads; i < newNumberOfThreads; i++)
		{
			// Keep track of jobs by counting them as they're sent to the
			// threads (starting a thread counts as a job)
			openJobCount++;

			// These threads will delete themselves after an EXIT job
			WorkerThread *newThread = new WorkerThread(jobQueue, i);
			// FIXME:  If we want to set priority, this is where it needs to happen
			newThread->Run();
		}
	}
	else if (newNumberOfThreads < numberOfThreads)
	{
		for (i = numberOfThreads; i > newNumberOfThreads; i--)
			jobQueue->AddJob(ThreadJob(ThreadJob::CommandThreadExit), JobQueue::PriorityVeryHigh);
	}

	numberOfThreads = newNumberOfThreads;
}

//==========================================================================
// Class:			MainFrame
// Function:		SetNumberOfThreads
//
// Description:		Sets the font to use for text output and assigns it to
//					the panel.
//
// Input Arguments:
//		font	= const wxFont&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::SetOutputFont(const wxFont& font)
{
	if (!font.IsNull() && font.IsOk())
	{
		outputFont = font;
		
		// Assign the font to the window
		wxTextAttr outputAttributes;
		outputAttributes.SetFont(outputFont);
		if (!debugPane->SetDefaultStyle(outputAttributes))
			Debugger::GetInstance() << "Error setting font style" << Debugger::PriorityHigh;
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		SetPlotFont
//
// Description:		Sets the font to use for plots.
//
// Input Arguments:
//		font	= const wxFont&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::SetPlotFont(const wxFont& font)
{
	if (!font.IsNull() && font.IsOk())
		plotFont = font;
	/*else
		Debugger::GetInstance().Print(_T("Error setting plot font"));*/// Sometimes we just want
	// this to sort out whether or not the font is valid without telling the user.
}

//==========================================================================
// Class:			MainFrame
// Function:		CreateMenuBar
//
// Description:		Creates the menu bar and all of the sub-menus.
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
void MainFrame::CreateMenuBar()
{
	menuBar = new wxMenuBar();

	// Car menu
	wxMenu *mnuFile = new wxMenu();
	wxMenu *mnuFileNew = new wxMenu();
	mnuFileNew->Append(IdMenuFileNewCar, _T("&Car\tCtrl+N"), _T("Create new car file"), wxITEM_NORMAL);
	mnuFileNew->Append(IdMenuFileNewIteration, _T("&Static Iteration\tCtrl+I"),
		_T("Create new static iteration analysis"), wxITEM_NORMAL);
	mnuFileNew->Append(IdMenuFileNewOptimization, _T("&Genetic Optimization\tCtrl+G"),
		_T("Create new genetic algorithm optimization"), wxITEM_NORMAL);
	mnuFile->AppendSubMenu(mnuFileNew, _T("New"));
	mnuFile->AppendSeparator();
	mnuFile->Append(IdMenuFileOpen, _T("&Open\tCtrl+O"), _T("Open saved files"), wxITEM_NORMAL);
	mnuFile->AppendSeparator();
	mnuFile->Append(IdMenuFileClose, _T("&Close"), _T("Close current car file"), wxITEM_NORMAL);
	mnuFile->Append(IdMenuFileCloseAll, _T("Close All"), _T("Close all files"), wxITEM_NORMAL);
	mnuFile->AppendSeparator();
	mnuFile->Append(IdMenuFileSave, _T("&Save\tCtrl+S"), _T("Save current file"), wxITEM_NORMAL);
	mnuFile->Append(IdMenuFileSaveAs, _T("Save &As"), _T("Save current file as new file"), wxITEM_NORMAL);
	mnuFile->Append(IdMenuFileSaveAll, _T("Save A&ll"), _T("Save all open files"), wxITEM_NORMAL);
	mnuFile->AppendSeparator();
	mnuFile->Append(IdMenuFileWriteImageFile, _T("&Write Image File\tCtrl+W"),
		_T("Save window contents to image file"), wxITEM_NORMAL);
	mnuFile->AppendSeparator();
	wxMenu *mnuRecentFiles = new wxMenu();
	mnuFile->AppendSubMenu(mnuRecentFiles, _T("&Recent Files"));
	mnuFile->Append(IdMenuFileOpenAllRecent, _T("Open All Recent Files"),
		_T("Open all files in the Recent Files list"), wxITEM_NORMAL);
	mnuFile->AppendSeparator();
	mnuFile->Append(IdMenuFileExit, _T("E&xit\tAlt+F4"), _T("Exit ") + carDesignerName, wxITEM_NORMAL);
	menuBar->Append(mnuFile, _T("&File"));

	// Edit menu
	wxMenu *mnuEdit = new wxMenu();
	mnuEdit->Append(IdMenuEditUndo, _T("&Undo\tCtrl+Z"), _T("Undo last action"), wxITEM_NORMAL);
	mnuEdit->Append(IdMenuEditRedo, _T("&Redo\tCtrl+Y"), _T("Redo last previously undone action"), wxITEM_NORMAL);
	mnuEdit->AppendSeparator();
	mnuEdit->Append(IdMenuEditCut, _T("&Cut\tCtrl+X"), _T("Cut selected to clipboard"), wxITEM_NORMAL);
	mnuEdit->Append(IdMenuEditCopy, _T("C&opy\tCtrl+C"), _T("Copy selected to clipboard"), wxITEM_NORMAL);
	mnuEdit->Append(IdMenuEditPaste, _T("&Paste\tCtrl+V"), _T("Paste from clipboard"), wxITEM_NORMAL);
	menuBar->Append(mnuEdit, _T("&Edit"));

	// View menu
	wxMenu *mnuView = new wxMenu();
	mnuView->AppendCheckItem(IdMenuViewSystemsTree, _T("Systems Tree"));
	mnuView->AppendCheckItem(IdMenuViewEditPanel, _T("Edit Panel"));
	mnuView->AppendCheckItem(IdMenuViewOutputPane, _T("Output Pane"));
	mnuView->AppendCheckItem(IdMenuViewOutputList, _T("Output List"));
	wxMenu *mnuViewToolbars = new wxMenu();
	mnuViewToolbars->AppendCheckItem(IdMenuViewToolbarsKinematic, _T("Kinematic Analysis"));
	//mnuViewToolbars->AppendCheckItem(IdMenuViewToolbarsQuasiStatic, _T("Quasi-Static Analysis"));
	mnuViewToolbars->AppendCheckItem(IdMenuViewToolbars3D, _T("3D View"));
	mnuView->AppendSubMenu(mnuViewToolbars, _T("Toolbars"));
	mnuView->AppendSeparator();
	mnuView->Append(IdMenuViewClearOutput, _T("&Clear Output Text"),
		_T("Clear all text from the output pane"), wxITEM_NORMAL);
	menuBar->Append(mnuView, _T("&View"));

	// Tools menu
	wxMenu *mnuTools = new wxMenu();
	/*mnuTools->Append(IdMenuToolsDoE, _T("Design of &Experiment"),
		_T("Open design of experiments tool"), wxITEM_NORMAL);
	mnuTools->Append(IdMenuToolsDynamic, _T("&Dynamic Analysis"),
		_T("Start Dynamic Analysis Wizard"), wxITEM_NORMAL);
	mnuTools->AppendSeparator();*/
	mnuTools->Append(IdMenuToolsOptions, _T("&Options"), _T("Edit application preferences"), wxITEM_NORMAL);
	menuBar->Append(mnuTools, _T("&Tools"));

	// Help menu
	wxMenu *mnuHelp = new wxMenu();
	mnuHelp->Append(IdMenuHelpManual, _T("&User's Manual\tF1"), _T("Display user's manual"), wxITEM_NORMAL);
	mnuHelp->AppendSeparator();
	mnuHelp->Append(IdMenuHelpAbout, _T("&About"), _T("Show About dialog"), wxITEM_NORMAL);
	menuBar->Append(mnuHelp, _T("&Help"));

	recentFileManager = new wxFileHistory(maxRecentFiles, IdMenuFileRecentStart);
	recentFileManager->UseMenu(mnuRecentFiles);

	SetMenuBar(menuBar);
}

//==========================================================================
// Class:			MainFrame
// Function:		CreateKinematicAnalysisToolbar
//
// Description:		Creates the toolbar and adds the buttons and icons. Also
//					adds the toolbar to the frame in the appropriate
//					position.
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
void MainFrame::CreateKinematicAnalysisToolbar()
{
	if (kinematicToolbar != NULL)
		return;

	kinematicToolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_NODIVIDER);

	// Create the controls
	wxStaticText *pitchLabel = new wxStaticText(kinematicToolbar, wxID_ANY, _T("Pitch"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	wxTextCtrl *pitchSet = new wxTextCtrl(kinematicToolbar, IdToolbarKinematicPitch,
		_T("0"), wxDefaultPosition, wxSize(40, -1));
	pitchSet->SetMaxLength(5);

	wxStaticText *rollLabel = new wxStaticText(kinematicToolbar, wxID_ANY, _T("Roll"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	wxTextCtrl *rollSet = new wxTextCtrl(kinematicToolbar, IdToolbarKinematicRoll,
		_T("0"), wxDefaultPosition, wxSize(40, -1));
	rollSet->SetMaxLength(5);

	wxStaticText *heaveLabel = new wxStaticText(kinematicToolbar, wxID_ANY, _T("Heave"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	wxTextCtrl *heaveSet = new wxTextCtrl(kinematicToolbar, IdToolbarKinematicHeave,
		_T("0"), wxDefaultPosition, wxSize(40, -1));
	heaveSet->SetMaxLength(5);

	wxStaticText *steerLabel = new wxStaticText(kinematicToolbar, wxID_ANY, _T("Steer"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	wxTextCtrl *steerSet = new wxTextCtrl(kinematicToolbar, IdToolbarKinematicSteer,
		_T("0"), wxDefaultPosition, wxSize(40, -1));
	steerSet->SetMaxLength(5);

	// Add the controls to the toolbar
	kinematicToolbar->AddControl(pitchLabel);
	kinematicToolbar->AddControl(pitchSet);
	kinematicToolbar->AddSeparator();
	kinematicToolbar->AddControl(rollLabel);
	kinematicToolbar->AddControl(rollSet);
	kinematicToolbar->AddSeparator();
	kinematicToolbar->AddControl(heaveLabel);
	kinematicToolbar->AddControl(heaveSet);
	kinematicToolbar->AddSeparator();
	kinematicToolbar->AddControl(steerLabel);
	kinematicToolbar->AddControl(steerSet);

	kinematicToolbar->Realize();

	manager.AddPane(kinematicToolbar, wxAuiPaneInfo().Name(paneNameKinematicsToolbar).
		Caption(_T("Kinematic Analysis")).ToolbarPane().Top().Row(1).Position(1).
		LeftDockable(false).RightDockable(false));
}

//==========================================================================
// Class:			MainFrame
// Function:		CreateQuasiStaticAnalysisToolbar
//
// Description:		Creates the toolbar and adds the buttons and icons. Also
//					adds the toolbar to the frame in the appropriate
//					position.
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
void MainFrame::CreateQuasiStaticAnalysisToolbar()
{
	if (quasiStaticToolbar != NULL)
		return;

	/*quasiStaticToolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_NODIVIDER);

	// Create the controls
	wxStaticText *gxLabel = new wxStaticText(quasiStaticToolbar, wxID_ANY, _T("Gx"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	wxTextCtrl *gxSet = new wxTextCtrl(quasiStaticToolbar, IdToolbarQuasiStaticGx,
		_T("0"), wxDefaultPosition, wxSize(40, -1));
	gxSet->SetMaxLength(5);

	wxStaticText *gyLabel = new wxStaticText(quasiStaticToolbar, wxID_ANY, _T("Gy"),
		wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
	wxTextCtrl *gySet = new wxTextCtrl(quasiStaticToolbar, IdToolbarQuasiStaticGy,
		_T("0"), wxDefaultPosition, wxSize(40, -1));
	gxSet->SetMaxLength(5);

	// Add the controls to the toolbar
	quasiStaticToolbar->AddControl(gxLabel);
	quasiStaticToolbar->AddControl(gxSet);
	quasiStaticToolbar->AddSeparator();
	quasiStaticToolbar->AddControl(gyLabel);
	quasiStaticToolbar->AddControl(gySet);

	quasiStaticToolbar->Realize();

	manager.AddPane(quasiStaticToolbar, wxAuiPaneInfo().Name(paneNameQuasiStaticToolbar).
		Caption(_T("Quasi-Static Analysis")).ToolbarPane().Top().Row(1).Position(2).
		LeftDockable(false).RightDockable(false));*/
}

//==========================================================================
// Class:			MainFrame
// Function:		Create3DToolbar
//
// Description:		Creates the 3D toolbar and adds the buttons and icons. Also
//					adds the toolbar to the frame in the appropriate
//					position.
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
void MainFrame::Create3DToolbar()
{
	if (toolbar3D != NULL)
		return;

	toolbar3D = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
		wxTB_FLAT | wxTB_NODIVIDER);

	// TODO:  Cannot figure out why toolbar won't go to proper width.  At the end of
	// this method, size is correct, but after manager.Update() it becomes far too narrow.
#ifdef __WXMSW__
	const int preferredIconSize(16);
	wxIcon perspectiveImage(_T("ICON_ID_PERSPECTIVE"), wxBITMAP_TYPE_ICO_RESOURCE, preferredIconSize, preferredIconSize);
	wxIcon orthoImage(_T("ICON_ID_ORTHO"), wxBITMAP_TYPE_ICO_RESOURCE, preferredIconSize, preferredIconSize);
#else
	wxImage perspectiveImage(perspective16_xpm);
	wxImage orthoImage(ortho16_xpm);
#endif
	wxBitmap perspectiveBitmap(perspectiveImage);
	wxBitmap orthoBitmap(orthoImage);
	toolbar3D->AddRadioTool(IdToolbar3DPerspective, _T("Perspective"),
		perspectiveBitmap, perspectiveBitmap, _T("Perspective view"));
	toolbar3D->AddRadioTool(IdToolbar3DOrtho, _T("Orthogonal"),
		orthoBitmap, orthoBitmap, _T("Orthographic view"));

	toolbar3D->Realize();

	manager.AddPane(toolbar3D, wxAuiPaneInfo().Name(_T("3DToolbar")).
		Caption(_T("3D View")).ToolbarPane().Top().Row(1).Position(3));
}

//==========================================================================
// Class:			MainFrame
// Function:		Event Table
//
// Description:		Links GUI events with event handler functions.
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
BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	// Frame top level
	EVT_CLOSE(MainFrame::WindowCloseEvent)
	EVT_SIZE(MainFrame::OnSizeEvent)
	EVT_AUI_PANE_CLOSE(MainFrame::OnPaneClose)

	// Menu bar
	EVT_MENU(IdMenuFileNewCar,					MainFrame::FileNewCarEvent)
	EVT_MENU(IdMenuFileNewIteration,			MainFrame::FileNewIterationEvent)
	EVT_MENU(IdMenuFileNewOptimization,			MainFrame::FileNewOptimizationEvent)
	EVT_MENU(IdMenuFileOpen,					MainFrame::FileOpenEvent)
	EVT_MENU(IdMenuFileClose,					MainFrame::FileCloseEvent)
	EVT_MENU(IdMenuFileCloseAll,				MainFrame::FileCloseAllEvent)
	EVT_MENU(IdMenuFileSave,					MainFrame::FileSaveEvent)
	EVT_MENU(IdMenuFileSaveAs,					MainFrame::FileSaveAsEvent)
	EVT_MENU(IdMenuFileSaveAll,					MainFrame::FileSaveAllEvent)
	EVT_MENU(IdMenuFileWriteImageFile,			MainFrame::FileWriteImageFileEvent)
	EVT_MENU(IdMenuFileOpenAllRecent,			MainFrame::FileOpenAllRecentEvent)
	EVT_MENU(IdMenuFileExit,					MainFrame::FileExitEvent)
	EVT_MENU(wxID_ANY,							MainFrame::OtherMenuClickEvents)

	EVT_MENU(IdMenuEditUndo,					MainFrame::EditUndoEvent)
	EVT_MENU(IdMenuEditRedo,					MainFrame::EditRedoEvent)
	EVT_MENU(IdMenuEditCut,						MainFrame::EditCutEvent)
	EVT_MENU(IdMenuEditCopy,					MainFrame::EditCopyEvent)
	EVT_MENU(IdMenuEditPaste,					MainFrame::EditPasteEvent)

	EVT_MENU(IdMenuCarAppearanceOptions,		MainFrame::CarAppearanceOptionsEvent)

	EVT_MENU(IdMenuIterationShowAssociatedCars,	MainFrame::IterationShowAssociatedCarsClickEvent)
	EVT_MENU(IdMenuIterationAssociatedWithAllCars,	MainFrame::IterationAssociatedWithAllCarsClickEvent)
	EVT_MENU(IdMenuIterationExportDataToFile,	MainFrame::IterationExportDataToFileClickEvent)
	EVT_MENU(IdMenuIterationXAxisPitch,			MainFrame::IterationXAxisPitchClickEvent)
	EVT_MENU(IdMenuIterationXAxisRoll,			MainFrame::IterationXAxisRollClickEvent)
	EVT_MENU(IdMenuIterationXAxisHeave,			MainFrame::IterationXAxisHeaveClickEvent)
	EVT_MENU(IdMenuIterationXAxisRackTravel,	MainFrame::IterationXAxisRackTravelClickEvent)

	EVT_MENU(IdMenuViewToolbarsKinematic,		MainFrame::ViewToolbarsKinematicEvent)
	EVT_MENU(IdMenuViewToolbarsQuasiStatic,		MainFrame::ViewToolbarsQuasiStaticEvent)
	EVT_MENU(IdMenuViewToolbars3D,				MainFrame::ViewToolbars3DEvent)
	EVT_MENU(IdMenuViewSystemsTree,				MainFrame::ViewSystemsTreeEvent)
	EVT_MENU(IdMenuViewEditPanel,				MainFrame::ViewEditPanelEvent)
	EVT_MENU(IdMenuViewOutputPane,				MainFrame::ViewOutputPaneEvent)
	EVT_MENU(IdMenuViewOutputList,				MainFrame::ViewOutputListEvent)
	EVT_MENU(IdMenuViewClearOutput,				MainFrame::ViewClearOutputEvent)

	EVT_MENU(IdMenuToolsDoE,					MainFrame::ToolsDoEEvent)
	EVT_MENU(IdMenuToolsDynamic,				MainFrame::ToolsDynamicEvent)
	EVT_MENU(IdMenuToolsOptions,				MainFrame::ToolsOptionsEvent)

	EVT_MENU(IdMenuHelpManual,					MainFrame::HelpManualEvent)
	EVT_MENU(IdMenuHelpAbout,					MainFrame::HelpAboutEvent)

	// Toolbars
	// Kinematic Analysis
	EVT_TEXT(IdToolbarKinematicPitch,			MainFrame::KinematicToolbarPitchChangeEvent)
	EVT_TEXT(IdToolbarKinematicRoll,			MainFrame::KinematicToolbarRollChangeEvent)
	EVT_TEXT(IdToolbarKinematicHeave,			MainFrame::KinematicToolbarHeaveChangeEvent)
	EVT_TEXT(IdToolbarKinematicSteer,			MainFrame::KinematicToolbarSteerChangeEvent)

	// Quasi-static Analysis
	EVT_TEXT(IdToolbarQuasiStaticGx,			MainFrame::QuasiStaticToolbarGxChangeEvent)
	EVT_TEXT(IdToolbarQuasiStaticGy,			MainFrame::QuasiStaticToolbarGyChangeEvent)

	// 3D
	EVT_MENU(IdToolbar3DPerspective,			MainFrame::Toolbar3DPerspectiveClickEvent)
	EVT_MENU(IdToolbar3DOrtho,					MainFrame::Toolbar3DOrthoClickEvent)

	// Threads
	EVT_COMMAND(wxID_ANY, EVT_THREAD,			MainFrame::ThreadCompleteEvent)
	EVT_COMMAND(wxID_ANY, EVT_DEBUG,			MainFrame::DebugMessageEvent)
END_EVENT_TABLE();

//==========================================================================
// Class:			MainFrame
// Function:		FileNewCarEvent
//
// Description:		Generates a new GuiCar object and adds the car to the
//					list of managed objects.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileNewCarEvent(wxCommandEvent& WXUNUSED(event))
{
	GuiObject *tempObject = new GuiCar(*this);
	SetActiveIndex(tempObject->GetIndex());
}

//==========================================================================
// Class:			MainFrame
// Function:		FileNewIterationEvent
//
// Description:		Generates a new Iteration object and adds the it to the
//					list of managed objects
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileNewIterationEvent(wxCommandEvent& WXUNUSED(event))
{
	GuiObject *tempObject = new Iteration(*this);
	if (tempObject->IsInitialized())
		SetActiveIndex(tempObject->GetIndex());
	else
		RemoveObjectFromList(tempObject->GetIndex());
}

//==========================================================================
// Class:			MainFrame
// Function:		FileNewOptimizationEvent
//
// Description:		Generates a new GeneticOptimization object and adds the
//					it to the list of managed objects
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileNewOptimizationEvent(wxCommandEvent& WXUNUSED(event))
{
	GuiObject *tempObject = new GeneticOptimization(*this);
	SetActiveIndex(tempObject->GetIndex());
}

//==========================================================================
// Class:			MainFrame
// Function:		FileOpenEvent
//
// Description:		Displays a dialog asking the user to specify the file to
//					read from.  Creates a new GuiObject, loading the
//					contents from the specified file name.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileOpenEvent(wxCommandEvent& WXUNUSED(event))
{
	// Set up the wildcard specifications
	// (Done here for readability)
	wxString wildcard("VVASE files (*.car; *.iteration; *.ga)|*.car;*.iteration;*.ga|");
	wildcard.append("Car files (*.car)|*.car");
	wildcard.append("|Iteration files (*.iteration)|*.iteration");
	wildcard.append("|Optimization files (*.ga)|*ga");

	// Get the file name to open from the user
	wxArrayString pathsAndFileNames = GetFileNameFromUser(_T("Open"), wxEmptyString, wxEmptyString,
		wildcard, wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);

	// Make sure the user didn't cancel
	if (pathsAndFileNames.GetCount() == 0)
		return;

	// Loop to make sure we open all selected files
	unsigned int file;
	for (file = 0; file < pathsAndFileNames.GetCount(); file++)
		LoadFile(pathsAndFileNames[file]);
}

//==========================================================================
// Class:			MainFrame
// Function:		FileCloseEvent
//
// Description:		Calls the object of interests's close method.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileCloseEvent(wxCommandEvent& WXUNUSED(event))
{
	if (openObjectList.GetCount() > 0)
		openObjectList[objectOfInterestIndex]->Close();
}

//==========================================================================
// Class:			MainFrame
// Function:		FileCloseAllEvent
//
// Description:		Calls all of the open GuiObject' close methods.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileCloseAllEvent(wxCommandEvent& WXUNUSED(event))
{
	// Close all of the objects
	unsigned int indexToDelete = 0;
	while (openObjectList.GetCount() > indexToDelete)
	{
		// If the user chooses not to close a particular object, we need to
		// increment our target index so that we don't keep asking about the same object.
		if (!openObjectList[indexToDelete]->Close())
			indexToDelete++;
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		FileSaveEvent
//
// Description:		Calls the object of interest's save method.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileSaveEvent(wxCommandEvent& WXUNUSED(event))
{
	// Check to make sure there is an object to save
	if (openObjectList.GetCount() > 0)
		// Save the object of interest
		openObjectList[objectOfInterestIndex]->SaveToFile();
}

//==========================================================================
// Class:			MainFrame
// Function:		FileSaveAsEvent
//
// Description:		Calls the active object's save method and asks for a new
//					file name.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileSaveAsEvent(wxCommandEvent& WXUNUSED(event))
{
	// Check to make sure there is an object to save
	if (openObjectList.GetCount() > 0)
		// Save the object of interest
		openObjectList[objectOfInterestIndex]->SaveToFile(true);
}

//==========================================================================
// Class:			MainFrame
// Function:		FileSaveAllEvent
//
// Description:		Calls all of the open GuiObjects' save methods
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileSaveAllEvent(wxCommandEvent& WXUNUSED(event))
{
	// Save all of the objects
	unsigned int indexToSave = 0;
	while (openObjectList.GetCount() > indexToSave)
	{
		// Save the object with index IndexToSave
		if (!openObjectList[indexToSave]->SaveToFile())
			break;

		// Increment our save index
		indexToSave++;
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		FileOpenAllRecentEvent
//
// Description:		Opens all files in the Recent Files list.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileOpenAllRecentEvent(wxCommandEvent& WXUNUSED(event))
{
	// Try to open every file in the list
	unsigned int i;
	for (i = 0; i < recentFileManager->GetCount(); i++)
		LoadFile(recentFileManager->GetHistoryFile(i));
}

//==========================================================================
// Class:			MainFrame
// Function:		FileWriteImageFileEvent
//
// Description:		Calls the object of interest's write image file method.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileWriteImageFileEvent(wxCommandEvent& WXUNUSED(event))
{
	// Check to make sure there is an object open
	if (openObjectList.GetCount() < 1)
		return;

	// Get the file name to open from the user
	wxArrayString pathAndFileName = GetFileNameFromUser(_T("Save Image File"), wxEmptyString, wxEmptyString,
		_T("Bitmap Image (*.bmp)|*.bmp|JPEG Image (*.jpg)|*.jpg|PNG Image (*.png)|*.png|TIFF Image (*.tif)|*.tif"),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	// Make sure the user didn't cancel
	if (pathAndFileName.IsEmpty())
		return;

	// Call the object's write image file method
	if (openObjectList[objectOfInterestIndex]->WriteImageToFile(pathAndFileName[0]))
		Debugger::GetInstance() << "Image file written to %s", pathAndFileName[0] << Debugger::PriorityHigh;
	else
		Debugger::GetInstance() << "Image file NOT written!" << Debugger::PriorityHigh;
}

//==========================================================================
// Class:			MainFrame
// Function:		FileExitEvent
//
// Description:		Attempts to close this form.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::FileExitEvent(wxCommandEvent& WXUNUSED(event))
{
	// Shut down this application
	// User confirmation, etc. is handled by the CloseEvent method,
	// which is called when the form tries to close.  If we put our own
	// code here, the user is asked for confirmation twice.
	Close(true);
}

//==========================================================================
// Class:			MainFrame
// Function:		OtherMenuClickEvents
//
// Description:		Handles menu events not specifically caught by other functions.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OtherMenuClickEvents(wxCommandEvent &event)
{
	// If the ID isn't in the right range, skip the event and return
	if (event.GetId() < IdMenuFileRecentStart || event.GetId() > IdMenuFileRecentLast)
	{
		event.Skip();
		return;
	}

	// Make sure the index isn't greater than the number of files we actually have in the list
	if ((unsigned int)(event.GetId() - IdMenuFileRecentStart) >= recentFileManager->GetCount())
		return;

	// Attempt to load the specified file
	LoadFile(recentFileManager->GetHistoryFile(event.GetId() - IdMenuFileRecentStart));
}

//==========================================================================
// Class:			MainFrame
// Function:		EditUndoEvent
//
// Description:		Event handler for the Edit menu's Undo item.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::EditUndoEvent(wxCommandEvent& WXUNUSED(event))
{
	// Undo the last operation
	undoRedo.Undo();
}

//==========================================================================
// Class:			MainFrame
// Function:		EditRedoEvent
//
// Description:		Event handler for the Edit menu's Redo item.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::EditRedoEvent(wxCommandEvent& WXUNUSED(event))
{
	// Redo the last undone operation
	undoRedo.Redo();
}

//==========================================================================
// Class:			MainFrame
// Function:		EditCutEvent
//
// Description:		Event handler for the Edit menu's Cut item.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::EditCutEvent(wxCommandEvent &event)
{
	event.Skip();
}

//==========================================================================
// Class:			MainFrame
// Function:		EditCopyEvent
//
// Description:		Event handler for the Edit menu's Copy item.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::EditCopyEvent(wxCommandEvent &event)
{
	event.Skip();
}

//==========================================================================
// Class:			MainFrame
// Function:		EditPasteEvent
//
// Description:		Event handler for the Edit menu's Paste item.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::EditPasteEvent(wxCommandEvent &event)
{
	event.Skip();
}

//==========================================================================
// Class:			MainFrame
// Function:		CarAppearanceOptionsEvent
//
// Description:		Calls the ShowAppearanceOptionsDialog() function if the
//					object of interest is a TYPE_CAR.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::CarAppearanceOptionsEvent(wxCommandEvent& WXUNUSED(event))
{
	// Make sure the object of interest is a car
	if (openObjectList[objectOfInterestIndex]->GetType() != GuiObject::TypeCar)
		return;

	// Show the appearance options dialog for the car
	static_cast<GuiCar*>(openObjectList[objectOfInterestIndex])
		->GetAppearanceOptions().ShowAppearanceOptionsDialog();
}

//==========================================================================
// Class:			MainFrame
// Function:		IterationShowAssociatedCarsClickEvent
//
// Description:		For Iteration objects - calls the method that displays
//					a dialog allowing the user to select the associated cars.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::IterationShowAssociatedCarsClickEvent(wxCommandEvent& WXUNUSED(event))
{
	// Make sure the object is TypeIteration
	if (openObjectList[objectOfInterestIndex]->GetType() != GuiObject::TypeIteration)
		return;

	// Call the ShowAssociatedCarsDialog() method
	static_cast<Iteration*>(openObjectList[objectOfInterestIndex])->ShowAssociatedCarsDialog();
}

//==========================================================================
// Class:			MainFrame
// Function:		IterationAssociatedWithAllCarsClickEvent
//
// Description:		For Iteration objects - toggles the auto-associate function
//					for the object of interest.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::IterationAssociatedWithAllCarsClickEvent(wxCommandEvent &event)
{
	// Make sure the object is TypeIteration
	if (openObjectList[objectOfInterestIndex]->GetType() != GuiObject::TypeIteration)
		return;

	// If this object is checked, set the auto-associate flag to true, otherwise
	// set it to false
	if (event.IsChecked())
		static_cast<Iteration*>(openObjectList[objectOfInterestIndex])->SetAutoAssociate(true);
	else
		static_cast<Iteration*>(openObjectList[objectOfInterestIndex])->SetAutoAssociate(false);
}

//==========================================================================
// Class:			MainFrame
// Function:		IterationExportDataToFileClickEvent
//
// Description:		For Iteration objects.  Calls a method that exports the
//					kinematic output data to a user-specified file.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::IterationExportDataToFileClickEvent(wxCommandEvent& WXUNUSED(event))
{
	// Make sure the object is TypeIteration
	if (openObjectList[objectOfInterestIndex]->GetType() != GuiObject::TypeIteration)
		return;

	// Get the file name to export to
	wxArrayString pathAndFileName = GetFileNameFromUser(_T("Save As"), wxGetHomeDir(), wxEmptyString,
		_T("Tab delimited (*.txt)|*.txt|Comma Separated Values (*.csv)|*.csv"),
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	// Make sure the user didn't cancel
	if (pathAndFileName.IsEmpty())
		return;

	static_cast<Iteration*>(openObjectList[objectOfInterestIndex])
		->ExportDataToFile(pathAndFileName.Item(0));
}

//==========================================================================
// Class:			MainFrame
// Function:		IterationXAxisPitchClickEvent
//
// Description:		Event handler for setting iteration x-axis to pitch.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::IterationXAxisPitchClickEvent(wxCommandEvent& WXUNUSED(event))
{
	// Make sure the object is TypeIteration
	if (openObjectList[objectOfInterestIndex]->GetType() != GuiObject::TypeIteration)
		return;

	// Set the X axis to pitch
	static_cast<Iteration*>(openObjectList[objectOfInterestIndex])
		->SetXAxisType(Iteration::AxisTypePitch);

	// Uncheck all other X axis options
	if (menuBar->FindItem(IdMenuIterationXAxisPitch) != NULL)
	{
		menuBar->Check(IdMenuIterationXAxisRoll, false);
		menuBar->Check(IdMenuIterationXAxisHeave, false);
		menuBar->Check(IdMenuIterationXAxisRackTravel, false);
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		IterationXAxisRollClickEvent
//
// Description:		Event handler for setting iteration x-axis to roll.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::IterationXAxisRollClickEvent(wxCommandEvent& WXUNUSED(event))
{
	// Make sure the object is TypeIteration
	if (openObjectList[objectOfInterestIndex]->GetType() != GuiObject::TypeIteration)
		return;

	// Set the X axis to roll
	static_cast<Iteration*>(openObjectList[objectOfInterestIndex])
		->SetXAxisType(Iteration::AxisTypeRoll);

	// Uncheck all other X axis options
	if (menuBar->FindItem(IdMenuIterationXAxisRoll) != NULL)
	{
		menuBar->Check(IdMenuIterationXAxisPitch, false);
		menuBar->Check(IdMenuIterationXAxisHeave, false);
		menuBar->Check(IdMenuIterationXAxisRackTravel, false);
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		IterationXAxisHeaveClickEvent
//
// Description:		Event handler for setting iteration x-axis to heave.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::IterationXAxisHeaveClickEvent(wxCommandEvent& WXUNUSED(event))
{
	// Make sure the object is TypeIteration
	if (openObjectList[objectOfInterestIndex]->GetType() != GuiObject::TypeIteration)
		return;

	// Set the X axis to heave
	static_cast<Iteration*>(openObjectList[objectOfInterestIndex])
		->SetXAxisType(Iteration::AxisTypeHeave);

	// Uncheck all other X axis options
	if (menuBar->FindItem(IdMenuIterationXAxisHeave) != NULL)
	{
		menuBar->Check(IdMenuIterationXAxisPitch, false);
		menuBar->Check(IdMenuIterationXAxisRoll, false);
		menuBar->Check(IdMenuIterationXAxisRackTravel, false);
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		IterationXAxisRackTravelClickEvent
//
// Description:		Event handler for setting iteration x-axis to steer.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::IterationXAxisRackTravelClickEvent(wxCommandEvent& WXUNUSED(event))
{
	// Make sure the object is TypeIteration
	if (openObjectList[objectOfInterestIndex]->GetType() != GuiObject::TypeIteration)
		return;

	// Set the X axis to rack travel
	static_cast<Iteration*>(openObjectList[objectOfInterestIndex])
		->SetXAxisType(Iteration::AxisTypeRackTravel);

	// Uncheck all other X axis options
	if (menuBar->FindItem(IdMenuIterationXAxisRackTravel) != NULL)
	{
		menuBar->Check(IdMenuIterationXAxisPitch, false);
		menuBar->Check(IdMenuIterationXAxisRoll, false);
		menuBar->Check(IdMenuIterationXAxisHeave, false);
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		ViewToolbarsKinematicEvent
//
// Description:		Event handler for the View menu's Kinematic Toolbar item.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ViewToolbarsKinematicEvent(wxCommandEvent &event)
{
	manager.GetPane(kinematicToolbar).Show(event.IsChecked());
	manager.Update();
}

//==========================================================================
// Class:			MainFrame
// Function:		ViewToolbarsQuasiStaticEvent
//
// Description:		Event handler for the View menu's Quasi-Static Toolbar item.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ViewToolbarsQuasiStaticEvent(wxCommandEvent &event)
{
	manager.GetPane(quasiStaticToolbar).Show(event.IsChecked());
	manager.Update();
}

//==========================================================================
// Class:			MainFrame
// Function:		ViewToolbars3DEvent
//
// Description:		Event handler for the View menu's Kinematic Toolbar item.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ViewToolbars3DEvent(wxCommandEvent &event)
{
	manager.GetPane(toolbar3D).Show(event.IsChecked());
	manager.Update();
}

//==========================================================================
// Class:			MainFrame
// Function:		ViewSystemsTreeEvent
//
// Description:		Toggles visibility of the system tree.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ViewSystemsTreeEvent(wxCommandEvent &event)
{
	manager.GetPane(systemsTree).Show(event.IsChecked());
	manager.Update();
}

//==========================================================================
// Class:			MainFrame
// Function:		ViewEditPanelEvent
//
// Description:		Toggles visibility of the edit panel.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ViewEditPanelEvent(wxCommandEvent &event)
{
	manager.GetPane(editPanel).Show(event.IsChecked());
	manager.Update();
}

//==========================================================================
// Class:			MainFrame
// Function:		ViewOutputPaneEvent
//
// Description:		Toggles visibility of the output pane.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ViewOutputPaneEvent(wxCommandEvent &event)
{
	manager.GetPane(debugPane).Show(event.IsChecked());
	manager.Update();
}

//==========================================================================
// Class:			MainFrame
// Function:		ViewOutputListEvent
//
// Description:		Toggles visibility of the output list.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ViewOutputListEvent(wxCommandEvent &event)
{
	manager.GetPane(outputPanel).Show(event.IsChecked());
	manager.Update();
}

//==========================================================================
// Class:			MainFrame
// Function:		ViewClearOutputEvent
//
// Description:		Clears all of the text in the OutputPane.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ViewClearOutputEvent(wxCommandEvent& WXUNUSED(event))
{
	// Set the text in the DebugPane to an empty string
	debugPane->ChangeValue(_T(""));
}

//==========================================================================
// Class:			MainFrame
// Function:		ToolsDoEEvent
//
// Description:		Event handler for the Tools menu's Design of Experiments item.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ToolsDoEEvent(wxCommandEvent &event)
{
	event.Skip();
}

//==========================================================================
// Class:			MainFrame
// Function:		ToolsDynamicsEvent
//
// Description:		Event handler for the Tools menu's Dynamics item.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ToolsDynamicEvent(wxCommandEvent &event)
{
	event.Skip();
}

//==========================================================================
// Class:			MainFrame
// Function:		ToolsOptionsEvent
//
// Description:		Displays the option dialog, allowing the user to specify
//					preferences.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ToolsOptionsEvent(wxCommandEvent& WXUNUSED(event))
{
	// Create the dialog box
	OptionsDialog optionsDialog(*this, kinematicInputs, wxID_ANY, wxDefaultPosition);

	// Display the dialog
	if (optionsDialog.ShowModal() == wxOK)
	{
		// FIXME:  Write the updated options to the registry

		// Update the edit panel
		editPanel->UpdateInformation();

		// Update the analyses
		UpdateAnalysis();
		UpdateOutputPanel();

		// Make sure we have an object to update before we try to update it
		if (openObjectList.GetCount() > 0)
			openObjectList[activeIndex]->UpdateData();
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		HelpManualEvent
//
// Description:		Event handler for the Help menu's Manual item.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::HelpManualEvent(wxCommandEvent& WXUNUSED(event))
{
	// Call the shell to display the user's manual
	wxString openPDFManualCommand;
	wxString manualFileName(_T("vvase manual.pdf"));
	wxMimeTypesManager mimeManager;

	// In Linux, we need to put the file name in quotes
#ifdef __WXGTK__
	manualFileName.Prepend(_T("'"));
	manualFileName.Append(_T("'"));
#endif

	wxFileType *pdfFileType = mimeManager.GetFileTypeFromExtension(_T("pdf"));// we now own this memory
	if (!pdfFileType)
		Debugger::GetInstance() << "ERROR:  Unknown extension 'pdf'" << Debugger::PriorityHigh;
	else if (!pdfFileType->GetOpenCommand(&openPDFManualCommand,
		wxFileType::MessageParameters(manualFileName)))
		Debugger::GetInstance() << "ERROR:  No known OPEN command for .pdf files" << Debugger::PriorityHigh;
	else
	{
		if (wxExecute(openPDFManualCommand) == 0)
			Debugger::GetInstance() << "ERROR:  Could not find '" << manualFileName << "'" << Debugger::PriorityHigh;
	}

	delete pdfFileType;
}

//==========================================================================
// Class:			MainFrame
// Function:		HelpAboutEvent
//
// Description:		Displays an about message box with some information
//					about the application.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::HelpAboutEvent(wxCommandEvent& WXUNUSED(event))
{
	wxAboutDialogInfo appInfo;

	appInfo.SetName(carDesignerLongName);
	appInfo.SetVersion(carDesignerVersion + _T(" (") + carDesignerGitHash + _T(")"));
	appInfo.SetDescription(_T("\n\
A work in progress...\n\
This is a vehicle design and analysis tool.  Please see the\n\
readme.md file for licensing and other information."));
	appInfo.SetCopyright(_T("(C) 2008-2016 Kerry Loux"));

	wxAboutBox(appInfo);
}

//==========================================================================
// Class:			MainFrame
// Function:		UpdateAnalysis
//
// Description:		Updates the information associated with each object.
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
void MainFrame::UpdateAnalysis()
{
	// For every object we've got open, call the update display method
	unsigned int i;
	for (i = 0; i < openObjectList.GetCount(); i++)
		// Update the display (this performs the kinematic analysis)
		openObjectList[i]->UpdateData();
}

//==========================================================================
// Class:			MainFrame
// Function:		UpdateOutputPanel
//
// Description:		Updates the output display with the information currently
//					in each car object.
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
void MainFrame::UpdateOutputPanel()
{
	// For every object we've got open, call the update display method
	unsigned int i, carCount = 0;
	for (i = 0; i < openObjectList.GetCount(); i++)
	{
		// Update the kinematics information (ONLY if this is a car)
		if (openObjectList[i]->GetType() == GuiObject::TypeCar)
		{
			carCount++;

			// Update the information for this car
			outputPanel->UpdateInformation(static_cast<GuiCar*>(openObjectList[i])->GetOutputs(),
				static_cast<GuiCar*>(openObjectList[i])->GetWorkingCar(),
				carCount, openObjectList[i]->GetCleanName());
		}
	}

	// Make sure the output panel doesn't have more data columns than there are cars
	outputPanel->FinishUpdate(carCount);
}

//==========================================================================
// Class:			MainFrame
// Function:		AddJob
//
// Description:		Adds a job to the job queue to be handled by the thread pool.
//
// Input Arguments:
//		newJob	= ThreadJob& containing in the information about the new job to
//				  be performed
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::AddJob(ThreadJob &newJob)
{
	assert(activeThreads > 0);
	if (applicationExiting)
		return;

	jobQueue->AddJob(newJob, JobQueue::PriorityNormal);
	openJobCount++;
}

//==========================================================================
// Class:			MainFrame
// Function:		KinematicToolbarPitchChangeEvent
//
// Description:		Event that fires when the pitch text box changes value.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::KinematicToolbarPitchChangeEvent(wxCommandEvent& WXUNUSED(event))
{
	// Get a pointer to the pitch text box
	wxTextCtrl *textBox = static_cast<wxTextCtrl*>(kinematicToolbar->FindControl(IdToolbarKinematicPitch));

	// Make sure the text box was found (may not be found on startup)
	if (textBox == NULL)
		return;

	// Get the value from the text box and convert it to a double
	double value;
	if (!textBox->GetValue().ToDouble(&value))
		// The value was non-numeric - don't do anything
		return;

	// Set the value for the kinematic analysis object
	kinematicInputs.pitch = UnitConverter::GetInstance().ConvertAngleInput(value);

	lastAnalysisWasKinematic = true;

	// Update the analysis
	UpdateAnalysis();
	UpdateOutputPanel();
}

//==========================================================================
// Class:			MainFrame
// Function:		KinematicToolbarRollChangeEvent
//
// Description:		Event that fires when the roll text box changes value.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::KinematicToolbarRollChangeEvent(wxCommandEvent& WXUNUSED(event))
{
	// Get a pointer to the roll text box
	wxTextCtrl *textBox = static_cast<wxTextCtrl*>(kinematicToolbar->FindControl(IdToolbarKinematicRoll));

	// Make sure the text box was found (may not be found on startup)
	if (textBox == NULL)
		return;

	// Get the value from the text box and convert it to a double
	double value;
	if (!textBox->GetValue().ToDouble(&value))
		// The value was non-numeric - don't do anything
		return;

	// Set the value for the kinematic analysis object
	kinematicInputs.roll = UnitConverter::GetInstance().ConvertAngleInput(value);

	lastAnalysisWasKinematic = true;

	// Update the analysis
	UpdateAnalysis();
	UpdateOutputPanel();
}

//==========================================================================
// Class:			MainFrame
// Function:		KinematicToolbarHeaveChangeEvent
//
// Description:		Event that fires when the heave text box changes value.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::KinematicToolbarHeaveChangeEvent(wxCommandEvent& WXUNUSED(event))
{
	// Get a pointer to the heave text box
	wxTextCtrl *textBox = static_cast<wxTextCtrl*>(kinematicToolbar->FindControl(IdToolbarKinematicHeave));

	// Make sure the text box was found (may not be found on startup)d
	if (textBox == NULL)
		return;

	// Get the value from the text box and convert it to a double
	double value;
	if (!textBox->GetValue().ToDouble(&value))
		// The value was non-numeric - don't do anything
		return;

	// Set the value for the kinematic analysis object
	kinematicInputs.heave = UnitConverter::GetInstance().ConvertDistanceInput(value);

	lastAnalysisWasKinematic = true;

	// Update the analysis
	UpdateAnalysis();
	UpdateOutputPanel();
}

//==========================================================================
// Class:			MainFrame
// Function:		KinematicToolbarSteerChangeEvent
//
// Description:		Event that fires when the steer text box changes value.
//
// Input Arguments:
//		event	= wxCommandEvent&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::KinematicToolbarSteerChangeEvent(wxCommandEvent& WXUNUSED(event))
{
	wxTextCtrl *textBox = static_cast<wxTextCtrl*>(kinematicToolbar->FindControl(IdToolbarKinematicSteer));
	if (textBox == NULL)
		return;

	// Get the value from the text box and convert it to a double
	double value;
	if (!textBox->GetValue().ToDouble(&value))
		return;

	// Set the value for the kinematic analysis object depending on what the steering input represents
	if (useRackTravel)
		kinematicInputs.rackTravel = UnitConverter::GetInstance().ConvertDistanceInput(value);
	else
		kinematicInputs.rackTravel = UnitConverter::GetInstance().ConvertAngleInput(value) * 1.0;// * RackRatio;// FIXME:  Use rack ratio

	quasiStaticInputs.rackTravel = kinematicInputs.rackTravel;

	// Update the analysis
	UpdateAnalysis();
	UpdateOutputPanel();
}

//==========================================================================
// Class:			MainFrame
// Function:		QuasiStaticToolbarGxChangeEvent
//
// Description:		Applies Gx to quasi-static analysis.
//
// Input Arguments:
//		event	= &wxCommandEvent
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::QuasiStaticToolbarGxChangeEvent(wxCommandEvent& /*event*/)
{
	wxTextCtrl *textBox = static_cast<wxTextCtrl*>(quasiStaticToolbar->FindControl(IdToolbarQuasiStaticGx));

	if (textBox == NULL)
		return;

	double value;
	if (!textBox->GetValue().ToDouble(&value))
		return;

	quasiStaticInputs.gx = value;

	lastAnalysisWasKinematic = false;

	// Update the analysis
	UpdateAnalysis();
	UpdateOutputPanel();
}

//==========================================================================
// Class:			MainFrame
// Function:		QuasiStaticToolbarGyChangeEvent
//
// Description:		Applies Gy to quasi-static analysis.
//
// Input Arguments:
//		event	= &wxCommandEvent
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::QuasiStaticToolbarGyChangeEvent(wxCommandEvent& /*event*/)
{
	wxTextCtrl *textBox = static_cast<wxTextCtrl*>(quasiStaticToolbar->FindControl(IdToolbarQuasiStaticGy));

	if (textBox == NULL)
		return;

	double value;
	if (!textBox->GetValue().ToDouble(&value))
		return;

	quasiStaticInputs.gy = value;

	lastAnalysisWasKinematic = false;

	// Update the analysis
	UpdateAnalysis();
	UpdateOutputPanel();
}

//==========================================================================
// Class:			MainFrame
// Function:		Toolbar3DPerspectiveClickEvent
//
// Description:		Switches to perspective view.
//
// Input Arguments:
//		event	= &wxCommandEvent
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::Toolbar3DPerspectiveClickEvent(wxCommandEvent &/*event*/)
{
	useOrthoView = false;
	unsigned int i;
	for (i = 0; i < openObjectList.GetCount(); i++)
	{
		if (openObjectList[i]->GetType() == GuiObject::TypeCar)
		{
			static_cast<GuiCar*>(openObjectList[i])->SetUseOrtho(useOrthoView);
			openObjectList[i]->UpdateDisplay();
		}
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		Toolbar3DOrthoClickEvent
//
// Description:		Switches to orthographic view.
//
// Input Arguments:
//		event	= &wxCommandEvent
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::Toolbar3DOrthoClickEvent(wxCommandEvent &/*event*/)
{
	useOrthoView = true;
	unsigned int i;
	for (i = 0; i < openObjectList.GetCount(); i++)
	{
		if (openObjectList[i]->GetType() == GuiObject::TypeCar)
		{
			static_cast<GuiCar*>(openObjectList[i])->SetUseOrtho(useOrthoView);
			openObjectList[i]->UpdateDisplay();
		}
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		ThreadCompleteEvent
//
// Description:		Handles events when threads complete their jobs.  Depending
//					on the type of event, we send the results to different places.
//
// Input Arguments:
//		event	= &wxCommandEvent
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::ThreadCompleteEvent(wxCommandEvent &event)
{
	int carCount(0), i;
	
	// If the application is closing, ignore anything that's not a thread exit event
	if (applicationExiting && event.GetInt() != ThreadJob::CommandThreadExit)
	{
		openJobCount--;
		return;
	}
	
	// Perform different operations depending on the type of job that has completed
	switch (event.GetInt())
	{
	case ThreadJob::CommandThreadExit:
		activeThreads--;
		Debugger::GetInstance() << "Thread " << event.GetId() << " exited" << Debugger::PriorityLow;

		// If there are no more active threads, it is now safe to kill this window
		if (activeThreads == 0)
		{
			Destroy();
			return;
		}
		break;

	case ThreadJob::CommandThreadStarted:
		// Increment the number of active threads
		activeThreads++;
		Debugger::GetInstance() << "Thread " << event.GetId() << " started" << Debugger::PriorityLow;
		break;

	case ThreadJob::CommandThreadKinematicsNormal:
		// When closing, if multiple objects are open, it is possible that thread exit commands
		// are processed prior to others, so we check here and abort if the ID is invalid
		if (event.GetExtraLong() >= (long)openObjectList.GetCount())
			break;

		// Get the car count for this car (number of objects before this in the list that are also cars)
		for (i = 0; i <= event.GetExtraLong(); i++)
		{
			// Iterate through the open objects up to the selected object, and if it is a car,
			// increment the car count
			if (openObjectList[i]->GetType() == GuiObject::TypeCar)
				carCount++;
		}

		// Update the information for this car
		outputPanel->UpdateInformation(static_cast<GuiCar*>(openObjectList[
			event.GetExtraLong()])->GetOutputs(),
			static_cast<GuiCar*>(openObjectList[event.GetExtraLong()])->GetWorkingCar(),
			carCount, openObjectList[event.GetExtraLong()]->GetCleanName());

		// Call the 3D display update method
		openObjectList[event.GetExtraLong()]->UpdateDisplay();
		break;

	case ThreadJob::CommandThreadKinematicsIteration:
		// When closing, if multiple objects are open, it is possible that thread exit commands
		// are processed prior to others, so we check here and abort if the ID is invalid
		if (event.GetExtraLong() >= (long)openObjectList.GetCount())
			break;

		static_cast<Iteration*>(openObjectList[event.GetExtraLong()])->MarkAnalysisComplete();
		break;

	case ThreadJob::CommandThreadKinematicsGA:
		// When closing, if multiple objects are open, it is possible that thread exit commands
		// are processed prior to others, so we check here and abort if the ID is invalid
		if (event.GetExtraLong() >= (long)openObjectList.GetCount())
			break;

		static_cast<GeneticOptimization*>(openObjectList[event.GetExtraLong()])->MarkAnalysisComplete();
		break;

	case ThreadJob::CommandThreadGeneticOptimization:
		// When closing, if multiple objects are open, it is possible that thread exit commands
		// are processed prior to others, so we check here and abort if the ID is invalid
		if (event.GetExtraLong() >= (long)openObjectList.GetCount())
			break;

		static_cast<GeneticOptimization*>(openObjectList[event.GetExtraLong()])->CompleteOptimization();

		UpdateAnalysis();
		UpdateOutputPanel();
		break;

	case ThreadJob::CommandThreadNull:
	default:
		break;
	}

	openJobCount--;
}

//==========================================================================
// Class:			MainFrame
// Function:		DebugMessageEvent
//
// Description:		Prints debug messages when the debugger object posts messages.
//
// Input Arguments:
//		event	= &wxCommandEvent
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::DebugMessageEvent(wxCommandEvent &event)
{
	debugPane->AppendText(event.GetString());

#ifdef DEBUG_TO_FILE_
	// Useful for cases where the application crashes and text can no longer be viewed in the output pane
	// FIXME:  Make this part of the Debugger class?
	std::ofstream file("debug.txt", std::ios::app);
	file << event.GetString();
	file.close();
#endif
}

//==========================================================================
// Class:			MainFrame
// Function:		AddObjectToList
//
// Description:		Add a GuiObject object to the list of managed cars.  This
//					function should be called immediately after creation of
//					a new GuiObject.  The usual syntax and calling
//					sequence looks something like this (it is also important
//					to assign the index back to the GuiObject for future use):
//						GuiObject *NewObject = new GuiObject(this);
//						NewObject->SetIndex(AddObjectToList(NewObject));
//
// Input Arguments:
//		objectToAdd	= *GuiObject
//
// Output Arguments:
//		None
//
// Return Value:
//		integer = new index for the car that was just added to the list
//
//==========================================================================
int MainFrame::AddObjectToList(GuiObject *objectToAdd)
{
	// Add the object to the list (must be done before the menu permissions are updated)
	return openObjectList.Add(objectToAdd);
}

//==========================================================================
// Class:			MainFrame
// Function:		RemoveObjectFromList
//
// Description:		Removes a GuiObject object from the list of managed cars.
//					Should be done instead of deleting the GuiObject object.
//					This function will handle the deletion of the GuiObject
//					internally.
//
// Input Arguments:
//		index	= integer specifying the object to be removed
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::RemoveObjectFromList(int index)
{
	beingDeleted = true;
	openObjectList.Remove(index);

	// Reset the cars' indices and check to see what types we have available
	// Also refresh the car's displays
	unsigned int i;
	for (i = 0; i < openObjectList.GetCount(); i++)
	{
		openObjectList[i]->SetIndex(i);

		if (!applicationExiting)
		{
			// Update the data and displays - data first, because in some cases data is
			// dependent on other open objects, and we may have just closed one
			openObjectList[i]->UpdateData();
			openObjectList[i]->UpdateDisplay();
		}
	}

	beingDeleted = false;

	// Reset the active index - if there is still an open object, show the one with index zero
	// Otherwise, set the index to an invalid number
	if (openObjectList.GetCount() > 0)
		SetActiveIndex(0);
	else
		SetActiveIndex(-1);

	UpdateOutputPanel();
}

//==========================================================================
// Class:			MainFrame
// Function:		SetNotebookPage
//
// Description:		Sets the notebook page to the specified index.  The
//					index here should start at 0, just like the
//					ActiveIndex.
//
// Input Arguments:
//		index	= integer specifying the notebook page to activate
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::SetNotebookPage(int index)
{
	// Make sure the index could possibly be a valid index
	if (index >= 0 && index < (signed int)openObjectList.GetCount())
	{
		// Bring the desired notebook page to the front
		notebook->SetSelection(index);

		// Update the active object's display
		// (haven't identified the root cause, but sometimes the car disappears if this isn't done)
		// FIXME:  This works as a workaround for disappearing cars/plots as happens sometimes, but
		// causes intermittent crashes (access violations) for cars and always crashes new iterations
		// in FormatPlot() (Z values are NaN)
		//openObjectList[ActiveIndex]->UpdateDisplay();
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		WindowCloseEvent
//
// Description:		Calls CloseThisForm and depending on whether or not the
//					user confirms the close, it allows or prevents the form
//					closing.
//
// Input Arguments:
//		event	= &wxCloseEvent
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::WindowCloseEvent(wxCloseEvent& WXUNUSED(event))
{
	WriteConfiguration();

	// Kill this window if there aren't any more worker threads (assumes we've already
	// been through this once, and issued the EXIT command to all of the threads -
	// no need to ask for confirmation again!).  In practice, this should never happen,
	// as the window will be destroyed when the last thread exits.
	if (activeThreads == 0)
	{
		Destroy();
		return;
	}

	applicationExiting = true;

	// Get the user confirmation
	if (!CloseThisForm())
	{
		applicationExiting = false;
		UpdateAnalysis();
		return;// Don't delete the threads (and don't skip the event) to prevent exiting
	}

	// Delete all of the threads in the thread pool
	int i;
	for (i = 0; i < activeThreads; i++)
		jobQueue->AddJob(ThreadJob(ThreadJob::CommandThreadExit), JobQueue::PriorityVeryHigh);
}

//==========================================================================
// Class:			MainFrame
// Function:		OnSizeEvent
//
// Description:		Event handler for re-sizing events.  When this event triggers,
//					we need to update the displays for all of our GuiObjectS.
//					This is particularly important for plots, which do not
//					automatically re-size with the main window.
//
// Input Arguments:
//		event	= &wxSizeEvent (UNUSED)
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::OnSizeEvent(wxSizeEvent& WXUNUSED(event))
{
	// For each open object, update the display
	/*int i;
	for (i = 0; i < openObjectList.GetCount(); i++)
		openObjectList[i]->UpdateDisplay();*/
	// FIXME:  This is out-of-phase with the actual re-sizing!
}

//==========================================================================
// Class:			MainFrame
// Function:		CloseThisForm
//
// Description:		Starts a process to close the application.  If no cars
//					are open, or all of the open cars are saved, the user
//					is asked for confirmation to close.  Otherwise, a save
//					confirmation appears for each unsaved car, with the
//					option to cancel and abort the close.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the user confirms the close, false otherwise
//
//==========================================================================
bool MainFrame::CloseThisForm()
{
	while (openObjectList.GetCount() > 0)
	{
		if (!openObjectList[0]->Close())
			return false;
	}

	return true;
}

//==========================================================================
// Class:			MainFrame
// Function:		ReadConfiguration
//
// Description:		Reads the application configuration information from
//					file.
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
void MainFrame::ReadConfiguration()
{
	// Create a configuration file object
	wxFileConfig *configurationFile = new wxFileConfig(_T(""), _T(""),
		wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPathWithSep() + pathToConfigFile, _T(""),
		wxCONFIG_USE_RELATIVE_PATH);

	// Read UNITS configuration from file
	UnitConverter::GetInstance().SetAccelerationUnits(UnitConverter::UnitsOfAcceleration(
		configurationFile->Read(_T("/Units/Acceleration"), 0l)));
	UnitConverter::GetInstance().SetAngleUnits(UnitConverter::UnitsOfAngle(
		configurationFile->Read(_T("/Units/Angle"), 1l)));
	UnitConverter::GetInstance().SetAreaUnits(UnitConverter::UnitsOfArea(
		configurationFile->Read(_T("/Units/Area"), 0l)));
	UnitConverter::GetInstance().SetDensityUnits(UnitConverter::UnitsOfDensity(
		configurationFile->Read(_T("/Units/Density"), 0l)));
	UnitConverter::GetInstance().SetDistanceUnits(UnitConverter::UnitsOfDistance(
		configurationFile->Read(_T("/Units/Distance"), 0l)));
	UnitConverter::GetInstance().SetEnergyUnits(UnitConverter::UnitsOfEnergy(
		configurationFile->Read(_T("/Units/Energy"), 0l)));
	UnitConverter::GetInstance().SetForceUnits(UnitConverter::UnitsOfForce(
		configurationFile->Read(_T("/Units/Force"), 0l)));
	UnitConverter::GetInstance().SetInertiaUnits(UnitConverter::UnitsOfInertia(
		configurationFile->Read(_T("/Units/Inertia"), 0l)));
	UnitConverter::GetInstance().SetMassUnits(UnitConverter::UnitsOfMass(
		configurationFile->Read(_T("/Units/Mass"), 1l)));
	UnitConverter::GetInstance().SetMomentUnits(UnitConverter::UnitsOfMoment(
		configurationFile->Read(_T("/Units/Moment"), 0l)));
	UnitConverter::GetInstance().SetPowerUnits(UnitConverter::UnitsOfPower(
		configurationFile->Read(_T("/Units/Power"), 0l)));
	UnitConverter::GetInstance().SetPressureUnits(UnitConverter::UnitsOfPressure(
		configurationFile->Read(_T("/Units/Pressure"), 0l)));
	UnitConverter::GetInstance().SetTemperatureUnits(UnitConverter::UnitsOfTemperature(
		configurationFile->Read(_T("/Units/Temperature"), 0l)));
	UnitConverter::GetInstance().SetVelocityUnits(UnitConverter::UnitsOfVelocity(
		configurationFile->Read(_T("/Units/Velocity"), 0l)));

	// Read NUMBER FORMAT configuration from file
	UnitConverter::GetInstance().SetNumberOfDigits(configurationFile->Read(_T("/NumberFormat/NumberOfDigits"), 3l));
	bool tempBool = UnitConverter::GetInstance().GetUseScientificNotation();
	configurationFile->Read(_T("/NumberFormat/UseScientificNotation"), &tempBool);
	UnitConverter::GetInstance().SetUseScientificNotation(tempBool);
	tempBool = UnitConverter::GetInstance().GetUseSignificantDigits();
	configurationFile->Read(_T("/NumberFormat/UseSignificantDigits"), &tempBool);
	UnitConverter::GetInstance().SetUseSignificantDigits(tempBool);

	// Read KINEMATICS configuration from file
	double tempDouble = 0.0;
	configurationFile->Read(_T("/Kinematics/CenterOfRotationX"), &tempDouble);
	kinematicInputs.centerOfRotation.x = tempDouble;
	tempDouble = 0.0;
	configurationFile->Read(_T("/Kinematics/CenterOfRotationY"), &tempDouble);
	kinematicInputs.centerOfRotation.y = tempDouble;
	tempDouble = 0.0;
	configurationFile->Read(_T("/Kinematics/CenterOfRotationZ"), &tempDouble);
	kinematicInputs.centerOfRotation.z = tempDouble;
	kinematicInputs.firstRotation = (Vector::Axis)configurationFile->Read(
		_T("/Kinematics/FirstRotation"), 0l);
	configurationFile->Read(_T("/Kinematics/UseRackTravel"), &useRackTravel, true);

	// Read DEBUGGING configuration from file
	Debugger::GetInstance().SetDebugLevel(Debugger::DebugLevel(configurationFile->Read(_T("/Debugging/DebugLevel"), 1l)));

	// Read GUI configuration from file
	wxString layoutString;
	if (configurationFile->Read(_T("/GUI/LayoutString"), &layoutString))
		manager.LoadPerspective(layoutString);
	configurationFile->Read(_T("/GUI/IsMaximized"), &tempBool, false);
	if (tempBool)
		Maximize();
	else
	{
		SetSize(configurationFile->Read(_T("/GUI/SizeX"), minFrameSize.GetWidth()),
			configurationFile->Read(_T("/GUI/SizeY"), minFrameSize.GetHeight()));
		int xPosition = 0, yPosition = 0;
		if (configurationFile->Read(_T("/GUI/PositionX"), &xPosition)
			&& configurationFile->Read(_T("/GUI/PositionY"), &yPosition))
			SetPosition(wxPoint(xPosition, yPosition));
		else
			Center();
	}

	// Read SOLVER configuration from file
	SetNumberOfThreads(configurationFile->Read(_T("/Solver/NumberOfThreads"), wxThread::GetCPUCount() * 2));
	
	// Read FONT configuration from file
	wxFont font;
	font.SetNativeFontInfo(configurationFile->Read(_T("/Fonts/OutputFont"), wxEmptyString));
	SetOutputFont(font);
	font.SetNativeFontInfo(configurationFile->Read(_T("/Fonts/PlotFont"), wxEmptyString));
	SetPlotFont(font);
	
	// Read rendering options
	configurationFile->Read(_T("/Renderer/useOrtho"), &useOrthoView, false);

	// Read recent file history
	recentFileManager->Load(*configurationFile);

	delete configurationFile;
	configurationFile = NULL;
}

//==========================================================================
// Class:			MainFrame
// Function:		WriteConfiguration
//
// Description:		Writes the application configuration information to file.
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
void MainFrame::WriteConfiguration()
{
	// Create a configuration file object
	wxFileConfig *configurationFile = new wxFileConfig(_T(""), _T(""),
		wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPathWithSep() + pathToConfigFile, _T(""),
		wxCONFIG_USE_RELATIVE_PATH);

	// Write UNITS configuration to file
	configurationFile->Write(_T("/Units/Acceleration"), (int)UnitConverter::GetInstance().GetAccelerationUnits());
	configurationFile->Write(_T("/Units/Angle"), (int)UnitConverter::GetInstance().GetAngleUnits());
	configurationFile->Write(_T("/Units/Area"), (int)UnitConverter::GetInstance().GetAreaUnits());
	configurationFile->Write(_T("/Units/Density"), (int)UnitConverter::GetInstance().GetDensityUnits());
	configurationFile->Write(_T("/Units/Distance"), (int)UnitConverter::GetInstance().GetDistanceUnits());
	configurationFile->Write(_T("/Units/Energy"), (int)UnitConverter::GetInstance().GetEnergyUnits());
	configurationFile->Write(_T("/Units/Force"), (int)UnitConverter::GetInstance().GetForceUnits());
	configurationFile->Write(_T("/Units/Inertia"), (int)UnitConverter::GetInstance().GetInertiaUnits());
	configurationFile->Write(_T("/Units/Mass"), (int)UnitConverter::GetInstance().GetMassUnits());
	configurationFile->Write(_T("/Units/Moment"), (int)UnitConverter::GetInstance().GetMomentUnits());
	configurationFile->Write(_T("/Units/Power"), (int)UnitConverter::GetInstance().GetPowerUnits());
	configurationFile->Write(_T("/Units/Pressure"), (int)UnitConverter::GetInstance().GetPressureUnits());
	configurationFile->Write(_T("/Units/Temperature"), (int)UnitConverter::GetInstance().GetTemperatureUnits());
	configurationFile->Write(_T("/Units/Velocity"), (int)UnitConverter::GetInstance().GetVelocityUnits());

	// Write NUMBER FORMAT configuration to file
	configurationFile->Write(_T("/NumberFormat/NumberOfDigits"), UnitConverter::GetInstance().GetNumberOfDigits());
	configurationFile->Write(_T("/NumberFormat/UseScientificNotation"), UnitConverter::GetInstance().GetUseScientificNotation());
	configurationFile->Write(_T("/NumberFormat/UseSignificantDigits"), UnitConverter::GetInstance().GetUseSignificantDigits());

	// Write KINEMATICS configuration to file
	configurationFile->Write(_T("/Kinematics/CenterOfRotationX"), kinematicInputs.centerOfRotation.x);
	configurationFile->Write(_T("/Kinematics/CenterOfRotationY"), kinematicInputs.centerOfRotation.y);
	configurationFile->Write(_T("/Kinematics/CenterOfRotationZ"), kinematicInputs.centerOfRotation.z);
	configurationFile->Write(_T("/Kinematics/FirstRotation"), (int)kinematicInputs.firstRotation);
	configurationFile->Write(_T("/Kinematics/UseRackTravel"), useRackTravel);

	// Write DEBUGGING configuration to file
	configurationFile->Write(_T("/Debugging/DebugLevel"), (int)Debugger::GetInstance().GetDebugLevel());

	// Write GUI configuration to file
	configurationFile->Write(_T("/GUI/LayoutString"), manager.SavePerspective());
	configurationFile->Write(_T("/GUI/IsMaximized"), IsMaximized());
	configurationFile->Write(_T("/GUI/SizeX"), GetSize().GetX());
	configurationFile->Write(_T("/GUI/SizeY"), GetSize().GetY());
	configurationFile->Write(_T("/GUI/PositionX"), GetPosition().x);
	configurationFile->Write(_T("/GUI/PositionY"), GetPosition().y);

	// Write SOLVER configuration to file
	configurationFile->Write(_T("/Solver/NumberOfThreads"), numberOfThreads);
	
	// Write FONTS configuration to file
	if (outputFont.IsOk())
		configurationFile->Write(_T("/Fonts/OutputFont"),
			outputFont.GetNativeFontInfoDesc());
	if (plotFont.IsOk())
		configurationFile->Write(_T("/Fonts/PlotFont"),
			plotFont.GetNativeFontInfoDesc());
			
	configurationFile->Write(_T("/Renderer/useOrtho"), useOrthoView);

	// Write recent file history
	recentFileManager->Save(*configurationFile);

	// Delete file object
	delete configurationFile;
	configurationFile = NULL;
}

//==========================================================================
// Class:			MainFrame
// Function:		UpdateActiveObjectMenu
//
// Description:		Updates the active object-specific menu to the new
//					active object's type.
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
void MainFrame::UpdateActiveObjectMenu()
{
	// Try to get a handle to object specific menus
	int carMenuIndex = menuBar->FindMenu(_T("Car"));
	int iterationMenuIndex = menuBar->FindMenu(_T("Iteration"));
	//int gaMenuIndex = menuBar->FindMenu(_T("GA"));

	// Store the type of the active object
	GuiObject::ItemType activeType;

	// Make sure the active index is valid
	if (activeIndex < 0)
		activeType = GuiObject::TypeNone;
	else
		activeType = openObjectList[activeIndex]->GetType();

	switch (activeType)
	{
	case GuiObject::TypeCar:
		// Check to see if the car menu already exists
		if (carMenuIndex == wxNOT_FOUND)
		{
			// Check to see if we first need to remove the iteration menu
			if (iterationMenuIndex == wxNOT_FOUND)
				// Just insert a new menu
				menuBar->Insert(3, CreateCarMenu(), _T("&Car"));
			else
				// Replace the iteration menu with a new car menu
				delete menuBar->Replace(iterationMenuIndex, CreateCarMenu(), _T("&Car"));
		}
		break;

	case GuiObject::TypeIteration:
		// Check to see if the iteration menu already exists
		if (iterationMenuIndex == wxNOT_FOUND)
		{
			// Check to see if we first need to remove the car menu
			if (carMenuIndex == wxNOT_FOUND)
				// Just insert a new menu
				menuBar->Insert(3, CreateIterationMenu(), _T("&Iteration"));
			else
				// Replace the car menu with a new iteration menu
				delete menuBar->Replace(carMenuIndex, CreateIterationMenu(), _T("&Iteration"));
		}
		else
			// For iteration objects, we need to update the menu for every object
			// Just having the menu there already isn't good enough (checks, etc.)
			// So we replace the existing Iteration menu with a new one
			delete menuBar->Replace(iterationMenuIndex, CreateIterationMenu(), _T("&Iteration"));
		break;

	// Unused cases
	case GuiObject::TypeNone:
	case GuiObject::TypeOptimization:
	default:
		// Remove all menus
		if (carMenuIndex != wxNOT_FOUND)
			delete menuBar->Remove(carMenuIndex);
		else if (iterationMenuIndex != wxNOT_FOUND)
			delete menuBar->Remove(iterationMenuIndex);
		/*else if (gaMenuIndex != wxNOT_FOUND)
			delete menuBar->Remove(gaMenuIndex);*/
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		SetActiveIndex
//
// Description:		Sets the active index to the specified value and brings
//					the associated notebook page to the front.
//
// Input Arguments:
//		index				= integer specifying the current active object
//		selectNotebookTab	= bool indicated whether or not to change the current
//							  notebook page (optional)
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::SetActiveIndex(int index, bool selectNotebookTab)
{
	// Make sure the index is valid
	if (index >= (signed int)openObjectList.GetCount())
		return;

	// Set the active index
	activeIndex = index;

	// Also update the object of interest index
	objectOfInterestIndex = activeIndex;

	// Bring the appropriate notebook page to the top and render the scene (if desired)
	if (selectNotebookTab)
		SetNotebookPage(activeIndex);

	if (activeIndex >= 0)
	{
		// Highlight the column in the output panel that corresponds to this car
		outputPanel->HighlightColumn(openObjectList[activeIndex]->GetCleanName());

		// Update the EditPanel
		editPanel->UpdateInformation(openObjectList[activeIndex]);
	}
	else
		editPanel->UpdateInformation(NULL);

	// If the active object is not selected in the SystemsTree, select it now
	// Get the selected item's ID
	wxTreeItemId selectedId;
	systemsTree->GetSelectedItem(&selectedId);

	// Make sure there was an item selected (ID is valid?)
	if (selectedId.IsOk() && activeIndex >= 0)
	{
		// Check to see if the ID belongs to our active object
		if (!openObjectList[activeIndex]->IsThisObjectSelected(selectedId))
			// If it does not, then select the root item for the newly active object
			openObjectList[activeIndex]->SelectThisObjectInTree();
	}

	// Update the object specific menus
	UpdateActiveObjectMenu();
}

//==========================================================================
// Class:			MainFrame
// Function:		CreateContextMenu
//
// Description:		Displays a context menu that is customized for the object
//					specified by ObjectIndex.
//
// Input Arguments:
//		objectIndex		= integer specifying the object in the openObjectList
//						  that this menu is being created for
//		allowClosing	= bool, specifying whether or not we should add the
//						  "Close" item to the menu
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::CreateContextMenu(int objectIndex, bool allowClosing)
{
	// Make sure the index is valid before continuing
	if (objectIndex < 0 || objectIndex >= (signed int)openObjectList.GetCount())
		return;

	// Set the object of interest to the specified object
	objectOfInterestIndex = objectIndex;

	// Declare the menu variable and get the position of the cursor
	wxMenu *contextMenu(NULL);

	// Depending on the type of the item, we might want additional options (or none at all)
	switch (openObjectList[objectIndex]->GetType())
	{
	case GuiObject::TypeCar:
		contextMenu = CreateCarMenu();
		break;

	case GuiObject::TypeIteration:
		contextMenu = CreateIterationMenu();
		break;

	// Unused types
	case GuiObject::TypeOptimization:
	case GuiObject::TypeNone:
		break;

	// Fail on unknown types to avoid forgetting any
	default:
		assert(0);
		return;
	}

	// Start building the context menu

	// Genetic algorithms do not have image files
	if (openObjectList[objectIndex]->GetType() != GuiObject::TypeOptimization)
	{
		contextMenu->PrependSeparator();
		contextMenu->Prepend(IdMenuFileWriteImageFile, _T("&Write Image File"));
	}

	// Only add the close item if it was specified that we should
	if (allowClosing)
		contextMenu->Prepend(IdMenuFileClose, _T("&Close"));

	contextMenu->Prepend(IdMenuFileSave, _T("&Save"));

	// Show the menu
	PopupMenu(contextMenu);

	// Delete the context menu object
	delete contextMenu;
	contextMenu = NULL;

	// The event handlers for whatever was clicked get called here
	// Reset the object index to the active object index
	objectOfInterestIndex = activeIndex;
}

//==========================================================================
// Class:			MainFrame
// Function:		CreateCarMenu
//
// Description:		Creates a drop-down menu for TypeCar objects.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		wxMenu* pointing to the newly created menu
//
//==========================================================================
wxMenu *MainFrame::CreateCarMenu()
{
	wxMenu *mnuCar = new wxMenu();
	mnuCar->Append(IdMenuCarAppearanceOptions, _T("Appearance Options"));

	return mnuCar;
}

//==========================================================================
// Class:			MainFrame
// Function:		CreateIterationMenu
//
// Description:		Creates a drop-down menu for TypeIteration objects.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		wxMenu* pointing to the newly created menu
//
//==========================================================================
wxMenu *MainFrame::CreateIterationMenu()
{
	wxMenu *mnuIteration = new wxMenu();

	// Make sure the active object is a TYPE_ITERATION object
	if (openObjectList[activeIndex]->GetType() != GuiObject::TypeIteration)
		return mnuIteration;

	// Allocate the sub-menus
	wxMenu *associatedCarsMenu = new wxMenu();
	wxMenu *xAxisMenu = new wxMenu();

	mnuIteration->Append(IdMenuIterationExportDataToFile, _T("Export Data"));

	// Create and append the associated cars sub-menu
	associatedCarsMenu->Append(IdMenuIterationShowAssociatedCars, _T("Choose Associated Cars"));
	associatedCarsMenu->AppendSeparator();
	associatedCarsMenu->AppendCheckItem(IdMenuIterationAssociatedWithAllCars, _T("Associate With All Cars"));
	mnuIteration->AppendSubMenu(associatedCarsMenu, _T("Associated Cars"));

	// Create and append the x-axis sub-menu
	xAxisMenu->AppendCheckItem(IdMenuIterationXAxisPitch, _T("Pitch"));
	xAxisMenu->AppendCheckItem(IdMenuIterationXAxisRoll, _T("Roll"));
	xAxisMenu->AppendCheckItem(IdMenuIterationXAxisHeave, _T("Heave"));
	xAxisMenu->AppendCheckItem(IdMenuIterationXAxisRackTravel, _T("Rack Travel"));
	mnuIteration->AppendSubMenu(xAxisMenu, _T("Set X-Axis"));

	// Determine which items need to be checked
	if (static_cast<Iteration*>(openObjectList[activeIndex])->GetAutoAssociate())
		associatedCarsMenu->Check(IdMenuIterationAssociatedWithAllCars, true);

	switch (static_cast<Iteration*>(openObjectList[activeIndex])->GetXAxisType())
	{
	case Iteration::AxisTypePitch:
		xAxisMenu->Check(IdMenuIterationXAxisPitch, true);
		break;

	case Iteration::AxisTypeRoll:
		xAxisMenu->Check(IdMenuIterationXAxisRoll, true);
		break;

	case Iteration::AxisTypeHeave:
		xAxisMenu->Check(IdMenuIterationXAxisHeave, true);
		break;

	case Iteration::AxisTypeRackTravel:
		xAxisMenu->Check(IdMenuIterationXAxisRackTravel, true);
		break;

	case Iteration::AxisTypeUnused:
		// Take no action
		break;

	default:
		assert(0);
		break;
	}

	return mnuIteration;
}

//==========================================================================
// Class:			MainFrame
// Function:		JobsPending
//
// Description:		Returns true if there are any outstanding jobs.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for outstanding jobs, false for all caught up
//
//==========================================================================
bool MainFrame::JobsPending() const
{
	// If there are any jobs in the queue, return false
	if (jobQueue->PendingJobs() > 0)
		return true;

	// If there are any open jobs, return false
	if (openJobCount > 0)
		return true;

	// Otherwise, return true
	return false;
}

//==========================================================================
// Class:			MainFrame
// Function:		GetFileNameFromUser
//
// Description:		Displays a dialog asking the user to specify a file name.
//					Arguments allow this to be for opening or saving files,
//					with different options for the wildcards.
//
// Input Arguments:
//		dialogTitle			= wxString containing the title for the dialog
//		defaultDirectory	= wxString specifying the initial directory
//		defaultFileName		= wxString specifying the default file name
//		wildcard			= wxString specifying the list of file types to
//							  allow the user to select
//		style				= long integer specifying the type of dialog
//							  (this is usually wxFD_OPEN or wxFD_SAVE)
//
// Output Arguments:
//		None
//
// Return Value:
//		wxArrayString containing the paths and file names of the specified files,
//		or and empty array if the user cancels
//
//==========================================================================
wxArrayString MainFrame::GetFileNameFromUser(wxString dialogTitle, wxString defaultDirectory,
										 wxString defaultFileName, wxString wildcard, long style)
{
	// Work-around for weird bug where this method causes tree item selection change event to fire
	beingDeleted = true;

	// Initialize the return variable
	wxArrayString pathsAndFileNames;

	// Step 1 is to ask the user to specify the file name
	wxFileDialog dialog(this, dialogTitle, defaultDirectory, defaultFileName,
		wildcard, style);

	// Set the dialog to display center screen at the user's home directory
	dialog.CenterOnParent();

	// Display the dialog and make sure the user clicked OK
	if (dialog.ShowModal() == wxID_OK)// FIXME:  This changes the objectOfInterestIndex, which screws stuff up - why is that?
	{
		// If this was an open dialog, we want to get all of the selected paths,
		// otherwise, just get the one selected path
		if (style & wxFD_OPEN)
			dialog.GetPaths(pathsAndFileNames);
		else
			pathsAndFileNames.Add(dialog.GetPath());
	}

	// Work-around for weird bug where this method causes tree item selection change event to fire
	beingDeleted = false;

	// Return the path and file name
	return pathsAndFileNames;
}

//==========================================================================
// Class:			MainFrame
// Function:		LoadFile
//
// Description:		Public method for loading a single object from file.
//
// Input Arguments:
//		pathAndFileName	= wxString
//
// Output Arguments:
//		None
//
// Return Value:
//		true for file successfully loaded, false otherwise
//
//==========================================================================
bool MainFrame::LoadFile(wxString pathAndFileName)
{
	int startOfExtension;
	wxString fileExtension;
	GuiObject *tempObject = NULL;

	// Decipher the file name to figure out what kind of object this is
	startOfExtension = pathAndFileName.Last('.') + 1;
	fileExtension = pathAndFileName.Mid(startOfExtension);

	// FIXME:  This could be extension agnostic if we read something from the file header

	// Create the appropriate object
	if (fileExtension.CmpNoCase("car") == 0)
		tempObject = new GuiCar(*this, pathAndFileName);
	else if (fileExtension.CmpNoCase("iteration") == 0)
		tempObject = new Iteration(*this, pathAndFileName);
	else if (fileExtension.CmpNoCase("ga") == 0)
		tempObject = new GeneticOptimization(*this, pathAndFileName);
	else
	{
		Debugger::GetInstance() << "ERROR:  Unrecognized file extension: '" << fileExtension << "'" << Debugger::PriorityHigh;
		return false;
	}

	// If the object is not initialized, remove it from the list
	// (user canceled or errors occurred)
	if (!tempObject->IsInitialized())
	{
		RemoveObjectFromList(tempObject->GetIndex());
		return false;
	}
	else
		// Make the last object loaded the active
		SetActiveIndex(tempObject->GetIndex());

	return true;
}

//==========================================================================
// Class:			MainFrame
// Function:		AddFileToHistory
//
// Description:		Adds the specified file to the recent history list.
//
// Input Arguments:
//		pathAndFileName	= wxString
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::AddFileToHistory(wxString pathAndFileName)
{
	// Add the file to the manager
	recentFileManager->AddFileToHistory(pathAndFileName);
}

//==========================================================================
// Class:			MainFrame
// Function:		RemoveFileFromHistory
//
// Description:		Removes the specified file from the recent file list.
//					Looks up the object index based on the file name.
//
// Input Arguments:
//		pathAndFileName	= wxString
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void MainFrame::RemoveFileFromHistory(wxString pathAndFileName)
{
	// Identify the entry that matches the specified path and file nam
	unsigned int i;
	for (i = 0; i < recentFileManager->GetCount(); i++)
	{
		// If we find the specified file, remove it from the list
		if (recentFileManager->GetHistoryFile(i).CompareTo(pathAndFileName) == 0)
		{
			recentFileManager->RemoveFileFromHistory(i);
			break;
		}
	}
}

//==========================================================================
// Class:			MainFrame
// Function:		SetAssociateWithAllCars
//
// Description:		Checks or unchecks the iteration menu item for associate
//					with all open cars.
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
void MainFrame::SetAssociateWithAllCars()
{
	if (openObjectList[activeIndex]->GetType() != GuiObject::TypeIteration)
		return;

	wxMenuItem *item = this->FindItemInMenuBar(IdMenuIterationAssociatedWithAllCars);
	item->Check(static_cast<Iteration*>(openObjectList[activeIndex])->GetAutoAssociate());
}
