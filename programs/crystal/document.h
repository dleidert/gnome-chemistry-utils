// -*- C++ -*-

/* 
 * Gnome Crystal
 * document.h 
 *
 * Copyright (C) 2000-2006 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef GCRYSTAL_DOCUMENT_H
#define GCRYSTAL_DOCUMENT_H

#include <libxml/parser.h>
#include <gcu/crystaldoc.h>
#include <gcu/dialog.h>
#include "atom.h"
#include "line.h"
#include "cleavage.h"
#include <libbonobo.h>

using namespace gcu;

class gcView;
class gcApplication;

class gcDocument: public CrystalDoc
{
	//Constructor and destructor
public:
	gcDocument (gcApplication *App);
	~gcDocument();
	
	//Interface
public:
	void Define(unsigned nPage = 0);
	CrystalAtomList* GetAtomList() {return &AtomDef;}
	CrystalLineList* GetLineList() {return &LineDef;}
	CrystalCleavageList* GetCleavageList() {return &Cleavages;}
	void Update();
	void UpdateAllViews();
	void SetWidget(GtkWidget* widget){m_widget = widget;}
	gdouble GetMaxDist() {return m_dDist;}
	void GetSize(gdouble* xmin, gdouble* xmax, gdouble* ymin, gdouble* ymax, gdouble* zmin, gdouble* zmax);
	void SetSize(gdouble xmin, gdouble xmax, gdouble ymin, gdouble ymax, gdouble zmin, gdouble zmax);
	void GetCell(gcLattices *lattice, gdouble *a, gdouble *b, gdouble *c, gdouble *alpha, gdouble *beta, gdouble *gamma);
	void SetCell(gcLattices lattice, gdouble a, gdouble b, gdouble c, gdouble alpha, gdouble beta, gdouble gamma);
	const gchar* GetFileName() {return m_filename;}
	void SetFileName(const gchar* filename);
	gchar* GetTitle() {return m_title;}
	void SetTitle(const gchar* title);
	void Save();
	bool Load(const gchar* filename);
	void ParseXMLTree(xmlNode* xml);
	void OnNewDocument();
	void OnExportVRML(const gchar* FileName, gcView* pView);
	gcView* GetNewView();
	void SetDirty();
	bool IsEmpty() {return m_bEmpty;}
	void AddView(gcView* pView);
	bool RemoveView(gcView* pView);
	bool VerifySaved();
	void SetBonoboPersist(BonoboPersist* ps) {m_ps = ps;}
	void NotifyDialog(Dialog* dialog);
	void RemoveDialog(Dialog* dialog);
	bool GetFixedSize() {return m_bFixedSize;}
	void SetFixedSize(bool FixedSize) {m_bFixedSize = FixedSize;}
	virtual CrystalView* CreateNewView();
	virtual CrystalAtom* CreateNewAtom();
	virtual CrystalLine* CreateNewLine();
	virtual CrystalCleavage* CreateNewCleavage();
	virtual const char* GetProgramId();
	gcApplication* GetApplication () {return m_App;}

private:
	void Error(int num);
	
	//Implementation
private:
	gchar *m_filename, *m_title;
	bool m_bClosing;
	GtkWidget* m_widget;
	std::list <Dialog *> m_Dialogs;
	BonoboPersist* m_ps;
	gcApplication* m_App;
};

#endif //GCRYSTAL_DOCUMENT_H
