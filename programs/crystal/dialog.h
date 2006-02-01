// -*- C++ -*-

/* 
 * Gnome Crystal
 * dialog.h 
 *
 * Copyright (C) 2001-2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCRYSTAL_DIALOG_H
#define GCRYSTAL_DIALOG_H
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <glade/glade.h>

enum gcCheckType
{
	gccNoCheck,
	gccMin,
	gccMax,
	gccMinMax,
	gccMinEq,
	gccMaxEq,
	gccMinEqMax,
	gccMinMaxEq,
	gccMinEqMaxEq
};

class gcDialog
{
public:
	gcDialog(const char* filename, const char* windowname);
	virtual ~gcDialog();

	virtual void Destroy();
	virtual bool Apply();
	virtual void Help();

protected:
	bool GetNumber(GtkEntry *Entry, double *x, gcCheckType c = gccNoCheck, double min = 0, double max = 0);

	GladeXML* xml;
	char m_buf[64];
	const gchar *m_WindowName;
	GtkWindow *dialog;
	GtkNotebook *notebook;
};

#endif //GCRYSTAL_DIALOG_H
