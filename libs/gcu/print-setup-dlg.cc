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
#include <glib/gi18n-lib.h>

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

static gint unit_sort_func (GtkTreeModel *model,
		GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
	char *str_a;
	char *str_b;
	gint result;

	gtk_tree_model_get (model, a, 0, &str_a, -1);
	gtk_tree_model_get (model, b, 0, &str_b, -1);
	
	result = g_utf8_collate (str_a, str_b);

	g_free (str_a);
	g_free (str_b);
	return result;
}

static bool select_unit (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, PrintSetupDlg *dlg)
{
	return dlg->SelectUnit(iter);
}

static void on_unit_changed (PrintSetupDlg *dlg)
{
	dlg->OnUnitChanged ();
}

static void on_top_margin_changed (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	dlg->OnTopMarginChanged (gtk_spin_button_get_value (btn));
}

static void on_bottom_margin_changed (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	dlg->OnBottomMarginChanged (gtk_spin_button_get_value (btn));
}

static void on_right_margin_changed (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	dlg->OnRightMarginChanged (gtk_spin_button_get_value (btn));
}

static void on_left_margin_changed (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	dlg->OnLeftMarginChanged (gtk_spin_button_get_value (btn));
}

static void on_header_height_changed (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	dlg->OnHeaderHeightChanged (gtk_spin_button_get_value (btn));
}

static void on_footer_height_changed (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	dlg->OnFooterHeightChanged (gtk_spin_button_get_value (btn));
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
		m_UnitList = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
		GtkTreeIter iter;
		for (int i = 1; i < 4; i++) {
			gtk_list_store_append (m_UnitList, &iter);
			gtk_list_store_set (m_UnitList, &iter,
								0, _(gtk_unit_to_string ((GtkUnit) i)),
								1, i,
								-1);
		}
		gtk_tree_sortable_set_default_sort_func
			(GTK_TREE_SORTABLE (m_UnitList),
			 unit_sort_func, NULL, NULL);
		gtk_tree_sortable_set_sort_column_id
			(GTK_TREE_SORTABLE (m_UnitList),
			 GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
			 GTK_SORT_ASCENDING);
		m_UnitBox = GTK_COMBO_BOX (gtk_combo_box_new_with_model (GTK_TREE_MODEL (m_UnitList)));
		m_UnitId = g_signal_connect_swapped (m_UnitBox, "changed", G_CALLBACK (on_unit_changed), this);
		GtkCellRenderer *text_renderer = gtk_cell_renderer_text_new ();
		gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(m_UnitBox), text_renderer, TRUE);
		gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(m_UnitBox), text_renderer, "text", 0);  
		gtk_table_attach (GTK_TABLE (glade_xml_get_widget (xml, "paper-selector-tbl")),
						 GTK_WIDGET (m_UnitBox), 3, 4, 8, 9, GTK_FILL, (GtkAttachOptions) 0, 0, 0);
		m_MarginTopBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "top-margin-btn"));
		m_MarginTopId = g_signal_connect ((GObject*) m_MarginTopBtn, "value-changed", G_CALLBACK (on_top_margin_changed), this);
		m_MarginBottomBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "bottom-margin-btn"));
		m_MarginBottomId = g_signal_connect ((GObject*) m_MarginBottomBtn, "value-changed", G_CALLBACK (on_bottom_margin_changed), this);
		m_MarginRightBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "right-margin-btn"));
		m_MarginRightId = g_signal_connect ((GObject*) m_MarginRightBtn, "value-changed", G_CALLBACK (on_right_margin_changed), this);
		m_MarginLeftBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "left-margin-btn"));
		m_MarginLeftId = g_signal_connect ((GObject*) m_MarginLeftBtn, "value-changed", G_CALLBACK (on_left_margin_changed), this);
		m_HeaderHeightBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "header-height-btn"));
		m_FooterHeightBtn = GTK_SPIN_BUTTON (glade_xml_get_widget (xml, "header-height-btn"));
		UpdatePageSetup (NULL);
		if (printable->SupportsHeaders ()) {
			m_HeaderHeightId = g_signal_connect ((GObject*) m_HeaderHeightBtn, "value-changed", G_CALLBACK (on_header_height_changed), this);
			m_FooterHeightId = g_signal_connect ((GObject*) m_FooterHeightBtn, "value-changed", G_CALLBACK (on_footer_height_changed), this);
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

#define SET_SPIN_BUTTON_VALUE(name,x) \
	g_signal_handler_block (G_OBJECT (m_##name##Btn), m_##name##Id); \
	gtk_spin_button_set_value (m_##name##Btn, x); \
	g_signal_handler_unblock (G_OBJECT (m_##name##Btn), m_##name##Id); \
	switch (m_Printable->GetUnit ()) { \
	case GTK_UNIT_MM: \
		gtk_spin_button_set_digits (m_##name##Btn, 1); \
		gtk_spin_button_set_increments (m_##name##Btn, 1., 0.); \
		break; \
	case GTK_UNIT_INCH: \
		gtk_spin_button_set_digits (m_##name##Btn, 3); \
		gtk_spin_button_set_increments (m_##name##Btn, 0.125, 0.); \
		break; \
	case GTK_UNIT_POINTS: \
		gtk_spin_button_set_digits (m_##name##Btn, 1); \
		gtk_spin_button_set_increments (m_##name##Btn, 1., 0.); \
		break; \
	case GTK_UNIT_PIXEL: \
		break; \
	}

void PrintSetupDlg::UpdatePageSetup (GtkPageSetup *page_setup)
{
	if (page_setup)
		m_Printable->SetPageSetup (page_setup);
	GtkPaperSize *size = gtk_page_setup_get_paper_size (m_Printable->GetPageSetup ());
	gtk_label_set_text (m_PageTypeLbl, gtk_paper_size_get_display_name (size));
	char const *unit = _(gtk_unit_to_string (m_Printable->GetUnit ()));
	char *buf = g_strdup_printf (((m_Printable->GetUnit () == GTK_UNIT_INCH)? _("%.1f %s wide by %.1f %s tall"): _("%.0f %s wide by %.0f %s tall")),
								 gtk_paper_size_get_width (size, m_Printable->GetUnit ()), unit,
								 gtk_paper_size_get_height (size, m_Printable->GetUnit ()), unit);
	gtk_label_set_text (m_PageSizeLbl, buf);
	g_free (buf);
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
	g_signal_handler_block (G_OBJECT (m_UnitBox), m_UnitId);
	gtk_tree_model_foreach (GTK_TREE_MODEL (m_UnitList), (GtkTreeModelForeachFunc) select_unit, this);	
	g_signal_handler_unblock (G_OBJECT (m_UnitBox), m_UnitId);
	SET_SPIN_BUTTON_VALUE (MarginTop, gtk_page_setup_get_top_margin (m_Printable->GetPageSetup (), m_Printable->GetUnit ()));
	SET_SPIN_BUTTON_VALUE (MarginBottom, gtk_page_setup_get_bottom_margin (m_Printable->GetPageSetup (), m_Printable->GetUnit ()));
	SET_SPIN_BUTTON_VALUE (MarginRight, gtk_page_setup_get_right_margin (m_Printable->GetPageSetup (), m_Printable->GetUnit ()));
	SET_SPIN_BUTTON_VALUE (MarginLeft, gtk_page_setup_get_left_margin (m_Printable->GetPageSetup (), m_Printable->GetUnit ()));
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

bool PrintSetupDlg::SelectUnit (GtkTreeIter *iter)
{
	GtkUnit unit;
	gtk_tree_model_get (GTK_TREE_MODEL (m_UnitList), iter, 1, &unit, -1);
	if (unit == m_Printable->GetUnit ()) {
		gtk_combo_box_set_active_iter (m_UnitBox, iter);
		return true;
	}
	return false;
}

void PrintSetupDlg::OnUnitChanged ()
{
	GtkTreeIter iter;
	if (gtk_combo_box_get_active_iter (m_UnitBox, &iter)) {
		GtkUnit unit;
		gtk_tree_model_get (GTK_TREE_MODEL (m_UnitList), &iter, 1, &unit, -1);
		m_Printable->SetUnit (unit);
		UpdatePageSetup (NULL); // use the new unit for display
	}
}

void PrintSetupDlg::OnTopMarginChanged (double x)
{
	gtk_page_setup_set_top_margin (m_Printable->GetPageSetup (), x, m_Printable->GetUnit ());
}

void PrintSetupDlg::OnBottomMarginChanged (double x)
{
	gtk_page_setup_set_bottom_margin (m_Printable->GetPageSetup (), x, m_Printable->GetUnit ());
}

void PrintSetupDlg::OnRightMarginChanged (double x)
{
	gtk_page_setup_set_right_margin (m_Printable->GetPageSetup (), x, m_Printable->GetUnit ());
}

void PrintSetupDlg::OnLeftMarginChanged (double x)
{
	gtk_page_setup_set_left_margin (m_Printable->GetPageSetup (), x, m_Printable->GetUnit ());
}

void PrintSetupDlg::OnHeaderHeightChanged (double x)
{
}

void PrintSetupDlg::OnFooterHeightChanged (double x)
{
}

}	//	namespace gcu
