/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2016

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  guiObject.h
// Created:  6/5/2008
// Author:  K. Loux
// Description:  Contains class declaration for GUI_OBJECT class.  This aids with GUI functionality
//				 by providing a common link between all the GUI controls and car objects.
// History:
//	7/27/2008	- Made ITEM_TYPE static and moved UNIT_TYPE to Convert, K. Loux.
//	8/18/2008	- Changed name from TREE_ITEM to GUI_OBJECT and broadened the functionality of the
//				  class to include a link between all of the controls, K. Loux.
//	1/24/2009	- Major application structure change - MAIN_FRAME uses GUI_OBJECT instead of
//				  GUI_CAR.  GUI_OBJECT changed to only contain either GUI_CAR or ITERATION
//				  objects.
//	5/19/2009	- Made abstract for base class for GUI_CAR and ITERATION, K. Loux.

#ifndef GUI_OBJECT_H_
#define GUI_OBJECT_H_

// wxWidgets headers
#include <wx/wx.h>
#include <wx/treebase.h>

// CarDesigner forward declarations
class GuiCar;
class Iteration;
class MainTree;
class MainNotebook;
class MainFrame;
class RenderWindow;
class Primitive;

class GuiObject
{
public:
	GuiObject(MainFrame &mainFrame, wxString pathAndFileName = wxEmptyString);
	virtual ~GuiObject();

	// Possible types of data contained in a GuiObject
	enum ItemType
	{
		TypeCar,
		TypeIteration,
		TypeOptimization,

		TypeNone// Indicates an object doesn't exist
	};

	// Returns true of false depending on whether or not this item is selected in
	// the systems tree
	bool IsThisObjectSelected(wxTreeItemId selected) const;

	// Initialization of these objects requires some calls to pure virtual functions, which
	// causes problems.  To avoid this, we make all of those calls here, and call this from
	// the derived object's constructors.
	void Initialize();

	// Checks to see if the object has been saved, asks for user confirmation, etc.
	bool Close(bool notebookPageAlreadyClosed = false);

	// Gets/sets a flag to let us know if the object has changed since it was last saved
	inline bool GetModified() const { return modifiedSinceLastSave; };
	void SetModified();

	// Returns the object that draws this on the screen
	inline wxWindow *GetNotebookTab() const { return notebookTab; }

	// Generates an image file of the render window contents
	bool WriteImageToFile(wxString pathAndFileName);

	// Private data accessors
	inline wxTreeItemId GetTreeItemId() const { return treeID; }

	// Returns this object's type (mandatory overload)
	virtual ItemType GetType() const = 0;

	// Return pointers to the main GUI components of this application
	inline MainFrame& GetMainFrame() const { return mainFrame; }

	// Gets/sets the index for this object
	inline int GetIndex() const { return index; }
	inline void SetIndex(int index) { this->index = index; }

	// Gets/sets the name for this object
	wxString GetName() const { return name; }
	wxString GetCleanName() const;
	void SetName(wxString name);

	// Calls the write methods for the data associated with this object
	bool SaveToFile(bool saveAsNewFileName = false);

	// Calls the update function for the data associated with this object
	// This method (usually) creates a job for a worker thread
	virtual void UpdateData() = 0;

	// Call the update function for the display associated with this object
	// This method does not create worker thread jobs
	virtual void UpdateDisplay() = 0;

	void SelectThisObjectInTree();
	bool IsInitialized() { return objectIsInitialized; }

protected:
	// The objects name (for display purposes)
	wxString name;

	// Performs the saving and loading to/from file
	virtual bool PerformLoadFromFile() = 0;
	virtual bool PerformSaveToFile() = 0;

	// Calls the read methods for the data associated with this object
	bool LoadFromFile();

	// Flag indicating whether or not the initialization routine is complete
	bool objectIsInitialized;

	// Strips down file names and paths to get the object name
	wxString GetNameFromFileName();

	// Gets the proper index for this object's icon in the systems tree
	virtual int GetIconHandle() const = 0;

	// The object's index
	// (associated with the list in the MAIN_FRAME object and the MAIN_NOTEBOOK's pages)
	int index;

	// For referencing this object in the SystemsTree
	wxTreeItemId treeID;

	// Pointers to the main GUI components for this application
	MainTree *systemsTree;
	MainFrame &mainFrame;
	MainNotebook *notebook;

	// The object to be added to the notebook
	wxWindow *notebookTab;

	// Flag indicating whether or not this object has been modified since it was last saved
	bool modifiedSinceLastSave;

	// The path and filename pointing to the location of this object
	// on the hard disk (or wxEmptyString if this hasn't been saved)
	wxString pathAndFileName;

	// Verifies that no other open object has the same filename
	bool VerifyUniqueness();
};

#endif// GUI_OBJECT_H_