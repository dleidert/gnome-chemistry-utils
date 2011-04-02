// -*- C++ -*-

/* 
 * GChemPaint library
 * preferences.cc 
 *
 * Copyright (C) 2006-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "preferences.h"
#include "application.h"
#include "document.h"
#include "fontsel.h"
#include "settings.h"
#include "theme.h"
#include <glib/gi18n-lib.h>
#include <sys/stat.h>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

// PrefsDlgPrivate hides private PrefsDlg API
class PrefsDlgPrivate {
public:
	static void OnNewTheme (PrefsDlg *dlg);
	static void OnSelectTheme (PrefsDlg *dlg, GtkTreeSelection *selection) {dlg->OnSelectTheme (selection);}
	static void OnBondLength (PrefsDlg *dlg, double length) {dlg->OnBondLength (length);}
	static void OnBondAngle (PrefsDlg *dlg, double angle) {dlg->OnBondAngle (angle);}
	static void OnBondWidth (PrefsDlg *dlg, double width) {dlg->OnBondWidth (width);}
	static void OnBondDist (PrefsDlg *dlg, double dist) {dlg->OnBondDist (dist);}
	static void OnStereoBondWidth (PrefsDlg *dlg, double width) {dlg->OnStereoBondWidth (width);}
	static void OnHashWidth (PrefsDlg *dlg, double width) {dlg->OnHashWidth (width);}
	static void OnHashDist (PrefsDlg *dlg, double dist) {dlg->OnHashDist (dist);}
	static void OnFont (PrefsDlg *dlg, GcpFontSel *fs) {dlg->OnFont (fs);}
	static void OnTextFont (PrefsDlg *dlg, GcpFontSel *fs) {dlg->OnTextFont (fs);}
	static void OnArrowLength (PrefsDlg *dlg, double length) {dlg->OnArrowLength (length);}
	static void OnArrowWidth (PrefsDlg *dlg, double width) {dlg->OnArrowWidth (width);}
	static void OnArrowDist (PrefsDlg *dlg, double dist) {dlg->OnArrowDist (dist);}
	static void OnArrowPadding (PrefsDlg *dlg, double padding) {dlg->OnArrowPadding (padding);}
	static void OnArrowHeadA (PrefsDlg *dlg, double headA) {dlg->OnArrowHeadA (headA);}
	static void OnArrowHeadB (PrefsDlg *dlg, double headB) {dlg->OnArrowHeadB (headB);}
	static void OnArrowHeadC (PrefsDlg *dlg, double headC) {dlg->OnArrowHeadC (headC);}
	static void OnScale (PrefsDlg *dlg, double scale) {dlg->OnScale (scale);}
	static void OnPadding (PrefsDlg *dlg, double padding) {dlg->OnPadding (padding);}
	static void OnObjectPadding (PrefsDlg *dlg, double padding) {dlg->OnObjectPadding (padding);}
	static void OnStoichPadding (PrefsDlg *dlg, double padding) {dlg->OnStoichPadding (padding);}
	static void OnSignPadding (PrefsDlg *dlg, double padding) {dlg->OnSignPadding (padding);}
	static void OnChargeSize (PrefsDlg *dlg, double size) {dlg->OnChargeSize (size);}
	static void OnThemeNameChanged (PrefsDlg *dlg, char const *name) {dlg->OnThemeNameChanged (name);}
	static bool CheckError (PrefsDlg *dlg) {return dlg->CheckError ();}
	static void SetDefaultTheme (PrefsDlg *dlg, char const *name) {dlg->SetDefaultTheme (name);}
};

static int get_fontstyle (PangoStyle val)
{
	switch (val) {
		case PANGO_STYLE_NORMAL: return 0;
		case PANGO_STYLE_OBLIQUE: return 1;
		case PANGO_STYLE_ITALIC: return 2;
		default: return 0;
	}
}

static int get_fontweight (PangoWeight val)
{
	switch (val) {
		case PANGO_WEIGHT_ULTRALIGHT: return 2;
		case PANGO_WEIGHT_LIGHT: return 3;
		case PANGO_WEIGHT_NORMAL: return 4;
		case PANGO_WEIGHT_SEMIBOLD: return 6;
		case PANGO_WEIGHT_BOLD: return 7;
		case PANGO_WEIGHT_ULTRABOLD: return 8;
		case PANGO_WEIGHT_HEAVY: return 9;
#if PANGO_VERSION_MAJOR > 1 || PANGO_VERSION_MINOR >= 24
		case PANGO_WEIGHT_THIN: return 1;
		case PANGO_WEIGHT_BOOK: return 38;
		case PANGO_WEIGHT_MEDIUM: return 5;
		case PANGO_WEIGHT_ULTRAHEAVY: return 10;
#endif
		default: return 4;
	}
}

static int get_fontvariant (PangoVariant val)
{
	switch (val) {
		case PANGO_VARIANT_NORMAL: return 0;
		case PANGO_VARIANT_SMALL_CAPS: return 1;
		default: return 0;
	}
}

static int get_fontstretch (PangoStretch val)
{
	switch (val) {
		case PANGO_STRETCH_ULTRA_CONDENSED: return 0;
		case PANGO_STRETCH_EXTRA_CONDENSED: return 1;
		case PANGO_STRETCH_CONDENSED: return 2;
		case PANGO_STRETCH_SEMI_CONDENSED: return 3;
		case PANGO_STRETCH_NORMAL: return 4;
		case PANGO_STRETCH_SEMI_EXPANDED: return 5;
		case PANGO_STRETCH_EXPANDED: return 6;
		case PANGO_STRETCH_EXTRA_EXPANDED: return 7;
		case PANGO_STRETCH_ULTRA_EXPANDED: return 8;
		default: return 4;
	}
}

static double get_fontsize (double val) {return (double) val / (double) PANGO_SCALE;}

static void on_compression_changed (GtkSpinButton *btn, Application *App)
{
	CompressionLevel = gtk_spin_button_get_value_as_int (btn);
	GOConfNode *node = go_conf_get_node (App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
	go_conf_set_int (node, "compression", CompressionLevel);
	go_conf_free_node (node);
}

static void on_invert_wedge_hashes_changed (GtkToggleButton *btn, Application *App)
{
	InvertWedgeHashes = gtk_toggle_button_get_active (btn);
	GOConfNode *node = go_conf_get_node (App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
	go_conf_set_bool (node, "invert-wedge-hashes", InvertWedgeHashes);
	go_conf_free_node (node);
}

static void on_new_theme (PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnNewTheme (dlg);
}

static void on_select_theme (GtkTreeSelection *selection, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnSelectTheme (dlg, selection);
}

static void on_bond_length_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnBondLength (dlg, gtk_spin_button_get_value (btn));
}

static void on_bond_angle_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnBondAngle (dlg, gtk_spin_button_get_value (btn));
}

static void on_bond_width_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnBondWidth (dlg, gtk_spin_button_get_value (btn));
}

static void on_bond_dist_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnBondDist (dlg, gtk_spin_button_get_value (btn));
}

static void on_stereo_bond_width_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnStereoBondWidth (dlg, gtk_spin_button_get_value (btn));
}

static void on_hash_width_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnHashWidth (dlg, gtk_spin_button_get_value (btn));
}

static void on_hash_dist_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnHashDist (dlg, gtk_spin_button_get_value (btn));
}

static void on_text_font_changed (GcpFontSel *fc, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnTextFont (dlg, fc);
}

static void on_font_changed (GcpFontSel *fc, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnFont (dlg, fc);
}

static void on_arrow_length_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnArrowLength (dlg, gtk_spin_button_get_value (btn));
}

static void on_arrow_width_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnArrowWidth (dlg, gtk_spin_button_get_value (btn));
}

static void on_arrow_dist_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnArrowDist (dlg, gtk_spin_button_get_value (btn));
}

static void on_arrow_padding_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnArrowPadding (dlg, gtk_spin_button_get_value (btn));
}

static void on_arrow_headA_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnArrowHeadA (dlg, gtk_spin_button_get_value (btn));
}

static void on_arrow_headB_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnArrowHeadB (dlg, gtk_spin_button_get_value (btn));
}

static void on_arrow_headC_changed (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnArrowHeadC (dlg, gtk_spin_button_get_value (btn));
}

static void on_scale (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnScale (dlg, gtk_spin_button_get_value (btn));
}

static void on_padding (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnPadding (dlg, gtk_spin_button_get_value (btn));
}

static void on_object_padding (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnObjectPadding (dlg, gtk_spin_button_get_value (btn));
}

static void on_stoich_padding(GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnStoichPadding (dlg, gtk_spin_button_get_value (btn));
}

static void on_sign_padding (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnSignPadding (dlg, gtk_spin_button_get_value (btn));
}

static void on_charge_size (GtkSpinButton *btn, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnChargeSize (dlg, gtk_spin_button_get_value (btn));
}

static void on_name_changed (GtkEntry *entry, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnThemeNameChanged (dlg, gtk_entry_get_text (entry));
}

static bool on_name_focused_out (GtkEntry *entry, G_GNUC_UNUSED GdkEventFocus *event, PrefsDlg *dlg)
{
	PrefsDlgPrivate::OnThemeNameChanged (dlg, gtk_entry_get_text (entry));
	return false;
}

static bool on_delete_event (GtkWidget* widget, G_GNUC_UNUSED GdkEvent *event, PrefsDlg* dlg)
{
	bool res = PrefsDlgPrivate::CheckError (dlg);
	if (res) {
		GtkWidget* box = gtk_message_dialog_new (GTK_WINDOW (widget), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Invalid name");
		g_signal_connect (G_OBJECT (box), "response", G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show_all (box);
	}
	return res;
}

#if GTK_CHECK_VERSION (2, 24, 0)
static void on_default_theme_changed (GtkComboBoxText *box, PrefsDlg* dlg)
{
	PrefsDlgPrivate::SetDefaultTheme (dlg, gtk_combo_box_text_get_active_text (box));
}
#else
static void on_default_theme_changed (GtkComboBox *box, PrefsDlg* dlg)
{
	PrefsDlgPrivate::SetDefaultTheme (dlg, gtk_combo_box_get_active_text (box));
}
#endif

PrefsDlg::PrefsDlg (Application *pApp):
	Dialog (pApp, UIDIR"/preferences.ui", "preferences", GETTEXT_PACKAGE, pApp),
	Object (),
	m_CurTheme (NULL),
	m_Path (NULL)
{
	g_signal_connect (G_OBJECT (dialog), "delete-event", G_CALLBACK (on_delete_event), this);
	// compression level
	GtkWidget *w = GetWidget ("compression");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), CompressionLevel);
	g_signal_connect (G_OBJECT (w), "value-changed", G_CALLBACK (on_compression_changed), pApp);
	// invert hashed bonds or not
	w = GetWidget ("invert-wedge-hashes");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), InvertWedgeHashes);
	g_signal_connect (G_OBJECT (w), "toggled", G_CALLBACK (on_invert_wedge_hashes_changed), pApp);
	// retrieve theme widgets and set signals
	m_BondLengthBtn = GTK_SPIN_BUTTON (GetWidget ("bond-length-btn"));
	g_signal_connect (G_OBJECT (m_BondLengthBtn), "value-changed", G_CALLBACK (on_bond_length_changed), this);
	m_BondAngleBtn = GTK_SPIN_BUTTON (GetWidget ("bond-angle-btn"));
	g_signal_connect (G_OBJECT (m_BondAngleBtn), "value-changed", G_CALLBACK (on_bond_angle_changed), this);
	m_BondWidthBtn = GTK_SPIN_BUTTON (GetWidget ("bond-width-btn"));
	g_signal_connect (G_OBJECT (m_BondWidthBtn), "value-changed", G_CALLBACK (on_bond_width_changed), this);
	m_BondDistBtn = GTK_SPIN_BUTTON (GetWidget ("bond-dist-btn"));
	g_signal_connect (G_OBJECT (m_BondDistBtn), "value-changed", G_CALLBACK (on_bond_dist_changed), this);
	m_StereoBondWidthBtn = GTK_SPIN_BUTTON (GetWidget ("stereo-width-btn"));
	g_signal_connect (G_OBJECT (m_StereoBondWidthBtn), "value-changed", G_CALLBACK (on_stereo_bond_width_changed), this);
	m_HashWidthBtn = GTK_SPIN_BUTTON (GetWidget ("hash-width-btn"));
	g_signal_connect (G_OBJECT (m_HashWidthBtn), "value-changed", G_CALLBACK (on_hash_width_changed), this);
	m_HashDistBtn = GTK_SPIN_BUTTON (GetWidget ("hash-dist-btn"));
	g_signal_connect (G_OBJECT (m_HashDistBtn), "value-changed", G_CALLBACK (on_hash_dist_changed), this);
	// add font selector
	m_FontSel = GCP_FONT_SEL (g_object_new (GCP_TYPE_FONT_SEL, NULL));
	w = GetWidget ("atom-font-box");
	gtk_box_pack_start (GTK_BOX (w), GTK_WIDGET (m_FontSel), true, true, 0);
	m_FontChanged = g_signal_connect (G_OBJECT (m_FontSel), "changed", G_CALLBACK (on_font_changed), this);
	// add text font selector
	m_TextFontSel = GCP_FONT_SEL (g_object_new (GCP_TYPE_FONT_SEL, NULL));
	w = GetWidget ("text-box");
	gtk_box_pack_start (GTK_BOX (w), GTK_WIDGET (m_TextFontSel), true, true, 0);
	m_TextFontChanged = g_signal_connect (G_OBJECT (m_TextFontSel), "changed", G_CALLBACK (on_text_font_changed), this);
	// arrow spin buttons
	m_ArrowLengthBtn = GTK_SPIN_BUTTON (GetWidget ("arrow-length-btn"));
	g_signal_connect (G_OBJECT (m_ArrowLengthBtn), "value-changed", G_CALLBACK (on_arrow_length_changed), this);
	m_ArrowWidthBtn = GTK_SPIN_BUTTON( GetWidget ("arrow-width-btn"));
	g_signal_connect (G_OBJECT (m_ArrowWidthBtn), "value-changed", G_CALLBACK (on_arrow_width_changed), this);
	m_ArrowDistBtn = GTK_SPIN_BUTTON (GetWidget ("arrow-dist-btn"));
	g_signal_connect (G_OBJECT (m_ArrowDistBtn), "value-changed", G_CALLBACK (on_arrow_dist_changed), this);
	m_ArrowPaddingBtn = GTK_SPIN_BUTTON (GetWidget ("arrow-padding-btn"));
	g_signal_connect (G_OBJECT (m_ArrowPaddingBtn), "value-changed", G_CALLBACK (on_arrow_padding_changed), this);
	m_ArrowHeadABtn = GTK_SPIN_BUTTON (GetWidget ("arrow-headA-btn"));
	g_signal_connect (G_OBJECT (m_ArrowHeadABtn), "value-changed", G_CALLBACK (on_arrow_headA_changed), this);
	m_ArrowHeadBBtn = GTK_SPIN_BUTTON (GetWidget ("arrow-headB-btn"));
	g_signal_connect (G_OBJECT (m_ArrowHeadBBtn), "value-changed", G_CALLBACK (on_arrow_headB_changed), this);
	m_ArrowHeadCBtn = GTK_SPIN_BUTTON (GetWidget ("arrow-headC-btn"));
	g_signal_connect (G_OBJECT (m_ArrowHeadCBtn), "value-changed", G_CALLBACK (on_arrow_headC_changed), this);
	m_ScaleBtn = GTK_SPIN_BUTTON (GetWidget ("scale-btn"));
	g_signal_connect (G_OBJECT (m_ScaleBtn), "value-changed", G_CALLBACK (on_scale), this);
	m_PaddingBtn = GTK_SPIN_BUTTON (GetWidget ("padding-btn"));
	g_signal_connect (G_OBJECT (m_PaddingBtn), "value-changed", G_CALLBACK (on_padding), this);
	m_ObjectPaddingBtn = GTK_SPIN_BUTTON (GetWidget ("object-padding-btn"));
	g_signal_connect (G_OBJECT (m_ObjectPaddingBtn), "value-changed", G_CALLBACK (on_object_padding), this);
	m_StoichPaddingBtn = GTK_SPIN_BUTTON (GetWidget ("stoich-padding-btn"));
	g_signal_connect (G_OBJECT (m_StoichPaddingBtn), "value-changed", G_CALLBACK (on_stoich_padding), this);
	m_SignPaddingBtn = GTK_SPIN_BUTTON (GetWidget ("sign-padding-btn"));
	g_signal_connect (G_OBJECT (m_SignPaddingBtn), "value-changed", G_CALLBACK (on_sign_padding), this);
	m_ChargeSizeBtn = GTK_SPIN_BUTTON (GetWidget ("charge-size-btn"));
	g_signal_connect (G_OBJECT (m_ChargeSizeBtn), "value-changed", G_CALLBACK (on_charge_size), this);
	m_NameEntry = GTK_ENTRY (GetWidget ("name"));
	gtk_entry_set_text (m_NameEntry, _("Default"));
	m_NameActivate = g_signal_connect (G_OBJECT (m_NameEntry), "activate", G_CALLBACK (on_name_changed), this);
	m_NameFocusOut = g_signal_connect (G_OBJECT (m_NameEntry), "focus-out-event", G_CALLBACK (on_name_focused_out), this);
	// get notebook
	m_Book = GTK_NOTEBOOK (GetWidget ("theme-book"));
	// create themes list and select active document theme
	m_ThemesView = GTK_TREE_VIEW (GetWidget ("themes"));
	themes = gtk_tree_store_new (1, G_TYPE_STRING);
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (themes), 0, GTK_SORT_ASCENDING);
	gtk_tree_view_set_model (m_ThemesView, GTK_TREE_MODEL (themes));
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "text", 0, NULL);
	gtk_tree_view_append_column (m_ThemesView, column);
	m_ThemesSelection = gtk_tree_view_get_selection (m_ThemesView);
	list<string> theme_names = TheThemeManager.GetThemesNames ();
	list<string>::iterator i, iend = theme_names.end ();
	GtkTreeIter iter, selected, child, grand_child;
	string default_name = pApp->GetActiveDocument ()->GetTheme ()->GetName ();
	if (default_name == "Default")
			default_name = _("Default");
	Theme *theme, *default_theme = TheThemeManager.GetDefaultTheme ();
#if GTK_CHECK_VERSION (2, 24, 0)
	m_DefaultThemeBox = GTK_COMBO_BOX_TEXT (gtk_combo_box_text_new ());
#else
	m_DefaultThemeBox = GTK_COMBO_BOX (gtk_combo_box_new_text ());
#endif
	gtk_table_attach (GTK_TABLE (GetWidget ("table1")), GTK_WIDGET (m_DefaultThemeBox), 1, 3, 2, 3,
													   (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
													   (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
	int n = 0;
	for (i = theme_names.begin (); i != iend; i++) {
		theme = TheThemeManager.GetTheme (*i);
#if GTK_CHECK_VERSION (2, 24, 0)
		gtk_combo_box_text_append_text (m_DefaultThemeBox, (*i).c_str ());
#else
		gtk_combo_box_append_text (m_DefaultThemeBox, (*i).c_str ());
#endif
		if (theme == default_theme)
			gtk_combo_box_set_active (GTK_COMBO_BOX (m_DefaultThemeBox), n);
		n++;
		if (theme)
			theme->AddClient (this);
		gtk_tree_store_append (themes, &iter, NULL);
		gtk_tree_store_set (themes, &iter,
				  0, (*i).c_str (),
				  -1);
		gtk_tree_store_append (themes, &child, &iter);
		gtk_tree_store_set (themes, &child,
				  0, _("General"),
				  -1);
		if (*i == default_name)
			selected = child;
		gtk_tree_store_append (themes, &child, &iter);
		gtk_tree_store_set (themes, &child,
				  0, _("Atoms"),
				  -1);
		gtk_tree_store_append (themes, &grand_child, &child);
		gtk_tree_store_set (themes, &grand_child,
				  0, _("Font"),
				  -1);
		gtk_tree_store_append (themes, &grand_child, &child);
		gtk_tree_store_set (themes, &grand_child,
				  0, _("Other"),
				  -1);
		gtk_tree_store_append (themes, &child, &iter);
		gtk_tree_store_set (themes, &child,
				  0, _("Bonds"),
				  -1);
		gtk_tree_store_append (themes, &child, &iter);
		gtk_tree_store_set (themes, &child,
				  0, _("Arrows"),
				  -1);
		gtk_tree_store_append (themes, &child, &iter);
		gtk_tree_store_set (themes, &child,
				  0, _("Text"),
				  -1);
	}
	g_signal_connect (m_DefaultThemeBox,"changed", G_CALLBACK (on_default_theme_changed), this);
	m_Path = gtk_tree_model_get_path (GTK_TREE_MODEL (themes), &selected);
	gtk_tree_selection_set_mode (m_ThemesSelection, GTK_SELECTION_BROWSE);
	g_signal_connect (m_ThemesSelection, "changed", G_CALLBACK (on_select_theme), this);
	if (m_Path) {
		gtk_tree_view_expand_to_path (m_ThemesView, m_Path);
		gtk_tree_selection_select_path (m_ThemesSelection, m_Path);
		gtk_tree_view_scroll_to_cell (m_ThemesView, m_Path, column, FALSE, 0., 0.);
	}
	
	// add event handler to new theme button
	g_signal_connect_swapped (G_OBJECT (GetWidget ("new-theme")), "clicked", G_CALLBACK (on_new_theme), this);
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

PrefsDlg::~PrefsDlg ()
{
	list <string> names = TheThemeManager.GetThemesNames ();
	list <string>::iterator i, end = names.end ();
	Theme *theme;
	for (i = names.begin (); i != end; i++) {
		theme = TheThemeManager.GetTheme (*i);
		if (theme)
			theme->RemoveClient (this);
	}
	if (m_Path)
		gtk_tree_path_free (m_Path);
}

void PrefsDlgPrivate::OnNewTheme (PrefsDlg *dlg)
{
	Theme *pTheme = TheThemeManager.CreateNewTheme (dlg->m_CurTheme);
	string &name = pTheme->GetName ();
	GtkTreeIter iter, child, grand_child;
	gtk_tree_store_append (dlg->themes, &iter, NULL);
	gtk_tree_store_set (dlg->themes, &iter,
			  0, name.c_str (),
			  -1);
	gtk_tree_store_append (dlg->themes, &child, &iter);
	gtk_tree_store_set (dlg->themes, &child,
			  0, _("General"),
			  -1);
	GtkTreePath *path = gtk_tree_model_get_path (GTK_TREE_MODEL (dlg->themes), &child);
	if (path) {
		gtk_tree_view_expand_to_path (dlg->m_ThemesView, path);
		gtk_tree_selection_select_path (dlg->m_ThemesSelection, path);
		gtk_tree_view_scroll_to_cell (dlg->m_ThemesView, path, 0, FALSE, 0., 0.);
		gtk_tree_path_free (path);
	}
	gtk_tree_store_append (dlg->themes, &child, &iter);
	gtk_tree_store_set (dlg->themes, &child,
			  0, _("Atoms"),
			  -1);
	gtk_tree_store_append (dlg->themes, &grand_child, &child);
	gtk_tree_store_set (dlg->themes, &grand_child,
			  0, _("Font"),
			  -1);
	gtk_tree_store_append (dlg->themes, &grand_child, &child);
	gtk_tree_store_set (dlg->themes, &grand_child,
			  0, _("Other"),
			  -1);
	gtk_tree_store_append (dlg->themes, &child, &iter);
	gtk_tree_store_set (dlg->themes, &child,
			  0, _("Bonds"),
			  -1);
	gtk_tree_store_append (dlg->themes, &child, &iter);
	gtk_tree_store_set (dlg->themes, &child,
			  0, _("Arrows"),
			  -1);
	gtk_tree_store_append (dlg->themes, &child, &iter);
	gtk_tree_store_set (dlg->themes, &child,
			  0, _("Text"),
			  -1);
	dynamic_cast <Application*> (dlg->m_App)->OnThemeNamesChanged ();
}

void PrefsDlg::OnSelectTheme (GtkTreeSelection *selection)
{
	char const *Name = gtk_entry_get_text (m_NameEntry);
	if (!strcmp (Name, _("Default")))
		Name = "Default";
	if (m_CurTheme && Name != m_CurTheme->m_Name)
		OnThemeNameChanged (Name);
	if (!*Name) {
		gtk_tree_selection_select_path (selection, m_Path);
		return;
	}
	// we must fill the theme widgets with the corresponding data
	GtkTreeIter iter, parent;
	char *name;
	char *page;
	GtkTreeModel *model;
	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) {
		gtk_notebook_set_current_page (m_Book, 0);
		return;
	}
	if (m_Path)
		gtk_tree_path_free (m_Path);
	m_Path = gtk_tree_model_get_path (GTK_TREE_MODEL (themes), &iter);
	if (gtk_tree_model_iter_parent (model, &parent, &iter)) {
		gtk_tree_model_get (model, &parent, 0, &name, -1);
		if (!strcmp (name, _("Atoms"))) {
			g_free (name);
			GtkTreeIter grand_parent;
			gtk_tree_model_iter_parent (model, &grand_parent, &parent);
			gtk_tree_model_get (model, &grand_parent, 0, &name, -1);
		}
		gtk_tree_model_get (model, &iter, 0, &page, -1);
		if (!strcmp (page, _("General")))
			gtk_notebook_set_current_page (m_Book, 1);
		else if (!strcmp (page, _("Font")))
			gtk_notebook_set_current_page (m_Book, 2);
		else if (!strcmp (page, _("Other")))
			gtk_notebook_set_current_page (m_Book, 6);
		else if (!strcmp (page, _("Bonds")))
			gtk_notebook_set_current_page (m_Book, 3);
		else if (!strcmp (page, _("Arrows")))
			gtk_notebook_set_current_page (m_Book, 4);
		else if (!strcmp (page, _("Text")))
			gtk_notebook_set_current_page (m_Book, 5);
		else
			gtk_notebook_set_current_page (m_Book, 0);
		g_free (page);
	} else {
		gtk_tree_model_get (model, &iter, 0, &name, -1);
		gtk_notebook_set_current_page (m_Book, 0);
	}
	m_CurTheme = TheThemeManager.GetTheme (name);
	g_free (name);
	bool rw = m_CurTheme->m_ThemeType != GLOBAL_THEME_TYPE;
	gtk_spin_button_set_value (m_BondLengthBtn, m_CurTheme->m_BondLength);
	gtk_widget_set_sensitive (GTK_WIDGET (m_BondLengthBtn), rw);
	gtk_spin_button_set_value (m_BondAngleBtn, m_CurTheme->m_BondAngle);
	gtk_widget_set_sensitive (GTK_WIDGET (m_BondAngleBtn), rw);
	gtk_spin_button_set_value (m_BondWidthBtn, m_CurTheme->m_BondWidth);
	gtk_widget_set_sensitive (GTK_WIDGET (m_BondWidthBtn), rw);
	gtk_spin_button_set_value (m_BondDistBtn, m_CurTheme->m_BondDist);
	gtk_widget_set_sensitive (GTK_WIDGET (m_BondDistBtn), rw);
	gtk_spin_button_set_value (m_StereoBondWidthBtn, m_CurTheme->m_StereoBondWidth);
	gtk_widget_set_sensitive (GTK_WIDGET (m_StereoBondWidthBtn), rw);
	gtk_spin_button_set_value (m_HashWidthBtn, m_CurTheme->m_HashWidth);
	gtk_widget_set_sensitive (GTK_WIDGET (m_HashWidthBtn), rw);
	gtk_spin_button_set_value (m_HashDistBtn, m_CurTheme->m_HashDist);
	gtk_widget_set_sensitive (GTK_WIDGET (m_HashDistBtn), rw);
	g_signal_handler_block (G_OBJECT (m_TextFontSel), m_TextFontChanged);
	g_object_set (G_OBJECT (m_TextFontSel),
					"family", m_CurTheme->m_TextFontFamily,
					"style", m_CurTheme->m_TextFontStyle,
					"weight", m_CurTheme->m_TextFontWeight,
					"variant", m_CurTheme->m_TextFontVariant,
					"stretch", m_CurTheme->m_TextFontStretch,
					"size", m_CurTheme->m_TextFontSize,
					NULL);
	g_signal_handler_unblock (G_OBJECT (m_TextFontSel), m_TextFontChanged);
	gtk_widget_set_sensitive (GTK_WIDGET (m_TextFontSel), rw);
	g_signal_handler_block (G_OBJECT (m_FontSel), m_FontChanged);
	g_object_set (G_OBJECT (m_FontSel),
					"family", m_CurTheme->m_FontFamily,
					"style", m_CurTheme->m_FontStyle,
					"weight", m_CurTheme->m_FontWeight,
					"variant", m_CurTheme->m_FontVariant,
					"stretch", m_CurTheme->m_FontStretch,
					"size", m_CurTheme->m_FontSize,
					NULL);
	g_signal_handler_unblock (G_OBJECT (m_FontSel), m_FontChanged);
	gtk_widget_set_sensitive (GTK_WIDGET (m_FontSel), rw);
	gtk_spin_button_set_value (m_ArrowLengthBtn, m_CurTheme->m_ArrowLength);
	gtk_widget_set_sensitive (GTK_WIDGET (m_ArrowLengthBtn), rw);
	gtk_spin_button_set_value (m_ArrowWidthBtn, m_CurTheme->m_ArrowWidth);
	gtk_widget_set_sensitive (GTK_WIDGET (m_ArrowWidthBtn), rw);
	gtk_spin_button_set_value (m_ArrowDistBtn, m_CurTheme->m_ArrowDist);
	gtk_widget_set_sensitive (GTK_WIDGET (m_ArrowDistBtn), rw);
	gtk_spin_button_set_value (m_ArrowPaddingBtn, m_CurTheme->m_ArrowPadding);
	gtk_widget_set_sensitive (GTK_WIDGET (m_ArrowPaddingBtn), rw);
	gtk_spin_button_set_value (m_ArrowHeadABtn, m_CurTheme->m_ArrowHeadA);
	gtk_widget_set_sensitive (GTK_WIDGET (m_ArrowHeadABtn), rw);
	gtk_spin_button_set_value (m_ArrowHeadBBtn, m_CurTheme->m_ArrowHeadB);
	gtk_widget_set_sensitive (GTK_WIDGET (m_ArrowHeadBBtn), rw);
	gtk_spin_button_set_value (m_ArrowHeadCBtn, m_CurTheme->m_ArrowHeadC);
	gtk_widget_set_sensitive (GTK_WIDGET (m_ArrowHeadCBtn), rw);
	gtk_spin_button_set_value (m_ScaleBtn, 1. / m_CurTheme->m_ZoomFactor);
	gtk_widget_set_sensitive (GTK_WIDGET (m_ScaleBtn), rw);
	gtk_spin_button_set_value (m_PaddingBtn, m_CurTheme->m_Padding);
	gtk_widget_set_sensitive (GTK_WIDGET (m_PaddingBtn), rw);
	gtk_spin_button_set_value (m_ObjectPaddingBtn, m_CurTheme->m_ObjectPadding);
	gtk_widget_set_sensitive (GTK_WIDGET (m_ObjectPaddingBtn), rw);
	gtk_spin_button_set_value (m_StoichPaddingBtn, m_CurTheme->m_StoichiometryPadding);
	gtk_widget_set_sensitive (GTK_WIDGET (m_StoichPaddingBtn), rw);
	gtk_spin_button_set_value (m_SignPaddingBtn, m_CurTheme->m_SignPadding);
	gtk_widget_set_sensitive (GTK_WIDGET (m_SignPaddingBtn), rw);
	gtk_spin_button_set_value (m_ChargeSizeBtn, m_CurTheme->m_ChargeSignSize);
	gtk_widget_set_sensitive (GTK_WIDGET (m_ChargeSizeBtn), rw);
	g_signal_handler_block (m_NameEntry, m_NameActivate);
	g_signal_handler_block (m_NameEntry, m_NameFocusOut);
	gtk_entry_set_text (m_NameEntry, _(m_CurTheme->m_Name.c_str ()));
	g_signal_handler_unblock (m_NameEntry, m_NameFocusOut);
	g_signal_handler_unblock (m_NameEntry, m_NameActivate);
	gtk_widget_set_sensitive (GTK_WIDGET (m_NameEntry), rw && m_CurTheme->m_ThemeType != DEFAULT_THEME_TYPE);
}

void PrefsDlg::OnBondLength (double length)
{
	if (length != m_CurTheme->m_BondLength) {
		m_CurTheme->m_BondLength = length;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "bond-length", length);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnBondAngle (double angle)
{
	if (angle != m_CurTheme->m_BondAngle) {
		m_CurTheme->m_BondAngle = angle;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "bond-angle", angle);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnBondWidth (double width)
{
	if (width != m_CurTheme->m_BondWidth) {
		m_CurTheme->m_BondWidth = width;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "bond-width", width);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnBondDist (double dist)
{
	if (dist != m_CurTheme->m_BondDist) {
		m_CurTheme->m_BondDist = dist;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "bond-dist", dist);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnStereoBondWidth (double width)
{
	if (width != m_CurTheme->m_StereoBondWidth) {
		m_CurTheme->m_StereoBondWidth = width;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "stereo-width", width);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnHashWidth (double width)
{
	if (width != m_CurTheme->m_HashWidth) {
		m_CurTheme->m_HashWidth = width;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "hash-width", width);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnHashDist (double dist)
{
	if (dist != m_CurTheme->m_HashDist) {
		m_CurTheme->m_HashDist = dist;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "hash-dist", dist);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnFont (GcpFontSel *fs)
{
	char *Name;
	PangoStyle Style;
	PangoWeight Weight;
	PangoStretch Stretch;
	PangoVariant Variant;
	int Size;
	bool changed = false;
	g_object_get (G_OBJECT (fs), "family", &Name, "style", &Style, "weight", &Weight, "stretch", &Stretch, "variant", &Variant, "size", &Size, NULL);

	if (strcmp (m_CurTheme->m_FontFamily, Name)) {
		g_free (m_CurTheme->m_FontFamily);
		m_CurTheme->m_FontFamily = Name;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_string (node, "font-family", Name);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
	if (m_CurTheme->m_FontStyle != Style) {
		m_CurTheme->m_FontStyle = Style;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_int (node, "font-style", get_fontstyle (Style));
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
	if (m_CurTheme->m_FontWeight != Weight) {
		m_CurTheme->m_FontWeight = Weight;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_int (node, "font-weight", get_fontweight (Weight));
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
	if (m_CurTheme->m_FontStretch != Stretch) {
		m_CurTheme->m_FontStretch = Stretch;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_int (node, "font-stretch", get_fontstretch (Stretch));
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
	if (m_CurTheme->m_FontVariant != Variant) {
		m_CurTheme->m_FontVariant = Variant;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_int (node, "font-variant", get_fontvariant (Variant));
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
	if (m_CurTheme->m_FontSize != Size) {
		m_CurTheme->m_FontSize = Size;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "font-size", get_fontsize (Size));
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
	if (changed)
		m_CurTheme->NotifyChanged ();
}

void PrefsDlg::OnTextFont (GcpFontSel *fs)
{
	char *Name;
	PangoStyle Style;
	PangoWeight Weight;
	PangoStretch Stretch;
	PangoVariant Variant;
	int Size;
	bool changed = false;
	g_object_get (G_OBJECT (fs), "family", &Name, "style", &Style, "weight", &Weight, "stretch", &Stretch, "variant", &Variant, "size", &Size, NULL);

	if (strcmp (m_CurTheme->m_TextFontFamily, Name)) {
		g_free (m_CurTheme->m_TextFontFamily);
		m_CurTheme->m_TextFontFamily = Name;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_string (node, "text-font-family", Name);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	} else
		g_free (Name);
	if (m_CurTheme->m_TextFontStyle != Style) {
		m_CurTheme->m_TextFontStyle = Style;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_int (node, "text-font-style", get_fontstyle (Style));
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
	if (m_CurTheme->m_TextFontWeight != Weight) {
		m_CurTheme->m_TextFontWeight = Weight;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_int (node, "text-font-weight", get_fontweight (Weight));
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
	if (m_CurTheme->m_TextFontStretch != Stretch) {
		m_CurTheme->m_TextFontStretch = Stretch;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_int (node, "text-font-stretch", get_fontstretch (Stretch));
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
	if (m_CurTheme->m_TextFontVariant != Variant) {
		m_CurTheme->m_TextFontVariant = Variant;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_int (node, "text-font-variant", get_fontvariant (Variant));
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
	if (m_CurTheme->m_TextFontSize != Size) {
		m_CurTheme->m_TextFontSize = Size;
		changed = true;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "text-font-size", get_fontsize (Size));
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
	if (changed)
		m_CurTheme->NotifyChanged ();
}

// arrows

void PrefsDlg::OnArrowLength (double length)
{
	if (length != m_CurTheme->m_ArrowLength) {
		m_CurTheme->m_ArrowLength = length;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "arrow-length", length);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnArrowWidth (double width)
{
	if (width != m_CurTheme->m_ArrowWidth) {
		m_CurTheme->m_ArrowWidth = width;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "arrow-width", width);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnArrowDist (double dist)
{
	if (dist != m_CurTheme->m_ArrowDist) {
		m_CurTheme->m_ArrowDist = dist;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "arrow-dist", dist);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnArrowPadding (double padding)
{
	if (padding != m_CurTheme->m_ArrowPadding) {
		m_CurTheme->m_ArrowPadding = padding;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "arrow-padding", padding);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
}

void PrefsDlg::OnArrowHeadA (double headA)
{
	if (headA != m_CurTheme->m_ArrowHeadA) {
		m_CurTheme->m_ArrowHeadA = headA;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "arrow-headA", headA);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnArrowHeadB (double headB)
{
	if (headB != m_CurTheme->m_ArrowHeadB) {
		m_CurTheme->m_ArrowHeadB = headB;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "arrow-headB", headB);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnArrowHeadC (double headC)
{
	if (headC != m_CurTheme->m_ArrowHeadC) {
		m_CurTheme->m_ArrowHeadC = headC;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "arrow-headC", headC);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnScale (double scale)
{
	double zoom = 1. / scale;
	if (zoom != m_CurTheme->m_ZoomFactor) {
		m_CurTheme->m_ZoomFactor = zoom;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "scale", scale);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
}

void PrefsDlg::OnPadding (double padding)
{
	if (padding != m_CurTheme->m_Padding) {
		m_CurTheme->m_Padding = padding;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "padding", padding);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
}

void PrefsDlg::OnObjectPadding (double padding)
{
	if (padding != m_CurTheme->m_ObjectPadding) {
		m_CurTheme->m_ObjectPadding = padding;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "object-padding", padding);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
}

void PrefsDlg::OnStoichPadding (double padding)
{
	if (padding != m_CurTheme->m_StoichiometryPadding) {
		m_CurTheme->m_StoichiometryPadding = padding;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "stoichiometry-padding", padding);
			go_conf_free_node (node);
		break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
}

void PrefsDlg::OnSignPadding (double padding)
{
	if (padding != m_CurTheme->m_SignPadding) {
		m_CurTheme->m_SignPadding = padding;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "sign-padding", padding);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
	}
}

void PrefsDlg::OnChargeSize(double size)
{
	if (size != m_CurTheme->m_ChargeSignSize) {
		m_CurTheme->m_ChargeSignSize = size;
		switch (m_CurTheme->m_ThemeType) {
		case DEFAULT_THEME_TYPE: {
			GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
			go_conf_set_double (node, "charge-sign-size", size);
			go_conf_free_node (node);
			break;
		}
		case LOCAL_THEME_TYPE:
			m_CurTheme->modified = true;
			break;
		default:
			break;
		}
		m_CurTheme->NotifyChanged ();
	}
}

void PrefsDlg::OnThemeNameChanged (char const *name)
{
	if (!name || !*name) {
		// No valid name found
		if (!gtk_window_has_toplevel_focus (GTK_WINDOW (dialog)))
			return;
		GtkWidget* box = gtk_message_dialog_new (GTK_WINDOW (dialog), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Invalid name");
		g_signal_handler_block (m_NameEntry, m_NameFocusOut);
		g_signal_connect (G_OBJECT (box), "response", G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show_all (box);
		g_signal_handler_unblock (m_NameEntry, m_NameFocusOut);
		gtk_window_set_focus (GTK_WINDOW (dialog), GTK_WIDGET (m_NameEntry));
		return;
	}
	GtkTreeIter iter, parent;
	gtk_tree_model_get_iter (GTK_TREE_MODEL (themes), &iter, m_Path);
	gtk_tree_model_iter_parent  (GTK_TREE_MODEL (themes), &parent, &iter);
	gtk_tree_store_set (themes, &parent, 0, name, -1);
	// delete old file
	if (m_CurTheme->m_ThemeType == LOCAL_THEME_TYPE) {
		xmlDocPtr doc = xmlNewDoc((xmlChar*)"1.0");
		xmlDocSetRootElement (doc,  xmlNewDocNode (doc, NULL, (xmlChar*) "chemistry", NULL));
		char *szhome = getenv ("HOME");
		string home, path;
		if (szhome)
			home = szhome;
		path = home + "/.gchempaint/themes";
		GDir *dir = g_dir_open (path.c_str (), 0, NULL);
		if (!dir) {
			string path;
			path = home + "/.gchempaint";
			dir = g_dir_open (path.c_str (), 0, NULL);
			if (dir)
				g_dir_close (dir);
			else
				mkdir (path.c_str (), 0x1ed);
			mkdir (path.c_str (), 0x1ed);
		} else {
			path += string ("/") + m_CurTheme->GetName ();
			remove (path.c_str ());
			g_dir_close (dir);
		}
		TheThemeManager.ChangeThemeName (m_CurTheme, name); // just set the name
		if (m_CurTheme->Save (doc)) {
			path = home + "/.gchempaint/themes/" + name;
			xmlSaveFormatFile (path.c_str (), doc, true);
			m_CurTheme->modified = false;
		}
	} else
		m_CurTheme->m_Name = name; // just set the name
	dynamic_cast <Application*> (m_App)->OnThemeNamesChanged ();
}

bool PrefsDlg::CheckError ()
{
	return (!*gtk_entry_get_text (m_NameEntry));
}

void PrefsDlg::SetDefaultTheme (char const *name)
{
	TheThemeManager.SetDefaultTheme (name);
	GOConfNode *node = go_conf_get_node (m_App->GetConfDir (), GCP_CONF_DIR_SETTINGS);
	go_conf_set_string (node, "default-theme", name);
	go_conf_free_node (node);
}

}	//	namespace gcp
