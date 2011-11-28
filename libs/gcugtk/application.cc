// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcugtk/application.cc
 *
 * Copyright (C) 2005-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "cmd-context-gtk.h"
#include "ui-builder.h"
#include <gcu/object.h>
#include <glib/gi18n-lib.h>

namespace gcugtk {

WindowState Application::DefaultWindowState = NormalWindowState;

class ApplicationPrivate
{
public:
	static void MaximizeWindows ();
	static void FullScreenWindows ();
};

void ApplicationPrivate::MaximizeWindows ()
{
	Application::DefaultWindowState = MaximizedWindowState;
}

void ApplicationPrivate::FullScreenWindows ()
{
	Application::DefaultWindowState = FullScreenWindowState;
}

static void on_res_changed (GtkSpinButton *btn, Application *app)
{
	app->SetImageResolution (gtk_spin_button_get_value_as_int (btn));
}

static void on_width_changed (GtkSpinButton *btn, Application *app)
{
	app->SetImageWidth (gtk_spin_button_get_value_as_int (btn));
}

static void on_height_changed (GtkSpinButton *btn, Application *app)
{
	app->SetImageHeight (gtk_spin_button_get_value_as_int (btn));
}

static void on_transparency_changed (GtkToggleButton *btn, Application *app)
{
	app->SetTransparentBackground (gtk_toggle_button_get_active (btn));
}

static GOptionEntry options[] =
{
  {"full-screen", 'F', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void *)ApplicationPrivate::MaximizeWindows, N_("Open new windows full screen"), NULL},
  {"maximize", 'M', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK, (void *) ApplicationPrivate::FullScreenWindows, N_("Maximize new windows"), NULL},
  {NULL, 0, 0, G_OPTION_ARG_NONE, NULL, NULL, NULL }
};

Application::Application (std::string name, std::string datadir, char const *help_name, char const *icon_name, CmdContextGtk *cc):
	gcu::Application (name, datadir, help_name, icon_name, cc)
{
	m_RecentManager = gtk_recent_manager_get_default ();
	RegisterOptions (options);
	GdkScreen *screen = gdk_screen_get_default ();
	m_ScreenResolution = (unsigned) rint (gdk_screen_get_width (screen) * 25.4 / gdk_screen_get_width_mm (screen));
}

Application::~Application ()
{
}

void Application::CreateDefaultCmdContext ()
{
	if (!m_CmdContext)
		m_CmdContext = new CmdContextGtk (this);
}

GtkWidget *Application::GetImageResolutionWidget ()
{
	UIBuilder *builder = new UIBuilder (UIDIR"/image-resolution.ui", GETTEXT_PACKAGE);
	GtkWidget *w = builder->GetWidget ("screen-lbl");
	char *buf = g_strdup_printf (_("(screen resolution is %u)"), GetScreenResolution ());
	gtk_label_set_text (GTK_LABEL (w), buf);
	g_free (buf);
	w = builder->GetWidget ("res-btn");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), GetImageResolution ());
	g_signal_connect (G_OBJECT (w), "value-changed", G_CALLBACK (on_res_changed), this);
	w = builder->GetWidget ("transparent-btn");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), GetTransparentBackground ());
	g_signal_connect (G_OBJECT (w), "toggled", G_CALLBACK (on_transparency_changed), this);
	w = builder->GetRefdWidget ("res-grid");
	delete builder;
	return w;
}

GtkWidget *Application::GetImageSizeWidget ()
{
	UIBuilder *builder = new UIBuilder (UIDIR"/image-size.ui", GETTEXT_PACKAGE);
	GtkWidget *w = builder->GetWidget ("width");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), GetImageWidth ());
	g_signal_connect (G_OBJECT (w), "value-changed", G_CALLBACK (on_width_changed), this);
	w = builder->GetWidget ("height");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (w), GetImageHeight ());
	g_signal_connect (G_OBJECT (w), "value-changed", G_CALLBACK (on_height_changed), this);
	w = builder->GetWidget ("transparent-btn");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (w), GetTransparentBackground ());
	g_signal_connect (G_OBJECT (w), "toggled", G_CALLBACK (on_transparency_changed), this);
	w = builder->GetRefdWidget ("size-grid");
	delete builder;
	return w;
}

}	//	namespace gcugtk
