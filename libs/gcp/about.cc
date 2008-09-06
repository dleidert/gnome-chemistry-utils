// -*- C++ -*-

/* 
 * GChemPaint library
 * about.cc 
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "about.h"
#include <glib/gi18n-lib.h>
#include <gtk/gtk.h>
#include <cstring>

void on_about (GtkWidget* widget, void* data)
{
	char const *authors[] = {"Jean Bréfort", NULL};
	char const *artists[] = {"Nestor Diaz", NULL};
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
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307\n"
		"USA";
/* Note to translators: replace the following string with the appropriate credits for you lang */
	char const *translator_credits = _("translator_credits");
	GdkPixbuf* pixbuf = gdk_pixbuf_new_from_file (PIXMAPSDIR"/gchempaint_logo.png", NULL);
	gtk_show_about_dialog (NULL,
					"name", "GChemPaint",
					"authors", authors,
					"artists", artists,
					"comments", _("GChemPaint is a 2D chemical structures editor for Gnome"),
					"copyright", _("Copyright © 2001-2008 by Jean Bréfort"),
					"license", license,
					"logo", pixbuf,
					"icon-name", "gchempaint",
					"translator_credits", strcmp (translator_credits, "translator_credits") != 0 ? 
											translator_credits : NULL,
					"version", VERSION,
					"website", "http://www.nongnu.org/gchempaint",
					NULL);
	if (pixbuf != NULL)
		g_object_unref (pixbuf);
}
