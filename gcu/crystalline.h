// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * crystalline.h 
 *
 * Copyright (C) 2002-2004
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#ifndef CRYSTAL_LINE_H
#define CRYSTAL_LINE_H

#include <libxml/parser.h>
#include <list>

using namespace std;

namespace gcu
{

/*!\enum CrystalLineType crystalviewer/crystalline.h
The type of a CrystalLine instance. Possible values are:
- edges: all the cell edges.
- diagonals: the lines joining opposite vertices of a cell.
- medians: the lines joining the centers of opposite faces of a cell.
- normal: a line with defined ends, repeated in each displayes cell.
- unique: a line with defined ends, not repeated.
*/	
enum CrystalLineType
{
	edges = 0,
	diagonals,
	medians,
	normal,
	unique
};

/*!\class CrystalLine gcu/crystalline.h
Describes lines represented as cylinders in the view. Thes cylinders are not capped.
*/
class CrystalLine
{
public:
/*!
The default constructor of CrystalLine.
*/
	CrystalLine ();
/*!
The destructior of CrystaLine
*/
	virtual ~CrystalLine ();

public :
/*!
@param Type: the type (CrystalLineType) of the new line.
@param X1: the x coordinate of the first end of the new line.
@param Y1: the y coordinate of the first end of the new line.
@param Z1: the z coordinate of the first end of the new line.
@param X2: the x coordinate of the second end of the new line.
@param Y2: the y coordinate of the second end of the new line.
@param Z2: the z coordinate of the second end of the new line.
@param r: the radius of the cylinder which will represent the new line.
@param red: the red component of the cylinder which will represent the new line.
@param green: the green component of the cylinder which will represent the new line.
@param blue: the blue component of the cylinder which will represent the new line.
@param alpha: the alpha component of the cylinder which will represent the new line.

Constructs a new line from its characterisitics.
*/
	CrystalLine (CrystalLineType Type, double X1, double Y1, double Z1, double X2, double Y2, double Z2, double r, float red, float green, float blue, float alpha);
/*!
@param clLine: the line to duplicate.

Creates a new line with the same characteristics as clLine.
*/
	CrystalLine (CrystalLine& clLine);
/*!
@param clLine: the line to copy.

Copies a line.
@return the copied line.
*/
	CrystalLine& operator= (CrystalLine& clLine);

/*!
Draws the line inside the active OpenGL window.
*/
	void Draw ();
/*!
@return the x coordinate of the first end of the line.
*/
	double X1 (void) {return m_dx;}
/*!
@return the y coordinate of the first end of the line.
*/
	double Y1 (void) {return m_dy;}
/*!
@return the z coordinate of the first end of the line.
*/
	double Z1 (void) {return m_dz;}
/*!
@return the x coordinate of the second end of the line.
*/
	double X2 (void) {return m_dx2;}
/*!
@return the y coordinate of the second end of the line.
*/
	double Y2 (void) {return m_dy2;}
/*!
@return the z coordinate of the second end of the line.
*/
	double Z2 (void) {return m_dz2;}
/*!
@return the greatest x coordinate of the line.
*/
	double Xmax ();
/*!
@return the greatest y coordinate of the line.
*/
	double Ymax ();
/*!
@return the greatest z coordinate of the line.
*/
	double Zmax ();
/*!
@return the lowest x coordinate of the line.
*/
	double Xmin ();
/*!
@return the lowest y coordinate of the line.
*/
	double Ymin ();
/*!
@return the lowest z coordinate of the line.
*/
	double Zmin ();
/*!
@return the length of the line.
*/
	double Long () {return m_dl;}
/*!
@return the type of the line (see CrystalLineType).
*/
	CrystalLineType Type () {return m_nType;}
/*!
@param x: the new x coordinate of the first end of the new line.
@param y: the new y coordinate of the first end of the new line.
@param z: the new z coordinate of the first end of the new line.
@param x1: the new x coordinate of the second end of the new line.
@param y1: the new y coordinate of the second end of the new line.
@param z1: the new z coordinate of the second end of the new line.

Moves a line to a new position.
*/
	void SetPosition (double x, double y, double z, double x1, double y1, double z1);
/*!
@param red: the red component of the new color of the line.
@param green: the green component of the new color of the line.
@param blue: the blue component of the new color of the line.
@param alpha: the alpha component of the new color of the line.

Changes the color used to display the line.
*/
	void SetColor (float red, float green, float blue, float alpha);
/*!
@param red: a pointer to the location to which the red component of the color of the line will be copied.
@param green: a pointer to the location to which the green component of the new color of the line will be copied.
@param blue: a pointer to the location to which the blue component of the new color of the line will be copied.
@param alpha: a pointer to the location to which the alpha component of the new color of the line will be copied.

Gets the components of the color used to display the line.
*/
	void GetColor (double *red, double *green, double *blue, double *alpha);
/*!
@param r: the new radius of the cylinder representing the line.

Changes the radius of the cylinder used to represent the line.
*/
	void SetRadius (double r);
/*!
@return the radius of the cylinder used to represent the line.
*/
	double GetRadius () {return m_dr;};
/*!
@param clLine: a CrystalLine instance.
@return true if the two lines have the same type and the same position.
*/
	bool operator== (CrystalLine& clLine);
/*!
@param x: the x component of the transation vector.
@param y: the y component of the transation vector.
@param z: the z component of the transation vector.

Used to move a line.
*/
	virtual void Move (double x, double y, double z);
/*!
@param h: the h Miller index of a plane.
@param k: the k Miller index of a plane.
@param l: the l Miller index of a plane.

@return the product hx+ky+lz where x, y and z are the coordinates of one of the ends the line.
The end giving the largest value is retained for the calculus.
This makes sense only if coordinates are related to the net and are not the cartesian coordinates.
This method should not be called after NetToCartesian().
*/
	double ScalProd (int h, int k, int l);
/*!
Method used to cleave a line. The inverse operation does not exist since the whole crystal must be recalculated
after a change in the definition.
*/
	void Cleave () {m_nCleave++;}
/*!
@param a: the a parameter of the unit cell.
@param b: the b parameter of the unit cell.
@param c: the c parameter of the unit cell.
@param alpha: the alpha angle of the unit cell.
@param beta: the beta angle of the unit cell.
@param gamma: the gamma angle of the unit cell.

Converts the coordinates of the line from net related ones to cartesian. Initially, lines are defined by their
position relative to the unit cell and the coordinates must be transformed to the cartesian ones before
displaying the line.
*/
	void NetToCartesian (double a, double b, double c, double alpha, double beta, double gamma);
/*!
@param x: the x coordinate of the center.
@param y: the y coordinate of the center.
@param z: the z coordinate of the center.
@param bFixed: tells if cleaved lines are taken into account.

This helper method is called when searching for the size of the crystal. When some cleavages are defined,
the procedure cn take into account lines cleaved to get the same position in the view for the cleaved crystal
than for the whole crystal. If bFixed is true, all lines are taken into account.

@return the largest distance of the line to the center of the view or 0 if bFixed is false and the line cleaved. 
*/
	double Distance (double x, double y, double z, bool bFixed);
/*!
@return true if the line is cleaved by at least one cleavage or false if the line is not cleaved at all.
*/
	bool IsCleaved () {return m_nCleave != 0;}
/*!
@param x: the x component of the vector of the rotation axis.
@param y: the y component of the vector of the rotation axis.
@param z: the z component of the vector of the rotation axis.
@param th: the angle of the rotation.

This helper method is used to get the orientation of the line relative to the z axis. It is used when exporting to the
VRML format.
*/
	void GetRotation (double& x, double& y, double& z, double& th);
/*!
@param xml: the xmlDoc used to save the document.

Saves the line.
@return the xmlnode containing the description of the line.
*/
	virtual xmlNodePtr Save (xmlDocPtr xml);
/*!
@param node: a pointer to the xmlNode containing the serialized line.

Loads a line from the XML document.
*/
	virtual bool Load (xmlNodePtr node);
	
protected :
/*!
The blue component of the color of the cylinder representing the line.
*/
	float m_fBlue;
/*!
The red component of the color of the cylinder representing the line.
*/
	float m_fRed;
/*!
The green component of the color of the cylinder representing the line.
*/
	float m_fGreen;
/*!
The alpha component of the color of the cylinder representing the line.
*/
	float m_fAlpha;
/*!
The x coordinate of the first end of the line.
*/
	double m_dx;
/*!
The y coordinate of the first end of the line.
*/
	double m_dy;
/*!
The z coordinate of the first end of the line.
*/
	double m_dz;
/*!
The x coordinate of the second end of the line.
*/
	double m_dx2;
/*!
The x coordinate of the second end of the line.
*/
	double m_dy2;
/*!
The x coordinate of the second end of the line.
*/
	double m_dz2;
/*!
The radius of the cylinder representing the line.
*/
	double m_dr;
/*!
When cleavages (see CrystalCleavage class documentation) are defined, the line might be cleaved. m_nCleave is
the number of CrystalCleavage instances which remove the line. If this member is not 0, the line will
not be displayed.
*/
	int m_nCleave; //0 if not cleaved
/*!
The type of the CrystalLine instance. Possible values are:
- edges: all the cell edges.
- diagonals: the lines joining opposite vertices of a cell.
- medians: the lines joining the centers of opposite faces of a cell.
- normal: a line with defined ends, repeated in each displayes cell.
- unique: a line with defined ends, not repeated.
*/
	CrystalLineType m_nType;

private:
	double m_dl;
	double m_dxrot;
	double m_dyrot;
	double m_darot;//rotation axis coordinates (z = 0) and angle
};

/*!\class CrystalLineList
a list of pointers to CrystalLine instances derived from std::list.
*/
typedef list<CrystalLine*> CrystalLineList;

}// namespace gcu

#endif // CRYSTAL_BOND_H
