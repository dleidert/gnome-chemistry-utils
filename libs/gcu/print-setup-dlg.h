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
#include <gtk/gtkpagesetup.h>
#include <gtk/gtkprintsettings.h>

namespace gcu {

class Application;

class PrintSetupDlg: public Dialog
{
public:
	PrintSetupDlg (Application* App, Printable *printable);
	virtual ~PrintSetupDlg ();

	void DoPrint ();
	void DoPrintPreview ();

private:
	Printable *m_Printable;
};
	
}	//	namespace gcu

#endif	//	GCU_PRINT_SETUP_DLG_H
