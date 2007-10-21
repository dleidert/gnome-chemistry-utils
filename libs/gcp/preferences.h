// -*- C++ -*-

/* 
 * GChemPaint library
 * preferences.h 
 *
 * Copyright (C) 2006-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef GCP_PREFERENCES_H
#define GCP_PREFERENCES_H

#include "fontsel.h"
#include <gcu/dialog.h>
#include <gcu/object.h>
#include <gcu/macros.h>
	
using namespace gcu;

namespace gcp {

class Application;
class Theme;

class PrefsDlg: public Dialog, Object
{
public:
	PrefsDlg (Application *pApp);
	virtual ~PrefsDlg ();

	void OnNewTheme ();
	void OnSelectTheme (GtkTreeSelection *selection);
	void OnBondLength (double length);
	void OnBondAngle (double angle);
	void OnBondWidth (double width);
	void OnBondDist (double dist);
	void OnStereoBondWidth (double width);
	void OnHashWidth (double width);
	void OnHashDist (double dist);
	void OnFont (GcpFontSel *fs);
	void OnTextFont (GcpFontSel *fs);
	void OnArrowLength (double length);
	void OnArrowWidth (double width);
	void OnArrowDist (double dist);
	void OnArrowPadding (double padding);
	void OnArrowHeadA (double headA);
	void OnArrowHeadB (double headB);
	void OnArrowHeadC (double headC);
	void OnScale (double scale);
	void OnPadding (double padding);
	void OnObjectPadding (double padding);
	void OnStoichPadding (double padding);
	void OnSignPadding (double padding);
	void OnChargeSize (double size);
	void OnThemeNameChanged (char const *name);
	bool CheckError ();
	void SetDefaultTheme (char const *name);

private:
	Theme *m_CurTheme;
	GtkTreeStore *themes;
	GtkTreeSelection *m_ThemesSelection;
	GtkTreeView *m_ThemesView;
	GtkNotebook *m_Book;
	GtkSpinButton *m_BondLengthBtn, *m_BondWidthBtn, *m_BondAngleBtn, *m_BondDistBtn;
	GtkSpinButton *m_StereoBondWidthBtn, *m_HashDistBtn, *m_HashWidthBtn;
	GtkSpinButton *m_ArrowLengthBtn, *m_ArrowWidthBtn, *m_ArrowDistBtn, *m_ArrowPaddingBtn;
	GtkSpinButton *m_ArrowHeadABtn, *m_ArrowHeadBBtn, *m_ArrowHeadCBtn;
	GtkSpinButton *m_ScaleBtn, *m_PaddingBtn, *m_ObjectPaddingBtn, *m_StoichPaddingBtn, *m_SignPaddingBtn;
	GtkSpinButton *m_ChargeSizeBtn;
	GtkEntry *m_NameEntry;
	GcpFontSel *m_TextFontSel, *m_FontSel;
	gulong m_NameActivate, m_NameFocusOut, m_TextFontChanged, m_FontChanged;
	GtkTreePath *m_Path;
	GtkComboBox *m_DefaultThemeBox;
};

}	//	namespace gcp

#endif	//	GCP_PREFERENCES_H
