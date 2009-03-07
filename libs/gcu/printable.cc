/* 
 * Gnome Chemistry Utils
 * printable.h
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
 */

#include "config.h"
#include "printable.h"
#include "application.h"
#include "macros.h"
#include <glib/gi18n-lib.h>
#include <cstring>

namespace gcu {


static char const *UnitNames[] = {
	N_("pixels"),
	N_("points"),
	N_("inches"),
	N_("mm")
};

GtkUnit gtk_unit_from_string (char const *name)
{
	int i = G_N_ELEMENTS (UnitNames);
	while (i > 0)
		if (!strcmp (name, UnitNames[--i]))
			return (GtkUnit) i;
	return GTK_UNIT_MM; // our default
}

char const *gtk_unit_to_string (GtkUnit unit)
{
	return UnitNames[unit];
}

class PrintSettings
{
public:
	PrintSettings ();
	virtual ~PrintSettings ();
	void Init ();
	void OnConfigChanged (GOConfNode *node, gchar const *name);
	GtkPrintSettings *settings;
	GtkPageSetup *setup;
	GtkUnit unit;
	guint m_NotificationId;
	GOConfNode *m_ConfNode;
};

static PrintSettings DefaultSettings;

PrintSettings::PrintSettings ()
{
	settings = NULL;
	setup = NULL;
}

static void on_config_changed (GOConfNode *node, gchar const *key, G_GNUC_UNUSED gpointer data)
{
	DefaultSettings.OnConfigChanged (node, key);
}

#define ROOTDIR "/apps/gchemutils/printsetup/"

void PrintSettings::Init ()
{
	settings = gtk_print_settings_new ();
	setup = gtk_page_setup_new ();
	m_ConfNode = go_conf_get_node (Application::GetConfDir (), "printsetup");
	char *name = NULL;
	GtkPaperSize *size = NULL;
	GCU_GCONF_GET_STRING ("paper", name, NULL)
	size = gtk_paper_size_new ((name && strlen (name))? name: NULL);
	gtk_page_setup_set_paper_size (setup, size);
	gtk_paper_size_free (size);
	g_free (name);
	name = NULL;
	GCU_GCONF_GET_STRING ("preferred-unit", name, "mm")
	unit = gtk_unit_from_string (name);
	g_free (name);
	double x;
	GCU_GCONF_GET_NO_CHECK ("margin-top", float, x, 72);
	gtk_page_setup_set_top_margin (setup, x, GTK_UNIT_POINTS);
	GCU_GCONF_GET_NO_CHECK ("margin-bottom", float, x, 72);
	gtk_page_setup_set_bottom_margin (setup, x, GTK_UNIT_POINTS);
	GCU_GCONF_GET_NO_CHECK ("margin-right", float, x, 72);
	gtk_page_setup_set_right_margin (setup, x, GTK_UNIT_POINTS);
	GCU_GCONF_GET_NO_CHECK ("margin-left", float, x, 72);
	gtk_page_setup_set_left_margin (setup, x, GTK_UNIT_POINTS);
	// TODO: import other default values from conf keys
	m_NotificationId = go_conf_add_monitor (m_ConfNode, NULL, (GOConfMonitorFunc) on_config_changed, NULL);
	go_conf_free_node (m_ConfNode);
}

PrintSettings::~PrintSettings ()
{
	if (setup)
		g_object_unref (setup);
	if (settings)
		g_object_unref (settings);
}

void PrintSettings::OnConfigChanged (GOConfNode *node, gchar const *name)
{
	char *val = NULL;
	GCU_UPDATE_STRING_KEY ("paper", val,
					{
						GtkPaperSize *size = NULL;
						size = gtk_paper_size_new ((val && strlen (val))? val: NULL);
						gtk_page_setup_set_paper_size (setup, size);
						gtk_paper_size_free (size);
						g_free (val);
						val = NULL;
					})
	GCU_UPDATE_STRING_KEY ("preferred-unit", val,
					{
						unit = gtk_unit_from_string (val);
						g_free (val);
					})
	double x;
	GCU_UPDATE_KEY ("margin-top", float, x,
					{
						gtk_page_setup_set_top_margin (setup, x, GTK_UNIT_POINTS);
					})
	GCU_UPDATE_KEY ("margin-bottom", float, x,
					{
						gtk_page_setup_set_bottom_margin (setup, x, GTK_UNIT_POINTS);
					})
	GCU_UPDATE_KEY ("margin-right", float, x,
					{
						gtk_page_setup_set_right_margin (setup, x, GTK_UNIT_POINTS);
					})
	GCU_UPDATE_KEY ("margin-left", float, x,
					{
						gtk_page_setup_set_left_margin (setup, x, GTK_UNIT_POINTS);
					})
}

Printable::Printable ():
	DialogOwner ()
{
	if (DefaultSettings.settings == NULL)
		DefaultSettings.Init ();
	m_PrintSettings = gtk_print_settings_copy (DefaultSettings.settings);
	m_PageSetup = gtk_page_setup_copy (DefaultSettings.setup);
	m_Unit = DefaultSettings.unit;
	m_HorizCentered = m_VertCentered = false;
	m_ScaleType = GCU_PRINT_SCALE_NONE;
	m_Scale = 1.;
	m_HorizFit = m_VertFit = true;
	m_HPages = m_VPages = 1;
}

Printable::~Printable ()
{
	g_object_unref (m_PageSetup);
	g_object_unref (m_PrintSettings);
}

static void begin_print (GtkPrintOperation *print, G_GNUC_UNUSED GtkPrintContext *context, gpointer data)
{
	gtk_print_operation_set_n_pages (print, ((Printable *) data)->GetPagesNumber ());
}

static void draw_page (GtkPrintOperation *print, GtkPrintContext *context, gint page_nr, gpointer data)
{
	((Printable *) data)->DoPrint (print, context, page_nr);
}

void Printable::Print (bool preview)
{
	GtkPrintOperation *print;
	GtkPrintOperationResult res;

	print = gtk_print_operation_new ();
	gtk_print_operation_set_use_full_page (print, false);

	gtk_print_operation_set_print_settings (print, GetPrintSettings ());
	gtk_print_operation_set_default_page_setup (print, GetPageSetup ());

	g_signal_connect (print, "begin_print", G_CALLBACK (begin_print), this);
	g_signal_connect (print, "draw_page", G_CALLBACK (draw_page), this);
	
	res = gtk_print_operation_run (print,
								   (preview)? GTK_PRINT_OPERATION_ACTION_PREVIEW:
								   			GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
								   GetGtkWindow (), NULL);

	if (res == GTK_PRINT_OPERATION_RESULT_APPLY) {
		if (m_PrintSettings != NULL)
			g_object_unref (m_PrintSettings);
		m_PrintSettings = GTK_PRINT_SETTINGS (g_object_ref (gtk_print_operation_get_print_settings (print)));
	}

	g_object_unref (print);
}

void Printable::SetPageSetup (GtkPageSetup *PageSetup)
{
	if (PageSetup != NULL) {
		if (m_PageSetup != NULL)
			g_object_unref (m_PageSetup);
		m_PageSetup = PageSetup;
	}
}

}	//	namespace gcu
