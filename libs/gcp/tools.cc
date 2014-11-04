// -*- C++ -*-

/*
 * GChemPaint library
 * tools.cc
 *
 * Copyright (C) 2001-2011 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
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
#include <glib/gi18n-lib.h>
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
/*
Tools::Tools (Application *App):
	gcugtk::Dialog (App, UIDIR"/tools.ui", "tools", GETTEXT_PACKAGE, App),
	m_Parent (NULL)
{
	g_signal_connect (G_OBJECT (dialog), "delete-event", G_CALLBACK (on_delete_event), NULL);
	Application *pApp = dynamic_cast<Application*> (App);
	m_ButtonsGrid = GTK_GRID (GetWidget ("tools-buttons"));
	m_Book = GTK_NOTEBOOK (GetWidget ("tools-book"));
	GtkWidget *grid = GetWidget ("element-grid");
	GtkWidget *w = gcu_combo_periodic_new ();
	m_Mendeleiev = reinterpret_cast <GcuComboPeriodic *> (w);
	gtk_container_add (GTK_CONTAINER (grid), w);
	gcu_combo_periodic_set_element (GCU_COMBO_PERIODIC (w), pApp->GetCurZ ());
	g_signal_connect_swapped (G_OBJECT (w), "changed", G_CALLBACK (element_changed_cb), this);
	w = GetWidget ("help-btn");
	g_signal_connect_swapped (G_OBJECT (w), "clicked", G_CALLBACK (help_cb), this);
	gtk_widget_show_all (GTK_WIDGET (dialog));
}
*/
class Toolbar {
public:
	Toolbar ();

	GtkToolbar *bar;
	std::vector < unsigned > sections;
};

Toolbar::Toolbar ():
	bar (NULL)
{
}

static void tool_toggled_cb (GtkToggleToolButton *btn, Application *app)
{
	if (gtk_toggle_tool_button_get_active (btn))
		app->OnToolChanged (gtk_widget_get_name (GTK_WIDGET (btn)));
}

Tools::Tools (Application *app, std::list < ToolDesc const * > const &descs):
	gcugtk::Dialog (app, UIDIR"/tools.ui", "tools", GETTEXT_PACKAGE, app),
	m_Parent (NULL)
{
	std::list < ToolDesc const * >::const_iterator d, end = descs.end ();
	std::vector < Toolbar > toolbars;
	int i;
	GtkToolItem *btn;
	GSList *group = NULL;
	m_ButtonsGrid = GTK_GRID (GetWidget ("tools-buttons"));
	toolbars.resize (MaxToolbar);
	for (i = 0; i < MaxToolbar; i++) {
		GtkWidget *bar = gtk_toolbar_new ();
		toolbars[i].bar = GTK_TOOLBAR (bar);
		gtk_toolbar_set_style (toolbars[i].bar, GTK_TOOLBAR_ICONS);
		gtk_toolbar_set_show_arrow (toolbars[i].bar, false);
		gtk_container_add (GTK_CONTAINER (m_ButtonsGrid), bar);
	}
	for (d = descs.begin(); d != end; d++) {
		ToolDesc const *desc = *d;
		while (desc->name != NULL) {
			bool separator = false;
			if (!strcmp (desc->name, "Separator"))
				separator = true;
			else if (desc->widget == NULL && desc->icon_name == NULL) {
				desc++;
				continue;
			}
			if (separator)
				btn = gtk_separator_tool_item_new ();
			else {
				btn = gtk_radio_tool_button_new (group);
				group = gtk_radio_tool_button_get_group (GTK_RADIO_TOOL_BUTTON (btn));

				GtkWidget *w;
				if (desc->widget) {
					w = desc->widget;
					gtk_widget_set_size_request (w, 24, 24);
				} else
					w = gtk_image_new_from_icon_name (desc->icon_name, GTK_ICON_SIZE_LARGE_TOOLBAR);
				gtk_tool_button_set_icon_widget (GTK_TOOL_BUTTON (btn), w);
				gtk_widget_set_name (GTK_WIDGET (btn), desc->name);
				g_signal_connect (G_OBJECT (btn), "toggled", G_CALLBACK (tool_toggled_cb), app);
				gtk_tool_item_set_tooltip_text (btn, _(desc->tip));
				app->SetToolItem (desc->name, reinterpret_cast < GtkWidget * > (btn));
				m_Pages[app->GetTool (desc->name)] = -1;
				m_Widgets[desc->name] = reinterpret_cast < GtkWidget * > (btn);
			}
			unsigned n = toolbars[desc->bar].sections.size (), p;
			if (n < desc->group) {
				toolbars[desc->bar].sections.resize (desc->group);
				p = gtk_toolbar_get_n_items (toolbars[desc->bar].bar);
				while (n < desc->group) {
					toolbars[desc->bar].sections[n] = p;
					n++;
				}
			}
			if (desc->group == toolbars[desc->bar].sections.size ())
				i = -1;
			else {
				i = toolbars[desc->bar].sections[p = desc->group];
				n = toolbars[desc->bar].sections.size ();
				while (p < n)
					toolbars[desc->bar].sections[p++]++;
			}
			gtk_toolbar_insert (toolbars[desc->bar].bar,  btn, i);
			desc++;
		}
	}
	gtk_widget_show_all (GTK_WIDGET (m_ButtonsGrid));
	m_Book = GTK_NOTEBOOK (GetWidget ("tools-book"));
	GtkWidget *grid = GetWidget ("element-grid");
	GtkWidget *w = gcu_combo_periodic_new ();
	m_Mendeleiev = reinterpret_cast <GcuComboPeriodic *> (w);
	gtk_container_add (GTK_CONTAINER (grid), w);
	gcu_combo_periodic_set_element (GCU_COMBO_PERIODIC (w), app->GetCurZ ());
	g_signal_connect_swapped (G_OBJECT (w), "changed", G_CALLBACK (element_changed_cb), this);
	w = GetWidget ("help-btn");
	g_signal_connect_swapped (G_OBJECT (w), "clicked", G_CALLBACK (help_cb), this);
	g_signal_connect (G_OBJECT (dialog), "delete-event", G_CALLBACK (on_delete_event), NULL);
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

Tools::~Tools ()
{
}

void Tools::Show (bool visible, GtkWindow *parent)
{
	if (visible) {
		if (m_Parent != parent)
			gtk_window_set_transient_for (dialog, m_Parent = parent);
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

void Tools::SetPage (Tool *tool, int i)
{
	m_Pages[tool] = i;
}

void Tools::OnSelectTool (Tool *tool)
{
	// make sure that the button is checked
	std::string &name = tool->GetName ();
	GtkToggleToolButton *btn = reinterpret_cast < GtkToggleToolButton * > (m_Widgets[name]);
	if (!gtk_toggle_tool_button_get_active (btn))
		gtk_toggle_tool_button_set_active (btn, true);
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
	if (strncmp (name, "Gtk", 3)) {
		App->SetToolItem (name, w);
		Tool *tool = App->GetTool (name);
		m_Pages[tool] = -1;
	}
}

void Tools::OnElementChanged (int Z)
{
	dynamic_cast<Application*> (m_App)->SetCurZ (Z);
	// Hmm, we should find a better way to do the following, the path might change.
	GtkWidget *w = m_Widgets["Element"];
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

void Tools::OnHelp ()
{
	m_App->OnHelp (m_Tool->GetHelpTag ());
}

void Tools::SetElement (int Z)
{
	gcu_combo_periodic_set_element (m_Mendeleiev, Z);
	OnElementChanged (Z);
}

}	//	namespace gcp
