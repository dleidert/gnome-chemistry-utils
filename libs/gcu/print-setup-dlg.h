/* 
 * Gnome Chemistry Utils
 * print-setup-dlg.h
 *
 * Copyright (C) 2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_PRINT_SETUP_DLG_H
#define GCU_PRINT_SETUP_DLG_H

#include "dialog.h"
#include "printable.h"

namespace gcu {

class Application;

class PrintSetupDlg: public Dialog
{
public:
	PrintSetupDlg (Application* App, Printable *printable);
	virtual ~PrintSetupDlg ();

	void DoPrint (bool preview);
	void OnPrinterSetup ();
	void UpdatePageSetup (GtkPageSetup *page_setup);
	void OnOrientation (GtkPageOrientation orientation);
	bool SelectUnit (GtkTreeIter *iter);
	void OnUnitChanged ();
	void OnTopMarginChanged (double x);
	void OnBottomMarginChanged (double x);
	void OnRightMarginChanged (double x);
	void OnLeftMarginChanged (double x);
	void OnHeaderHeightChanged (double x);
	void OnFooterHeightChanged (double x);
	void OnHorizCenter ();
	void OnVertCenter ();

private:
	Printable *m_Printable;
	GtkLabel *m_PageSizeLbl, *m_PageTypeLbl;
	GtkToggleButton *m_PortraitBtn, *m_RPortraitBtn, *m_LandscapeBtn, *m_RLandscapeBtn;
	gulong m_PortraitId, m_RPortraitId, m_LandscapeId, m_RLandscapeId;
	GtkSpinButton *m_MarginTopBtn, *m_MarginBottomBtn, *m_MarginRightBtn, *m_MarginLeftBtn, *m_HeaderHeightBtn, *m_FooterHeightBtn;
	gulong m_MarginTopId, m_MarginBottomId, m_MarginRightId, m_MarginLeftId, m_HeaderHeightId, m_FooterHeightId;
	GtkListStore *m_UnitList;
	GtkComboBox *m_UnitBox;
	GtkToggleButton *m_HBtn, *m_VBtn;
	guint m_UnitId, m_HId, m_VId;
};
	
}	//	namespace gcu

#endif	//	GCU_PRINT_SETUP_DLG_H
