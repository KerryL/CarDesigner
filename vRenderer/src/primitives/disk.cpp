/*===================================================================================
                                    CarDesigner
                         Copyright Kerry R. Loux 2008-2011

     No requirement for distribution of wxWidgets libraries, source, or binaries.
                             (http://www.wxwidgets.org/)

===================================================================================*/

// File:  disk.cpp
// Created:  5/14/2009
// Author:  K. Loux
// Description:  Derived from Primitive for creating disk objects.
// History:
//	6/3/2009	- Modified GenerateGeometry() to make use of openGL matrices for positioning
//				  and orienting the object, K.Loux.

// Local headers
#include "vRenderer/primitives/disk.h"
#include "vRenderer/renderWindow.h"
#include "vMath/carMath.h"
#include "vUtilities/convert.h"

//==========================================================================
// Class:			Disk
// Function:		Disk
//
// Description:		Constructor for the Disk class.
//
// Input Arguments:
//		_renderWindow	= RenderWindow* pointing to the object that owns this
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
Disk::Disk(RenderWindow &_renderWindow) : Primitive(_renderWindow)
{
	// Initialize private data
	outerRadius = 0.0;
	innerRadius = 0.0;
	center.Set(0.0, 0.0, 0.0);
	normal.Set(0.0, 0.0, 0.0);
	resolution = 4;
}

//==========================================================================
// Class:			Disk
// Function:		~Disk
//
// Description:		Destructor for the Disk class.
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
Disk::~Disk()
{
}

//==========================================================================
// Class:			Disk
// Function:		GenerateGeometry
//
// Description:		Creates the OpenGL instructions to create this object in
//					the scene.
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
void Disk::GenerateGeometry(void)
{
	// Resolution must be at least 3
	if (resolution < 3)
		resolution = 3;

	// Our reference direction will be the X-axis direction
	Vector referenceDirection(1.0, 0.0, 0.0);

	// Determine the angle and axis of rotation
	Vector axisOfRotation = referenceDirection.Cross(normal);
	double angle = acos(normal * referenceDirection);// [rad]

	// Push the current matrix
	glPushMatrix();

		// Translate the current matrix
		glTranslated(center.x, center.y, center.z);

		// Rotate the current matrix, if the rotation axis is non-zero
		if (!VVASEMath::IsZero(axisOfRotation.Length()))
			glRotated(Convert::RAD_TO_DEG(angle), axisOfRotation.x, axisOfRotation.y, axisOfRotation.z);

		// Set the normal direction
		glNormal3d(normal.x, normal.y, normal.z);

		// We'll use a triangle strip to draw the disk
		glBegin(GL_TRIANGLE_STRIP);

		// Loop to generate the triangles
		Vector insidePoint(0.0, 0.0, 0.0);
		Vector outsidePoint(0.0, 0.0, 0.0);
		int i;
		for (i = 0; i <= resolution; i++)
		{
			// Determine the angle to the current set of points
			angle = (double)i * 2.0 * VVASEMath::Pi / (double)resolution;

			// Determine the Y and Z ordinates based on this angle and the radii
			outsidePoint.y = outerRadius * cos(angle);
			outsidePoint.z = outerRadius * sin(angle);

			insidePoint.y = innerRadius * cos(angle);
			insidePoint.z = innerRadius * sin(angle);

			// Add the next two points
			glVertex3d(outsidePoint.x, outsidePoint.y, outsidePoint.z);
			glVertex3d(insidePoint.x, insidePoint.y, insidePoint.z);
		}

		// Complete the triangle strip
		glEnd();

	// Pop the matrix
	glPopMatrix();
}

//==========================================================================
// Class:			Disk
// Function:		HasValidParameters
//
// Description:		Checks to see if the information about this object is
//					valid and complete (gives permission to create the object).
//
// Input Arguments:
//		None
//
// Output Arguments:
//		None
//
// Return Value:
//		bool, true for OK to draw, false otherwise
//
//==========================================================================
bool Disk::HasValidParameters(void)
{
	// Disks must have a positive
	if (outerRadius > 0.0 && !VVASEMath::IsZero(normal.Length()))
		return true;

	// Otherwise return false
	return false;
}

//==========================================================================
// Class:			Disk
// Function:		SetResolution
//
// Description:		Sets the number of faces use to approximate the disk.
//
// Input Arguments:
//		_resolution	= const int&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void Disk::SetResolution(const int &_resolution)
{
	// Set the resolution to the argument
	resolution = _resolution;
	
	// Reset the modified flag
	modified = true;
}

//==========================================================================
// Class:			Disk
// Function:		SetOuterRadius
//
// Description:		Sets the outer radius of the disk.
//
// Input Arguments:
//		_outerRadius	= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void Disk::SetOuterRadius(const double &_outerRadius)
{
	// Set the outer radius to the argument
	outerRadius = _outerRadius;
	
	// Reset the modified flag
	modified = true;
}

//==========================================================================
// Class:			Disk
// Function:		SetInnerRadius
//
// Description:		Sets the inner radius of the disk.
//
// Input Arguments:
//		_innerRadius	= const double&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void Disk::SetInnerRadius(const double &_innerRadius)
{
	// Set the inner radius to the argument
	innerRadius = _innerRadius;
	
	// Reset the modified flag
	modified = true;
}

//==========================================================================
// Class:			Disk
// Function:		SetCenter
//
// Description:		Sets the location of the center of the disk.
//
// Input Arguments:
//		_center	= const Vector&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void Disk::SetCenter(const Vector &_center)
{
	// Set the center point to the argument
	center = _center;
	
	// Reset the modified flag
	modified = true;
}

//==========================================================================
// Class:			Disk
// Function:		SetNormal
//
// Description:		Sets the disk's normal direction.
//
// Input Arguments:
//		_normal	= const Vector&
//
// Output Arguments:
//		None
//
// Return Value:
//		None
//
//==========================================================================
void Disk::SetNormal(const Vector &_normal)
{
	// Set the normal vector to the argument
	normal = _normal.Normalize();
	
	// Reset the modified flag
	modified = true;
}