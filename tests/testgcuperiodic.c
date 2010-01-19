/* 
 * Gnome Chemisty Utils
 * tests/testgcuperiodic.c 
 *
 * Copyright (C) 2008-2010 Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the 
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <gcu/gcuperiodic.h>
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
	GtkVBox* vbox;
	GtkHBox* hbox;
	GtkLabel* label;
	GtkRadioButton *btn;
	GSList* btn_group;
	
	gtk_init (&argc, &argv);
	
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "GcuPeriodic test");
	g_signal_connect (G_OBJECT (window), "destroy",
		 G_CALLBACK (gtk_main_quit),
		 NULL);

	g_object_set (G_OBJECT (window), "allow-shrink", FALSE, NULL);
	
	periodic = gcu_periodic_new ();
	vbox = (GtkVBox*) gtk_vbox_new (FALSE, 0);
	hbox = (GtkHBox*) gtk_hbox_new (FALSE, 0);
	label = (GtkLabel*) gtk_label_new ("Color scheme:");
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), TRUE, TRUE, 0);
	btn = (GtkRadioButton*) gtk_radio_button_new_with_label (NULL, "None");
	g_signal_connect (G_OBJECT (btn), "toggled", (GCallback) on_color_scheme_none, (gpointer) periodic);
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (btn), TRUE, TRUE, 0);
	btn_group = gtk_radio_button_get_group (btn);
	btn = (GtkRadioButton*) gtk_radio_button_new_with_label (btn_group, "Default");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (btn), TRUE);
	g_signal_connect (G_OBJECT (btn), "toggled", (GCallback) on_color_scheme_default, (gpointer) periodic);
	gtk_box_pack_end (GTK_BOX (hbox), GTK_WIDGET (btn), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (hbox), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), gtk_hseparator_new (), TRUE, TRUE, 0);

	g_object_set (G_OBJECT (periodic), "color-style", GCU_PERIODIC_COLOR_DEFAULT, NULL);
	g_signal_connect (G_OBJECT (periodic), "element_changed", (GCallback) on_changed, NULL);
	gtk_box_pack_end (GTK_BOX (vbox), GTK_WIDGET (GCU_PERIODIC (periodic)), TRUE, TRUE, 0);
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (vbox));
	gtk_widget_show_all (window);

	gtk_main ();
	
	return 0;
}
