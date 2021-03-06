/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2016

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  carMath.cpp
// Created:  3/24/2008
// Author:  K. Loux
// Description:  Contains useful functions that don't fit better in another class.  Hopefully this
//				 file will one day be absolved into a real class instead of just being a kludgy
//				 collection of functions.  The goal would be to be able to use Debugger classes again.
// History:
//	11/22/2009	- Moved to vMath.lib, K. Loux.
//	11/7/2011	- Corrected camelCase, K. Loux.

// Standard C++ headers
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <limits>
#include <cstdarg>

// wxWidgets headers
#include <wx/wx.h>

// VVASE headers
#include "vMath/carMath.h"
#include "vMath/vector.h"
#include "vMath/matrix.h"
#include "vMath/dataset2D.h"

//==========================================================================
// Namespace:		VVASEMath
// Function:		IsZero
//
// Description:		Returns true if a number is small enough to regard as zero.
//
// Input Arguments:
//		n	= const double& to be checked for being close to zero
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the number is less than NEARLY_ZERO
//
//==========================================================================
bool VVASEMath::IsZero(const double &n, const double &eps)
{
	if (fabs(n) < eps)
		return true;
	else
		return false;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		IsZero
//
// Description:		Returns true if a number is small enough to regard as zero.
//					This function checks the magnitude of the Vector.
//
// Input Arguments:
//		v	= const Vector& to be checked for being close to zero
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the magnitude is less than NEARLY_ZERO
//
//==========================================================================
bool VVASEMath::IsZero(const Vector &v, const double &eps)
{
	// Check each component of the vector
	if (v.Length() < eps)
		return true;

	return false;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		Clamp
//
// Description:		Ensures the specified value is between the limits.  In the
//					event that the value is out of the specified bounds, the
//					value that is returned is equal to the limit that the value
//					has exceeded.
//
// Input Arguments:
//		value		= const double& reference to the value which we want to clamp
//		lowerLimit	= const double& lower bound of allowable values
//		upperLimit	= const double& upper bound of allowable values
//
// Output Arguments:
//		None
//
// Return Value:
//		double, equal to the clamped value
//
//==========================================================================
double VVASEMath::Clamp(const double &value, const double &lowerLimit, const double &upperLimit)
{
	// Make sure the arguments are valid
	assert(lowerLimit < upperLimit);

	if (value < lowerLimit)
		return lowerLimit;
	else if (value > upperLimit)
		return upperLimit;

	return value;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		RangeToPlusMinusPi
//
// Description:		Adds or subtracts 2 * pi to the specified angle until the
//					angle is between -pi and pi.
//
// Input Arguments:
//		angle		= const double& reference to the angle we want to bound
//
// Output Arguments:
//		None
//
// Return Value:
//		double, equal to the re-ranged angle
//
//==========================================================================
double VVASEMath::RangeToPlusMinusPi(const double &angle)
{
	// NOTE:  fmod function returns a *signed* remainder of truncated division.
	// Other floating-point modulo operations may return a the result differently,
	// which can make it hard to test the math outside of C/C++ (the result
	// returned by Excel is always positive, for example).
	if (angle > -Pi)
		return fmod(angle + Pi, 2.0 * Pi) - Pi;
	return fmod(angle + Pi, 2.0 * Pi) + Pi;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		RangeToPlusMinus180
//
// Description:		Adds or subtracts 180 to the specified angle until the
//					angle is between -180 and 180.
//
// Input Arguments:
//		angle		= const double& reference to the angle we want to bound
//
// Output Arguments:
//		None
//
// Return Value:
//		double, equal to the re-ranged angle
//
//==========================================================================
double VVASEMath::RangeToPlusMinus180(const double &angle)
{
	// NOTE:  fmod function returns a *signed* remainder of truncated division.
	// Other floating-point modulo operations may return a the result differently,
	// which can make it hard to test the math outside of C/C++ (the result
	// returned by Excel is always positive, for example).
	if (angle > -180.0)
		return fmod(angle + 180.0, 360.0) - 180.0;
	return fmod(angle + 180.0, 360.0) + 180.0;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		Unwrap
//
// Description:		Minimizes the jump between adjacent points by adding/subtracting
//					multiples of 2 * Pi.
//
// Input Arguments:
//		data	= Dataset2D&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void VVASEMath::Unwrap(Dataset2D &data)
{
	const double threshold(Pi);
	unsigned int i;
	for (i = 1; i < data.GetNumberOfPoints(); i++)
	{
		if (data.GetYData(i) - data.GetYData(i - 1) > threshold)
			data.GetYPointer()[i] -= 2 * Pi;
		if (data.GetYData(i) - data.GetYData(i - 1) < -threshold)
			data.GetYPointer()[i] += 2 * Pi;
	}
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		Sign
//
// Description:		Returns 1.0 for positive, -1.0 for negative and 0.0 for zero.
//
// Input Arguments:
//		value		= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double VVASEMath::Sign(const double &value)
{
	if (value > 0.0)
		return 1.0;
	else if (value < 0.0)
		return -1.0;
	else
		return 0.0;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		ApplyBitMask
//
// Description:		Extracts a single bit from values of the specified dataset.
//
// Input Arguments:
//		data	= const Dataset2D&
//		bit		= const unsigned int&
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
Dataset2D VVASEMath::ApplyBitMask(const Dataset2D &data, const unsigned int &bit)
{
	Dataset2D set(data);
	unsigned int i;
	for (i = 0; i < set.GetNumberOfPoints(); i++)
		set.GetYPointer()[i] = ApplyBitMask((unsigned int)set.GetYPointer()[i], bit);
	return set;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		ApplyBitMask
//
// Description:		Extracts a single bit from the value.
//
// Input Arguments:
//		value	= const unsigned int&
//		bit		= const unsigned int&
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
unsigned int VVASEMath::ApplyBitMask(const unsigned &value, const unsigned int &bit)
{
	return (value >> bit) & 1;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		XDataConsistentlySpaced
//
// Description:		Checks to see if the X-data has consistent deltas.
//
// Input Arguments:
//		data				= const Dataset2D&
//		tolerancePercent	= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the x-data spacing is within the tolerance
//
//==========================================================================
bool VVASEMath::XDataConsistentlySpaced(const Dataset2D &data, const double &tolerancePercent)
{
	assert(data.GetNumberOfPoints() > 1);

	unsigned int i;
	double minSpacing, maxSpacing, spacing;

	minSpacing = data.GetAverageDeltaX();
	maxSpacing = minSpacing;

	for (i = 2; i < data.GetNumberOfPoints(); i++)
	{
		spacing = data.GetXData(i) - data.GetXData(i - 1);
		if (spacing < minSpacing)
			minSpacing = spacing;
		if (spacing > maxSpacing)
			maxSpacing = spacing;
	}

	// Handle decreasing data, too
	if (fabs(minSpacing) > fabs(maxSpacing))
	{
		double temp(minSpacing);
		minSpacing = maxSpacing;
		maxSpacing = temp;
	}

	return 1.0 - minSpacing / maxSpacing < tolerancePercent;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		GetAverageXSpacing
//
// Description:		Finds the average period of the data in the set.
//
// Input Arguments:
//		data	= const Dataset2D&
//
// Output Arguments:
//		None
//
// Return Value:
//		double
//
//==========================================================================
double VVASEMath::GetAverageXSpacing(const Dataset2D &data)
{
	return data.GetXData(data.GetNumberOfPoints() - 1) / (data.GetNumberOfPoints() - 1.0);
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		GetPrecision
//
// Description:		Determines the best number of digits after the decimal place
//					for a string representation of the specified value (for
//					use with printf-style %0.*f formatting.
//
// Input Arguments:
//		value				= const double&
//		significantDigits	= const unsigned int&
//		dropTrailingZeros	= const bool&
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true if the x-data spacing is within the tolerance
//
//==========================================================================
unsigned int VVASEMath::GetPrecision(const double &value,
	const unsigned int &significantDigits, const bool &dropTrailingZeros)
{
	int precision(significantDigits - (unsigned int)floor(log10(value)) - 1);
	if (precision < 0)
		precision = 0;
	if (!dropTrailingZeros)
		return precision;

	const unsigned int sSize(512);
	char s[sSize];
	sprintf(s, sSize, "%0.*f", precision, value);

	std::string number(s);
	unsigned int i;
	for (i = number.size() - 1; i > 0; i--)
	{
		if (s[i] == '0')
			precision--;
		else
			break;
	}

	if (precision < 0)
		precision = 0;

	return precision;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		CountSignificantDigits
//
// Description:		Returns the number of significant digits in the string.
//
// Input Arguments:
//		valueString	= const wxString&
//
// Output Arguments:
//		None
//
// Return Value:
//		unsigned int
//
//==========================================================================
unsigned int VVASEMath::CountSignificantDigits(const wxString &valueString)
{
	double value;
	if (!valueString.ToDouble(&value))
		return 0;

	wxString trimmedValueString = wxString::Format("%+0.15f", value);
	unsigned int firstDigit, lastDigit;
	for (firstDigit = 1; firstDigit < trimmedValueString.Len(); firstDigit++)
	{
		if (trimmedValueString[firstDigit] != '0' && trimmedValueString[firstDigit] != '.')
			break;
	}

	for (lastDigit = trimmedValueString.Len() - 1; lastDigit > firstDigit; lastDigit--)
	{
		if (trimmedValueString[lastDigit] != '0' && trimmedValueString[lastDigit] != '.')
			break;
	}

	unsigned int i;
	for (i = firstDigit + 1; i < lastDigit - 1; i++)
	{
		if (trimmedValueString[i] == '.')
		{
			firstDigit++;
			break;
		}
	}

	return lastDigit - firstDigit + 1;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		sprintf
//
// Description:		Cross-platform friendly sprintf(_s) macro.  Calls sprintf_s
//					under MSW, sprintf otherwise.
//
// Input Arguments:
//		dest	= char*
//		size	= const unsigned int&
//		format	= const char*
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
#ifdef __WXMSW__
void VVASEMath::sprintf(char *dest, const unsigned int &size, const char *format, ...)
#else
void VVASEMath::sprintf(char *dest, const unsigned int&, const char *format, ...)
#endif
{
	va_list list;
	va_start(list, format);

#ifdef __WXMSW__
	vsprintf_s(dest, size, format, list);
#else
	vsprintf(dest, format, list);
#endif

	va_end(list);
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		GetPrecision
//
// Description:		Returns the required precision (digits past zero) to
//					distinguish between adjacent graduations.
//
// Input Arguments:
//		minimum			= const double&
//		majorResolution	= const double&
//		isLogarithmic	= const bool&
//
// Output Arguments:
//		None
//
// Return Value:
//		unsigned int
//
//==========================================================================
unsigned int VVASEMath::GetPrecision(const double &minimum, const double &majorResolution, const bool &isLogarithmic)
{
	double baseValue;
	if (isLogarithmic)
		baseValue = minimum;
	else
		baseValue = majorResolution;

	if (log10(baseValue) >= 0.0)
		return 0;

	return -log10(baseValue) + 1;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		GetPlaneNormal
//
// Description:		Calculates the direction that is normal to the plane
//					that passes through the three input points.
//
// Input Arguments:
//		point1	= const Vector& describing location of first point on the plane
//		point2	= const Vector& describing location of second point on the plane
//		point3	= const Vector& describing location of third point on the plane
//
// Output Arguments:
//		None
//
// Return Value:
//		Vector indicating direction that is normal to the plane that is
//		defined by the three input arguments
//
//==========================================================================
Vector VVASEMath::GetPlaneNormal(const Vector &point1, const Vector &point2, const Vector &point3)
{
	// Check for existence of a solution
	if (point1 == point2 || point1 == point3 || point2 == point3)
	{
		Vector noSolution(QNAN, QNAN, QNAN);
		//Debugger->Print(_T("Warning (GetPlaneNormal):  Coincident points"), Debugger::PRIORITY_LOW);

		return noSolution;
	}

	// To find the plane normal, we subtract the locations of two points to obtain
	// a vector that lies in the plane.  Repeat with a different pair of points to
	// obtain another vector that lies in the plane.  The cross product of these
	// vectors yields the plane normal.

	return (point1 - point2).Cross(point1 - point3).Normalize();
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		GetIntersectionOfTwoPlanes
//
// Description:		Calculates the axis that is created by the intersection
//					of two planes.
//
// Input Arguments:
//		normal1			= const Vector& describing normal direciton of first plane
//		pointOnPlane1	= const Vector& describing a point on the first plane
//		normal2			= const Vector& describing normal direciton of second plane
//		pointOnPlane2	= const Vector& describing a point on the second plane
//
// Output Arguments:
//		axisDirection	= Vector& describing the direction of the axis
//		pointOnAxis		= Vector& describing a point on the axis
//
// Return Value:
//		bool indicating whether or not a solution was found
//
//==========================================================================
bool VVASEMath::GetIntersectionOfTwoPlanes(const Vector &normal1, const Vector &pointOnPlane1,
										   const Vector &normal2, const Vector &pointOnPlane2,
										   Vector &axisDirection, Vector &pointOnAxis)
{
	// Make sure the planes are not parallel - this ensures the existence of a solution
	if (!IsZero(normal1 - normal2))
	{
		// The direction is simply the cross product of the two normal vectors
		axisDirection = normal1.Cross(normal2).Normalize();

		// Now find a point on that axis
		// This comes from the vector equation of a plane:
		// If the normal vector of a plane is known, then all points on the plane satisfy
		// planeNormal * point = someConstant.  Since we know a point that lies on each plane,
		// we can solve for that constant for each plane, then solve two simultaneous systems
		// of equations to find a common point between the two planes.
		double planeConstant1 = normal1 * pointOnPlane1;
		double planeConstant2 = normal2 * pointOnPlane2;

		// To ensure numeric stability (avoid dividing by numbers close to zero),
		// we will be "smart" about solving these equations.  Since we have two
		// equations and three unknowns, we need to choose one of the components
		// of the points ourselves.  To do this, we look at the axis direction,
		// and we choose the component that is farthest from zero.  For example,
		// if the axis direction is (1.0, 0.0, 0.0), we set X = 0 because we know
		// the axis must pass through the Y-Z plane.
		if (fabs(axisDirection.x) > fabs(axisDirection.y) && fabs(axisDirection.x) > fabs(axisDirection.z))
		{
			// Choose x = 0
			pointOnAxis.x = 0.0;

			// Again, to ensure numerical stability we need to be smart about whether we
			// solve y or z next and whether we use the first or second plane's normal
			// vector in the denominator of the last solved component.
			// FIxME:  This can probably be made to be safer (can we prove we will never have a divide by zero?)
			if (fabs(normal1.y) > fabs(normal1.z))
			{
				pointOnAxis.z = (planeConstant1 * normal2.y - planeConstant2 * normal1.y)
					/ (normal2.y * normal1.z - normal2.z * normal1.y);
				pointOnAxis.y = (planeConstant1 - normal1.z * pointOnAxis.z) / normal1.y;
			}
			else
			{
				pointOnAxis.y = (planeConstant1 * normal2.z - planeConstant2 * normal1.z)
					/ (normal2.z * normal1.y - normal2.y * normal1.z);
				pointOnAxis.z = (planeConstant1 - normal1.y * pointOnAxis.y) / normal1.z;
			}
		}
		else if (fabs(axisDirection.y) > fabs(axisDirection.x) && fabs(axisDirection.y) > fabs(axisDirection.z))
		{
			// Choose y = 0
			pointOnAxis.y = 0.0;

			// Solve the other two components
			if (fabs(normal1.x) > fabs(normal1.z))
			{
				pointOnAxis.z = (planeConstant1 * normal2.x - planeConstant2 * normal1.x)
					/ (normal2.x * normal1.z - normal2.z * normal1.x);
				pointOnAxis.x = (planeConstant1 - normal1.z * pointOnAxis.z) / normal1.x;
			}
			else
			{
				pointOnAxis.x = (planeConstant1 * normal2.z - planeConstant2 * normal1.z)
					/ (normal2.z * normal1.x - normal2.x * normal1.z);
				pointOnAxis.z = (planeConstant1 - normal1.x * pointOnAxis.x) / normal1.z;
			}
		}
		else
		{
			// Choose z = 0
			pointOnAxis.z = 0.0;

			// Solve the other two components
			if (fabs(normal1.x) > fabs(normal1.y))
			{
				pointOnAxis.y = (planeConstant1 * normal2.x - planeConstant2 * normal1.x)
					/ (normal2.x * normal1.y - normal2.y * normal1.x);
				pointOnAxis.x = (planeConstant1 - normal1.y * pointOnAxis.y) / normal1.x;
			}
			else
			{
				pointOnAxis.x = (planeConstant1 * normal2.y - planeConstant2 * normal1.y)
					/ (normal2.y * normal1.x - normal2.x * normal1.y);
				pointOnAxis.y = (planeConstant1 - normal1.x * pointOnAxis.x) / normal1.y;
			}
		}

		return true;
	}

	// No solution exists - set the output arguments to QNAN and return false
	axisDirection.Set(QNAN, QNAN, QNAN);
	pointOnAxis.Set(QNAN, QNAN, QNAN);

	return false;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		NearestPointOnAxis
//
// Description:		Returns the point on the given line that is closest to
//					the specified point. The arguments are a point on the
//					axis, the direction along the axis, and the point that
//					is not on the axis.
//
// Input Arguments:
//		pointOnAxis		= const Vector& describing any point on the axis
//		directionOfAxis	= const Vector& that points in the direction of the axis
//		targetPoint		= const Vector& describing the point to compare to the
//						  axis
//
// Output Arguments:
//		None
//
// Return Value:
//		Vector describing the point on the axis that is closest to
//		targetPoint
//
//==========================================================================
Vector VVASEMath::NearestPointOnAxis(const Vector &pointOnAxis, const Vector &directionOfAxis,
						  const Vector &targetPoint)
{
	double t;
	Vector temp;

	// The shortest distance is going to to a point on the axis where the line between
	// TargetPoint and that point is perpendicular to the axis.  This means their dot
	// product will be zero.  If we use a parametric equation for the line, we can use
	// the dot product to find the value of the parameteric parameter (t) in the
	// equation of the line.

	t = (directionOfAxis * (targetPoint - pointOnAxis)) / (directionOfAxis * directionOfAxis);
	temp = pointOnAxis + directionOfAxis * t;

	return temp;
}

/*
//==========================================================================
// Namespace:		VVASEMath
// Function:		NearestPointInPlane
//
// Description:		Returns the point in a specified plane that is closest to
//					the specified point.
//
// Input Arguments:
//		pointInPlane	= const Vector& describing any point on the plane
//		planeNormal		= const Vector& defining the plane's orientation
//		targetPoint		= const Vector& describing the point to compare to the
//						  axis
//
// Output Arguments:
//		None
//
// Return Value:
//		Vector describing the point on in the plane that is closest to targetPoint
//
//==========================================================================
Vector VVASEMath::NearestPointInPlane(const Vector &pointInPlane, const Vector &planeNormal,
						   const Vector &targetPoint)
{
	double t;
	Vector temp;

	t = (directionOfAxis * (targetPoint - pointOnAxis)) / (directionOfAxis * directionOfAxis);
	temp = pointOnAxis + directionOfAxis * t;

	return temp;
}*/

//==========================================================================
// Namespace:		VVASEMath
// Function:		ProjectOntoPlane
//
// Description:		Returns the vector after it is projected onto the
//					specified plane.  Note that this assumes the plane passes
//					through the origin.
//
// Input Arguments:
//		vectorToProject	= const Vector& to be projected
//		planeNormal		= const Vector& that defines the plane orientation
//
// Output Arguments:
//		None
//
// Return Value:
//		Vector after the projection
//
//==========================================================================
Vector VVASEMath::ProjectOntoPlane(const Vector &vectorToProject, const Vector &planeNormal)
{
	Vector normalComponent = vectorToProject * planeNormal * planeNormal.Normalize();
	return vectorToProject - normalComponent;
}

//==========================================================================
// Namespace:		VVASEMath
// Function:		IntersectWithPlane
//
// Description:		Returns the point where the specified axis passes through
//					the specified plane.
//
// Input Arguments:
//		planeNormal		= const Vector& indicating the direction that is normal to
//						  the plane
//		pointOnPlane	= const Vector& specifying a point that lies on the plane
//		axisDirection	= Vector specifying the direction of the axis
//		pointOnAxis		= const Vector& specifying a point on the axis
//
// Output Arguments:
//		None
//
// Return Value:
//		Vector where the intersection occurs
//
//==========================================================================
Vector VVASEMath::IntersectWithPlane(const Vector &planeNormal, const Vector &pointOnPlane,
									 Vector axisDirection, const Vector &pointOnAxis)
{
	// Determine what will be used as the denominator to calculate the parameter
	// in our parametric equation
	double denominator = planeNormal * axisDirection;

	// Make sure this isn't zero (if it is, then there is no solution!)
	if (IsZero(denominator))
		return Vector(QNAN, QNAN, QNAN);

	// If we didn't return yet, then a solution does exist.  Determine the paramter
	// in the parametric equation of the line: P = PointOnAxis + t * AxisDirection
	double t = planeNormal * (pointOnPlane - pointOnAxis) / denominator;

	// Use the parametric equation to find the point
	return pointOnAxis + axisDirection * t;
}
