/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2016

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  drivetrain.cpp
// Created:  11/3/2007
// Author:  K. Loux
// Description:  Contains class functionality for Drivetrain class.
// History:
//	2/24/2008	- Fixed SetNumberOfGears and added delete[] in destructor, K. Loux.
//	3/9/2008	- Changed the structure of the Debugger class, K. Loux.
//	11/22/2009	- Moved to vCar.lib, K. Loux.

// wxWidgets headers
#include <wx/wx.h>

// VVASE headers
#include "vCar/differential.h"
#include "vCar/drivetrain.h"
#include "vUtilities/wheelSetStructures.h"
#include "vUtilities/debugger.h"
#include "vUtilities/machineDefinitions.h"
#include "vUtilities/binaryReader.h"
#include "vUtilities/binaryWriter.h"

//==========================================================================
// Class:			Drivetrain
// Function:		Drivetrain
//
// Description:		Constructor for the Drivetrain class.
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
Drivetrain::Drivetrain()
{
	SetNumberOfGears(1);

	differentials.push_back(new Differential());

	driveType = DriveRearWheel;
}

//==========================================================================
// Class:			Drivetrain
// Function:		Drivetrain
//
// Description:		Copy constructor for the Drivetrain class.
//
// Input Arguments:
//		drivetrain	= const Drivetrain&, object to be copied
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
Drivetrain::Drivetrain(const Drivetrain &drivetrain)
{
	*this = drivetrain;
}

//==========================================================================
// Class:			Drivetrain
// Function:		~Drivetrain
//
// Description:		Destructor for the Drivetrain class.
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
Drivetrain::~Drivetrain()
{
	DeleteDifferentials();
}

//==========================================================================
// Class:			Drivetrain
// Function:		SetNumberOfGears
//
// Description:		Sets the number of gears we have available and allocates
//					memory for the gear ratios accordingly.
//
// Input Arguments:
//		NumGears	= const short& describing the number of gears we have
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void Drivetrain::SetNumberOfGears(const short &numGears)
{
	// Make sure we have at least one gear
	if (numGears < 1)
	{
		Debugger::GetInstance() << "ERROR:  Must have at least 1 gear!" << Debugger::PriorityHigh;
		return;
	}

	gearRatios.resize(numGears);
}

//==========================================================================
// Class:			Drivetrain
// Function:		Write
//
// Description:		Writes this drivetrain to file.
//
// Input Arguments:
//		file	= BinaryWriter&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void Drivetrain::Write(BinaryWriter& file) const
{
	// Write this object to file
	// This is done item by item because NumberOfGears needs to be used to
	// determine size of GearRatio.  Also, Differential needs to be handled
	// separately.
	file.Write((unsigned int)driveType);
	file.Write(gearRatios);

	// Write the differentials
	unsigned int diffCount(differentials.size());
	file.Write(diffCount);

	unsigned int i;
	for (i = 0; i < differentials.size(); i++)
		differentials[i]->Write(file);
}

//==========================================================================
// Class:			Drivetrain
// Function:		Read
//
// Description:		Read from file to fill this drivetrain.
//
// Input Arguments:
//		file		= BinaryReader&
//		fileVersion	= const int& specifying which file version we're reading from
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void Drivetrain::Read(BinaryReader& file, const int& fileVersion)
{
	// Read this object from file - again, this is the same order as found
	// in the Read() function.
	unsigned int temp;
	file.Read(temp);
	driveType = static_cast<DriveWheels>(temp);

	temp = sizeof(driveType);

	if (fileVersion >= 5)
	{
		file.Read(gearRatios);
	}
	else if (fileVersion >= 0)
	{
		short tempShort;
		file.Read(tempShort);
		SetNumberOfGears(tempShort);

		double dummy;
		file.Read(dummy);

		unsigned int i;
		for (i = 0; i < gearRatios.size(); i++)
			file.Read(gearRatios[i]);
	}
	else
		assert(false);

	DeleteDifferentials();

	// Read the differentials
	unsigned int diffCount(1);
	if (fileVersion >= 5)
		file.Read(diffCount);

	unsigned int i;
	for (i = 0; i < diffCount; i++)
	{
		differentials.push_back(new Differential);
		differentials.front()->Read(file, fileVersion);
	}
}

//==========================================================================
// Class:			Drivetrain
// Function:		GetDriveWheelsName
//
// Description:		Returns the name of the drivetrain type.
//
// Input Arguments:
//		_driveWheels	= const DriveWheels& specifying the type of interest
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing the name of the drive style
//
//==========================================================================
wxString Drivetrain::GetDriveWheelsName(const DriveWheels &driveWheels)
{
	switch (driveWheels)
	{
	case DriveRearWheel:
		return _T("Rear Wheel Drive");
		break;

	case DriveFrontWheel:
		return _T("Front Wheel Drive");
		break;

	case DriveAllWheel:
		return _T("All Wheel Drive");
		break;

	default:
		assert(0);
		break;
	}

	return wxEmptyString;
}

//==========================================================================
// Class:			Drivetrain
// Function:		operator=
//
// Description:		Assignment operator for Drivetrain class.
//
// Input Arguments:
//		drivetrain	= const Drivetrain& to copy from
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
Drivetrain& Drivetrain::operator=(const Drivetrain &drivetrain)
{
	// Check for self-assignment
	if (this == &drivetrain)
		return *this;

	// Allocate memory for the gear ratios depending on the number of gears
	SetNumberOfGears(drivetrain.gearRatios.size());

	unsigned int i;
	for (i = 0; i < gearRatios.size(); i++)
		gearRatios[i] = drivetrain.gearRatios[i];

	DeleteDifferentials();

	for (i = 0; i < drivetrain.differentials.size(); i++)
		differentials.push_back(new Differential(*drivetrain.differentials[i]));

	driveType = drivetrain.driveType;

	assert((differentials.size() == 1 && (driveType == DriveFrontWheel || driveType == DriveRearWheel)) ||
		(differentials.size() == 3 && driveType == DriveAllWheel));

	return *this;
}

//==========================================================================
// Class:			Drivetrain
// Function:		SetAllWheelDrive
//
// Description:		Sets the drive type to all wheel drive.
//
// Input Arguments:
//		rearBias	= const double&
//		midBias		= const double&
//		frontBias	= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void Drivetrain::SetAllWheelDrive(const double& rearBias, const double& midBias, const double& frontBias)
{
	DeleteDifferentials();
	driveType = DriveAllWheel;
	differentials.push_back(new Differential(rearBias));
	differentials.push_back(new Differential(midBias));
	differentials.push_back(new Differential(frontBias));
}

//==========================================================================
// Class:			Drivetrain
// Function:		SetFrontWheelDrive
//
// Description:		Sets the drive type to front wheel drive.
//
// Input Arguments:
//		bias	= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void Drivetrain::SetFrontWheelDrive(const double& bias)
{
	DeleteDifferentials();
	driveType = DriveFrontWheel;
	differentials.push_back(new Differential(bias));
}

//==========================================================================
// Class:			Drivetrain
// Function:		SetRearWheelDrive
//
// Description:		Sets the drive type to rear wheel drive.
//
// Input Arguments:
//		bias	= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void Drivetrain::SetRearWheelDrive(const double& bias)
{
	DeleteDifferentials();
	driveType = DriveRearWheel;
	differentials.push_back(new Differential(bias));
}

//==========================================================================
// Class:			Drivetrain
// Function:		GetBiasRatios
//
// Description:		Returns all bias ratios.
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		std::vector<double>
//
//==========================================================================
std::vector<double> Drivetrain::GetBiasRatios() const
{
	std::vector<double> ratios;
	unsigned int i;
	for (i = 0; i < differentials.size(); i++)
		ratios.push_back(differentials[i]->biasRatio);

	return ratios;
}

//==========================================================================
// Class:			Drivetrain
// Function:		DeleteDifferentials
//
// Description:		Frees memory for differential objects.
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
void Drivetrain::DeleteDifferentials()
{
	unsigned int i;
	for (i = 0; i < differentials.size(); i++)
		delete differentials[i];
	differentials.clear();
}
