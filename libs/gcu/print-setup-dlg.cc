/* 
 * Gnome Chemistry Utils
 * print-setup-dlg.cc
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
 *
 * Part of this code has been copied from gnumeric/src/dialogs/dialog-printer-setup.c
 * Authors:
 *  Wayne Schuller (k_wayne@linuxpower.org)
 *  Miguel de Icaza (miguel@gnu.org)
 *  Andreas J. Guelzow (aguelzow@pyrshep.ca)
 */

#include "config.h"
#include "print-setup-dlg.h"

namespace gcu {

static void on_print (PrintSetupDlg *dlg)
{
	dlg->DoPrint ();
}

static void on_print_preview (PrintSetupDlg *dlg)
{
	dlg->DoPrintPreview ();
}

PrintSetupDlg::PrintSetupDlg (Application* App, Printable *printable):
	Dialog (App, GLADEDIR"/print-setup.glade", "print-setup", printable),
	m_Printable (printable)
{
		GtkWidget *w;
		w = glade_xml_get_widget (xml, "print");
		g_signal_connect_swapped (w, "clicked", G_CALLBACK (on_print), this);
		w = glade_xml_get_widget (xml, "preview");
		g_signal_connect_swapped (w, "clicked", G_CALLBACK (on_print_preview), this);
		if (printable->SupportsHeaders ()) {
		} else {
			// hide everything related to headers and footers
			// first delete the notebook page
			GtkNotebook *book = GTK_NOTEBOOK (glade_xml_get_widget (xml, "print-setup-book"));
			gtk_notebook_remove_page (book, 2);
			// now hide related buttons and labels
			w = glade_xml_get_widget (xml, "header-height-btn");
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), 0);
			gtk_widget_set_sensitive (glade_xml_get_widget (xml, "header-height-lbl"), false);
			gtk_widget_set_sensitive (w, false);
			w = glade_xml_get_widget (xml, "footer-height-btn");
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), 0);
			gtk_widget_set_sensitive (glade_xml_get_widget (xml, "footer-height-lbl"), false);
			gtk_widget_set_sensitive (w, false);
		}
		gtk_widget_show_all (GTK_WIDGET (dialog));
}

PrintSetupDlg::~PrintSetupDlg ()
{
}

void PrintSetupDlg::DoPrint ()
{
}

void PrintSetupDlg::DoPrintPreview ()
{
}

}	//	namespace gcu
