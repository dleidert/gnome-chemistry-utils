// -*- C++ -*-

/* 
 * Gnome Chemisty Utils
 * crystalviewer/crystaldoc.h 
 *
 * Copyright (C) 2002-2003
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
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

#ifndef CRYSTAL_DOC_H
#define CRYSTAL_DOC_H

#include <libxml/tree.h>
#include <glib.h>
#include "crystalatom.h"
#include "crystalbond.h"
#include "crystalline.h"
#include "crystalcleavage.h"

namespace gcu
{

class CrystalView;

enum gcLattices {cubic=0,
				 body_centered_cubic,
				 face_centered_cubic,
				 hexagonal,
				 tetragonal,
				 body_centered_tetragonal,
				 orthorhombic,
				 base_centered_orthorhombic,
				 body_centered_orthorhombic,
				 face_centered_orthorhombic,
				 rhombohedral,
				 monoclinic,
				 base_centered_monoclinic,
				 triclinic};
				 
class CrystalDoc
{
public:
	CrystalDoc();
	virtual ~CrystalDoc();

	void ParseXMLTree(xmlNode* xml);
	void Update();
	CrystalView* GetView();
	bool IsDirty() {return m_bDirty;}
	virtual void SetDirty();
	void Draw();
	gdouble GetMaxDist() {return m_dDist;}
	virtual CrystalView* CreateNewView();
	virtual CrystalAtom* CreateNewAtom();
	virtual CrystalLine* CreateNewLine();
	virtual CrystalCleavage* CreateNewCleavage();
	xmlDocPtr BuildXMLTree();
	virtual const char* GetProgramId();
	
protected:
	void Init();
	void Reinit();
	void Duplicate(CrystalAtom& Atom);
	void Duplicate(CrystalLine& Line);
	virtual bool LoadNewView();

protected:
	bool m_bMultiView;
	gcLattices m_lattice;
	gdouble m_a, m_b, m_c, m_alpha, m_beta, m_gamma;
	gdouble m_xmin, m_ymin, m_zmin, m_xmax, m_ymax, m_zmax;
	gdouble m_dDist; //maximum distance between an object and the center
	gboolean m_bFixedSize;  //true if cleavages must not change positions in the view
	CrystalAtomList AtomDef, Atoms;
	CrystalLineList LineDef, Lines;
	CrystalCleavageList Cleavages;
	list <CrystalView *> m_Views;
	bool m_bDirty, m_bEmpty;
};
	
} //namespace gcu

#endif //CRYSTAL_DOC_H
