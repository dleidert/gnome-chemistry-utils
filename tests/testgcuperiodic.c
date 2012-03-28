/*
 * Gnome Chemisty Utils
 * tests/testgcuperiodic.c
 *
 * Copyright (C) 2008-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include <gcugtk/gcuperiodic.h>
#include <gcu/chemistry.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>

/*!\file
A simple example of the use of the GcuPeriodic widget.
*/

/*!
The function called  when the selected element changes. It just prints the atomic
number of the selected element to the standard output.
*/
void on_changed (G_GNUC_UNUSED GcuPeriodic* periodic, guint Z, G_GNUC_UNUSED gpointer data)
{
	printf ("Selected element:%d\n", Z);
}

/*!
The function called  when the "None" button. It Just sets the "color-scheme" property
of the GtkPeridic widget to GCU_PERIODIC_COLOR_NONE if the button is active.
*/
void on_color_scheme_none (GtkToggleButton* btn, GtkWidget* periodic)
{
	if (gtk_toggle_button_get_active (btn))
		g_object_set (G_OBJECT (periodic), "color-style", GCU_PERIODIC_COLOR_NONE, NULL);
}

/*!
The function called  when the "Default" button. It Just sets the "color-scheme" property
of the GtkPeridic widget to GCU_PERIODIC_COLOR_DEFAULT if the button is active.
*/
void on_color_scheme_default (GtkToggleButton* btn, GtkWidget* periodic)
{
	if (gtk_toggle_button_get_active (btn))
		g_object_set (G_OBJECT (periodic), "color-style", GCU_PERIODIC_COLOR_DEFAULT, NULL);
}

/*!
The \a main function of the test program. It builds the window containing the GcuPeriodic
widget, adds some buttons and installs the appropriate signals.
*/
int main (int argc, char *argv[])
{
	GtkWidget *window;
	GtkWidget *periodic;
	GtkGrid *grid;
	GtkLabel *label;
	GtkRadioButton *btn;
	GSList *btn_group;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "GcuPeriodic test");
	g_signal_connect (G_OBJECT (window), "destroy",
		 G_CALLBACK (gtk_main_quit),
		 NULL);

	periodic = gcu_periodic_new ();
	grid = (GtkGrid *) gtk_grid_new ();
	label = (GtkLabel*) gtk_label_new ("Color scheme:");
	gtk_grid_attach (grid, GTK_WIDGET (label), 0, 0, 1, 1);
	btn = (GtkRadioButton*) gtk_radio_button_new_with_label (NULL, "None");
	g_signal_connect (G_OBJECT (btn), "toggled", (GCallback) on_color_scheme_none, (gpointer) periodic);
	gtk_grid_attach (grid, GTK_WIDGET (btn), 1, 0, 1, 1);
	btn_group = gtk_radio_button_get_group (btn);
	btn = (GtkRadioButton*) gtk_radio_button_new_with_label (btn_group, "Default");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (btn), TRUE);
	g_signal_connect (G_OBJECT (btn), "toggled", (GCallback) on_color_scheme_default, (gpointer) periodic);
	gtk_grid_attach (grid, GTK_WIDGET (btn), 2, 0, 1, 1);
	gtk_grid_attach (grid, gtk_separator_new (GTK_ORIENTATION_HORIZONTAL), 0, 1, 3, 1);

	g_object_set (G_OBJECT (periodic), "color-style", GCU_PERIODIC_COLOR_DEFAULT, "expand", TRUE, NULL);
	g_signal_connect (G_OBJECT (periodic), "element_changed", (GCallback) on_changed, NULL);
	gtk_grid_attach (grid, GTK_WIDGET (GCU_PERIODIC (periodic)), 0, 2, 3, 1);
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (grid));
	gtk_widget_show_all (window);

	gtk_main ();

	return 0;
}
