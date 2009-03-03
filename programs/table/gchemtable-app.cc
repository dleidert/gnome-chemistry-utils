// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-app.cc 
 *
 * Copyright (C) 2005-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include "gchemtable-app.h"
#include "gchemtable-curve.h"
#include "gchemtable-data.h"
#include "gchemtable-elt.h"
#include <gcu/chemistry.h>
#include <gcu/element.h>
#include <gcu/filechooser.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gtk/gtkaboutdialog.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkwindow.h>
#include <gio/gio.h>
#include <cstdlib>
#include <cstring>
#include <list>
#include <string>

using namespace std;

static void on_quit (GtkWidget *widget, GChemTableApp *App)
{
	delete App;
	gtk_main_quit();
}

static void on_new_chart (GtkWidget *widget, GChemTableApp *App)
{
	App->OnNewChart ();
}

static void on_no_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("none");
}

void on_default_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("default");
}

static void on_state_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("state");
}

static void on_family_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("family");
}

void on_acidity_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("acidity");
}

void on_electroneg_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("electroneg");
}

static void on_radius_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("radius");
}

static void on_block_colors (GtkWidget *widget, GChemTableApp *App)
{
	App->SetColorScheme ("block");
}

static void on_about_activate_url (GtkAboutDialog *about, const gchar *url, gpointer data)
{
	reinterpret_cast <Application *> (data)->OnWeb (url);
}

static void on_help (GtkWidget *widget, GChemTableApp *app)
{
	app->OnHelp ();
}

static void on_web (GtkWidget *widget, GChemTableApp *app)
{
	app->OnWeb ();
}

static void on_mail (GtkWidget *widget, GChemTableApp *app)
{
	app->OnMail ();
}

static void on_live_assistance (GtkWidget *widget, GChemTableApp *app)
{
	app->OnLiveAssistance ();
}

static void on_bug (GtkWidget *widget, GChemTableApp *app)
{
	app->OnBug ();
}

static void on_about (GtkWidget *widget, GChemTableApp *app)
{
	app->OnAbout ();
}

static void on_changed (GtkPeriodic* periodic, guint Z, GChemTableApp *app)
{
	app->OnElement (Z);
}

static GtkActionEntry entries[] = {
  { "FileMenu", NULL, N_("_File") },
	  { "NewChart", NULL, N_("New _Chart"), "<control>N",
		  N_("Create a new chart"), G_CALLBACK (on_new_chart) },
	  { "Quit", GTK_STOCK_QUIT, N_("_Quit"), "<control>Q",
		  N_("Quit GChemTable"), G_CALLBACK (on_quit) },
  { "ViewMenu", NULL, N_("_View") },
    {"ColorMenu", NULL, N_("Color scheme") },
	  {"NoColors", NULL, N_("No colors"), NULL,
		  N_("Use default Gtk theme colors"), G_CALLBACK (on_no_colors) },
	  {"DefaultColors", NULL, N_("Default"), NULL,
		  N_("Use default symbolic element colors"), G_CALLBACK (on_default_colors), },
	  {"StateColors", NULL, N_("Physical states"), NULL,
		  N_("Use colors to display physical state at a given temperature"),
		  G_CALLBACK (on_state_colors), },
	  {"FamilyColors", NULL, N_("Family"), NULL,
		  N_("Use colors to display the family grouping of the elements"),
                  G_CALLBACK (on_family_colors) },
	  {"AcidityColors", NULL, N_("Acidity"), NULL,
		  N_("Use colors to display the acidity of the elements"),
                  G_CALLBACK (on_acidity_colors) },
	  {"ElectronegColors", NULL, N_("Electronegativity"), NULL,
		  N_("Use colors to display the electronegativity of the elements"),
                  G_CALLBACK (on_electroneg_colors) },
	  {"RadiusColors", NULL, N_("Atomic radius"), NULL,
		  N_("Use colors to display the covalent radii of the elements"),
                  G_CALLBACK (on_radius_colors) },
	  {"BlockColors", NULL, N_("Block"), NULL,
		  N_("Use colors to display the blocks elements belong to"),
                  G_CALLBACK (on_block_colors) },
  { "HelpMenu", NULL, N_("_Help") },
	  { "Help", GTK_STOCK_HELP, N_("_Contents"), "F1",
		  N_("View help for the Periodic Table"), G_CALLBACK (on_help) },
	  { "Web", NULL, N_("Gnome Chemistry Utils on the _web"), NULL,
		  N_("Browse the Gnome Chemistry Utils's web site"), G_CALLBACK (on_web) },
	  { "LiveAssistance", NULL, N_("Live assistance"), NULL,
		  N_("Open the Gnome Chemistry Utils IRC channel"), G_CALLBACK (on_live_assistance) },
	  { "Mail", NULL, N_("_Ask a question"), NULL,
		  N_("Ask a question about the Gnome Chemistry Utils"), G_CALLBACK (on_mail) },
	  { "Bug", NULL, N_("Report _Bugs"), NULL,
		  N_("Submit a bug report for the Gnome Chemistry Utils"), G_CALLBACK (on_bug) },
	  { "About", GTK_STOCK_ABOUT, N_("_About"), NULL,
		  N_("About GChemTable"), G_CALLBACK (on_about) }
};

static const char *ui_description =
"<ui>"
"  <menubar name='MainMenu'>"
"    <menu action='FileMenu'>"
"	   <separator name='file-sep1'/>"
"      <menuitem action='NewChart'/>"
"	   <separator name='file-sep2'/>"
"      <menuitem action='Quit'/>"
"    </menu>"
"    <menu action='ViewMenu'>"
"      <menu action='ColorMenu'>"
"        <menuitem action='NoColors'/>"
"        <menuitem action='DefaultColors'/>"
"        <menuitem action='StateColors'/>"
"        <menuitem action='FamilyColors'/>"
//"        <menuitem action='AcidityColors'/>"
"        <menuitem action='ElectronegColors'/>"
"        <menuitem action='RadiusColors'/>"
"        <menuitem action='BlockColors'/>"
"      </menu>"
"    </menu>"
"    <menu action='HelpMenu'>"
"      <menuitem action='Help'/>"
"      <menuitem action='Mail'/>"
"      <menuitem action='Web'/>"
"      <menuitem action='LiveAssistance'/>"
"      <menuitem action='Bug'/>"
"      <menuitem action='About'/>"
"    </menu>"
"  </menubar>"
"</ui>";

static void get_state_color (int Z, GdkColor *color, GChemTableApp *App)
{
	App->GetStateColor (Z, color);
}

static void on_changed_temp (GtkRange *range, GChemTableApp *app)
{
	app->SetTemperature (gtk_range_get_value (range));
}

static void on_changed_family (GtkComboBox *box,  GChemTableApp *app)
{
	app->SetFamily (gtk_combo_box_get_active (box));
}

static void get_family_color (int Z, GdkColor *color, GChemTableApp *App)
{
	App->GetFamilyColor (Z, color);
}

static void get_acidity_color (int Z, GdkColor *color, GChemTableApp *App)
{
	App->GetAcidityColor (Z, color);
}

static void get_electroneg_color (int Z, GdkColor *color, GChemTableApp *App)
{
	App->GetElectronegColor (Z, color);
}

static void get_radius_color (int Z, GdkColor *color, GChemTableApp *App)
{
	App->GetRadiusColor (Z, color);
}

static void get_block_color (int Z, GdkColor *color, GChemTableApp *App)
{
	App->GetBlockColor (Z, color);
}

static void on_destroy (GChemTableApp *App)
{
	delete App;
	gtk_main_quit ();
}

GChemTableApp::GChemTableApp (): Application ("gchemtable")
{
	GtkVBox* vbox;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW(window), _("Periodic table of the elements"));
	g_signal_connect_swapped (G_OBJECT (window), "delete-event",
		 G_CALLBACK (on_destroy),
		 this);

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
	gtk_box_pack_end (GTK_BOX(vbox), GTK_WIDGET(GTK_PERIODIC(periodic)), true, true, 0);
	gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(vbox));
	gtk_widget_show_all(window);
	for (int i = 0; i < 118; i++)
		Pages[i] = NULL;
	
	gcu::Element::LoadAllData ();
	colorschemes["none"] = GTK_PERIODIC_COLOR_NONE;
	colorschemes["default"] = GTK_PERIODIC_COLOR_DEFAULT;

	GladeXML *xml = glade_xml_new (GLADEDIR"/state-thermometer.glade", "state-thermometer", NULL);
	GtkWidget *thermometer = glade_xml_get_widget (xml, "state-thermometer");
	colorschemes["state"] = gtk_periodic_add_color_scheme (periodic, (GtkPeriodicColorFunc) get_state_color, thermometer, this);
	gtk_widget_show_all (thermometer);
	thermometer = glade_xml_get_widget (xml, "temperature");
	g_signal_connect (G_OBJECT (thermometer), "value-changed", G_CALLBACK (on_changed_temp), this);
	temperature = gtk_range_get_value (GTK_RANGE (thermometer));

	GladeXML *familyxml = glade_xml_new (GLADEDIR"/family.glade", "family-legend", NULL);
	GtkWidget *familywidget = glade_xml_get_widget (familyxml, "family-legend");
	colorschemes["family"] = gtk_periodic_add_color_scheme (periodic, (GtkPeriodicColorFunc) get_family_color, familywidget, this);	
	gtk_widget_show_all (familywidget);
	familywidget = glade_xml_get_widget (familyxml, "family-box");
	gtk_combo_box_set_active (GTK_COMBO_BOX(familywidget), 0);
	family = -1;
	g_signal_connect (G_OBJECT (familywidget), "changed", G_CALLBACK (on_changed_family), this);

	GladeXML *acidityxml = glade_xml_new (GLADEDIR"/acidity.glade", "acidity-legend", NULL);
	GtkWidget *aciditylegend = glade_xml_get_widget (acidityxml, "acidity-legend");
	colorschemes["acidity"] = gtk_periodic_add_color_scheme (periodic, (GtkPeriodicColorFunc) get_acidity_color, aciditylegend, this);
	gtk_widget_show_all (aciditylegend);

	colorschemes["electroneg"] = gtk_periodic_add_color_scheme (periodic, (GtkPeriodicColorFunc) get_electroneg_color, NULL, this);

	colorschemes["radius"] = gtk_periodic_add_color_scheme (periodic, (GtkPeriodicColorFunc) get_radius_color, NULL, this);

	GladeXML *blockxml = glade_xml_new (GLADEDIR"/block.glade", "block-legend", NULL);
	GtkWidget *blocklegend = glade_xml_get_widget (blockxml, "block-legend");
	colorschemes["block"] = gtk_periodic_add_color_scheme (periodic, (GtkPeriodicColorFunc) get_block_color, blocklegend, this);
	gtk_widget_show_all (blocklegend);
	gct_data_init ();

}

GChemTableApp::~GChemTableApp ()
{
	gct_data_clear ();
}

void GChemTableApp::OnAbout ()
{
	const gchar * authors[] = {"Jean Bréfort", NULL};
	const gchar * comments = _("GChemTable is a chemical periodic table of the elements application");
	/* const gchar * documentors[] = {NULL}; */
	const gchar * copyright = _("Copyright © 2005-2008 Jean Bréfort");
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
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307\n"
		"USA";
	
	gtk_about_dialog_set_url_hook (on_about_activate_url, this, NULL);

	/* Note to translators: replace the following string with the appropriate credits for you lang */
	const gchar * translator_credits = _("translator_credits");
	gtk_show_about_dialog (GetWindow (),
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

void GChemTableApp::SetTemperature (double T)
{
	temperature = T;
	gtk_periodic_set_colors (periodic);
}

void GChemTableApp::SetFamily (int family_N)
{
	family = (family_N)? 1 << (family_N - 1): -1;
	gtk_periodic_set_colors (periodic);
}

void GChemTableApp::GetStateColor (int Z, GdkColor *color)
{
	color->red= color->green = color->blue = 0;
	Element *elt = Element::GetElement (Z);
	Value const *value = elt->GetProperty ("meltingpoint");
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
		if (family & 1)
			color->blue = 0x8eff;
		return;
	}

	if (value == "Alkaline_Earth") {
		if (family & 2)
			color->blue = 0xffff;
		return;
	}

	if (value == "Non-Metal") {
		if (family & 0x20)
			color->green = 0xffff;
		return;
	}

	if (value == "Metalloids") {
		if (family & 8)
			color->green = 0x8eff;
		return;
	}

	if (value == "Transition") {
		if (family & 0x80) {
			color->red = 0xffff;
			color->green = 0xffff;
		}
		return;
	}

	if (value == "Other_Metal") {
		if (family & 0x100) {
			color->red = 0xffff;
			color->green = 0x8eff;
		}
		return;
	}

	if (value == "Halogene") {
		if (family & 4)
			color->red = 0xffff;
		return;
	}

	if (value == "Noblegas") {
		if (family & 0x10)
			color->red = 0x8eff; 
		return;
	}

	if (value == "Rare_Earth") {
		if (family & 0x40) {
			color->red = 0xffff;
			color->blue = 0xffff;
		}
		return;
	}
}

void GChemTableApp::GetAcidityColor (int Z, GdkColor *color)
{
	color->red= color->green = color->blue = 0;
	Element *elt = Element::GetElement (Z);
	int value = elt->GetIntegerProperty ("acidicbehaviour");
	if (value == GCU_ERROR)
		return;

/*
	0 means acidic
	1 means basic
	2 means neutral
	3 means amphoteric
*/

	switch (value) {
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

void GChemTableApp::GetElectronegColor (int Z, GdkColor *color)
{
	double max=3.98;
	double min=0.7;
	double limit;

	color->red= color->green = color->blue = 0;
	Element *elt = Element::GetElement (Z);
	Value const *value = elt->GetProperty ("electronegativityPauling");
	if (!value)
		return;

	double en = value->GetAsDouble ();

	limit = 0.5 * (max - min);

	if (en < limit) {
		color->red = 0xffff;
		color->blue = static_cast<guint16> ((en - min) * 0xffff / (limit - min));
	} else {
		color->blue = 0xffff;
		color->red= static_cast<guint16> ((en - max) * 0xffff / (limit - max));
	}

}

void GChemTableApp::GetRadiusColor (int Z, GdkColor *color)
{
	double max=2.25;
	double min=0.32;
	double limit;

	color->red = color->green = color->blue = 0;
	Element *elt = Element::GetElement (Z);
	Value const *value = elt->GetProperty ("radiusCovalent");
	if (!value)
		return;

	double radius = value->GetAsDouble ();

	limit = 0.5 * (max - min);

	if (radius < limit) {
		color->red = 0xffff;
		color->blue = static_cast<guint16> ((radius - min) * 0xffff / (limit - min));
	} else {
		color->blue = 0xffff;
		color->red= static_cast<guint16> ((radius - max) * 0xffff / (limit - max));
	}

}

void GChemTableApp::GetBlockColor (int Z, GdkColor *color)
{
	color->red= color->green = color->blue = 0;
        Element *elt = Element::GetElement (Z);
	std::string &value = elt->GetStringProperty ("periodTableBlock");
	if (!value.length())
		return;

	if (value == "s") {
		color->blue = 0x8eff;
		return;
	}

	if (value == "p") {
		color->red = 0x8eff;
		return;
	}

	if (value == "d") {
		color->green = 0x8eff;
		return;
	}

	if (value == "f") {
		color->blue = 0x8eff;
		color->red = 0x8eff;
		return;
	}
}

void GChemTableApp::OnNewChart ()
{
	new GChemTableCurve (this, NULL);
}

void GChemTableApp::OnSaveAsImage (GChemTableCurve *curve)
{
	if (!curve)
		return;
	list<string> l;
	unsigned n = 0;
	char const *mime;
	map<string, GdkPixbufFormat*>::iterator i, end = m_SupportedPixbufFormats.end ();
	for (i = m_SupportedPixbufFormats.begin (); i != end; i++)
		l.push_front ((*i).first.c_str ());
	if (go_image_get_format_from_name ("eps") != GO_IMAGE_FORMAT_UNKNOWN) {
		mime = go_image_format_to_mime ("eps");
		if (mime)
			l.push_front (mime);
	}
	l.push_front ("application/postscript");
	l.push_front ("application/pdf");
	l.push_front ("image/svg+xml");
	FileChooser (this, true, l, reinterpret_cast <Document *> (curve), _("Save as image"), GetImageSizeWidget ());
}

bool GChemTableApp::FileProcess (const gchar* filename, const gchar* mime_type, bool bSave, GtkWindow *window, Document *Doc)
{
	GChemTableCurve *curve = reinterpret_cast <GChemTableCurve *> (Doc);
	if(bSave) {
		GFile *file = g_file_new_for_uri (filename);
		bool err = g_file_query_exists (file, NULL);
		gint result = GTK_RESPONSE_YES;
		if (err) {
			char *unescaped = g_uri_unescape_string (filename, NULL);
			gchar * message = g_strdup_printf (_("File %s\nexists, overwrite?"), unescaped);
			g_free (unescaped);
			GtkDialog* Box = GTK_DIALOG (gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, message));
			gtk_window_set_icon_name (GTK_WINDOW (Box), "gspectrum");
			result = gtk_dialog_run (Box);
			gtk_widget_destroy (GTK_WIDGET (Box));
			g_free (message);
		}
		if (result == GTK_RESPONSE_YES) {
			g_file_delete (file, NULL, NULL);
			curve->SaveAsImage (filename, mime_type, GetImageWidth (), GetImageHeight ());
		}
		g_object_unref (file);
	}
	return false;
}
