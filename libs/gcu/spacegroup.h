// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * spacegroup.h - Handle Crystallographic Space Groups.
 *  
 * Copyright (C) 2007-2008 by Jean Bréfort
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
public:
	SpaceGroup();
	~SpaceGroup();

	void AddTransform(const std::string &s);

	std::list<Vector> Transform (Vector const &v) const;

	Transform3d const *GetFirstTransform (std::list <Transform3d*>::const_iterator &i) const;
	Transform3d const *GetNextTransform (std::list <Transform3d*>::const_iterator &i) const;

	// static methods
	/* The name might be either a HM or Hall name */
	static SpaceGroup const *GetSpaceGroup (char const *name);
	static SpaceGroup const *GetSpaceGroup (std::string const &name);
	static SpaceGroup const *GetSpaceGroup (unsigned id);
	static std::list <SpaceGroup const *> &GetSpaceGroups (unsigned id);
	static SpaceGroup const *Find (SpaceGroup* group);
	/* Use it if the space group is unknown (might happen if no database has
	been loaded or if the HM name is not usual. */
	void RegisterSpaceGroup (int nb = 0, ...);

	bool operator ==(const SpaceGroup &) const;
	int operator!=(const SpaceGroup &other) const
	{
	return !((*this) == other);
	}
	bool IsValid() const;
	unsigned GetTransformsNumber () const {return m_Transforms.size ();}

private:
	std::list<Transform3d*> m_Transforms;

GCU_PROP (std::string, HMName)
GCU_PROP (std::string, HallName)
GCU_PROP (unsigned, Id)
};

}

#endif // GCU_SPACE_GROUP_H

//! \file spacegroup.h
//! \brief Handle Crystallographic Space Groups
