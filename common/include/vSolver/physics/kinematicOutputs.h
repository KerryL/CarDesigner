/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  kinematicOutputs.h
// Created:  3/23/2008
// Author:  K. Loux
// Description:  Contains class declaration for KinematicOutputs class.  This class does the
//				 calculations for all of the kinematic simulation outputs.  That includes any
//				 kind of wheel angle/orientation, chassis attitude, spring/shock positions, but
//				 doesn't include any thing that requires forces to calculate (force-based roll
//				 center, etc.).
// History:
//	3/24/2008	- Created CornerOutputs structure, K. Loux.
//	2/23/2009	- Changed to enum/array style for outputs instead of individually declared variables
//				  for each output, K. Loux.
//	3/11/2009	- Finished implementation of enum/array style data members, K. Loux.

#ifndef _KINEMATIC_OUTPUTS_H_
#define _KINEMATIC_OUTPUTS_H_

// wxWidgets headers
#include <wx/thread.h>

// VVASE headers
#include "vMath/vector.h"
#include "vUtilities/convert.h"
#include "vCar/corner.h"

// VVASE forward declarations
class Car;
class Suspension;

class KinematicOutputs
{
public:
	// Constructor
	KinematicOutputs();

	// Destructor
	~KinematicOutputs();

	// Updates the kinematic variables associated with the current Suspension
	void Update(const Car *original, const Suspension *current);

	// Enumeration for double outputs that get computed for every corner
	enum CornerOutputsDouble
	{
		Caster,						// [rad]
		Camber,						// [rad]
		KPI,						// [rad]
		Steer,						// [rad]
		Spring,						// [in]
		Shock,						// [in]
		AxlePlunge,					// [in]
		CasterTrail,				// [in]
		ScrubRadius,				// [in]
		Scrub,						// [in]
		SpringInstallationRatio,	// [in Spring/in Wheel]
		ShockInstallationRatio,		// [in Shock/in Wheel]
		ARBInstallationRatio,		// [rad Bar/in Wheel] (assumes opposite side stays fixed)
		SpindleLength,				// [in]
		SideViewSwingArmLength,		// [in]
		FrontViewSwingArmLength,	// [in]
		AntiBrakePitch,				// [%] : anti-dive in the front, anti-lift in the rear
		AntiDrivePitch,				// [%] : anti-lift in the front, anti-squat in the rear

		NumberOfCornerOutputDoubles
	};

	// Enumeration for Vector outputs that get computed for every corner
	enum CornerOutputsVector
	{
		InstantCenter,			// [in]
		InstantAxisDirection,	// [-]

		NumberOfCornerOutputVectors
	};

	// Enumeration for double outputs that only get computed once per car
	enum OutputsDouble
	{
		FrontARBTwist,				// [rad]
		RearARBTwist,				// [rad]
		FrontThirdSpring,			// [in]
		FrontThirdShock,			// [in]
		RearThirdSpring,			// [in]
		RearThirdShock,				// [in]
		FrontNetSteer,				// [rad]
		RearNetSteer,				// [rad]
		FrontNetScrub,				// [in]
		RearNetScrub,				// [in]
		FrontTrackGround,			// [in]
		RearTrackGround,			// [in]
		RightWheelbaseGround,		// [in]
		LeftWheelbaseGround,		// [in]
		FrontTrackHub,				// [in]
		RearTrackHub,				// [in]
		RightWheelbaseHub,			// [in]
		LeftWheelbaseHub,			// [in]

		NumberOfOutputDoubles
	};

	// Enumeration for Vector outputs that only get computed once per car
	enum OutputsVector
	{
		// Kinematic centers
		FrontKinematicRC,			// [in]
		RearKinematicRC,			// [in]
		RightKinematicPC,			// [in]
		LeftKinematicPC,			// [in]

		// Kinematic axis
		FrontRollAxisDirection,
		RearRollAxisDirection,
		RightPitchAxisDirection,
		LeftPitchAxisDirection,

		NumberOfOutputVectors
	};

	// Sway bar twist will be total for all bar types... normal for U-bar and T-bar
	// but geared bars will include the twist along both lengths of bar.  This way
	// it can be used to determine forces/stresses directly.
	double doubles[NumberOfOutputDoubles];
	Vector vectors[NumberOfOutputVectors];

	// The outputs that are associated with just one corner of the car
	double rightFront[NumberOfCornerOutputDoubles];
	double leftFront[NumberOfCornerOutputDoubles];
	double rightRear[NumberOfCornerOutputDoubles];
	double leftRear[NumberOfCornerOutputDoubles];

	Vector rightFrontVectors[NumberOfCornerOutputVectors];
	Vector leftFrontVectors[NumberOfCornerOutputVectors];
	Vector rightRearVectors[NumberOfCornerOutputVectors];
	Vector leftRearVectors[NumberOfCornerOutputVectors];

	// Enumeration that encompasses all of the outputs for the whole car
	// This makes referencing these outputs from other classes a little easier
	enum OutputsComplete
	{
		StartRightFrontDoubles,
		EndRightFrontDoubles = StartRightFrontDoubles + NumberOfCornerOutputDoubles - 1,
		StartRightFrontVectors,
		EndRightFrontVectors = StartRightFrontVectors + 3 * NumberOfCornerOutputVectors - 1,

		StartLeftFrontDoubles,
		EndLeftFrontDoubles = StartLeftFrontDoubles + NumberOfCornerOutputDoubles - 1,
		StartLeftFrontVectors,
		EndLeftFrontVectors = StartLeftFrontVectors + 3 * NumberOfCornerOutputVectors - 1,

		StartRightRearDoubles,
		EndRightRearDoubles = StartRightRearDoubles + NumberOfCornerOutputDoubles - 1,
		StartRightRearVectors,
		EndRightRearVectors = StartRightRearVectors + 3 * NumberOfCornerOutputVectors - 1,

		StartLeftRearDoubles,
		EndLeftRearDoubles = StartLeftRearDoubles + NumberOfCornerOutputDoubles - 1,
		StartLeftRearVectors,
		EndLeftRearVectors = StartLeftRearVectors + 3 * NumberOfCornerOutputVectors - 1,

		StartDoubles,
		EndDoubles = StartDoubles + NumberOfOutputDoubles - 1,

		StartVectors,
		EndVectors = StartVectors + 3 * NumberOfOutputVectors - 1,

		NumberOfOutputScalars// Called "scalars" because each vector component is treated as one output
	};

	// For converting from an output + location to OutputsComplete
	static OutputsComplete OutputsCompleteIndex(const Corner::Location &location,
		const CornerOutputsDouble &cornerDouble, const CornerOutputsVector &cornerVector,
		const OutputsDouble &midDouble, const OutputsVector &vector, const Vector::Axis &axis);

	// For accessing an output via the OutputsComplete list
	double GetOutputValue(const OutputsComplete &_output) const;

	// For determining unit type of the outputs
	static Convert::UnitType GetOutputUnitType(const OutputsComplete &_output);

	// For determining the name of an output from the OutputsComplete list
	static wxString GetOutputName(const OutputsComplete &_output);

	// Mutex accessor
	//wxMutex &GetMutex(void) const { return kinematicOutputsMutex; };

private:
	// The currently associated car object
	const Car *currentCar;

	// Updates the outputs associated with the associated corner
	void UpdateCorner(const Corner *originalCorner, const Corner *currentCorner);

	// For retrieving names of the outputs
	static wxString GetCornerDoubleName(const CornerOutputsDouble &_output);
	static wxString GetCornerVectorName(const CornerOutputsVector &_output);
	static wxString GetDoubleName(const OutputsDouble &_output);
	static wxString GetVectorName(const OutputsVector &_output);

	// For retrieving units of the outputs
	static Convert::UnitType GetCornerDoubleUnitType(const CornerOutputsDouble &_output);
	static Convert::UnitType GetCornerVectorUnitType(const CornerOutputsVector &_output);
	static Convert::UnitType GetDoubleUnitType(const OutputsDouble &_output);
	static Convert::UnitType GetVectorUnitType(const OutputsVector &_output);

	// Initializes all outputs to QNAN
	void InitializeAllOutputs(void);
};

#endif// _KINEMATIC_OUTPUTS_H_