/* 
 * Gnome Chemisty Utils
 * gcuperiodic.c 
 *
 * Copyright (C) 2002-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gcuperiodic.h"
#include <gcu/chemistry.h>
#include <goffice/goffice.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gsf/gsf-impl-utils.h>
#include <glib/gi18n-lib.h>

struct _GcuPeriodic
{
	GtkBin bin;
	
	GtkGrid *grid;
	GtkToggleButton* buttons[119];
	GtkLabel* labels[119];
	GtkNotebook *book;
	guint Z;
	gboolean can_unselect;
	unsigned colorstyle;
	GArray *colorschemes;
	unsigned nbschemes;
	unsigned tips;
};

struct _GcuPeriodicClass
{
	GtkBinClass parent_class;

	void (* element_changed_event)(GcuPeriodic *periodic);
};

GType
gcu_periodic_color_style_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GCU_PERIODIC_COLOR_NONE, "GCU_PERIODIC_COLOR_NONE", "none" },
      { GCU_PERIODIC_COLOR_DEFAULT, "GCU_PERIODIC_COLOR_DEFAULT", "default" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GcuPeriodicColorStyle", values);
  }
  return etype;
}
#define GCU_TYPE_PERIODIC_COLOR_STYLE_TYPE (gcu_periodic_color_style_get_type())

static GtkBinClass *parent_class = NULL;

struct ColorScheme {
	GcuPeriodicColorFunc f;
	int page;
	gpointer data;
};

enum {
  ELEMENT_CHANGED,
  LAST_SIGNAL
};

enum {
	PROP_0,
	PROP_CAN_UNSELECT,
	PROP_COLOR_STYLE
};

static guint gcu_periodic_signals[LAST_SIGNAL] = { 0 };

void on_clicked (GtkToggleButton *button, GcuPeriodic* periodic)
{
	static gboolean change = FALSE;
	if (button != periodic->buttons[0]) {
		const gchar* name;
		change = TRUE;
		if (periodic->buttons[0])
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (periodic->buttons[0]), FALSE);
		periodic->buttons[0] = button;
		name = gtk_buildable_get_name (GTK_BUILDABLE (periodic->buttons[0]));
		periodic->Z = atoi (name + 3);
		g_signal_emit (periodic, gcu_periodic_signals[ELEMENT_CHANGED], 0, periodic->Z);
		change = FALSE;
	} else if (!change) {
		if (periodic->can_unselect) {
			periodic->buttons[0] = NULL;
			periodic->Z = 0;
			g_signal_emit (periodic, gcu_periodic_signals[ELEMENT_CHANGED], 0, 0);
		} else if (periodic->buttons[0])
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (periodic->buttons[0]), TRUE);
	}
}

static void
gcu_periodic_set_property (GObject              *object,
				guint                 param_id,
				const GValue         *value,
				GParamSpec           *pspec)
{
	GcuPeriodic *periodic;
	g_return_if_fail (object != NULL);
	g_return_if_fail (GCU_IS_PERIODIC (object));

	periodic = GCU_PERIODIC (object);

	switch (param_id) {
	case PROP_CAN_UNSELECT:
		periodic->can_unselect = g_value_get_boolean (value);
		break;

	case PROP_COLOR_STYLE: {
		unsigned style = g_value_get_uint (value);
		if (style < GCU_PERIODIC_COLOR_MAX + periodic->nbschemes) {
			periodic->colorstyle = style;
			int page = (style >= GCU_PERIODIC_COLOR_MAX)? 
				g_array_index (periodic->colorschemes, struct ColorScheme, style - GCU_PERIODIC_COLOR_MAX).page: 0;
			gtk_notebook_set_current_page (periodic->book, page);
			gcu_periodic_set_colors (periodic);
		} else
			g_warning (_("Out of range value %d for property \"color-style\" for GcuPeriodic instance %p\n"), style, periodic);
		break;
	}

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gcu_periodic_get_property (GObject              *object,
				guint                 param_id,
				GValue               *value,
				GParamSpec           *pspec)
{
	GcuPeriodic *periodic;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GCU_IS_PERIODIC (object));

	periodic = GCU_PERIODIC (object);

	switch (param_id) {
	case PROP_CAN_UNSELECT:
		g_value_set_boolean (value, periodic->can_unselect);
		break;

	case PROP_COLOR_STYLE:
		g_value_set_uint (value, periodic->colorstyle);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void gcu_periodic_get_preferred_height (GtkWidget *w, gint *minimum_height, gint *natural_height)
{
	GtkWidget *child = gtk_bin_get_child (GTK_BIN (w));
	gboolean visible = FALSE;
	if (child)
		g_object_get (G_OBJECT (child), "visible", &visible, NULL);
	if (visible)
		gtk_widget_get_preferred_height (child, minimum_height, natural_height);
	else
		*minimum_height = *natural_height = 0;
}

static void gcu_periodic_get_preferred_width (GtkWidget *w, gint *minimum_width, gint *natural_width)
{
	GtkWidget *child = gtk_bin_get_child (GTK_BIN (w));
	gboolean visible = FALSE;
	if (child)
		g_object_get (G_OBJECT (child), "visible", &visible, NULL);
	if (visible)
		gtk_widget_get_preferred_width (child, minimum_width, natural_width);
	else
		*minimum_width = *natural_width = 0;
}

static void gcu_periodic_size_allocate (GtkWidget *w, GtkAllocation *alloc)
{
	GtkWidget *child = gtk_bin_get_child (GTK_BIN (w));
	gboolean visible = FALSE;
	if (child)
		g_object_get (G_OBJECT (child), "visible", &visible, NULL);
	if (visible)
		gtk_widget_size_allocate (child, alloc);
	(GTK_WIDGET_CLASS (parent_class))->size_allocate (w, alloc);
}

static void gcu_periodic_finalize (GObject *object)
{
	GcuPeriodic *periodic = (GcuPeriodic*) object;

	g_array_free (periodic->colorschemes, FALSE);

	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void gcu_periodic_class_init (GcuPeriodicClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	parent_class = (GtkBinClass*) g_type_class_peek_parent (klass);
	
	gobject_class->set_property = gcu_periodic_set_property;
	gobject_class->get_property = gcu_periodic_get_property;
	klass->element_changed_event = NULL;
	gcu_periodic_signals[ELEMENT_CHANGED] =
	g_signal_new ("element_changed",
				  G_TYPE_FROM_CLASS(gobject_class),
				  G_SIGNAL_RUN_LAST,
				  G_STRUCT_OFFSET(GcuPeriodicClass, element_changed_event),
				  NULL, NULL,
				  g_cclosure_marshal_VOID__UINT,
				  G_TYPE_NONE, 1,
				  G_TYPE_UINT
				  );
	g_object_class_install_property
			(gobject_class,
			 PROP_CAN_UNSELECT,
			 g_param_spec_boolean ("can_unselect", NULL, NULL,
				   FALSE,
				   (G_PARAM_READABLE | G_PARAM_WRITABLE)));
	g_object_class_install_property
			(gobject_class,
			 PROP_COLOR_STYLE,
			 g_param_spec_uint ("color-style", NULL, NULL,
								GCU_PERIODIC_COLOR_NONE, G_MAXUINT,
								GCU_PERIODIC_COLOR_NONE,
								(G_PARAM_READABLE | G_PARAM_WRITABLE)));
	gobject_class->finalize = gcu_periodic_finalize;
	widget_class->get_preferred_height = gcu_periodic_get_preferred_height;
	widget_class->get_preferred_width = gcu_periodic_get_preferred_width;
	widget_class->size_allocate = gcu_periodic_size_allocate;
}

static void gcu_periodic_init (GcuPeriodic *periodic)
{
	GtkBuilder* xml;
	char name[8] = "elt";
	GtkToggleButton* button;
	int i;
	xml = go_gtk_builder_new (UIDIR"/gcuperiodic.ui", GETTEXT_PACKAGE, NULL);
	g_return_if_fail (xml);
	periodic->grid = GTK_GRID (gtk_builder_get_object (xml, "periodic-grid"));
	periodic->book = GTK_NOTEBOOK (gtk_builder_get_object (xml, "book"));
	periodic->colorstyle = GCU_PERIODIC_COLOR_NONE;
	memset(periodic->buttons, 0, sizeof (GtkToggleButton*) * 119);
	for (i = 1; i <= 118; i++) {
		sprintf(name + 3, "%d", i);
		button = (GtkToggleButton*) gtk_builder_get_object (xml, name);
		if (GTK_IS_TOGGLE_BUTTON (button)) {
			gtk_widget_set_tooltip_text (GTK_WIDGET(button), gcu_element_get_name(i));
			periodic->buttons[i] = button;
			periodic->labels[i] = GTK_LABEL (gtk_bin_get_child (GTK_BIN (button)));
			g_signal_connect (G_OBJECT (button), "toggled", G_CALLBACK (on_clicked), periodic);
		}
	}
	periodic->Z = 0;
	gtk_container_add (GTK_CONTAINER (periodic), GTK_WIDGET (periodic->grid));
	gtk_widget_show_all (GTK_WIDGET (periodic));
	periodic->colorschemes = g_array_new (FALSE, FALSE, sizeof (struct ColorScheme));
	g_object_unref (xml);
}

GSF_CLASS (GcuPeriodic, gcu_periodic,
           gcu_periodic_class_init, gcu_periodic_init,
           GTK_TYPE_BIN)

GtkWidget* gcu_periodic_new ()
{
	return GTK_WIDGET (g_object_new (GCU_TYPE_PERIODIC, NULL));
}

guint gcu_periodic_get_element(GcuPeriodic* periodic)
{
	g_return_val_if_fail(GCU_IS_PERIODIC(periodic), 0);
	return periodic->Z;
}

void gcu_periodic_set_element (GcuPeriodic* periodic, guint element)
{
	g_return_if_fail(GCU_IS_PERIODIC(periodic));
	if (periodic->can_unselect && periodic->buttons[0]) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(periodic->buttons[0]), FALSE);
	if (element) 
	{
		gtk_toggle_button_set_active(periodic->buttons[element], TRUE);
		periodic->buttons[0] = periodic->buttons[element];
		periodic->Z = element;
	}
	else if (periodic->can_unselect)
	{
		periodic->buttons[0] = NULL;
		periodic->Z = 0;
	}
}

void gcu_periodic_set_colors(GcuPeriodic *periodic)
{
	const double *colors;
	PangoAttribute *attr;
	PangoAttrList *l;
	int i;
	GdkRGBA rgba;
	rgba.alpha = 1.;
	GcuPeriodicColorFunc func = NULL;
	gpointer data = NULL;
	GtkWidget *w;
	if (periodic->colorstyle >= GCU_PERIODIC_COLOR_MAX) {
		func = g_array_index (periodic->colorschemes, struct ColorScheme, periodic->colorstyle - GCU_PERIODIC_COLOR_MAX).f;
		data = g_array_index (periodic->colorschemes, struct ColorScheme, periodic->colorstyle - GCU_PERIODIC_COLOR_MAX).data;
	}
	for (i = 1; i <= 118; i++)
	{
		if (!periodic->buttons[i])
			continue;
		w = GTK_WIDGET (periodic->buttons[i]);
		switch (periodic->colorstyle)
		{
		case GCU_PERIODIC_COLOR_NONE:
			gtk_widget_override_background_color (w, GTK_STATE_FLAG_NORMAL, NULL);
			attr = pango_attr_foreground_new (0, 0, 0);
			attr->start_index = 0;
			attr->end_index = 100;
			l = pango_attr_list_new ();
			pango_attr_list_insert (l, attr);
			gtk_label_set_attributes (periodic->labels[i], l);
		break;
		case GCU_PERIODIC_COLOR_DEFAULT:
			colors = gcu_element_get_default_color(i);
			rgba.red = colors[0];
			rgba.green = colors[1];
			rgba.blue = colors[2];
			gtk_widget_override_background_color (w, GTK_STATE_FLAG_NORMAL, &rgba);
			if (colors[0] > 0.6 ||  colors[1] > 0.6 || colors[2] > 0.6)
				attr = pango_attr_foreground_new (0, 0, 0);
			else
				attr = pango_attr_foreground_new (65535, 65535, 65535);
			attr->start_index = 0;
			attr->end_index = 100;
			l = pango_attr_list_new ();
			pango_attr_list_insert (l, attr);
			gtk_label_set_attributes (periodic->labels[i], l);
			break;
		default: {
			func (i, &rgba, data);
			gtk_widget_override_background_color (w, GTK_STATE_FLAG_NORMAL, &rgba);
			if (rgba.red > 0.6 ||  rgba.green > 0.6 || rgba.blue > 0.6)
				attr = pango_attr_foreground_new (0, 0, 0);
			else
				attr = pango_attr_foreground_new (65535, 65535, 65535);
			attr->start_index = 0;
			attr->end_index = 100;
			l = pango_attr_list_new ();
			pango_attr_list_insert (l, attr);
			gtk_label_set_attributes (periodic->labels[i], l);
			break;
		}
		}
	}
}

int	gcu_periodic_add_color_scheme (GcuPeriodic *periodic,
		GcuPeriodicColorFunc func, GtkWidget *extra_widget, gpointer user_data)
{
	struct ColorScheme s;
	s.f = func;
	if (extra_widget)
		s.page = gtk_notebook_append_page (periodic->book, extra_widget, NULL);
	else
		s.page = 0;
	s.data = user_data;
	g_array_append_val (periodic->colorschemes, s);
	return GCU_PERIODIC_COLOR_MAX + periodic->nbschemes++;
}

void gcu_periodic_set_tips (GcuPeriodic *periodic, unsigned scheme)
{
	if (scheme != periodic->tips) {
		int i;
		periodic->tips = scheme;
		switch (scheme) {
		default:
		case GCU_PERIODIC_TIP_NAME:
			for (i = 1; i <= 118; i++) {
				if (periodic->buttons[i])
					gtk_widget_set_tooltip_text (GTK_WIDGET (periodic->buttons[i]), gcu_element_get_name (i));
			}
			break;
		case GCU_PERIODIC_TIP_STANDARD:
			for (i = 1; i <= 118; i++) {
				GtkWidget *win, *grid, *w;
				char *markup, *str;
				char const *conf;
				if (!periodic->buttons[i])
					continue;
				win = gtk_window_new (GTK_WINDOW_POPUP);
				gtk_widget_set_name (win, "gtk-tooltip");
				grid = gtk_grid_new ();
				gtk_container_add (GTK_CONTAINER (win), grid);
				w = GTK_WIDGET (g_object_new (GTK_TYPE_LABEL, "xalign", 0., NULL));
				markup = g_strdup_printf ("%u", i);
				gtk_label_set_text (GTK_LABEL (w), markup);
				g_free (markup);
				gtk_grid_attach (GTK_GRID (grid), w, 0, 0, 1, 1);
				str = gcu_element_get_weight_as_string (i);
				conf = gcu_element_get_electronic_configuration (i);
				w = GTK_WIDGET (g_object_new (GTK_TYPE_LABEL, "justify", GTK_JUSTIFY_CENTER, NULL));
				markup = g_strdup_printf ("<span face=\"Sans\" size=\"xx-large\">%s</span>\n%s\n%s\n%s",
				                          gcu_element_get_symbol (i), gcu_element_get_name (i), (conf)? conf: "", (str)? str: "");
				g_free (str);
				gtk_label_set_markup (GTK_LABEL (w), markup);
				g_free (markup);
				gtk_grid_attach (GTK_GRID (grid), w, 0, 1, 1, 1);
				gtk_widget_show_all (grid);
				gtk_widget_set_tooltip_window (GTK_WIDGET (periodic->buttons[i]), GTK_WINDOW (win));
			}
			break;
		}
	}
}
