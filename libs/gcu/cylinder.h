/**********************************************************************
  Cylinder - Class for drawing cylinders in OpenGL

  Copyright (C) 2007-2009 Jean Brefort <jean.brefort@normalesup.org>
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

#ifndef GCU_CYLINDER_H
#define GCU_CYLINDER_H

#include "vector.h"

/*!\file*/

namespace gcu {

  /**
   * \class Cylinder gcu/cylinder.h
   * @brief This class represents and draws a cylinder
   * @author Benoit Jacob <jacob@math.jussieu.fr>
   */
  class Vector;
  class CylinderPrivate;
  class Cylinder
  {
    private:
      void initialize ();
      void freeBuffers ();

    public:
      /** creates a cylinder with given number of faces. */
      Cylinder (int faces=0);
      ~Cylinder ();
      /** initializes the cylinder with given number of faces. If the
       * cylinder was already initialized, any pre-allocated buffers
       * are freed and then re-allocated */
		  void setup (int faces);
      /**
       * draws the cylinder at specified position, with specified
       * radius.
       @param end1 the position of the first end of the cylinder.
       that is, the center of the first disc-shaped face.
       @param end2 the position of the second end of the cylinder.
       that is, the center of the second disc-shaped face.
       @param radius the radius of the cylinder
       */
      void draw (const Vector &end1, const Vector &end2,
          double radius) const;
      /**
       * draws the cylinder at specified position, with specified
       * radius. the order and shift arguments allow to render
       * multiple cylinders at once, which is useful in libavogadro.
       * for multiple bonds between atoms. If you only want to render one
       * cylinder, leave order and shift at their default values.
       @param end1 the position of the first end of the cylinder.
       that is, the center of the first disc-shaped face.
       @param end2 the position of the second end of the cylinder.
       that is, the center of the second disc-shaped face.
       @param radius the radius of the cylinder
       @param order to render only one cylinder, leave this set to
       the default value, which is 1. If order>1, then order
       parallel cylinders are drawn around the axis
       (end1 - end2).
       @param shift this is only meaningful of order>1, otherwise
       just let this set to the default value. When order>1,
       this is interpreted as the displacement of the axis
       of the drawn cylinders from the axis (end1 - end2).
       @param planeNormalVector the unit normal vector of the plane
       in which we will try to fit the cylinders. This is useful
       to draw double bonds in a molecule in such a way that they
       avoid looking like single bonds. To achieve that, just pass
       the molecule's fitting plane's unit normal vector here.
       */
      void drawMulti (const Vector &end1, const Vector &end2,
          double radius, int order, double shift,
          const Vector &planeNormalVector) const;

    private:
      CylinderPrivate * const d;
  };

}

#endif	//	GCU_CYLINDER_H
