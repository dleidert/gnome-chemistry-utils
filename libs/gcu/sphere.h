/**********************************************************************
  Sphere - Class for drawing spheres in OpenGL

  Copyright (C) 2007-2008 Jean Brefort <jean.brefort@normalesup.org>
  Copyright (C) 2006,2007 Benoit Jacob <jacob@math.jussieu.fr>

  This file is part of the Avogadro molecular editor project.
  For more information, see <http://avogadro.sourceforge.net/>

  Avogadro is free software; you can redistribute it and/or modify 
  it under the terms of the GNU General Public License as published by 
  the Free Software Foundation; either version 2 of the License, or 
  (at your option) any later version.

  Avogadro is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.
 **********************************************************************/

#ifndef GCU_SPHERE_H
#define GCU_SPHERE_H

namespace OpenBabel {
	class vector3;
}

/*!\file*/
namespace gcu {

/** \class Sphere gcu/sphere.h
* This class represents and draws a sphere. The level of detail can be controlled.
* At level 0, the sphere is a octahedron. At levels >=1, the sphere is a
* "geosphere", that is, one starts with an icosahedron, which is the regular
* solid with 20 triangular faces, and one then sub-tesselates each face into
* smaller triangles. This is a classical tesselation, known to give a very good
* quality/complexity ratio.
*
* \author Benoit Jacob <jacob@math.jussieu.fr>
*/

class SpherePrivate;

class Sphere
{
private:
	SpherePrivate * const d;

protected:

	/** computes the index (position inside the index buffer)
	* of a vertex given by its position (strip, column, row)
	* inside a certain flat model of the sub-tesselated
	* icosahedron */
	inline unsigned short indexOfVertex (int strip, int column, int row);

	/** computes the coordinates
	* of a vertex given by its position (strip, column, row)
	* inside a certain flat model of the sub-tesselated
	* icosahedron */
	void computeVertex (int strip, int column, int row);

	void freeBuffers ();
	void initialize ();

public:
	Sphere (int detail = 0);
	~Sphere ();

	/** initializes the sphere with given level of detail. If the
	* sphere was already initialized, any pre-allocated buffers
	* are freed and then re-allocated.
	@param detail the wanted level of detail. See m_detail member */
	void setup (int detail);

	/** draws the sphere at specified position and with
	* specified radius */
	void draw (OpenBabel::vector3 const &center, double radius) const;
};

}

#endif	//	GCU_SPHERE_H
