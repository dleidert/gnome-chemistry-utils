// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcu/glview.h
 *
 * Copyright (C) 2006-2012 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef GCU_GL_VIEW_H
#define GCU_GL_VIEW_H

#include "macros.h"
#include "matrix.h"
#include <goffice/goffice.h>
#include <map>
#include <string>
#include <stdexcept>

extern double DefaultPsi, DefaultTheta, DefaultPhi;

/*!\file*/
namespace gcu {

class GLDocument;

/*!
\class GLView gcu/glview.h
View class based on OpenGL for rendering. Used to display 3d chemical structures
such as molecules or crystals cells.
*/
class GLView
{
public:
//!Constructor.
/*!
@param pDoc: a pointer to the GLDocument instance.

Creates a view for the document.
*/
	GLView (GLDocument* pDoc) throw (std::runtime_error);
//!Destructor.
/*!
The destructor of GLView.
*/
	virtual ~GLView ();
/*!
Update the contents of the associated widget if any. This method must be called
each time the document or the view are modified. Default implementation doesn't do anything.
*/
	virtual void Update ();
/*!
@param psi the first Euler's angle.
@param theta the second Euler's angle.
@param phi the third Euler's angle.

Sets the orientation of the model, using the Euler's angles.
*/
	void SetRotation (double psi, double theta, double phi);
/*!
@param filename the name of the file.
@param type the type as supported by GdkPixbuf (e.g. "png" or "jpeg").
@param options the pairs of keys/values to pass GdkPixbuf.
@param width the width of the generated image.
@param height the height of the generated image.
@param use_bg whether to use the window background or a transparent background.

Export the view contents as an image. The size of the new image is defined by the width
and height parameters.
*/
	void SaveAsImage (std::string const &filename, char const *type, std::map<std::string, std::string>& options, unsigned width, unsigned height, bool use_bg) const;
/*!
@param width the width of the generated image.
@param height the height of the generated image.
@param use_bg whether to use the window background or a transparent background.

Generates a pixbuf from the current view.

@return the pixbuf containing the generated image
*/
	virtual GdkPixbuf *BuildPixbuf (unsigned width, unsigned height, bool use_bg) const;
/*!
@param cr a cairo_t.
@param width the width used for rendering.
@param height the height used for rendering.
@param use_bg whether to use the window background or a transparent background.

Outputs a bitmap to cairo. Used internally for printing and various image formats exports.
*/
	void RenderToCairo (cairo_t *cr, unsigned width, unsigned height, bool use_bg) const;

protected:
/*!
@param x the x component of the rotation.
@param y the y component of the rotation.
std::
Called by OnMotion(). x and y are the displacement coordinates of the mouse.
*/
	void Rotate (double x, double y);

/*!
Starts an OpenGL drawing operation. Will set the appropriate context.
This method is pure virtual and must be implemented in derived classes.
@return true if successful.
*/
	virtual bool GLBegin () = 0;

/*!
Ends an OpenGL drawing operation.
This method is pure virtual and must be implemented in derived classes.
*/
	virtual void GLEnd () = 0;

protected:
/*!
A Matrix reprenting the scene spatial orientation
*/
	Matrix m_Euler;
/*!
The mouse cursor last horizontal position. 
*/
	double m_Lastx;
/*!
The mouse cursor last vertical position. 
*/
	double m_Lasty;
/*!
The view height. 
*/
	int m_WindowHeight;
/*!
The view witdth.
*/
	int m_WindowWidth;
/*!
The scene height.
*/
	double m_Height;
/*!
The scene width. 
*/
	double m_Width;
/*!
The distance from the view point to the nearest scene point. 
*/
	double m_Near;
/*!
The distance from the view point to the farest scene point. 
*/
	double m_Far;

// Properties
/*!\fn SetAngle(double angle)
@param angle the new half field of view.
*/
/*!\fn GetAngle()
@return the current half field of view.
*/
/*!\fn GetRefAngle()
@return the current half field of view as a reference.
*/
GCU_PROP (double, Angle)
/*!\fn SetPsi(double psi)
@param psi the new psi Euler's angle.
*/
/*!\fn GetPsi()
@return the current psi Euler's angle.
*/
/*!\fn GetRefPsi()
@return the current psi Euler's angle as a reference.
*/
GCU_PROP (double, Psi)
/*!\fn SetPhi(double phi)
@param phi the new phi Euler's angle.
*/
/*!\fn GetPhi()
@return the current phi Euler's angle.
*/
/*!\fn GetRefPhi()
@return the current psh Euler's angle as a reference.
*/
GCU_PROP (double, Phi)
/*!\fn SetTheta(double theta)
@param theta the new theta Euler's angle.
*/
/*!\fn GetTheta()
@return the current theta Euler's angle.
*/
/*!\fn GetRefTheta()
@return the current theta Euler's angle as a reference.
*/
GCU_PROP (double, Theta)
/*!\fn SetRed(float red)
@param red the new red component for the background color.
*/
/*!\fn GetRed()
@return the current red component of the background color.
*/
/*!\fn GetRefRed()
@return the current red component of the background color as a reference.
*/
GCU_PROP (float, Red)
/*!\fn SetGreen(float green)
@param green the new green component for the background color.
*/
/*!\fn GetGreen()
@return the current green component of the background color.
*/
/*!\fn GetRefGreen()
@return the current green component of the background color as a reference.
*/
GCU_PROP (float, Green)
/*!\fn SetBlue(float blue)
@param blue the new blue component for the background color.
*/
/*!\fn GetBlue()
@return the current blue component of the background color.
*/
/*!\fn GetRefBlue()
@return the current blue component of the background color as a reference.
*/
GCU_PROP (float, Blue)
/*!\fn SetAlpha(float alpha)
@param alpha the new alpha value for the background.

The alpha value for the background is not supported in this version and this is only
a place holder.
*/
/*!\fn GetAlpha()
The alpha value for the background is not supported in this version and this is only
a place holder.
@return the current alpha value of the background color.
*/
/*!\fn GetRefAlpha()
The alpha value for the background is not supported in this version and this is only
a place holder.
@return the current alpha value of the background color as a reference.
*/
GCU_PROP (float, Alpha)
/*!\fn GetDoc()
@return the associated document.
*/
GCU_PROT_POINTER_PROP (GLDocument, Doc)
/*!\var m_Radius
The distance between the center of the model and the point of view.
*/
/*!\fn GetRadius()
@return the distance between the center of the model and the point of view.
*/
GCU_PROT_PROP (double, Radius);
};

}	// namespace gcu

#endif	//	GCU_GL_VIEW_H
