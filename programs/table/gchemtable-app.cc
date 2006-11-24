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
#include <libgnomevfs/gnome-vfs.h>
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

void on_family_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("family");
}

void on_acidity_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("acidity");
}

#endif

static void on_about_activate_url (GtkAboutDialog *about, const gchar *url, gpointer data)
{
	GnomeVFSResult error = gnome_vfs_url_show(url);
	if (error != GNOME_VFS_OK) {
		g_print("GnomeVFSResult while trying to launch URL in about dialog: error %u\n", error);
	}
}

static void on_about (GtkWidget *widget, GChemTableApp *app)
{
	const gchar * authors[] = {"Jean Bréfort", NULL};
	const gchar * comments = _("GChemTable is a chemical periodic table of the elements application");
	/* const gchar * documentors[] = {NULL}; */
	const gchar * copyright = "Copyright \xc2\xa9 2005,2006 Jean Bréfort";
	const gchar * license =
		"This program is free software; you can redistribute it and/or\n"
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
	
	gtk_about_dialog_set_url_hook(on_about_activate_url, NULL, NULL);

	/* Note to translators: replace the following string with the appropriate credits for you lang */
	const gchar * translator_credits = _("translator_credits");
	gtk_show_about_dialog (app->GetWindow (),
	                       "name", "GChemTable",
	                       "authors", authors,
	                       "comments", comments,
	                       "copyright", copyright,
	                       "license", license,
	                       "translator_credits", translator_credits,
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
		  G_CALLBACK (on_state_colors), },
	  {"FamilyColors", NULL, N_("Family"), NULL,
		  N_("Use colors to display the family grouping of the elements"),
                  G_CALLBACK (on_family_colors) },
	  {"AcidityColors", NULL, N_("Acidity"), NULL,
		  N_("Use colors to display the acidity of the elements"),
                  G_CALLBACK (on_acidity_colors) },
#endif
  { "HelpMenu", NULL, N_("_Help") },
	  { "About", GTK_STOCK_ABOUT, N_("_About"), NULL,
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
"        <menuitem action='FamilyColors'/>"
"        <menuitem action='AcidityColors'/>"
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

static void get_family_color (int Z, GdkColor *color, GChemTableApp *App)
{
	App->GetFamilyColor (Z, color);
}

static void get_acidity_color (int Z, GdkColor *color, GChemTableApp *App)
{
	App->GetAcidityColor (Z, color);
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

	colorschemes["family"] = gtk_periodic_add_color_scheme (periodic, (GtkPeriodicColorFunc) get_family_color, NULL, this);
	colorschemes["acidity"] = gtk_periodic_add_color_scheme (periodic, (GtkPeriodicColorFunc) get_acidity_color, NULL, this);

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

void GChemTableApp::GetFamilyColor (int Z, GdkColor *color)
{
	color->red= color->green = color->blue = 0;
        Element *elt = Element::GetElement (Z);
	std::string &value = elt->GetStringProperty ("family");
	if (!value.length())
		return;

/*
	Alkali_Earth
	Alkaline_Earth
	Non-Metal
	Metalloids
	Transition
	Other_Metal
	Halogene
	Noblegas
	Rare_Earth
*/
	
	if (value == "Alkali_Earth") {
		color->blue = 0x8eff;
		return;
	}

	if (value == "Alkaline_Earth") {
		color->blue = 0xffff;
		return;
	}

	if (value == "Non-Metal") {
		color->green = 0xffff;
		return;
	}

	if (value == "Metalloids") {
		color->green = 0x8eff;
		return;
	}

	if (value == "Transition") {
		color->red = 0xffff;
		color->green = 0xffff;
		return;
	}

	if (value == "Other_Metal") {
		color->red = 0xffff;
		color->green = 0x8eff;
		return;
	}

	if (value == "Halogene") {
		color->red = 0xffff;
		return;
	}

	if (value == "Noblegas") {
		color->red = 0x8eff; 
		return;
	}

	if (value == "Rare_Earth") {
		color->red = 0xffff;
		color->blue = 0xffff;
		return;
	}
}

void GChemTableApp::GetAcidityColor (int Z, GdkColor *color)
{
	color->red= color->green = color->blue = 0;
	Element *elt = Element::GetElement (Z);
	int value = elt->GetIntegerProperty ("acidicbehaviour");
	if (!value) 
		return;

/*
	0 means acidic
	1 means basic
	2 means neutral
	3 means amphoteric
*/

	switch (value {
	case 0:
		color->red = 0xffff;
		return;

	case 1:
		color->blue = 0xffff;
		return;

	case 2:
		color->green = 0xffff;
		return;

	case 3:
		color->red = 0xffff;
		color->blue = 0xffff;
		return;
	}
}
#endif
