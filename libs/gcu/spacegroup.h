// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * spacegroup.h - Handle Crystallographic Space Groups.
 *
 * Copyright (C) 2007-2010 by Jean Bréfort
 *
 * This file was originally part of the Open Babel project.
 * For more information, see <http://openbabel.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef GCU_SPACE_GROUP_H
#define GCU_SPACE_GROUP_H

#include "macros.h"
#include <string>
#include <list>

namespace gcu
{

class Transform3d;
class Vector;

/*!\class SpaceGroup spacegroup.h <openbabel/math/spacegroup.h>
@brief Handle crystallographic space group symmetry
@since version 0.12
@sa transform3d
*/
class SpaceGroup
{
friend class SpaceGroupPrivate;
public:
/*!
Constructs a new empty, and then invalid, SpaceGroup.
*/
	SpaceGroup();
/*!
The desrtuctor. Destructs everything.
*/
	~SpaceGroup();

/*!
@param s a string representing a transformation.

Adds the transformation described by \a s to the group. \a s can follow either
the CIF convention (identity is "x,y,z") or the CML convention
("1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0 0.0 0.0 0.0 0.0 1.0")
*/
	void AddTransform(const std::string &s);

/*!
@param v a Vector.

Evaluates the list of the effect of each transform in the group on \a v and
eliminates duplicates.
@return the list of the images of \a v.
*/
	std::list<Vector> Transform (Vector const &v) const;

/*!
@param i an uninitialized iterator.

Initializes the iterator and retrieves the fist Transform3d in the group.
@return the first Transform3d.
*/
	Transform3d const *GetFirstTransform (std::list <Transform3d*>::const_iterator &i) const;
/*!
@param i an iterator, initialized by GetFirstTransform()

@return the next Transform3d.
*/
	Transform3d const *GetNextTransform (std::list <Transform3d*>::const_iterator &i) const;

	// static methods
	/* The name might be either a HM or Hall name */
/*!
@param name a group name.

Retrieves the group corresopnding to \a name, which can be in Hermann-Mauguin
or Hall convention with spaces between the groups such as
"P 63/m c m" or "-P 6c 2".
@return the found SpaceGroup if any.
*/
	static SpaceGroup const *GetSpaceGroup (char const *name);
/*!
@param name a group name.

Retrieves the group corresopnding to \a name, which can be in Hermann-Mauguin
or Hall convention with spaces between the groups such as
"P 63/m c m" or "-P 6c 2".
@return the found SpaceGroup if any.
*/
	static SpaceGroup const *GetSpaceGroup (std::string const &name);
/*!
@param id a space group conventional identifier (between 1 and 230).

@return a SpaceGroup corresponding to the identifier.
*/
	static SpaceGroup const *GetSpaceGroup (unsigned id);
/*!
@param id a space group conventional identifier (between 1 and 230).

@return a list of all group variants corresponding to the identifier.
*/
	static std::list <SpaceGroup const *> &GetSpaceGroups (unsigned id);
/*!
@param group a space group.

@return a space group from the database identical to \a group if any.
*/
	static SpaceGroup const *Find (SpaceGroup* group);
/*!
@param nb a number of names

Use it if the space group is unknown (might happen if no database has
been loaded or if the HM name is not usual). \a nb is the number of optional
alternative names known for the space group. It must be followed by these names.
*/
	void RegisterSpaceGroup (int nb = 0, ...);

/*!
@param group a SpaceGroup.

Equality operator.
@return true if the groups are identical, that is have the same list of
transformations.
*/
	bool operator ==(const SpaceGroup &group) const;
/*!
@param group a SpaceGroup.

Inequality operator.
@return true if the groups are different.
*/
	int operator!=(const SpaceGroup &group) const {return !((*this) == group);}
/*!
@return true if the group definition is valid.
*/
	bool IsValid() const;
/*!
@return the number of transformations in the group definition.
*/
	unsigned GetTransformsNumber () const {return m_Transforms.size ();}

private:
	std::list<Transform3d*> m_Transforms;

/*!\fn SetHMName(std::string name)
@param name the Hermann-Maugin name.

Sets the Hermann-Mauguin name for the group.
*/
/*!\fn GetHMName()
@return the Hermann-Mauguin name for the group.
*/
/*!\fn GetRefHMName()
@return the Hermann-Mauguin name for the group as a reference.
*/
GCU_PROP (std::string, HMName)
/*!\fn SetHallName(std::string name)
@param name the Hall name.

Sets the Hall name for the group.
*/
/*!\fn GetHallName()
@return the Hall name for the group.
*/
/*!\fn GetRefHallName()
@return the Hall name for the group as a reference.
*/
GCU_PROP (std::string, HallName)
/*!\fn SetId(unsigned id)
@param id a crystallographic group identifier.

Sets the crystallographic group identifier.
*/
/*!\fn GetId()
@return the crystallographic group identifier.
*/
/*!\fn GetRefId()
@return the crystallographic group identifier as a reference.
*/
GCU_PROP (unsigned, Id)
/*!\fn GetRefCoordinateAlternative()
Returns the coordinate system code for groups for which it is meaningful
or 0 for other groups as a reference.
*/
GCU_RO_PROP (unsigned, CoordinateAlternative)
};

}

#endif // GCU_SPACE_GROUP_H

//! \file spacegroup.h
//! \brief Handle Crystallographic Space Groups
