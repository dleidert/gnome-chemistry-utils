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

static void
on_dialog_printer_setup_done (GtkPageSetup *page_setup,
			      gpointer data)
{
	((PrintSetupDlg *) data)->UpdatePageSetup (page_setup);
}

static void on_print (PrintSetupDlg *dlg)
{
	dlg->DoPrint (false);
}

static void on_print_preview (PrintSetupDlg *dlg)
{
	dlg->DoPrint (true);
}

static void on_dialog_gtk_printer_setup (PrintSetupDlg *dlg)
{
	dlg->OnPrinterSetup ();
}

static void on_orientation_changed (GtkToggleButton *btn, PrintSetupDlg *dlg)
{
	if (gtk_toggle_button_get_active (btn))
		dlg->OnOrientation ((GtkPageOrientation) GPOINTER_TO_INT (g_object_get_data (G_OBJECT (btn), "orientation")));
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
		w = glade_xml_get_widget (xml, "paper-btn");
		g_signal_connect_swapped (w, "clicked", G_CALLBACK (on_dialog_gtk_printer_setup), this);
		// set current page size
		m_PageTypeLbl = GTK_LABEL (glade_xml_get_widget (xml, "paper-type-lbl"));
		m_PageSizeLbl = GTK_LABEL (glade_xml_get_widget (xml, "paper-size-lbl"));
		m_PortraitBtn = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "portrait-btn"));
		g_object_set_data ((GObject*) m_PortraitBtn ,"orientation", GINT_TO_POINTER (GTK_PAGE_ORIENTATION_PORTRAIT));
		m_PortraitId = g_signal_connect ((GObject*) m_PortraitBtn, "clicked", G_CALLBACK (on_orientation_changed), this);
		m_LandscapeBtn = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "landscape-btn"));
		g_object_set_data ((GObject*) m_LandscapeBtn ,"orientation", GINT_TO_POINTER (GTK_PAGE_ORIENTATION_LANDSCAPE));
		m_LandscapeId = g_signal_connect ((GObject*) m_LandscapeBtn, "clicked", G_CALLBACK (on_orientation_changed), this);
		m_RPortraitBtn = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "r-portrait-btn"));
		g_object_set_data ((GObject*) m_RPortraitBtn ,"orientation", GINT_TO_POINTER (GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT));
		m_RPortraitId = g_signal_connect ((GObject*) m_RPortraitBtn, "clicked", G_CALLBACK (on_orientation_changed), this);
		m_RLandscapeBtn = GTK_TOGGLE_BUTTON (glade_xml_get_widget (xml, "r-landscape-btn"));
		g_object_set_data ((GObject*) m_RLandscapeBtn ,"orientation", GINT_TO_POINTER (GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE));
		m_RLandscapeId = g_signal_connect ((GObject*) m_RLandscapeBtn, "clicked", G_CALLBACK (on_orientation_changed), this);
		UpdatePageSetup (NULL);
		m_MarginTopBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "margin-top-btn"));
		m_MarginBottomBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "margin-bottom-btn"));
		m_MarginRightBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "margin-right-btn"));
		m_MarginLeftBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "margin-left-btn"));
		m_HeaderHeightBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "header-height-btn"));
		m_FooterHeightBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "header-height-btn"));
		if (printable->SupportsHeaders ()) {
		} else {
			// hide everything related to headers and footers
			// first delete the notebook page
			GtkNotebook *book = GTK_NOTEBOOK (glade_xml_get_widget (xml, "print-setup-book"));
			gtk_notebook_remove_page (book, 2);
			// now hide related buttons and labels
			gtk_spin_button_set_value (m_HeaderHeightBtn, 0.);
			gtk_widget_set_sensitive (glade_xml_get_widget (xml, "header-height-lbl"), false);
			gtk_widget_set_sensitive (GTK_WIDGET (m_HeaderHeightBtn), false);
			gtk_spin_button_set_value (m_FooterHeightBtn, 0.);
			gtk_widget_set_sensitive (glade_xml_get_widget (xml, "footer-height-lbl"), false);
			gtk_widget_set_sensitive (GTK_WIDGET (m_FooterHeightBtn), false);
			m_HeaderHeightId = m_FooterHeightId = 0;
		}
		gtk_widget_show_all (GTK_WIDGET (dialog));
}

PrintSetupDlg::~PrintSetupDlg ()
{
}

void PrintSetupDlg::DoPrint (bool preview)
{
	m_Printable->Print (preview);
}

void PrintSetupDlg::OnPrinterSetup ()
{
	gtk_print_run_page_setup_dialog_async
		(GetWindow (),
		 m_Printable->GetPageSetup (),
		 NULL,
		 on_dialog_printer_setup_done,
		 this);
}

#define TOGGLE_BUTTON(name) \
		g_signal_handler_block (G_OBJECT (m_##name##Btn), m_##name##Id); \
		gtk_toggle_button_set_active (m_##name##Btn, true); \
		g_signal_handler_unblock (G_OBJECT (m_##name##Btn), m_##name##Id); \
		break;

void PrintSetupDlg::UpdatePageSetup (GtkPageSetup *page_setup)
{
	if (page_setup)
		m_Printable->SetPageSetup (page_setup);
	gtk_label_set_text (m_PageTypeLbl, gtk_paper_size_get_display_name (gtk_page_setup_get_paper_size (m_Printable->GetPageSetup ())));
	switch (gtk_page_setup_get_orientation (m_Printable->GetPageSetup ())) {
	case GTK_PAGE_ORIENTATION_PORTRAIT:
		TOGGLE_BUTTON (Portrait)
	case GTK_PAGE_ORIENTATION_LANDSCAPE:
		TOGGLE_BUTTON (Landscape)
	case GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT:
		TOGGLE_BUTTON (RPortrait)
	case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
		TOGGLE_BUTTON (RLandscape)
	}
	
}

void PrintSetupDlg::OnOrientation (GtkPageOrientation orientation)
{
	if (gtk_page_setup_get_orientation (m_Printable->GetPageSetup ()) != orientation) {
		gtk_page_setup_set_orientation (m_Printable->GetPageSetup (), orientation);
		switch (orientation) {
	case GTK_PAGE_ORIENTATION_PORTRAIT:
		TOGGLE_BUTTON (Portrait)
	case GTK_PAGE_ORIENTATION_LANDSCAPE:
		TOGGLE_BUTTON (Landscape)
	case GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT:
		TOGGLE_BUTTON (RPortrait)
	case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
		TOGGLE_BUTTON (RLandscape)
		}
	}
}

}	//	namespace gcu
