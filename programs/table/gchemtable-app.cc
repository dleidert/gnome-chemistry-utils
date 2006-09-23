// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-app.cc 
 *
 * Copyright (C) 2005-2006 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include "gchemtable-app.h"
#include "gchemtable-elt.h"
#include <gcu/chemistry.h>
#include <gcu/element.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtkaboutdialog.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkwindow.h>
#include <stdlib.h>
#include <string.h>


static void on_quit (GtkWidget *widget, void *data)
{
	gtk_main_quit();
}

void on_no_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("none");
}

void on_default_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("default");
}

#ifdef WITH_BODR
void on_state_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("state");
}
#endif

static void on_about (GtkWidget *widget, GChemTableApp *app)
{
	char * authors[] = {"Jean Bréfort", NULL};
//	char * documentors[] = {NULL};
	char license[] = "This program is free software; you can redistribute it and/or\n" 
		"modify it under the terms of the GNU General Public License as\n"
 		"published by the Free Software Foundation; either version 2 of the\n"
		"License, or (at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful,\n"
		"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License\n"
		"along with this program; if not, write to the Free Software\n"
		"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307\n"
		"USA";
/* Note to translators: replace the following string with the appropriate credits for you lang */
	char *translator_credits = _("translator_credits");
	gtk_show_about_dialog (app->GetWindow (),
					"name", "GChemTable",
					"authors", authors,
					"comments", _("GChemTable is a chemical periodic table of the elements application"),
					"copyright", _("(C) 2005-2006 by Jean Bréfort"),
					"license", license,
					"translator_credits", strcmp (translator_credits, "translator_credits") != 0 ? 
											(const char *)translator_credits : NULL,
					"version", VERSION,
					"website", "http://www.nongnu.org/gchemutils",
					NULL);
}

void on_changed (GtkPeriodic* periodic, guint Z, GChemTableApp *app)
{
	app->OnElement (Z);
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File") },
	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GChemTable"), G_CALLBACK (on_quit) },
  { "ViewMenu", NULL, N_("_View") },
    {"ColorMenu", NULL, N_("Color scheme") },
	  {"NoColors", NULL, N_("No colors"), NULL,
		  N_("Use default Gtk theme colors"), G_CALLBACK (on_no_colors) },
	  {"DefaultColors", NULL, N_("Default"), NULL,
		  N_("Use default symbolic element colors"), G_CALLBACK (on_default_colors), },
#ifdef WITH_BODR
	  {"StateColors", NULL, N_("Physical states"), NULL,
		  N_("Use colors to display physical state at a given temperature"),
		  G_CALLBACK (on_state_colors) },
#endif
  { "HelpMenu", NULL, N_("_Help") },
	  { "About", NULL, N_("_About"), NULL,
		  N_("About GChemTable"), G_CALLBACK (on_about) }
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='ViewMenu'>"
"      <menu action='ColorMenu'>"
"        <menuitem action='NoColors'/>"
"        <menuitem action='DefaultColors'/>"
#ifdef WITH_BODR
"        <menuitem action='StateColors'/>"
#endif
"      </menu>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

#ifdef WITH_BODR
static void get_state_color (int Z, GdkColor *color, GChemTableApp *App)
{
	App->GetStateColor (Z, color);
}

void on_changed_temp (GtkRange *range, GChemTableApp *app)
{
	app->SetTemperature (gtk_range_get_value (range));
}
#endif

#warning "the following line should be edited for stable releases"
GChemTableApp::GChemTableApp (): Application ("gchemtable-unstable")
{
	GtkVBox* vbox;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(window), _("Periodic table of the elements"));
	g_signal_connect (G_OBJECT (window), "destroy",
		 G_CALLBACK (gtk_main_quit),
		 NULL);

	g_object_set (G_OBJECT(window), "allow-shrink", FALSE, NULL);
	
	vbox = (GtkVBox*)gtk_vbox_new(FALSE, 0);
	// add menus
	GtkUIManager *ui_manager = gtk_ui_manager_new ();
	GtkActionGroup *action_group = gtk_action_group_new ("MenuActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);
	gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), this);
	gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
	GtkAccelGroup *accel_group = gtk_ui_manager_get_accel_group (ui_manager);
	gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
	GError *error = NULL;
	if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error)) {
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		exit (EXIT_FAILURE);
	}
	GtkWidget *bar = gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
	gtk_box_pack_start (GTK_BOX (vbox), bar, FALSE, FALSE, 0);
	periodic = GTK_PERIODIC (gtk_periodic_new());
	g_object_set(G_OBJECT(periodic),
			"color-style", GTK_PERIODIC_COLOR_DEFAULT,
			"can_unselect", true,
			NULL);
	g_signal_connect(G_OBJECT(periodic), "element_changed", (GCallback)on_changed, this);
	gtk_box_pack_end_defaults(GTK_BOX(vbox), GTK_WIDGET(GTK_PERIODIC(periodic)));
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));
	gtk_widget_show_all(window);
	for (int i = 0; i < 118; i++)
		Pages[i] = NULL;
	
	gcu::Element::LoadAllData ();
	colorschemes["none"] = GTK_PERIODIC_COLOR_NONE;
	colorschemes["default"] = GTK_PERIODIC_COLOR_DEFAULT;
#ifdef WITH_BODR
	GladeXML *xml = glade_xml_new (GLADEDIR"/state-thermometer.glade", "state-thermometer", NULL);
	GtkWidget *thermometer = glade_xml_get_widget (xml, "state-thermometer");
	colorschemes["state"] = gtk_periodic_add_color_scheme (periodic, (GtkPeriodicColorFunc) get_state_color, thermometer, this);
	gtk_widget_show_all (thermometer);
	thermometer = glade_xml_get_widget (xml, "temperature");
	g_signal_connect (G_OBJECT (thermometer), "value-changed", G_CALLBACK (on_changed_temp), this);
	temperature = gtk_range_get_value (GTK_RANGE (thermometer));
#endif
}

GChemTableApp::~GChemTableApp ()
{
}

void GChemTableApp::OnElement (int Z)
{
	m_CurZ = Z;
	if (Z == 0)
		return;
	int t = Z - 1;
	if (Pages[t] != NULL)
		gtk_window_present (Pages[t]->GetWindow ());
	else
		Pages[t] = new GChemTableElt (this, Z);
}

void GChemTableApp::ClearPage (int Z)
{
	Pages[Z - 1] = NULL;
	gtk_periodic_set_element (periodic, 0);
	if (Z == m_CurZ) {
		SetCurZ (0);
	}
}

void GChemTableApp::SetCurZ (int Z)
{
	if (Z != m_CurZ) {
		gtk_periodic_set_element (periodic, Z);
		m_CurZ = Z;
	}
}

void GChemTableApp::SetColorScheme (char const *name)
{
	g_object_set (G_OBJECT (periodic), "color-style", colorschemes[name], NULL);
}

#ifdef WITH_BODR
void GChemTableApp::SetTemperature (double T)
{
	temperature = T;
	gtk_periodic_set_colors (periodic);
}

void GChemTableApp::GetStateColor (int Z, GdkColor *color)
{
	color->red= color->green = color->blue = 0;
	Element *elt = Element::GetElement (Z);
	Value *value = elt->GetProperty ("meltingpoint");
	if (!value)
		return;
	double t = value->GetAsDouble ();
	if (t > temperature) {
		color->blue = 0xffff;
		return;
	}
	value = elt->GetProperty ("boilingpoint");
	if (!value)
		return;
	t = value->GetAsDouble ();
	if (t > temperature) {
		color->green = 0xffff;
		return;
	}
	color->red = 0xffff;
}

#endif
