// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * crystalviewer/crystalview.h 
 *
 * Copyright (C) 2002
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

#ifndef CRYSTAL_VIEW_H
#define CRYSTAL_VIEW_H

#include <libxml/parser.h>
#include <chemistry/matrix.h>
#include <list>
#include <gtk/gtkwidget.h>

using namespace std;

namespace gcu
{
class CrystalDoc;
	
class CrystalView
{
public:
	CrystalView(CrystalDoc* pDoc);
	virtual ~CrystalView();
	
	GtkWidget* CreateNewWidget();
	void Init(GtkWidget *widget);
	void Reshape(GtkWidget *widget);
	void Draw(GtkWidget *widget);
	void Update();
	void Update(GtkWidget *widget);
	bool OnPressed(GtkWidget *widget, GdkEventButton *event);
	void OnMotion(GtkWidget *widget, GdkEventMotion *event);
	void OnDestroyed(GtkWidget *widget);

	virtual bool Load(xmlNodePtr node);
	virtual xmlNodePtr Save(xmlDocPtr xml);

protected:
	void Rotate(gdouble x, gdouble y);

protected:
	gdouble m_lastx, m_lasty;
	gdouble m_fAngle, m_fRadius;
	gdouble m_psi, m_theta, m_phi;
	gdouble m_height, m_width, m_near, m_far;
	Matrix m_Euler;
	unsigned m_nGLList;
	bool m_bInit;
	//background color
	float m_fBlue, m_fRed, m_fGreen, m_fAlpha;
	CrystalDoc* m_pDoc;
	GtkWidget* m_pWidget;
	list<GtkWidget*> m_Widgets;
};

} //namespace gcu

#endif //CRYSTAL_VIEW_H
