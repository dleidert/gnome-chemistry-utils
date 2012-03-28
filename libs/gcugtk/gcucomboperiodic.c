/*
 * Gnome Chemisty Utils
 * gcucomboperiodic.c
 *
 * Copyright (C) 2006-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gcuperiodic.h"
#include "gcucomboperiodic.h"
#include <gcu/chemistry.h>
#include <goffice/goffice.h>
#include <gsf/gsf-impl-utils.h>
#include <glib/gi18n-lib.h>

struct _GcuComboPeriodic {
	GOComboBox     base;

	GtkWidget    *periodic, *preview_button, *label;
	gulong handler_id;
};

typedef struct {
	GOComboBoxClass base;
	void (* changed) (GcuComboPeriodic *combo, int id);
} GcuComboPeriodicClass;

enum {
	CHANGED,
	LAST_SIGNAL
};

static guint go_combo_pixmaps_signals [LAST_SIGNAL] = { 0, };

static void
cb_screen_changed (GcuComboPeriodic *combo, G_GNUC_UNUSED GdkScreen *previous_screen)
{
	GtkWidget *w = GTK_WIDGET (combo);
	GdkScreen *screen = gtk_widget_has_screen (w)
		? gtk_widget_get_screen (w)
		: NULL;

	if (screen) {
		GtkWidget *toplevel = gtk_widget_get_toplevel (combo->periodic);
		gtk_window_set_screen (GTK_WINDOW (toplevel), screen);
	}
}

static void
element_changed_cb (GcuComboPeriodic *combo)
{
	int newZ = gcu_periodic_get_element (GCU_PERIODIC (combo->periodic));
	gtk_label_set_text (GTK_LABEL (combo->label), gcu_element_get_symbol (newZ));
	if (_go_combo_is_updating (GO_COMBO_BOX (combo)))
		return;
	g_signal_emit (combo, go_combo_pixmaps_signals [CHANGED], 0, newZ);
	go_combo_box_popup_hide (GO_COMBO_BOX (combo));
}

static void
gcu_combo_periodic_init (GcuComboPeriodic *combo)
{
	combo->preview_button = gtk_toggle_button_new ();
	combo->label = gtk_label_new ("");
	gtk_widget_show (combo->label);
	gtk_container_add (GTK_CONTAINER (combo->preview_button),
		GTK_WIDGET (combo->label));

	g_signal_connect (G_OBJECT (combo),
		"screen-changed",
		G_CALLBACK (cb_screen_changed), NULL);

	gtk_widget_show_all (combo->preview_button);
	combo->periodic = gcu_periodic_new ();
	combo->handler_id = g_signal_connect_swapped (combo->periodic,
		"element_changed", G_CALLBACK (element_changed_cb), combo);
	gtk_widget_show_all (combo->periodic);
	go_combo_box_construct (GO_COMBO_BOX (combo),
		combo->preview_button, combo->periodic, combo->periodic);
	go_combo_box_set_title(GO_COMBO_BOX (combo), _("Periodic table of the elements"));
	gtk_widget_show_all (GTK_WIDGET (combo));
}

static void
gcu_combo_periodic_class_init (GObjectClass *gobject_class)
{
	go_combo_pixmaps_signals [CHANGED] =
		g_signal_new ("changed",
			      G_OBJECT_CLASS_TYPE (gobject_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GcuComboPeriodicClass, changed),
			      NULL, NULL,
			      g_cclosure_marshal_VOID__INT,
			      G_TYPE_NONE, 1, G_TYPE_INT);
}

GSF_CLASS (GcuComboPeriodic, gcu_combo_periodic,
	   gcu_combo_periodic_class_init, gcu_combo_periodic_init,
	   GO_TYPE_COMBO_BOX)

GtkWidget *gcu_combo_periodic_new (void)
{
	return GTK_WIDGET (g_object_new (GCU_TYPE_COMBO_PERIODIC, NULL));
}

guint	gcu_combo_periodic_get_element	(GcuComboPeriodic* combo)
{
	return gcu_periodic_get_element (GCU_PERIODIC (combo->periodic));
}

void	gcu_combo_periodic_set_element	(GcuComboPeriodic* combo, guint element)
{
	g_signal_handler_block (combo->periodic, combo->handler_id);
	gcu_periodic_set_element (GCU_PERIODIC (combo->periodic), element);
	g_signal_handler_unblock (combo->periodic, combo->handler_id);
	gtk_label_set_text (GTK_LABEL (combo->label), gcu_element_get_symbol (element));
}
