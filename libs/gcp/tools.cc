// -*- C++ -*-

/* 
 * GChemPaint library
 * tools.cc 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "application.h"
#include "settings.h"
#include "tool.h"
#include "tools.h"
#include <gcu/element.h>
#include <gcu/gtkcomboperiodic.h>
#include <cstring>

using namespace gcu;
using namespace std;

namespace gcp {

static void element_changed_cb (Tools *box, unsigned newZ)
{
	box->OnElementChanged (newZ);
}

static bool on_delete_event (G_GNUC_UNUSED GtkWidget* widget, G_GNUC_UNUSED GdkEvent *event, G_GNUC_UNUSED gpointer data)
{
	return true;
}

static void help_cb (Tools *box)
{
	box->OnHelp ();
}

Tools::Tools (Application *App):
	Dialog (App, UIDIR"/tools.ui", "tools", GETTEXT_PACKAGE, App),
	m_UIManager (NULL)
{
	g_signal_connect (G_OBJECT (dialog), "delete-event", G_CALLBACK (on_delete_event), NULL);
	Application *pApp = dynamic_cast<Application*> (App);
	m_UIManager = NULL;
	m_ButtonsBox = GTK_BOX (GetWidget ("tools-buttons"));
	m_Book = GTK_NOTEBOOK (GetWidget ("tools-book"));
	GtkWidget *box = GetWidget ("element-box");
	GtkWidget *w = gtk_combo_periodic_new ();
	gtk_box_pack_start (GTK_BOX (box), w, false, false, 0);
	gtk_combo_periodic_set_element (GTK_COMBO_PERIODIC (w), pApp->GetCurZ ());
	go_combo_box_set_tearable (GO_COMBO_BOX (w), TearableMendeleiev);
	g_signal_connect_swapped (G_OBJECT (w), "changed", G_CALLBACK (element_changed_cb), this);
	w = GetWidget ("help-btn");
	g_signal_connect_swapped (G_OBJECT (w), "clicked", G_CALLBACK (help_cb), this);
}

Tools::~Tools ()
{
	if (m_UIManager) {
		g_object_unref (m_UIManager);
		m_UIManager = NULL;
	}
}

void Tools::Show (bool visible)
{
	if (visible) {
		gtk_widget_show (GTK_WIDGET (dialog));
		GtkWindow *w = reinterpret_cast<Application*> (m_App)->GetWindow ();
		if (w)
			gtk_window_present (w);
	} else
		gtk_widget_hide (GTK_WIDGET (dialog));
}

void register_item_cb (GtkWidget *w, Tools *Dlg)
{
	Dlg->RegisterTool (w);
}

void Tools::AddToolbar (string &name)
{
	if (m_UIManager) {
		GtkWidget *w = gtk_ui_manager_get_widget (m_UIManager, name.c_str ()),
			*h = gtk_handle_box_new ();
		gtk_container_foreach (GTK_CONTAINER (w), (GtkCallback) register_item_cb, this);
		gtk_toolbar_set_style (GTK_TOOLBAR (w), GTK_TOOLBAR_ICONS);
		gtk_toolbar_set_show_arrow (GTK_TOOLBAR (w), false);
		gtk_container_add (GTK_CONTAINER (h), w);
		gtk_box_pack_start (m_ButtonsBox, h, true, true, 0);
		gtk_widget_show_all (h);
	}
}

void Tools::SetUIManager (GtkUIManager *manager)
{
	m_UIManager = manager;
	g_object_ref (m_UIManager);
}

void Tools::SetPage (Tool *tool, int i)
{
	m_Pages[tool] = i;
}

void Tools::OnSelectTool (Tool *tool)
{
	if (m_Pages[tool] < 0) {
		GtkWidget *w = tool->GetPropertyPage ();
		if (w)
			m_Pages[tool] = gtk_notebook_append_page (m_Book, w, NULL);
		else
			m_Pages[tool] = 0;
	}
	gtk_notebook_set_current_page (m_Book, m_Pages[tool]);
	m_Tool = tool;
}

void Tools::RegisterTool (GtkWidget *w)
{
	char const *name = gtk_widget_get_name (w);
	Application *App = dynamic_cast<Application*> (m_App);
	if (strncmp (name, "Gtk", 3))
		App->SetToolItem (name, w);
	Tool *tool = App->GetTool (name);
	m_Pages[tool] = -1;
}

void Tools::OnElementChanged (int Z)
{
	dynamic_cast<Application*> (m_App)->SetCurZ (Z);
	// Hmm, we should find a better way to do the following, the path might change.
	GtkWidget *w = gtk_ui_manager_get_widget (m_UIManager, "ui/AtomsToolbar/Atom1/Element");
	if (!w)
		return;
	GtkWidget* icon = gtk_tool_button_get_icon_widget (GTK_TOOL_BUTTON (w));
	if (!GTK_IS_LABEL (icon)) {
		icon = gtk_label_new (Element::Symbol (Z));
		gtk_widget_show (icon);
		gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (w), icon);
		gtk_widget_show_all (w);
	} else
		gtk_label_set_text (GTK_LABEL (icon), Element::Symbol (Z));
}

void Tools::Update (void)
{
	GtkWidget *w = GetWidget ("mendeleiev");
	go_combo_box_set_tearable (GO_COMBO_BOX (w), TearableMendeleiev);
}

void Tools::OnHelp ()
{
	m_App->OnHelp (m_Tool->GetHelpTag ());
}

void Tools::SetElement (int Z)
{
	GtkComboPeriodic *w = reinterpret_cast<GtkComboPeriodic*> (GetWidget ("mendeleiev"));
	gtk_combo_periodic_set_element (w, Z);
	OnElementChanged (Z);
}

}	//	namespace gcp
