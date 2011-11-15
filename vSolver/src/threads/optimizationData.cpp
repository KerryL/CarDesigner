/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  optimizationData.cpp
// Created:  1/12/2009
// Author:  K. Loux
// Description:  Contains the class declaration for the OptimizationData class.  This contains
//				 information required to conduct genetic optimizations.
// History:

// VVASE headers
#include "vSolver/threads/optimizationData.h"
#include "vSolver/threads/threadJob.h"

//==========================================================================
// Class:			OptimizationData
// Function:		OptimizationData
//
// Description:		Constructor for the OptimizationData class.
//
// Input Arguments:
//		_geneticAlgorithm	= GeneticAlgorithm* pointing to the optimization object
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
OptimizationData::OptimizationData(GeneticAlgorithm *_geneticAlgorithm)
									 : ThreadData(), geneticAlgorithm(_geneticAlgorithm)
{
}

//==========================================================================
// Class:			OptimizationData
// Function:		~OptimizationData
//
// Description:		Destructor for the OptimizationData class.
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
OptimizationData::~OptimizationData()
{
}

//==========================================================================
// Class:			OptimizationData
// Function:		OkForCommand
//
// Description:		Checks to make sure this type of data is correct for the
//					specified command.
//
// Input Arguments:
//		Command		= ThreadJob::ThreadCommands& to be checked
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
bool OptimizationData::OkForCommand(ThreadJob::ThreadCommands &Command)
{
	// Make sure the command is one of the expected types
	return command == ThreadJob::CommandThreadGeneticOptimization;
}