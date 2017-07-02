/*=============================================================================
                                     VVASE
                        Copyright Kerry R. Loux 2007-2017
=============================================================================*/

// File:  undoRedoStack.h
// Date:  10/18/2010
// Auth:  K. Loux
// Lics:  GPL v3 (see https://www.gnu.org/licenses/gpl-3.0.en.html)
// Desc:  Class implementing std::stack to store information about recent operations
//        to provide the opportunity to undo and redo.

#ifndef UNDO_REDO_STACK_H_
#define UNDO_REDO_STACK_H_

// Standard C++ headers
#include <stack>

// Local headers
#include "VVASE/gui/gaObject.h"

namespace VVASE
{

// Local forward declarations
class MainFrame;

class UndoRedoStack
{
public:
	UndoRedoStack(MainFrame &mainFrame);
	~UndoRedoStack();

	// Data object for storing undo/redo information
	// Must be defined before some public functions are defined
	struct Operation
	{
		// Data type
		enum OperationDataType
		{
			DataTypeBool,
			DataTypeShort,
			DataTypeInteger,
			DataTypeLong,
			DataTypeFloat,
			DataTypeDouble,
			DataTypeVector,
			DataTypeGAGeneAdd,
			DataTypeGAGeneModify,
			DataTypeGAGeneDelete,
			DataTypeGAGoalAdd,
			DataTypeGAGoalModify,
			DataTypeGAGoalDelete
		} dataType;

		// Pointer to changed data
		void *dataLocation;

		// Old data value
		union OperationData
		{
			bool boolean;
			short shortInteger;
			int integer;
			long longInteger;
			float singlePrecision;
			double doublePrecision;

			double vector[3];

			struct GeneData
			{
				GAObject *optimization;
				GAObject::Gene gene;
			} geneData;

			// FIXME:  Can't do goals because Kinematics::Inputs containts a Vector, which has a user-defined constructor
			/*struct GoalData
			{
				GAObject *optimization;
				GAObject::Goal goal;
			} GoalData;*/

		} oldValue;

		// Associated GUI_OBJECT
		int guiObjectIndex;
	};

	// Methods for maintaining the stacks
	void AddOperation(int index,
		UndoRedoStack::Operation::OperationDataType dataType, void *location);
	void Undo();
	void Redo();
	void ClearStacks();
	void RemoveGuiObjectFromStack(int index);

	// Operator overloads (explicitly overloaded due to warning C4512
	// caused by reference to MAIN_FRAME)
	UndoRedoStack& operator = (const UndoRedoStack &undoRedo);

private:
	MainFrame &mainFrame;

	// Method for applying changes (either undo or redo)
	void ApplyOperation(Operation &operation);

	// Method for getting data from location associated with the OPERATION
	Operation UpdateValue(Operation operation);

	// Method for updating the application as a result of a undo/redo
	void Update() const;

	// The stacks
	std::stack<Operation> undoStack;
	std::stack<Operation> redoStack;
};

}// namespace VVASE

#endif// UNDO_REDO_STACK_H_
