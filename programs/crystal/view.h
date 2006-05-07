// -*- C++ -*-

/* 
 * Gnome Crystal
 * view.h 
 *
 * Copyright (C) 2000-2006 Jean Br�fort <jean.brefort@normalesup.org>
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

#ifndef GCRYSTAL_VIEW_H
#define GCRYSTAL_VIEW_H

#include <libxml/parser.h>
#include <libgnomeprint/gnome-print.h>
#include <list>
#include <gcu/crystalview.h>
#include <gcu/matrix.h>
#include <gcu/dialog.h>

class gcDocument;
class gcApplication;
class gcWindow;

using namespace gcu;

class gcView: public CrystalView
{
public:
	gcView (gcDocument *pDoc);
	gcView (gcView *pView);
	~gcView ();
	
	void SetDocument (gcDocument *pDoc);
	gcDocument* GetDocument() {return (gcDocument *) m_pDoc;}
	void SetBackgroundColor (float red, float green, float blue, float alpha);
	void GetBackgroundColor (double *red, double *green, double *blue, double *alpha);
	gdouble& GetFoV () {return m_fAngle;}
	gdouble& GetPos () {return m_fRadius;}
	void GetRotation (double *psi, double *theta, double *phi);
	void SetRotation (double psi, double theta, double phi);
	bool LoadOld (xmlNodePtr node);
	void Print (GnomePrintContext *pc, gdouble width, gdouble height);
	void SetLabel (GtkLabel* Label) {m_pLabel = Label;}
	void SetMenu (GtkMenuItem* item);
	GtkLabel* GetLabel () {return m_pLabel;}
	GtkLabel* GetMenuLabel () {return m_pMenuLabel;}
	void NotifyDialog (Dialog* dialog);
	void RemoveDialog (Dialog* dialog);
	void Lock () {m_bLocked = true;}
	void Unlock () {m_bLocked = false;}
	bool IsLocked () {return m_bLocked;}
	gcWindow *GetWindow () {return m_Window;}
	void SetWindow (gcWindow *window) {m_Window = window;}
	
private:

	GtkLabel *m_pLabel, *m_pMenuLabel;
	GtkMenuItem* m_pMenu;
	bool m_bLocked;
	std::list <Dialog *> m_Dialogs;
	gcWindow *m_Window;
};

#endif //GCRYSTAL_VIEW_H
