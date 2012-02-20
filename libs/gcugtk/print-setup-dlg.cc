/*
 * Gnome Chemistry Utils
 * print-setup-dlg.cc
 *
 * Copyright (C) 2008-2012 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include <glib/gi18n-lib.h>

namespace gcugtk {

class PrintSetupDlgPrivate {
public:
	static void DoPrint (PrintSetupDlg *dlg, bool preview);
	static void OnPrint (PrintSetupDlg *dlg) {DoPrint (dlg, false);}
	static void OnPrintPreview (PrintSetupDlg *dlg) {DoPrint (dlg, true);}
	static void OnPrinterSetup (PrintSetupDlg *dlg);
	static void UpdatePageSetup (GtkPageSetup *page_setup, PrintSetupDlg *dlg);
	static void OnOrientation (GtkToggleButton *btn, PrintSetupDlg *dlg);
	static bool SelectUnit (GtkTreeModel *, GtkTreePath *, GtkTreeIter *iter, PrintSetupDlg *dlg);
	static void OnUnitChanged (PrintSetupDlg *dlg);
	static void OnTopMarginChanged (GtkSpinButton *btn, PrintSetupDlg *dlg);
	static void OnBottomMarginChanged (GtkSpinButton *btn, PrintSetupDlg *dlg);
	static void OnRightMarginChanged (GtkSpinButton *btn, PrintSetupDlg *dlg);
	static void OnLeftMarginChanged (GtkSpinButton *btn, PrintSetupDlg *dlg);
	static void OnHeaderHeightChanged (GtkSpinButton *btn, PrintSetupDlg *dlg);
	static void OnFooterHeightChanged (GtkSpinButton *btn, PrintSetupDlg *dlg);
	static void OnHorizCenter (PrintSetupDlg *dlg);
	static void OnVertCenter (PrintSetupDlg *dlg);
	static void UpdateScale (PrintSetupDlg *dlg);
	static void OnScaleType (GtkToggleButton *btn, PrintSetupDlg *dlg);
	static void OnScale (GtkSpinButton *btn, PrintSetupDlg *dlg);
	static void OnHFit (GtkToggleButton *btn, PrintSetupDlg *dlg);
	static void OnVFit (GtkToggleButton *btn, PrintSetupDlg *dlg);
	static void OnHPages (GtkSpinButton *btn, PrintSetupDlg *dlg);
	static void OnVPages (GtkSpinButton *btn, PrintSetupDlg *dlg);
};

void PrintSetupDlgPrivate::DoPrint (PrintSetupDlg *dlg, bool preview)
{
	dlg->m_Printable->Print (preview);
}

void PrintSetupDlgPrivate::OnPrinterSetup (PrintSetupDlg *dlg)
{
	gtk_print_run_page_setup_dialog_async
		(dlg->GetWindow (),
		 dlg->m_Printable->GetPageSetup (),
		 NULL,
		 reinterpret_cast < GtkPageSetupDoneFunc > (PrintSetupDlgPrivate::UpdatePageSetup),
		 dlg);
}

#define TOGGLE_BUTTON(name) \
	g_signal_handler_block (G_OBJECT (dlg->m_##name##Btn), dlg->m_##name##Id); \
	gtk_toggle_button_set_active (dlg->m_##name##Btn, true); \
	g_signal_handler_unblock (G_OBJECT (dlg->m_##name##Btn), dlg->m_##name##Id); \
	break;

#define SET_SPIN_BUTTON_VALUE(name,x) \
	g_signal_handler_block (G_OBJECT (dlg->m_##name##Btn), dlg->m_##name##Id); \
	gtk_spin_button_set_value (dlg->m_##name##Btn, x); \
	g_signal_handler_unblock (G_OBJECT (dlg->m_##name##Btn), dlg->m_##name##Id); \
	switch (dlg->m_Printable->GetUnit ()) { \
	case GTK_UNIT_MM: \
		gtk_spin_button_set_digits (dlg->m_##name##Btn, 1); \
		gtk_spin_button_set_increments (dlg->m_##name##Btn, 1., 0.); \
		break; \
	case GTK_UNIT_INCH: \
		gtk_spin_button_set_digits (dlg->m_##name##Btn, 3); \
		gtk_spin_button_set_increments (dlg->m_##name##Btn, 0.125, 0.); \
		break; \
	case GTK_UNIT_POINTS: \
		gtk_spin_button_set_digits (dlg->m_##name##Btn, 1); \
		gtk_spin_button_set_increments (dlg->m_##name##Btn, 1., 0.); \
		break; \
	case GTK_UNIT_PIXEL: \
		break; \
	}

void PrintSetupDlgPrivate::UpdatePageSetup (GtkPageSetup *page_setup, PrintSetupDlg *dlg)
{
	if (page_setup)
		dlg->m_Printable->SetPageSetup (page_setup);
	GtkPaperSize *size = gtk_page_setup_get_paper_size (dlg->m_Printable->GetPageSetup ());
	gtk_label_set_text (dlg->m_PageTypeLbl, gtk_paper_size_get_display_name (size));
	char const *unit = _(gtk_unit_to_string (dlg->m_Printable->GetUnit ()));
	char *buf = g_strdup_printf (((dlg->m_Printable->GetUnit () == GTK_UNIT_INCH)? _("%.1f %s wide by %.1f %s tall"): _("%.0f %s wide by %.0f %s tall")),
								 gtk_paper_size_get_width (size, dlg->m_Printable->GetUnit ()), unit,
								 gtk_paper_size_get_height (size, dlg->m_Printable->GetUnit ()), unit);
	gtk_label_set_text (dlg->m_PageSizeLbl, buf);
	g_free (buf);
	switch (gtk_page_setup_get_orientation (dlg->m_Printable->GetPageSetup ())) {
	case GTK_PAGE_ORIENTATION_PORTRAIT:
		TOGGLE_BUTTON (Portrait)
	case GTK_PAGE_ORIENTATION_LANDSCAPE:
		TOGGLE_BUTTON (Landscape)
	case GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT:
		TOGGLE_BUTTON (RPortrait)
	case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
		TOGGLE_BUTTON (RLandscape)
	}
	g_signal_handler_block (G_OBJECT (dlg->m_UnitBox), dlg->m_UnitId);
	gtk_tree_model_foreach (GTK_TREE_MODEL (dlg->m_UnitList),
	                        reinterpret_cast < GtkTreeModelForeachFunc > (PrintSetupDlgPrivate::SelectUnit), dlg);
	g_signal_handler_unblock (G_OBJECT (dlg->m_UnitBox), dlg->m_UnitId);
	SET_SPIN_BUTTON_VALUE (MarginTop, gtk_page_setup_get_top_margin (dlg->m_Printable->GetPageSetup (), dlg->m_Printable->GetUnit ()));
	SET_SPIN_BUTTON_VALUE (MarginBottom, gtk_page_setup_get_bottom_margin (dlg->m_Printable->GetPageSetup (), dlg->m_Printable->GetUnit ()));
	SET_SPIN_BUTTON_VALUE (MarginRight, gtk_page_setup_get_right_margin (dlg->m_Printable->GetPageSetup (), dlg->m_Printable->GetUnit ()));
	SET_SPIN_BUTTON_VALUE (MarginLeft, gtk_page_setup_get_left_margin (dlg->m_Printable->GetPageSetup (), dlg->m_Printable->GetUnit ()));
}

bool PrintSetupDlgPrivate::SelectUnit (GtkTreeModel *, GtkTreePath *, GtkTreeIter *iter, PrintSetupDlg *dlg)
{
	GtkUnit unit;
	gtk_tree_model_get (GTK_TREE_MODEL (dlg->m_UnitList), iter, 1, &unit, -1);
	if (unit == dlg->m_Printable->GetUnit ()) {
		gtk_combo_box_set_active_iter (dlg->m_UnitBox, iter);
		return true;
	}
	return false;
}

void PrintSetupDlgPrivate::OnOrientation (GtkToggleButton *btn, PrintSetupDlg *dlg)
{
	if (gtk_toggle_button_get_active (btn)) {
		GtkPageOrientation orientation = static_cast < GtkPageOrientation> ( GPOINTER_TO_INT (g_object_get_data (G_OBJECT (btn), "orientation")));
		if (gtk_page_setup_get_orientation (dlg->m_Printable->GetPageSetup ()) != orientation) {
			gtk_page_setup_set_orientation (dlg->m_Printable->GetPageSetup (), orientation);
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
}


void PrintSetupDlgPrivate::OnScaleType (GtkToggleButton *btn, PrintSetupDlg *dlg)
{
	if (gtk_toggle_button_get_active (btn)) {
		dlg->m_Printable->SetScaleType (static_cast < PrintScaleType > (GPOINTER_TO_INT (g_object_get_data (G_OBJECT (btn), "scale-type"))));
		UpdateScale (dlg);
	}
}

static gint unit_sort_func (GtkTreeModel *model,
		GtkTreeIter *a, GtkTreeIter *b, G_GNUC_UNUSED gpointer user_data)
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

void PrintSetupDlgPrivate::OnUnitChanged (PrintSetupDlg *dlg)
{
	GtkTreeIter iter;
	if (gtk_combo_box_get_active_iter (dlg->m_UnitBox, &iter)) {
		GtkUnit unit;
		gtk_tree_model_get (GTK_TREE_MODEL (dlg->m_UnitList), &iter, 1, &unit, -1);
		dlg->m_Printable->SetUnit (unit);
		UpdatePageSetup (NULL, dlg); // use the new unit for display
	}
}

void PrintSetupDlgPrivate::OnTopMarginChanged (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	gtk_page_setup_set_top_margin (dlg->m_Printable->GetPageSetup (),
	                               gtk_spin_button_get_value (btn),
	                               dlg->m_Printable->GetUnit ());
}

void PrintSetupDlgPrivate::OnBottomMarginChanged (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	gtk_page_setup_set_bottom_margin (dlg->m_Printable->GetPageSetup (),
	                               gtk_spin_button_get_value (btn),
	                               dlg->m_Printable->GetUnit ());
}

void PrintSetupDlgPrivate::OnRightMarginChanged (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	gtk_page_setup_set_right_margin (dlg->m_Printable->GetPageSetup (),
	                               gtk_spin_button_get_value (btn),
	                               dlg->m_Printable->GetUnit ());
}

void PrintSetupDlgPrivate::OnLeftMarginChanged (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	gtk_page_setup_set_left_margin (dlg->m_Printable->GetPageSetup (),
	                               gtk_spin_button_get_value (btn),
	                               dlg->m_Printable->GetUnit ());
}

void PrintSetupDlgPrivate::OnHeaderHeightChanged (GtkSpinButton *, PrintSetupDlg *)
{
	// FIXME: not implemented
}

void PrintSetupDlgPrivate::OnFooterHeightChanged (GtkSpinButton *, PrintSetupDlg *)
{
	// FIXME: not implemented
}

void PrintSetupDlgPrivate::OnHorizCenter (PrintSetupDlg *dlg)
{
	dlg->m_Printable->SetHorizCentered (gtk_toggle_button_get_active (dlg->m_HBtn));
}

void PrintSetupDlgPrivate::OnVertCenter (PrintSetupDlg *dlg)
{
	dlg->m_Printable->SetVertCentered (gtk_toggle_button_get_active (dlg->m_VBtn));
}

void PrintSetupDlgPrivate::UpdateScale (PrintSetupDlg *dlg)
{
	switch (dlg->m_Printable->GetScaleType ()) {
	case GCU_PRINT_SCALE_NONE:
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_HFitBtn), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_VFitBtn), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_HPagesBtn), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_FitHLbl), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_VPagesBtn), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_FitVLbl), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_ScaleBtn), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_ScaleLbl), false);
		TOGGLE_BUTTON (ScalingNone)
		break;
	case GCU_PRINT_SCALE_FIXED:
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_HFitBtn), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_VFitBtn), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_HPagesBtn), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_FitHLbl), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_VPagesBtn), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_FitVLbl), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_ScaleBtn), true);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_ScaleLbl), true);
		TOGGLE_BUTTON (ScalingFixed)
		break;
	case GCU_PRINT_SCALE_AUTO: {
		bool has_pages = dlg->m_Printable->SupportMultiplePages () && dlg->m_Printable->GetHorizFit ();
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_HFitBtn), true);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_VFitBtn), true);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_HPagesBtn), has_pages);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_FitHLbl), has_pages);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_VPagesBtn), has_pages);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_FitVLbl), has_pages);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_ScaleBtn), false);
		gtk_widget_set_sensitive (GTK_WIDGET (dlg->m_ScaleLbl), false);
		TOGGLE_BUTTON (ScalingAuto)
		break;
	}
	}
}

void PrintSetupDlgPrivate::OnScale (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	dlg->m_Printable->SetScale (gtk_spin_button_get_value (btn) / 100.);
}

void PrintSetupDlgPrivate::OnHFit (GtkToggleButton *btn, PrintSetupDlg *dlg)
{
	dlg->m_Printable->SetHorizFit (gtk_toggle_button_get_active (btn));
}

void PrintSetupDlgPrivate::OnVFit (GtkToggleButton *btn, PrintSetupDlg *dlg)
{
	dlg->m_Printable->SetVertFit (gtk_toggle_button_get_active (btn));
}

void PrintSetupDlgPrivate::OnHPages (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	dlg->m_Printable->SetHPages (gtk_spin_button_get_value_as_int (btn));
}

void PrintSetupDlgPrivate::OnVPages (GtkSpinButton *btn, PrintSetupDlg *dlg)
{
	dlg->m_Printable->SetVPages (gtk_spin_button_get_value_as_int (btn));
}

static void on_print_background_toggled (GtkToggleButton *btn, Printable *printable)
{
	printable->SetPrintBackground (gtk_toggle_button_get_active (btn));
}

PrintSetupDlg::PrintSetupDlg (Application* App, Printable *printable):
	Dialog (App, UIDIR"/print-setup.ui", "print-setup", GETTEXT_PACKAGE, printable),
	m_Printable (printable)
{
	GtkWidget *w;
	w = GetWidget ("print");
	g_signal_connect_swapped (w, "clicked", G_CALLBACK (PrintSetupDlgPrivate::OnPrint), this);
	w = GetWidget ("preview");
	g_signal_connect_swapped (w, "clicked", G_CALLBACK (PrintSetupDlgPrivate::OnPrintPreview), this);
	w = GetWidget ("paper-btn");
	g_signal_connect_swapped (w, "clicked", G_CALLBACK (PrintSetupDlgPrivate::OnPrinterSetup), this);
	// set current page size
	m_PageTypeLbl = GTK_LABEL (GetWidget ("paper-type-lbl"));
	m_PageSizeLbl = GTK_LABEL (GetWidget ("paper-size-lbl"));
	m_PortraitBtn = GTK_TOGGLE_BUTTON (GetWidget ("portrait-btn"));
	g_object_set_data ((GObject*) m_PortraitBtn ,"orientation", GINT_TO_POINTER (GTK_PAGE_ORIENTATION_PORTRAIT));
	m_PortraitId = g_signal_connect ((GObject*) m_PortraitBtn, "clicked", G_CALLBACK (PrintSetupDlgPrivate::OnOrientation), this);
	m_LandscapeBtn = GTK_TOGGLE_BUTTON (GetWidget ("landscape-btn"));
	g_object_set_data ((GObject*) m_LandscapeBtn ,"orientation", GINT_TO_POINTER (GTK_PAGE_ORIENTATION_LANDSCAPE));
	m_LandscapeId = g_signal_connect ((GObject*) m_LandscapeBtn, "clicked", G_CALLBACK (PrintSetupDlgPrivate::OnOrientation), this);
	m_RPortraitBtn = GTK_TOGGLE_BUTTON (GetWidget ("r-portrait-btn"));
	g_object_set_data ((GObject*) m_RPortraitBtn ,"orientation", GINT_TO_POINTER (GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT));
	m_RPortraitId = g_signal_connect ((GObject*) m_RPortraitBtn, "clicked", G_CALLBACK (PrintSetupDlgPrivate::OnOrientation), this);
	m_RLandscapeBtn = GTK_TOGGLE_BUTTON (GetWidget ("r-landscape-btn"));
	g_object_set_data ((GObject*) m_RLandscapeBtn ,"orientation", GINT_TO_POINTER (GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE));
	m_RLandscapeId = g_signal_connect ((GObject*) m_RLandscapeBtn, "clicked", G_CALLBACK (PrintSetupDlgPrivate::OnOrientation), this);
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
	m_UnitId = g_signal_connect_swapped (m_UnitBox, "changed", G_CALLBACK (PrintSetupDlgPrivate::OnUnitChanged), this);
	GtkCellRenderer *text_renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT(m_UnitBox), text_renderer, TRUE);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT(m_UnitBox), text_renderer, "text", 0);
	gtk_grid_attach (GTK_GRID (GetWidget ("paper-selector-grid")), GTK_WIDGET (m_UnitBox), 3, 8, 1, 1);
	m_MarginTopBtn = GTK_SPIN_BUTTON (GetWidget ("top-margin-btn"));
	m_MarginTopId = g_signal_connect ((GObject*) m_MarginTopBtn, "value-changed", G_CALLBACK (PrintSetupDlgPrivate::OnTopMarginChanged), this);
	m_MarginBottomBtn = GTK_SPIN_BUTTON (GetWidget ("bottom-margin-btn"));
	m_MarginBottomId = g_signal_connect ((GObject*) m_MarginBottomBtn, "value-changed", G_CALLBACK (PrintSetupDlgPrivate::OnBottomMarginChanged), this);
	m_MarginRightBtn = GTK_SPIN_BUTTON (GetWidget ("right-margin-btn"));
	m_MarginRightId = g_signal_connect ((GObject*) m_MarginRightBtn, "value-changed", G_CALLBACK (PrintSetupDlgPrivate::OnRightMarginChanged), this);
	m_MarginLeftBtn = GTK_SPIN_BUTTON (GetWidget ("left-margin-btn"));
	m_MarginLeftId = g_signal_connect ((GObject*) m_MarginLeftBtn, "value-changed", G_CALLBACK (PrintSetupDlgPrivate::OnLeftMarginChanged), this);
	m_HeaderHeightBtn = GTK_SPIN_BUTTON (GetWidget ("header-height-btn"));
	m_FooterHeightBtn = GTK_SPIN_BUTTON (GetWidget ("footer-height-btn"));
	PrintSetupDlgPrivate::UpdatePageSetup (NULL, this);
	m_HBtn = GTK_TOGGLE_BUTTON (GetWidget ("hcenter-btn"));
	gtk_toggle_button_set_active (m_HBtn, m_Printable->GetHorizCentered ());
	m_HId = g_signal_connect_swapped ((GObject*) m_HBtn, "toggled", G_CALLBACK (PrintSetupDlgPrivate::OnHorizCenter), this);
	m_VBtn = GTK_TOGGLE_BUTTON (GetWidget ("vcenter-btn"));
	gtk_toggle_button_set_active (m_VBtn, m_Printable->GetVertCentered ());
	m_VId = g_signal_connect_swapped ((GObject*) m_VBtn, "toggled", G_CALLBACK (PrintSetupDlgPrivate::OnVertCenter), this);
	m_ScalingNoneBtn = GTK_TOGGLE_BUTTON (GetWidget ("scale-no-btn"));
	g_object_set_data ((GObject*) m_ScalingNoneBtn ,"scale-type", GINT_TO_POINTER (GCU_PRINT_SCALE_NONE));
	m_ScalingNoneId = g_signal_connect ((GObject*) m_ScalingNoneBtn, "clicked", G_CALLBACK (PrintSetupDlgPrivate::OnScaleType), this);
	m_ScalingFixedBtn = GTK_TOGGLE_BUTTON (GetWidget ("scale-fixed-btn"));
	g_object_set_data ((GObject*) m_ScalingFixedBtn ,"scale-type", GINT_TO_POINTER (GCU_PRINT_SCALE_FIXED));
	m_ScalingFixedId = g_signal_connect ((GObject*) m_ScalingFixedBtn, "clicked", G_CALLBACK (PrintSetupDlgPrivate::OnScaleType), this);
	m_ScalingAutoBtn = GTK_TOGGLE_BUTTON (GetWidget ("scale-auto-btn"));
	g_object_set_data ((GObject*) m_ScalingAutoBtn ,"scale-type", GINT_TO_POINTER (GCU_PRINT_SCALE_AUTO));
	m_ScalingAutoId = g_signal_connect ((GObject*) m_ScalingAutoBtn, "clicked", G_CALLBACK (PrintSetupDlgPrivate::OnScaleType), this);
	m_HFitBtn = GTK_TOGGLE_BUTTON (GetWidget ("scale-fit-h-btn"));
	gtk_toggle_button_set_active (m_HFitBtn, m_Printable->GetHorizFit ());
	g_signal_connect ((GObject*) m_HFitBtn, "toggled", G_CALLBACK (PrintSetupDlgPrivate::OnHFit), this);
	m_VFitBtn = GTK_TOGGLE_BUTTON (GetWidget ("scale-fit-v-btn"));
	gtk_toggle_button_set_active (m_VFitBtn, m_Printable->GetVertFit ());
	g_signal_connect ((GObject*) m_VFitBtn, "toggled", G_CALLBACK (PrintSetupDlgPrivate::OnVFit), this);
	m_HPagesBtn = GTK_SPIN_BUTTON (GetWidget ("scale-h-btn"));
	gtk_spin_button_set_value (m_HPagesBtn, m_Printable->GetHPages ());
	g_signal_connect ((GObject*) m_HPagesBtn, "value-changed", G_CALLBACK (PrintSetupDlgPrivate::OnHPages), this);
	m_FitHLbl = GTK_LABEL (GetWidget ("fit-h-lbl"));
	m_VPagesBtn = GTK_SPIN_BUTTON (GetWidget ("scale-v-btn"));
	gtk_spin_button_set_value (m_VPagesBtn, m_Printable->GetVPages ());
	g_signal_connect ((GObject*) m_VPagesBtn, "value-changed", G_CALLBACK (PrintSetupDlgPrivate::OnVPages), this);
	m_FitVLbl = GTK_LABEL (GetWidget ("fit-v-lbl"));
	m_ScaleBtn = GTK_SPIN_BUTTON (GetWidget ("scale-percent-btn"));
	g_signal_connect ((GObject*) m_ScaleBtn, "value-changed", G_CALLBACK (PrintSetupDlgPrivate::OnScale), this);
	m_ScaleLbl = GTK_LABEL (GetWidget ("scale-percent-lbl"));
	gtk_spin_button_set_value (m_ScaleBtn, m_Printable->GetScale () * 100.);
	PrintSetupDlgPrivate::UpdateScale (this);
	if (printable->SupportsHeaders ()) {
		m_HeaderHeightId = g_signal_connect ((GObject*) m_HeaderHeightBtn, "value-changed", G_CALLBACK (PrintSetupDlgPrivate::OnHeaderHeightChanged), this);
		m_FooterHeightId = g_signal_connect ((GObject*) m_FooterHeightBtn, "value-changed", G_CALLBACK (PrintSetupDlgPrivate::OnFooterHeightChanged), this);
	} else {
		// hide everything related to headers and footers
		// first delete the notebook page
		GtkNotebook *book = GTK_NOTEBOOK (GetWidget ("print-setup-book"));
		gtk_notebook_remove_page (book, 2);
		// now hide/disable related buttons and labels
		gtk_spin_button_set_value (m_HeaderHeightBtn, 0.);
		gtk_widget_set_sensitive (GetWidget ("header-height-lbl"), false);
		gtk_widget_set_sensitive (GTK_WIDGET (m_HeaderHeightBtn), false);
		gtk_spin_button_set_value (m_FooterHeightBtn, 0.);
		gtk_widget_set_sensitive (GetWidget ("footer-height-lbl"), false);
		gtk_widget_set_sensitive (GTK_WIDGET (m_FooterHeightBtn), false);
		m_HeaderHeightId = m_FooterHeightId = 0;
	}
	gtk_widget_show_all (GTK_WIDGET (dialog));
	w = GetWidget ("background-btn");
	if (printable->GetHasBackground ()) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), printable->GetPrintBackground ());
		g_signal_connect (G_OBJECT (w), "toggled", G_CALLBACK (on_print_background_toggled), printable);
	} else
		gtk_widget_hide (w);
}

PrintSetupDlg::~PrintSetupDlg ()
{
}

}	//	namespace gcugtk
