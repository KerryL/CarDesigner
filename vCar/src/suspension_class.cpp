/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2010

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  suspension_class.cpp
// Created:  11/3/2007
// Author:  K. Loux
// Description:  Contains class functionality for suspension class.
// History:
//	2/17/2008	- Made SolveForPoint and SolveForXY use newly found analytical solution
//				  for intersection of three spheres problem, K. Loux.
//	3/9/2008	- Changed strucutre of DEBUGGER class, K. Loux.
//	2/16/2009	- Changed to use enumeration style array of points instead of having each
//				  point declared individually, K. Loux.
//	4/11/2009	- Changed structure of SolveForPoint and SolveForXY to take original
//				  and new centers and locations as first step toward making point
//				  solving more robust, K. Loux.
//	4/19/2009	- Made all of the Solve...() functions static, and made Debugger static, K. Loux.
//	11/22/2009	- Moved to vCar.lib, K. Loux.
//	1/8/2010	- Modified to correct potential divide by zero in SolveForXY, K. Loux.

// Standard C++ headers
#include <fstream>

// wxWidgets headers
#include <wx/wx.h>

// VVASE headers
#include "vCar/suspension_class.h"
#include "vMath/car_math.h"
#include "vUtilities/machine_defs.h"

//==========================================================================
// Class:			SUSPENSION
// Function:		SUSPENSION
//
// Description:		Constructor for the SUSPENSION class.
//
// Input Arguments:
//		_Debugger	= const DEBUGGER& reference to applications debug printing utility
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
SUSPENSION::SUSPENSION(const DEBUGGER &_Debugger)
					   : RightFront(CORNER::LocationRightFront, _Debugger),
					   LeftFront(CORNER::LocationLeftFront, _Debugger),
					   RightRear(CORNER::LocationRightRear, _Debugger),
					   LeftRear(CORNER::LocationLeftRear, _Debugger)
{
	// Initialize the hardpoints within this object as well
	int i;
	for (i = 0; i < NumberOfHardpoints; i++)
		Hardpoints[i].Set(0.0, 0.0, 0.0);

	// Initialize the other suspension parameters
	BarRate.Front = 0.0;
	BarRate.Rear = 0.0;
	RackRatio = 1.0;
	IsSymmetric = false;
	FrontBarStyle = SwayBarNone;
	RearBarStyle = SwayBarNone;
	FrontBarAttachment = BarAttachmentBellcrank;
	RearBarAttachment = BarAttachmentBellcrank;
	FrontHasThirdSpring = false;
	RearHasThirdSpring = false;
	// FIXME:  Third springs and dampers!
}

//==========================================================================
// Class:			SUSPENSION
// Function:		~SUSPENSION
//
// Description:		Destructor for the SUSPENSION class.
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
SUSPENSION::~SUSPENSION()
{
}

//==========================================================================
// Class:			SUSPENSION
// Function:		Constant Declarations
//
// Description:		Constant declarations for the SUSPENSION class.
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
const DEBUGGER *SUSPENSION::Debugger = NULL;

//==========================================================================
// Class:			SUSPENSION
// Function:		SolveForPoint
//
// Description:		This function performs the calculations required to
//					obtain an analytical solution to the intersection of
//					three spheres problem.  The Original vector provides a
//					means of determining which of the two solutions is the
//					correct one.  If there is no solution (will only occur
//					if suspension would bind, etc.), a warning is returned.
//					More detail on the method used to obtain the solution
//					is below.  In the event of an error, the original value
//					is returned.
//
// Input Arguments:
//		Center1			= const VECTOR& specifying the center of the first sphere
//		Center2			= const VECTOR& specifying the center of the second sphere
//		Center3			= const VECTOR& specifying the center of the third sphere
//		OriginalCenter1	= const VECTOR& specifying the original center of the
//						  first sphere
//		OriginalCenter2	= const VECTOR& specifying the original center of the
//						  second sphere
//		OriginalCenter3	= const VECTOR& specifying the original center of the
//						  third sphere
//		Original		= const VECTOR& specifying the original location of the point
//						  we are solving for
//
// Output Arguments:
//		Current	= VECTOR& specifying the result of the intersection of
//				  three spheres algorithm
//
// Return Value:
//		bool, true for success, false for error
//		
//==========================================================================
bool SUSPENSION::SolveForPoint(const VECTOR &Center1, const VECTOR &Center2, const VECTOR &Center3,
							   const VECTOR &OriginalCenter1, const VECTOR &OriginalCenter2,
							   const VECTOR &OriginalCenter3, const VECTOR &Original, VECTOR &Current)
{
	// Compute the circle radii
	double R1 = OriginalCenter1.Distance(Original);
	double R2 = OriginalCenter2.Distance(Original);
	double R3 = OriginalCenter3.Distance(Original);

	// Check for the existence of a solution
	if (Center1.Distance(Center2) > R1 + R2 || Center1.Distance(Center3) > R1 + R3 ||
		Center2.Distance(Center3) > R2 + R3)
	{
		Debugger->Print(_T("Error (SolveForPoint): Center distance exceeds sum of radii"), DEBUGGER::PriorityLow);

		return false;
	}
	else if (Center1.Distance(Center2) + min(R1, R2) < max(R1, R2) ||
		Center1.Distance(Center3) + min(R1, R3) < max(R1, R3) ||
		Center2.Distance(Center3) + min(R2, R3) < max(R2, R3))
	{
		Debugger->Print(_T("Error (SolveForPoint): Center distance and smaller radius less than larger radius"),
			DEBUGGER::PriorityLow);

		return false;
	}

	// The method:
	//  1.	The intersection of two spheres creates a circle.  That circle lies on a plane.
	//		Determine this plane for any two spheres. This plane is determined by subtracting
	//		the equations of two spheres.  This is different from (better than) subtitution,
	//		because this will ensure that the higher order terms drop out.
	//  2.	Determine the same plane as in step 1 for a different set of two spheres.
	//  3.	Find the line created by the intersection of the planes found in steps 1 and 2.
	//		Lines only have one degree of freedom, so this will be two equations in the same
	//		variable.
	//  4.  The intersection of the line and any sphere will yeild two points (unless the
	//		spheres don't intersect or they intersect at only one point).  These points are
	//		the solutions.  Here, we employ the quadratic equation and the equation of the
	//		line determined in step 3.

	// Declare our plane constants
	double A1, B1, C1, D1, A2, B2, C2, D2;

	// Step 1 (Plane defined by intersection of spheres 1 and 2)
	A1 = Center1.X - Center2.X;
	B1 = Center1.Y - Center2.Y;
	C1 = Center1.Z - Center2.Z;
	D1 = (pow(Center2.Length(), 2) - pow(Center1.Length(), 2) - R2 * R2 + R1 * R1) / 2.0;

	// Step 2 (Plane defined by intersection of spheres 1 and 3)
	A2 = Center1.X - Center3.X;
	B2 = Center1.Y - Center3.Y;
	C2 = Center1.Z - Center3.Z;
	D2 = (pow(Center3.Length(), 2) - pow(Center1.Length(), 2) - R3 * R3 + R1 * R1) / 2.0;

	// Step 3 (Line defined by intersection of planes from steps 1 and 2)
	// The if..else stuff avoid numerical instabilities - we'll choose the denominators
	// farthest from zero for all divisions (denominators are below):
	double den1 = B1 * C2 - B2 * C1;
	double den2 = A1 * C2 - A2 * C1;
	double den3 = A1 * B2 - A2 * B1;

	// Let's declare our answers now
	VECTOR Solution1, Solution2;

	// And our quadratic equation coefficients
	double a, b, c;

	// In which order do we want to solve for the components of the points?
	if (max(max(fabs(den1), fabs(den2)), fabs(den3)) == fabs(den1))
	{
		if (max(max(max(fabs(C1), fabs(C2)), fabs(B1)), fabs(B2)) == fabs(C1))
		{
			// Solve X first (use first set of plane coefficients)
			double myx = (A2 * C1 - A1 * C2) / den1;
			double byx = (C1 * D2 - C2 * D1) / den1;
			
			a = 1.0 + myx * myx + pow(A1 + B1 * myx, 2) / (C1 * C1);
			b = 2.0 * (myx * (byx - Center1.Y) - Center1.X + (A1 + B1 * myx) / C1
				* ((B1 * byx + D1) / C1 + Center1.Z));
			c = Center1.X * Center1.X + pow(byx - Center1.Y, 2) - R1 * R1
				+ pow((B1 * byx + D1) / C1 + Center1.Z, 2);
			// First solution
			Solution1.X = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.X = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Y next
			Solution1.Y = Solution1.X * myx + byx;
			Solution2.Y = Solution2.X * myx + byx;
			// Solve Z last
			Solution1.Z = (-A1 * Solution1.X - B1 * Solution1.Y - D1) / C1;
			Solution2.Z = (-A1 * Solution2.X - B1 * Solution2.Y - D1) / C1;
		}
		else if (max(max(max(fabs(C1), fabs(C2)), fabs(B1)), fabs(B2)) == fabs(C2))
		{
			// Solve X first (use second set of plane coefficients)
			double myx = (A2 * C1 - A1 * C2) / den1;
			double byx = (C1 * D2 - C2 * D1) / den1;
			a = 1.0 + myx * myx + pow(A2 + B2 * myx, 2) / (C2 * C2);
			b = 2.0 * (myx * (byx - Center1.Y) - Center1.X + (A2 + B2 * myx) / C2
				* ((B2 * byx + D2) / C2 + Center1.Z));
			c = Center1.X * Center1.X + pow(byx - Center1.Y, 2) - R1 * R1
				+ pow((B2 * byx + D2) / C2 + Center1.Z, 2);
			// First solution
			Solution1.X = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.X = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Y next
			Solution1.Y = Solution1.X * myx + byx;
			Solution2.Y = Solution2.X * myx + byx;
			// Solve Z last
			Solution1.Z = (-A2 * Solution1.X - B2 * Solution1.Y - D2) / C2;
			Solution2.Z = (-A2 * Solution2.X - B2 * Solution2.Y - D2) / C2;
		}
		else if (max(max(max(fabs(C1), fabs(C2)), fabs(B1)), fabs(B2)) == fabs(B1))
		{
			// Solve X first (use first set of plane coefficients)
			double mzx = (A1 * B2 - A2 * B1) / den1;
			double bzx = (B2 * D1 - B1 * D2) / den1;
			a = 1.0 + mzx * mzx + pow(A1 + C1 * mzx, 2) / (B1 * B1);
			b = 2.0 * (mzx * (bzx - Center1.Z) - Center1.X + (A1 + C1 * mzx) / B1
				* ((C1 * bzx + D1) / B1 + Center1.Y));
			c = Center1.X * Center1.X + pow(bzx - Center1.Z, 2) - R1 * R1
				+ pow((C1 * bzx + D1) / B1 + Center1.Y, 2);
			// First solution
			Solution1.X = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.X = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Z next
			Solution1.Z = Solution1.X * mzx + bzx;
			Solution2.Z = Solution2.X * mzx + bzx;
			// Solve Y last
			Solution1.Y = (-A1 * Solution1.X - C1 * Solution1.Z - D1) / B1;
			Solution2.Y = (-A1 * Solution2.X - C1 * Solution2.Z - D1) / B1;
		}
		else// if (max(max(max(fabs(C1), fabs(C2)), fabs(B1)), fabs(B2)) == fabs(B2))
		{
			// Solve X first (use second set of plane coefficients)
			double mzx = (A1 * B2 - A2 * B1) / den1;
			double bzx = (B2 * D1 - B1 * D2) / den1;
			a = 1.0 + mzx * mzx + pow(A2 + C2 * mzx, 2) / (B2 * B2);
			b = 2.0 * (mzx * (bzx - Center1.Z) - Center1.X + (A2 + C2 * mzx) / B2
				* ((C2 * bzx + D2) / B2 + Center1.Y));
			c = Center1.X * Center1.X + pow(bzx - Center1.Z, 2) - R1 * R1
				+ pow((C2 * bzx + D2) / B2 + Center1.Y, 2);
			// First solution
			Solution1.X = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.X = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Z next
			Solution1.Z = Solution1.X * mzx + bzx;
			Solution2.Z = Solution2.X * mzx + bzx;
			// Solve Y last
			Solution1.Y = (-A2 * Solution1.X - C2 * Solution1.Z - D2) / B2;
			Solution2.Y = (-A2 * Solution2.X - C2 * Solution2.Z - D2) / B2;
		}
	}
	else if (max(max(fabs(den1), fabs(den2)), fabs(den3)) == fabs(den2))
	{
		if (max(max(max(fabs(A1), fabs(A2)), fabs(C1)), fabs(C2)) == fabs(A1))
		{
			// Solve Y first (use first set of plane coefficients)
			double mzy = (A2 * B1 - A1 * B2) / den2;
			double bzy = (A2 * D1 - A1 * D2) / den2;
			a = 1 + mzy * mzy + pow(B1 + C1 * mzy, 2) / (A1 * A1);
			b = 2 * (mzy * (bzy - Center1.Z) - Center1.Y + (B1 + C1 * mzy) / A1
				* ((C1 * bzy + D1) / A1 + Center1.X));
			c = Center1.Y * Center1.Y + pow(bzy - Center1.Z, 2) - R1 * R1
				+ pow((C1 * bzy + D1) / A1 + Center1.X, 2);
			// First solution
			Solution1.Y = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.Y = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Z next
			Solution1.Z = Solution1.Y * mzy + bzy;
			Solution2.Z = Solution2.Y * mzy + bzy;
			// Solve X last
			Solution1.X = (-B1 * Solution1.Y - C1 * Solution1.Z - D1) / A1;
			Solution2.X = (-B1 * Solution2.Y - C1 * Solution2.Z - D1) / A1;
		}
		else if (max(max(max(fabs(A1), fabs(A2)), fabs(C1)), fabs(C2)) == fabs(A2))
		{
			// Solve Y first (use second set of plane coefficients)
			double mzy = (A2 * B1 - A1 * B2) / den2;
			double bzy = (A2 * D1 - A1 * D2) / den2;
			a = 1 + mzy * mzy + pow(B2 + C2 * mzy, 2) / (A2 * A2);
			b = 2 * (mzy * (bzy - Center1.Z) - Center1.Y + (B2 + C2 * mzy) / A2
				* ((C2 * bzy + D2) / A2 + Center1.X));
			c = Center1.Y * Center1.Y + pow(bzy - Center1.Z, 2) - R1 * R1
				+ pow((C2 * bzy + D2) / A2 + Center1.X, 2);
			// First solution
			Solution1.Y = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.Y = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Z next
			Solution1.Z = Solution1.Y * mzy + bzy;
			Solution2.Z = Solution2.Y * mzy + bzy;
			// Solve X last
			Solution1.X = (-B2 * Solution1.Y - C2 * Solution1.Z - D2) / A2;
			Solution2.X = (-B2 * Solution2.Y - C2 * Solution2.Z - D2) / A2;
		}
		else if (max(max(max(fabs(A1), fabs(A2)), fabs(C1)), fabs(C2)) == fabs(C1))
		{
			// Solve Y first (use first set of plane coefficients)
			double mxy = (B2 * C1 - B1 * C2) / den2;
			double bxy = (C1 * D2 - C2 * D1) / den2;
			a = 1 + mxy * mxy + pow(B1 + A1 * mxy, 2) / (C1 * C1);
			b = 2 * (mxy * (bxy - Center1.X) - Center1.Y + (B1 + A1 * mxy) / C1
				* ((A1 * bxy + D1) / C1 + Center1.Z));
			c = Center1.Y * Center1.Y + pow(bxy - Center1.X, 2) - R1 * R1
				+ pow((A1 * bxy + D1) / C1 + Center1.Z, 2);
			// First solution
			Solution1.Y = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.Y = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve X next
			Solution1.X = Solution1.Y * mxy + bxy;
			Solution2.X = Solution2.Y * mxy + bxy;
			// Solve Z last
			Solution1.Z = (-A1 * Solution1.X - B1 * Solution1.Y - D1) / C1;
			Solution2.Z = (-A1 * Solution2.X - B1 * Solution2.Y - D1) / C1;
		}
		else// if (max(max(max(fabs(A1), fabs(A2)), fabs(C1)), fabs(C2)) == fabs(C2))
		{
			// Solve Y first (use second set of plane coefficients)
			double mxy = (B2 * C1 - B1 * C2) / den2;
			double bxy = (C1 * D2 - C2 * D1) / den2;
			a = 1 + mxy * mxy + pow(B2 + A2 * mxy, 2) / (C2 * C2);
			b = 2 * (mxy * (bxy - Center1.X) - Center1.Y + (B2 + A2 * mxy) / C2
				* ((A2 * bxy + D2) / C2 + Center1.Z));
			c = Center1.Y * Center1.Y + pow(bxy - Center1.X, 2) - R1 * R1
				+ pow((A2 * bxy + D2) / C2 + Center1.Z, 2);
			// First solution
			Solution1.Y = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.Y = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve X next
			Solution1.X = Solution1.Y * mxy + bxy;
			Solution2.X = Solution2.Y * mxy + bxy;
			// Solve Z last
			Solution1.Z = (-A2 * Solution1.X - B2 * Solution1.Y - D2) / C2;
			Solution2.Z = (-A2 * Solution2.X - B2 * Solution2.Y - D2) / C2;
		}
	}
	else// if (max(max(fabs(den1), fabs(den2)), fabs(den3)) == fabs(den3))
	{
		if (max(max(max(fabs(A1), fabs(A2)), fabs(B1)), fabs(B2)) == fabs(A1))
		{
			// Solve Z first (use first set of plane coefficients)
			double myz = (A2 * C1 - A1 * C2) / den3;
			double byz = (A2 * D1 - A1 * D2) / den3;
			a = 1 + myz * myz + pow(C1 + B1 * myz, 2) / (A1 * A1);
			b = 2 * (myz * (byz - Center1.Y) - Center1.Z + (C1 + B1 * myz) / A1
				* ((B1 * byz + D1) / A1 + Center1.X));
			c = Center1.Z * Center1.Z + pow(byz - Center1.Y, 2) - R1 * R1
				+ pow((B1 * byz + D1) / A1 + Center1.X, 2);
			// First solution
			Solution1.Z = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.Z = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Y next
			Solution1.Y = Solution1.Z * myz + byz;
			Solution2.Y = Solution2.Z * myz + byz;
			// Solve X last
			Solution1.X = (-B1 * Solution1.Y - C1 * Solution1.Z - D1) / A1;
			Solution2.X = (-B1 * Solution2.Y - C1 * Solution2.Z - D1) / A1;
		}
		else if (max(max(max(fabs(A1), fabs(A2)), fabs(B1)), fabs(B2)) == fabs(A2))
		{
			// Solve Z first (use second set of plane coefficients)
			double myz = (A2 * C1 - A1 * C2) / den3;
			double byz = (A2 * D1 - A1 * D2) / den3;
			a = 1 + myz * myz + pow(C2 + B2 * myz, 2) / (A2 * A2);
			b = 2 * (myz * (byz - Center1.Y) - Center1.Z + (C2 + B2 * myz) / A2
				* ((B2 * byz + D2) / A2 + Center1.X));
			c = Center1.Z * Center1.Z + pow(byz - Center1.Y, 2) - R1 * R1
				+ pow((B2 * byz + D2) / A2 + Center1.X, 2);
			// First solution
			Solution1.Z = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.Z = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Y next
			Solution1.Y = Solution1.Z * myz + byz;
			Solution2.Y = Solution2.Z * myz + byz;
			// Solve X last
			Solution1.X = (-B2 * Solution1.Y - C2 * Solution1.Z - D2) / A2;
			Solution2.X = (-B2 * Solution2.Y - C2 * Solution2.Z - D2) / A2;
		}
		else if (max(max(max(fabs(A1), fabs(A2)), fabs(B1)), fabs(B2)) == fabs(B1))
		{
			// Solve Z first (use first set of plane coefficients)
			double mxz = (B1 * C2 - B2 * C1) / den3;
			double bxz = (B1 * D2 - B2 * D1) / den3;
			a = 1 + mxz * mxz + pow(C1 + A1 * mxz, 2) / (B1 * B1);
			b = 2 * (mxz * (bxz - Center1.X) - Center1.Z + (C1 + A1 * mxz) / B1
				* ((A1 * bxz + D1) / B1 + Center1.Y));
			c = Center1.Z * Center1.Z + pow(bxz - Center1.X, 2) - R1 * R1
				+ pow((A1 * bxz + D1) / B1 + Center1.Y, 2);
			// First solution
			Solution1.Z = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.Z = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve X next
			Solution1.X = Solution1.Z * mxz + bxz;
			Solution2.X = Solution2.Z * mxz + bxz;
			// Solve Y last
			Solution1.Y = (-A1 * Solution1.X - C1 * Solution1.Z - D1) / B1;
			Solution2.Y = (-A1 * Solution2.X - C1 * Solution2.Z - D1) / B1;
		}
		else// if (max(max(max(fabs(A1), fabs(A2)), fabs(B1)), fabs(B2)) == fabs(B2))
		{
			// Solve Z first (use second set of plane coefficients)
			double mxz = (B1 * C2 - B2 * C1) / den3;
			double bxz = (B1 * D2 - B2 * D1) / den3;
			a = 1 + mxz * mxz + pow(C2 + A2 * mxz, 2) / (B2 * B2);
			b = 2 * (mxz * (bxz - Center1.X) - Center1.Z + (C2 + A2 * mxz) / B2
				* ((A2 * bxz + D2) / B2 + Center1.Y));
			c = Center1.Z * Center1.Z + pow(bxz - Center1.X, 2) - R1 * R1
				+ pow((A2 * bxz + D2) / B2 + Center1.Y, 2);
			// First solution
			Solution1.Z = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.Z = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve X next
			Solution1.X = Solution1.Z * mxz + bxz;
			Solution2.X = Solution2.Z * mxz + bxz;
			// Solve Y last
			Solution1.Y = (-A2 * Solution1.X - C2 * Solution1.Z - D2) / B2;
			Solution2.Y = (-A2 * Solution2.X - C2 * Solution2.Z - D2) / B2;
		}
	}

	// Make sure the solution is valid
	if (Solution1 != Solution1 || Solution2 != Solution2)
	{
		Debugger->Print(_T("Error (SolveForPoint): Invalid solution"), DEBUGGER::PriorityLow);

		return false;
	}

	// We now have two solutions.  To choose between them, we must examine the original
	// location of the point and the original sphere centers.  The three sphere centers
	// define a plane, and the the two solutions lie on opposite sides of the plane.
	// The first step in identifying the correct solution is to determine which side of the
	// original plane the original point was on, and which side of the new plane either one
	// of the solutions is on.

	// Get the plane normals
	VECTOR OriginalNormal = VVASEMath::GetPlaneNormal(OriginalCenter1, OriginalCenter2, OriginalCenter3);
	VECTOR NewNormal = VVASEMath::GetPlaneNormal(Center1, Center2, Center3);

	// Get a vector from the location of the point to some point in the plane
	VECTOR OriginalVectorToPlane = OriginalCenter1 - Original;
	VECTOR NewVectorToPlane = Center1 - Solution1;

	// The dot products of the normal and the vector to the plane will give an indication
	// of which side of the plane the point is on
	double OriginalSide = OriginalNormal * OriginalVectorToPlane;
	double NewSide = NewNormal * NewVectorToPlane;

	// We can compare the sign of the original side with the new side to choose the correct solution
	if ((NewSide > 0 && OriginalSide > 0) || (NewSide < 0 && OriginalSide < 0))
		Current = Solution1;
	else
		Current = Solution2;

	return true;
}

//==========================================================================
// Class:			SUSPENSION
// Function:		MoveSteeringRack
//
// Description:		This function moves the two inboard points that
//					represent the ends of the steering rack.  Travel is in
//					inches.  Positive travel moves the rack to the right.
//
// Input Arguments:
//		Travel	= const double& specifying the distance to move the steering rack [in]
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//		
//==========================================================================
void SUSPENSION::MoveSteeringRack(const double &Travel)
{
	// The two front inboard tie-rod locations will define the axis along which the
	// rack will slide.  Both points get moved by Travel in the same direction.
	// Here we'll use a parametric equation of the line in 3-space for convenience.

	double t = 1.0;// Parametric parameter
	VECTOR Slope;

	Slope = RightFront.Hardpoints[CORNER::InboardTieRod] - LeftFront.Hardpoints[CORNER::InboardTieRod];

	// We need to calibrate t to the distance between the rack ends so we know how
	// much t we need to move the rack ends by Travel.  Using the left end as the
	// reference, we have the following parameteric equations:
	//    x = left.X + Slope.X * t
	//    y = left.Y + Slope.Y * t
	//    z = left.Z + Slope.Z * t
	// So with t = 0, we have the original location of the left end of the rack,
	// and if t = 1 we'll get the right end of the rack.

	// We can normalize t to the distance between the ends of the rack and we'll
	// have t with units [1/in].
	t /= RightFront.Hardpoints[CORNER::InboardTieRod].Distance(LeftFront.Hardpoints[CORNER::InboardTieRod]);

	// Now we multiply by how far we actually want the rack to move...
	t *= Travel;// t is now unitless again

	// And we're ready to move the rack.  Since the slope and t calibration are the
	// same for both points, we can just use each point as their own reference points.
	LeftFront.Hardpoints[CORNER::InboardTieRod] += Slope * t;
	RightFront.Hardpoints[CORNER::InboardTieRod] += Slope * t;

	return;
}

//==========================================================================
// Class:			SUSPENSION
// Function:		SolveForXY
//
// Description:		This is a modification of the solver for the rest of
//					the suspension hardpoints.  Here, the Z component of
//					Original is assumed to be correct and is never modified.
//					Otherwise, it works the same way as SolveForPoint.  In
//					the event of an error, the original value is returned.
//
// Input Arguments:
//		Center1			= const VECTOR& specifying the center of the first sphere
//		Center2			= const VECTOR& specifying the center of the second sphere
//		OriginalCenter1	= const VECTOR& specifying the original center of the
//						  first sphere
//		OriginalCenter2	= const VECTOR& specifying the original center of the
//						  second sphere
//		Original		= const VECTOR& specifying the original location of the point
//						  we are solving for
//
// Output Arguments:
//		Current	= VECTOR& specifying the result of the intersection of
//				  three spheres algorithm
//
// Return Value:
//		bool, true for success, false for error
//		
//==========================================================================
bool SUSPENSION::SolveForXY(const VECTOR &Center1, const VECTOR &Center2, const VECTOR &OriginalCenter1,
							const VECTOR &OriginalCenter2, const VECTOR &Original, VECTOR &Current)
{
	// Compute the circle radii
	double R1 = OriginalCenter1.Distance(Original);
	double R2 = OriginalCenter2.Distance(Original);

	// Check for the existence of a solution
	if (Center1.Distance(Center2) > R1 + R2)
	{
		Debugger->Print(_T("Error (SolveForXY): Center distance exceeds sum of radii"), DEBUGGER::PriorityLow);

		return false;
	}
	else if (Center1.Distance(Center2) + min(R1, R2) < max(R1, R2))
	{
		Debugger->Print(_T("Error (SolveForXY): Center distance and smaller radius less than larger radius"),
			DEBUGGER::PriorityLow);

		return false;
	}

	// Declare our plane constants
	double A1, B1, C1, D1;

	// Step 1 (Plane defined by intersection of spheres 1 and 2)
	A1 = Center1.X - Center2.X;
	B1 = Center1.Y - Center2.Y;
	C1 = Center1.Z - Center2.Z;
	D1 = (pow(Center2.Length(), 2) - pow(Center1.Length(), 2) - R2 * R2 + R1 * R1) / 2.0;

	// Step 2 (Line defined by intersection of planes from steps 1 and <0 0 1>)
	// The if..else stuff avoid numerical instabilities - we'll choose the denominators
	// farthest from zero for all divisions.

	// Let's declare our answers now
	VECTOR Solution1, Solution2;

	// And our quadratic equation coefficients
	double a, b, c;

	// In which order do we want to solve for the components of the points?
	// TODO:  This code can probably be leaned out, but it works fine...
	if (max(fabs(B1), fabs(A1)) == fabs(B1))
	{
		if (max(max(fabs(C1), fabs(1.0)), fabs(B1)) == fabs(C1))
		{
			// Solve X first (use first set of plane coefficients)
			double myx = -A1 / B1;
			double byx = -(C1 * Current.Z + D1) / B1;
			
			a = 1.0 + myx * myx + pow(A1 + B1 * myx, 2) / (C1 * C1);
			b = 2.0 * (myx * (byx - Center1.Y) - Center1.X + (A1 + B1 * myx) / C1
				* ((B1 * byx + D1) / C1 + Center1.Z));
			c = Center1.X * Center1.X + pow(byx - Center1.Y, 2) - R1 * R1
				+ pow((B1 * byx + D1) / C1 + Center1.Z, 2);
			// First solution
			Solution1.X = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.X = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Y next
			Solution1.Y = Solution1.X * myx + byx;
			Solution2.Y = Solution2.X * myx + byx;
			// Solve Z last
			Solution1.Z = (-A1 * Solution1.X - B1 * Solution1.Y - D1) / C1;
			Solution2.Z = (-A1 * Solution2.X - B1 * Solution2.Y - D1) / C1;
		}
		else if (max(max(fabs(C1), fabs(1.0)), fabs(B1)) == fabs(1.0))
		{
			// Solve X first (use second set of plane coefficients)
			double myx = 0.0;
			double byx = -C1 * Current.Z / B1;
			a = 1.0 + myx * myx;
			b = 2.0 * (myx * (byx - Center1.Y) - Center1.X);
			c = Center1.X * Center1.X + pow(byx - Center1.Y, 2) - R1 * R1
				+ pow(Center1.Z - Current.Z, 2);
			// First solution
			Solution1.X = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.X = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Y next
			Solution1.Y = Solution1.X * myx + byx;
			Solution2.Y = Solution2.X * myx + byx;
			// Solve Z last
			Solution1.Z = Current.Z;
			Solution2.Z = Current.Z;
		}
		else// if (max(max(fabs(C1), fabs(1.0)), fabs(B1)) == fabs(B1))
		{
			// Solve X first (use first set of plane coefficients)
			double mzx = 0.0;
			double bzx = Current.Z;
			a = 1.0 + mzx * mzx + pow(A1 + C1 * mzx, 2) / (B1 * B1);
			b = 2.0 * (mzx * (bzx - Center1.Z) - Center1.X + (A1 + C1 * mzx) / B1
				* ((C1 * bzx + D1) / B1 + Center1.Y));
			c = Center1.X * Center1.X + pow(bzx - Center1.Z, 2) - R1 * R1
				+ pow((C1 * bzx + D1) / B1 + Center1.Y, 2);
			// First solution
			Solution1.X = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.X = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Z next
			Solution1.Z = Solution1.X * mzx + bzx;
			Solution2.Z = Solution2.X * mzx + bzx;
			// Solve Y last
			Solution1.Y = (-A1 * Solution1.X - C1 * Solution1.Z - D1) / B1;
			Solution2.Y = (-A1 * Solution2.X - C1 * Solution2.Z - D1) / B1;
		}
	}
	else// if (max(fabs(B1), fabs(A1)) == fabs(den2))
	{
		if (max(max(fabs(A1), fabs(C1)), fabs(1.0)) == fabs(A1))
		{
			// Solve Y first (use first set of plane coefficients)
			double mzy = 0.0;
			double bzy = Current.Z;
			a = 1 + mzy * mzy + pow(B1 + C1 * mzy, 2) / (A1 * A1);
			b = 2 * (mzy * (bzy - Center1.Z) - Center1.Y + (B1 + C1 * mzy) / A1
				* ((C1 * bzy + D1) / A1 + Center1.X));
			c = Center1.Y * Center1.Y + pow(bzy - Center1.Z, 2) - R1 * R1
				+ pow((C1 * bzy + D1) / A1 + Center1.X, 2);
			// First solution
			Solution1.Y = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.Y = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve Z next
			Solution1.Z = Solution1.Y * mzy + bzy;
			Solution2.Z = Solution2.Y * mzy + bzy;
			// Solve X last
			Solution1.X = (-B1 * Solution1.Y - C1 * Solution1.Z - D1) / A1;
			Solution2.X = (-B1 * Solution2.Y - C1 * Solution2.Z - D1) / A1;
		}
		else if (max(max(fabs(A1), fabs(C1)), fabs(1.0)) == fabs(C1))
		{
			// Solve Y first (use first set of plane coefficients)
			double mxy = -B1 / A1;
			double bxy = -(C1 * Current.Z + D1) / A1;
			a = 1 + mxy * mxy + pow(B1 + A1 * mxy, 2) / (C1 * C1);
			b = 2 * (mxy * (bxy - Center1.X) - Center1.Y + (B1 + A1 * mxy) / C1
				* ((A1 * bxy + D1) / C1 + Center1.Z));
			c = Center1.Y * Center1.Y + pow(bxy - Center1.X, 2) - R1 * R1
				+ pow((A1 * bxy + D1) / C1 + Center1.Z, 2);
			// First solution
			Solution1.Y = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.Y = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve X next
			Solution1.X = Solution1.Y * mxy + bxy;
			Solution2.X = Solution2.Y * mxy + bxy;
			// Solve Z last
			Solution1.Z = (-A1 * Solution1.X - B1 * Solution1.Y - D1) / C1;
			Solution2.Z = (-A1 * Solution2.X - B1 * Solution2.Y - D1) / C1;
		}
		else// if (max(max(fabs(A1), fabs(C1)), fabs(1.0)) == fabs(1.0))
		{
			// Solve Y first (use second set of plane coefficients)
			double mxy = -B1 / A1;
			double bxy = -(C1 * Current.Z + D1) / A1;
			a = 1 + mxy * mxy;
			b = 2 * (mxy * (bxy - Center1.X) - Center1.Y);
			c = Center1.Y * Center1.Y + pow(bxy - Center1.X, 2) - R1 * R1
				+ pow(Center1.Z - Current.Z, 2);
			// First solution
			Solution1.Y = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			// Second solution
			Solution2.Y = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);
			// Solve X next
			Solution1.X = Solution1.Y * mxy + bxy;
			Solution2.X = Solution2.Y * mxy + bxy;
			// Solve Z last
			Solution1.Z = Current.Z;
			Solution2.Z = Current.Z;
		}
	}

	// Make sure the solution is valid
	if (Solution1 != Solution1 || Solution2 != Solution2)
	{
		Debugger->Print(_T("Error (SolveForXY): Invalid solution"), DEBUGGER::PriorityLow);

		return false;
	}

	// We now have two solutions.  To choose between them, we must examine the original
	// location of the point and the original sphere centers.  The two sphere centers
	// define a plane (assume plane is perpendicular to the ground plane), and the the two
	// solutions lie on opposite sides of the plane.  The first step in identifying the
	// correct solution is to determine which side of the original plane the original
	// point was on, and which side of the new plane either one of the solutions is on.

	// Get the plane normals
	VECTOR PointInPlane = OriginalCenter1;
	PointInPlane.Z = 0.0;
	VECTOR OriginalNormal = (OriginalCenter1 - OriginalCenter2).Cross(OriginalCenter1 - PointInPlane);
	PointInPlane = Center1;
	PointInPlane.Z = 0.0;
	VECTOR NewNormal = (Center1 - Center2).Cross(Center1 - PointInPlane);

	// Get a vector from the location of the point to some point in the plane
	VECTOR OriginalVectorToPlane = OriginalCenter1 - Original;
	VECTOR NewVectorToPlane = Center1 - Solution1;

	// The dot products of the normal and the vector to the plane will give an indication
	// of which side of the plane the point is on
	double OriginalSide = OriginalNormal * OriginalVectorToPlane;
	double NewSide = NewNormal * NewVectorToPlane;

	// We can compare the sign of the original side with the new side to choose the correct solution
	if ((NewSide > 0 && OriginalSide > 0) || (NewSide < 0 && OriginalSide < 0))
		Current = Solution1;
	else
		Current = Solution2;

	return true;
}

//==========================================================================
// Class:			SUSPENSION
// Function:		SolveForContactPatch
//
// Description:		This function solves for the circle created by the
//					intersection of a sphere of radius TireRadius centered
//					around point WheelCenter, with the plane that passes
//					through WheelCenter and has normal vector
//					WheelPlaneNormal.  The vector returned by the function
//					is the point on the circle with minimum Z value.  In
//					the event of an error, a zero length vector is returned.
//
// Input Arguments:
//		WheelCenter			= const VECTOR& specifying the center of the wheel
//		WheelPlaneNormal	= const VECTOR& specifying the orientation of the wheel
//		TireRadius			= const double& specifying the loaded radius of the tire
//
// Output Arguments:
//		Output	= VECTOR& specifying the location of the center of the tire's
//				  contact patch
//
// Return Value:
//		bool, true for success, false for error
//		
//==========================================================================
bool SUSPENSION::SolveForContactPatch(const VECTOR &WheelCenter, const VECTOR &WheelPlaneNormal,
									  const double &TireRadius, VECTOR &Output)
{
	VECTOR MinimumZPoint;

	// Equation for sphere as described above:
	// (x - WheelCenter.X)^2 + (y - WheelCenter.Y)^2 + (z - WheelCenter.Z)^2 - TireRadius^2 = 0
	// Equation for plane as described above:
	// x * WheelPlaneNormal.X + y * WheelPlaneNormal.Y + z * WheelPlaneNormal.Z - WheelCenter * WheelPlaneNormal = 0
	// Solving the plane equation for y yields an equation for y in terms of x and z.
	// y was chosen here, because WheelPlaneNormal is least likely to have a zero Y componenet,
	// versus X or Z.
	// y(x, z) = (x * WheelPlaneNormal.X + z * WheelPlaneNormal.Z - WheelCenter * WheelPlaneNormal) / WheelPlaneNormal.Y
	// Next, we substitute this equation for y back into the sphere equation, leaving us with a
	// quadratic function for z in terms of x.  We now have all of the components necessary to describe
	// the circle we originally set out to find -> z(x) and y(x, z).  To find our minimum Z point, we
	// differentiate z(x) and set equal to zero.  This gives us x as a function of the inputs to
	// SolveForContactPatch only.
	MinimumZPoint.X = (WheelCenter.X * (pow(WheelPlaneNormal.X, 4) + 2 * pow(WheelPlaneNormal.X, 2)
		* pow(WheelPlaneNormal.Y, 2) + pow(WheelPlaneNormal.Y, 4) + pow(WheelPlaneNormal.X, 2)
		* pow(WheelPlaneNormal.Z, 2) + pow(WheelPlaneNormal.Y, 2) * pow(WheelPlaneNormal.Z, 2))
		- sqrt(pow(TireRadius * WheelPlaneNormal.X * WheelPlaneNormal.Z, 2)	* (pow(WheelPlaneNormal.X, 4)
		+ 2 * pow(WheelPlaneNormal.X, 2) * pow(WheelPlaneNormal.Y, 2) + pow(WheelPlaneNormal.Y, 4)
		+ pow(WheelPlaneNormal.X * WheelPlaneNormal.Z, 2) + pow(WheelPlaneNormal.Y * WheelPlaneNormal.Z, 2))))
		/ (pow(WheelPlaneNormal.X, 4) + 2 * pow(WheelPlaneNormal.X * WheelPlaneNormal.Y, 2)
		+ pow(WheelPlaneNormal.Y, 4) + pow(WheelPlaneNormal.X * WheelPlaneNormal.Z, 2)
		+ pow(WheelPlaneNormal.Y * WheelPlaneNormal.Z, 2));

	// Now we can plug back into our equations for Y and Z.  For Z, we'll solve the quadratic with
	// the quadratic equation.  Since A is always positive, we know we'll always want the minus
	// solution when we're looking for the minimum.
	double A, B, C;
	A = 1 + pow(WheelPlaneNormal.Z / WheelPlaneNormal.Y, 2);
	B = 2 * (WheelPlaneNormal.Z / WheelPlaneNormal.Y * WheelCenter.Y - WheelCenter.Z - WheelPlaneNormal.Z
		/ pow(WheelPlaneNormal.Y, 2) * (WheelCenter * WheelPlaneNormal - WheelPlaneNormal.X * MinimumZPoint.X));
	C = pow(MinimumZPoint.X - WheelCenter.X, 2) + pow(WheelCenter * WheelPlaneNormal - WheelPlaneNormal.X
		* MinimumZPoint.X, 2) / pow(WheelPlaneNormal.Y, 2) - 2 * WheelCenter.Y / WheelPlaneNormal.Y
		* (WheelCenter * WheelPlaneNormal - WheelPlaneNormal.X * MinimumZPoint.X) + pow(WheelCenter.Y, 2)
		+ pow(WheelCenter.Z, 2) - pow(TireRadius, 2);
	MinimumZPoint.Z = (-B - sqrt(B * B - 4 * A * C)) / (2 * A);
	MinimumZPoint.Y = (WheelCenter * WheelPlaneNormal - MinimumZPoint.X * WheelPlaneNormal.X
		- MinimumZPoint.Z * WheelPlaneNormal.Z) / WheelPlaneNormal.Y;

	// Check to make sure the solution is valid
	if (MinimumZPoint != MinimumZPoint)
	{
		// Return a zero-length vector
		Output.Set(0.0, 0.0, 0.0);
		Debugger->Print(_T("Error (SolveForContactPatch):  Invalid solution"), DEBUGGER::PriorityLow);

		return false;
	}

	// Assign the solution
	Output = MinimumZPoint;

	return true;
}

//==========================================================================
// Class:			SUSPENSION
// Function:		Write
//
// Description:		Writes this suspension to file.
//
// Input Arguments:
//		OutFile	= std::ofstream* pointing to the output stream
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void SUSPENSION::Write(std::ofstream *OutFile) const
{
	// Write the components that make up this object to file
	RightFront.Write(OutFile);
	LeftFront.Write(OutFile);
	RightRear.Write(OutFile);
	LeftRear.Write(OutFile);

	OutFile->write((char*)Hardpoints, sizeof(VECTOR) * NumberOfHardpoints);

	OutFile->write((char*)&BarRate, sizeof(FRONT_REAR_DOUBLE));
	OutFile->write((char*)&RackRatio, sizeof(double));

	// Flags and styles
	OutFile->write((char*)&IsSymmetric, sizeof(bool));
	OutFile->write((char*)&FrontBarStyle, sizeof(BAR_STYLE));
	OutFile->write((char*)&RearBarStyle, sizeof(BAR_STYLE));
	OutFile->write((char*)&FrontBarAttachment, sizeof(BAR_ATTACHMENT));
	OutFile->write((char*)&RearBarAttachment, sizeof(BAR_ATTACHMENT));
	OutFile->write((char*)&FrontHasThirdSpring, sizeof(bool));
	OutFile->write((char*)&RearHasThirdSpring, sizeof(bool));

	return;
}

//==========================================================================
// Class:			SUSPENSION
// Function:		Read
//
// Description:		Read from file to fill this suspension.
//
// Input Arguments:
//		InFile		= std::ifstream* pointing to the input stream
//		FileVersion	= int specifying which file version we're reading from
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void SUSPENSION::Read(std::ifstream *InFile, int FileVersion)
{
	// Read the components that make up this object from file
	RightFront.Read(InFile, FileVersion);
	LeftFront.Read(InFile, FileVersion);
	RightRear.Read(InFile, FileVersion);
	LeftRear.Read(InFile, FileVersion);

	InFile->read((char*)Hardpoints, sizeof(VECTOR) * NumberOfHardpoints);

	InFile->read((char*)&BarRate, sizeof(FRONT_REAR_DOUBLE));
	InFile->read((char*)&RackRatio, sizeof(double));

	// Flags and styles
	InFile->read((char*)&IsSymmetric, sizeof(bool));
	InFile->read((char*)&FrontBarStyle, sizeof(BAR_STYLE));
	InFile->read((char*)&RearBarStyle, sizeof(BAR_STYLE));
	InFile->read((char*)&FrontBarAttachment, sizeof(BAR_ATTACHMENT));
	InFile->read((char*)&RearBarAttachment, sizeof(BAR_ATTACHMENT));
	InFile->read((char*)&FrontHasThirdSpring, sizeof(bool));
	InFile->read((char*)&RearHasThirdSpring, sizeof(bool));

	// Third spring and damper objects
	// NOT YET USED!!!
	/*SPRING FrontThirdSpring;
	SPRING RearThirdSpring;
	DAMPER FrontThirdDamper;
	DAMPER RearThirdDamper;*/

	return;
}

//==========================================================================
// Class:			SUSPENSION
// Function:		GetBarStyleName
//
// Description:		Returns the name of the sway bar style.
//
// Input Arguments:
//		_BarStyle	= const BAR_STYLE& of interest
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing the name of the bar style
//
//==========================================================================
wxString SUSPENSION::GetBarStyleName(const BAR_STYLE &_BarStyle)
{
	switch (_BarStyle)
	{
	case SwayBarNone:
		return _T("None");
		break;

	case SwayBarUBar:
		return _T("U-Bar");
		break;

	case SwayBarTBar:
		return _T("T-Bar");
		break;

	case SwayBarGeared:
		return _T("Geared");
		break;

	default:
		assert(0);
		break;
	}

	return wxEmptyString;
}

//==========================================================================
// Class:			SUSPENSION
// Function:		GetHardpointName
//
// Description:		Returns the name of the point.
//
// Input Arguments:
//		Point	= const HARDPOINTS&, specifying the point to retrieve
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing the name of the point
//
//==========================================================================
wxString SUSPENSION::GetHardpointName(const HARDPOINTS& Point)
{
	switch (Point)
	{
	case FrontBarMidPoint:
		return _T("Front Bar Mid-Point");
		break;

	case FrontThirdSpringInboard:
		return _T("Front Third Spring Inboard");
		break;

	case FrontThirdSpringOutboard:
		return _T("Front Third Spring Outboard");
		break;

	case FrontThirdShockInboard:
		return _T("Front Third Shock Inboard");
		break;

	case FrontThirdShockOutboard:
		return _T("Front Third Shock Outboard");
		break;

	case RearBarMidPoint:
		return _T("Rear Bar Mid-Point");
		break;

	case RearThirdSpringInboard:
		return _T("Rear Third Spring Inboard");
		break;

	case RearThirdSpringOutboard:
		return _T("Rear Third Spring Outboard");
		break;

	case RearThirdShockInboard:
		return _T("Rear Third Shock Inboard");
		break;

	case RearThirdShockOutboard:
		return _T("Rear Third Shock Outboard");
		break;

	default:
		assert(0);
		break;
	}

	return wxEmptyString;
}

//==========================================================================
// Class:			SUSPENSION
// Function:		GetBarAttachmentname
//
// Description:		Returns the name of the bar attachment method.
//
// Input Arguments:
//		_BarAttachment	= const BAR_ATTACHMENT&, specifying the type of attachment
//
// Output Arguments:
//		None
//
// Return Value:
//		wxString containing the name of the attachment method
//
//==========================================================================
wxString SUSPENSION::GetBarAttachmentname(const BAR_ATTACHMENT &_BarAttachment)
{
	switch (_BarAttachment)
	{
	case BarAttachmentBellcrank:
		return _T("Bellcrank");
		break;

	case BarAttachmentLowerAArm:
		return _T("Lower A-Arm");
		break;

	case BarAttachmentUpperAArm:
		return _T("Upper A-Arm");
		break;

	case BarAttachmentUpright:
		return _T("Upright");
		break;

	default:
		assert(0);
		break;
	}

	return wxEmptyString;
}

//==========================================================================
// Class:			SUSPENSION
// Function:		ComputeWheelCenters
//
// Description:		Calls the methods at each corner that compute the wheel
//					center location.
//
// Input Arguments:
//		RFTireDiameter	= const double& specifying the diameter of the right front tire
//		LFTireDiameter	= const double& specifying the diameter of the left front tire
//		RRTireDiameter	= const double& specifying the diameter of the right rear tire
//		LRTireDiameter	= const double& specifying the diameter of the left rear tire
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void SUSPENSION::ComputeWheelCenters(const double &RFTireDiameter, const double &LFTireDiameter,
									 const double &RRTireDiameter, const double &LRTireDiameter)
{
	// Call the methods for each corner
	RightFront.ComputeWheelCenter(RFTireDiameter);
	LeftFront.ComputeWheelCenter(LFTireDiameter);
	RightRear.ComputeWheelCenter(RRTireDiameter);
	LeftRear.ComputeWheelCenter(LRTireDiameter);

	return;
}

//==========================================================================
// Class:			SUSPENSION
// Function:		operator=
//
// Description:		Overloaded assignment operator.  This is necessary due to
//					the references that this class contains.
//
// Input Arguments:
//		Suspension	= const SUSPENSION& to be assigned to this
//
// Output Arguments:
//		None
//
// Return Value:
//		SUSPENSION&, reference to this
//
//==========================================================================
SUSPENSION& SUSPENSION::operator=(const SUSPENSION& Suspension)
{
	// Check for self assignment
	if (this == &Suspension)
		return *this;

	// Copy the corners
	RightFront = Suspension.RightFront;
	LeftFront = Suspension.LeftFront;
	RightRear = Suspension.RightRear;
	LeftRear = Suspension.LeftRear;

	// Copy the hardpoints
	int i;
	for (i = 0; i < NumberOfHardpoints; i++)
		Hardpoints[i] = Suspension.Hardpoints[i];

	// Suspension parameters
	BarRate = Suspension.BarRate;
	RackRatio = Suspension.RackRatio;

	// Flags and styles
	IsSymmetric = Suspension.IsSymmetric;
	FrontBarStyle = Suspension.FrontBarStyle;
	RearBarStyle = Suspension.RearBarStyle;
	FrontBarAttachment = Suspension.FrontBarAttachment;
	RearBarAttachment = Suspension.RearBarAttachment;
	FrontHasThirdSpring = Suspension.FrontHasThirdSpring;
	RearHasThirdSpring = Suspension.RearHasThirdSpring;

	// Third spring and damper objects
	FrontThirdSpring = Suspension.FrontThirdSpring;
	RearThirdSpring = Suspension.RearThirdSpring;
	FrontThirdDamper = Suspension.FrontThirdDamper;
	RearThirdDamper = Suspension.RearThirdDamper;

	return *this;
}