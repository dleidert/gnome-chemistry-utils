/* 
 * Gnome Chemisty Utils
 * gtkperiodic.c 
 *
 * Copyright (C) 2002-2004
 *
 * Developed by Jean Bréfort <jean.brefort@ac-dijon.fr>
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
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include "config.h"
#include "gtkperiodic.h"
#include "chemistry/chemistry.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtkstyle.h>
#include <glade/glade.h>
#include <libintl.h>

static unsigned DefaultRed[4], DefaultGreen[4], DefaultBlue[4];

GType
gtk_periodic_color_style_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { GTK_PERIODIC_COLOR_NONE, "GTK_PERIODIC_COLOR_NONE", "none" },
      { GTK_PERIODIC_COLOR_DEFAULT, "GTK_PERIODIC_COLOR_DEFAULT", "default" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("GtkPeriodicColorStyle", values);
  }
  return etype;
}
#define GTK_TYPE_PERIODIC_COLOR_STYLE_TYPE (gtk_periodic_color_style_get_type())

static GtkBinClass *parent_class = NULL;

struct _GtkPeriodicPrivate
{
	GtkVBox* vbox;
	GtkToggleButton* buttons[119];
	guint Z;
	gboolean can_unselect;
	GtkTooltips* tips;
	GtkPeriodicColorStyle colorstyle;
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

static guint gtk_periodic_signals[LAST_SIGNAL] = { 0 };

static void gtk_periodic_class_init (GtkPeriodicClass  *klass);
static void gtk_periodic_init(GtkPeriodic *periodic);
static void gtk_periodic_finalize(GObject *object);
static void gtk_periodic_size_allocate(GtkPeriodic* w, GtkAllocation *allocation);
static void gtk_periodic_size_request(GtkPeriodic* w, GtkRequisition *requisition);
static void gtk_periodic_set_property (GObject              *object,
					    guint                 param_id,
					    const GValue         *value,
					    GParamSpec           *pspec);
static void gtk_periodic_get_property (GObject              *object,
					    guint                 param_id,
					    GValue               *value,
					    GParamSpec           *pspec);
static void on_clicked(GtkToggleButton *button, GtkPeriodic* periodic);
static void gtk_periodic_set_colors(GtkPeriodic *periodic);

GType
gtk_periodic_get_type (void)
{
	static GType periodic_type = 0;
  
	if (!periodic_type)
	{
		static const GTypeInfo periodic_info =
		{
			sizeof (GtkPeriodicClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) gtk_periodic_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (GtkPeriodic),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gtk_periodic_init,
		};

		periodic_type = g_type_register_static (GTK_TYPE_BIN, "GtkPeriodic", &periodic_info, 0);
	}
  
	return periodic_type;
}

void gtk_periodic_class_init (GtkPeriodicClass *class)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (class);
	GtkWidgetClass * widget_class = GTK_WIDGET_CLASS (class);
	parent_class = gtk_type_class(gtk_bin_get_type());
	
	gobject_class->set_property = gtk_periodic_set_property;
	gobject_class->get_property = gtk_periodic_get_property;
	class->element_changed_event = NULL;
	gtk_periodic_signals[ELEMENT_CHANGED] =
	g_signal_new ("element_changed",
				  G_TYPE_FROM_CLASS(gobject_class),
				  G_SIGNAL_RUN_LAST,
				  G_STRUCT_OFFSET(GtkPeriodicClass, element_changed_event),
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
			 g_param_spec_enum ("color-style", NULL, NULL,
								GTK_TYPE_PERIODIC_COLOR_STYLE_TYPE,
								GTK_PERIODIC_COLOR_NONE,
								(G_PARAM_READABLE | G_PARAM_WRITABLE)));
	gobject_class->finalize = gtk_periodic_finalize;
	widget_class->size_request = (void(*)(GtkWidget*, GtkRequisition*)) gtk_periodic_size_request;
	widget_class->size_allocate = (void(*)(GtkWidget*, GtkAllocation*)) gtk_periodic_size_allocate;
}

void gtk_periodic_init (GtkPeriodic *periodic)
{
	GladeXML* xml;
	GtkStyle* style;
	char name[8] = "elt";
	GtkToggleButton* button;
	int i;
	char* domain = g_strdup(textdomain(NULL));
	textdomain(GETTEXT_PACKAGE);
	xml =  glade_xml_new(DATADIR"/gchemutils/glade/gtkperiodic.glade", "vbox1", NULL);
	if (xml)  glade_xml_signal_autoconnect (xml);
	periodic->priv = g_new0(GtkPeriodicPrivate, 1);
	periodic->priv->tips = gtk_tooltips_new();
	periodic->priv->vbox = GTK_VBOX(glade_xml_get_widget(xml, "vbox1"));
	periodic->priv->colorstyle = GTK_PERIODIC_COLOR_NONE;
	memset(periodic->priv->buttons, 0, sizeof(GtkToggleButton*) * 119);
	for (i = 1; i <= 118; i++)
	{
		sprintf(name + 3, "%d", i);
		button = (GtkToggleButton*)glade_xml_get_widget(xml, name);
		if (GTK_IS_TOGGLE_BUTTON(button))
		{
			gtk_tooltips_set_tip(periodic->priv->tips, GTK_WIDGET(button), gcu_element_get_name(i), NULL);
			periodic->priv->buttons[i] = button;
			g_signal_connect(G_OBJECT(button), "toggled", (GCallback)on_clicked, periodic);
		}
	}
	style = gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(periodic->priv->buttons[1])));
	DefaultRed[0] = style->bg[0].red;
	DefaultGreen[0] = style->bg[0].green;
	DefaultBlue[0] = style->bg[0].blue;
	DefaultRed[1] = style->bg[1].red;
	DefaultGreen[1] = style->bg[1].green;
	DefaultBlue[1] = style->bg[1].blue;
	DefaultRed[2] = style->bg[2].red;
	DefaultGreen[2] = style->bg[2].green;
	DefaultBlue[2] = style->bg[2].blue;
	DefaultRed[3] = style->bg[3].red;
	DefaultGreen[3] = style->bg[3].green;
	DefaultBlue[3] = style->bg[3].blue;
	periodic->priv->Z = 0;
	gtk_container_add(GTK_CONTAINER(periodic), GTK_WIDGET(periodic->priv->vbox));
	gtk_widget_show_all(GTK_WIDGET(periodic));
	textdomain(domain);
	g_free(domain);
}

static void gtk_periodic_finalize (GObject *object)
{
	GtkPeriodic *periodic = (GtkPeriodic*) object;

	g_free (periodic->priv);

	if (G_OBJECT_CLASS (parent_class)->finalize)
		(* G_OBJECT_CLASS (parent_class)->finalize) (object);
}

GtkWidget* gtk_periodic_new()
{
	GtkBin* bin = GTK_BIN(g_object_new(GTK_TYPE_PERIODIC, NULL));
	return GTK_WIDGET(bin);
}

void on_clicked(GtkToggleButton *button, GtkPeriodic* periodic)
{
	static gboolean change = FALSE;
	if (button != periodic->priv->buttons[0])
	{
		const gchar* name;
		change = TRUE;
		if (periodic->priv->buttons[0]) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(periodic->priv->buttons[0]), FALSE);
		periodic->priv->buttons[0] = button;
		name = gtk_widget_get_name(GTK_WIDGET(periodic->priv->buttons[0]));
		periodic->priv->Z = atoi(name + 3);
		g_signal_emit(periodic, gtk_periodic_signals[ELEMENT_CHANGED], 0, periodic->priv->Z);
		change = FALSE;
	}
	else if (!change)
	{
		if (periodic->priv->can_unselect)
		{
			periodic->priv->buttons[0] = NULL;
			periodic->priv->Z = 0;
			g_signal_emit(periodic, gtk_periodic_signals[ELEMENT_CHANGED], 0, 0);
		}
		else if (periodic->priv->buttons[0]) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(periodic->priv->buttons[0]), TRUE);
	}
}

void gtk_periodic_size_allocate(GtkPeriodic* w, GtkAllocation *allocation)
{
	if (GTK_BIN(w)->child && GTK_WIDGET_VISIBLE (GTK_BIN(w)->child))
		gtk_widget_size_allocate (GTK_BIN(w)->child, allocation);
	(GTK_WIDGET_CLASS(parent_class))->size_allocate(GTK_WIDGET(w), allocation);
}

void gtk_periodic_size_request(GtkPeriodic* w, GtkRequisition *requisition)
{
	gtk_widget_size_request ((GTK_BIN(w))->child, requisition);
}

guint gtk_periodic_get_element(GtkPeriodic* periodic)
{
	g_return_if_fail(GTK_IS_PERIODIC(periodic));
	return periodic->priv->Z;
}

void gtk_periodic_set_element (GtkPeriodic* periodic, guint element)
{
	g_return_if_fail(GTK_IS_PERIODIC(periodic));
	if (periodic->priv->can_unselect && periodic->priv->buttons[0]) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(periodic->priv->buttons[0]), FALSE);
	if (element) 
	{
		gtk_toggle_button_set_active(periodic->priv->buttons[element], TRUE);
		periodic->priv->buttons[0] = periodic->priv->buttons[element];
		periodic->priv->Z = element;
	}
	else if (periodic->priv->can_unselect)
	{
		periodic->priv->buttons[0] = NULL;
		periodic->priv->Z = 0;
	}
}

static void
gtk_periodic_set_property (GObject              *object,
				guint                 param_id,
				const GValue         *value,
				GParamSpec           *pspec)
{
	GtkPeriodic *periodic;
	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_PERIODIC (object));

	periodic = GTK_PERIODIC (object);

	switch (param_id) {
	case PROP_CAN_UNSELECT:
		periodic->priv->can_unselect = g_value_get_boolean (value);
		break;

	case PROP_COLOR_STYLE:
		periodic->priv->colorstyle = g_value_get_enum (value);
		if (periodic->priv->colorstyle <= GTK_PERIODIC_COLOR_DEFAULT) gtk_periodic_set_colors(periodic);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gtk_periodic_get_property (GObject              *object,
				guint                 param_id,
				GValue               *value,
				GParamSpec           *pspec)
{
	GtkPeriodic *periodic;

	g_return_if_fail (object != NULL);
	g_return_if_fail (GTK_IS_PERIODIC (object));

	periodic = GTK_PERIODIC (object);

	switch (param_id) {
	case PROP_CAN_UNSELECT:
		g_value_set_boolean (value, periodic->priv->can_unselect);
		break;

	case PROP_COLOR_STYLE:
		g_value_set_enum (value, periodic->priv->colorstyle);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

void gtk_periodic_set_colors(GtkPeriodic *periodic)
{
	GtkStyle* style;
	const double *colors;
	int i;
	for (i = 1; i <= 118; i++)
	{
		if (!periodic->priv->buttons[i]) continue;
		style = gtk_style_copy(gtk_widget_get_style(GTK_WIDGET(periodic->priv->buttons[i])));
		switch (periodic->priv->colorstyle)
		{
			case GTK_PERIODIC_COLOR_NONE:
				style->bg[0].red = DefaultRed[0];
				style->bg[0].green = DefaultGreen[0];
				style->bg[0].blue = DefaultBlue[0];
				style->bg[1].red = DefaultRed[0];
				style->bg[1].green = DefaultGreen[1];
				style->bg[1].blue = DefaultBlue[1];
				style->bg[2].red = DefaultRed[1];
				style->bg[2].green = DefaultGreen[2];
				style->bg[2].blue = DefaultBlue[2];
				style->bg[3].red = DefaultRed[3];
				style->bg[3].green = DefaultGreen[3];
				style->bg[3].blue = DefaultBlue[3];
			break;
			case GTK_PERIODIC_COLOR_DEFAULT:
				colors = gcu_element_get_default_color(i);
				style->bg[0].red = style->bg[1].red = style->bg[2].red = style->bg[3].red = (guint16) (colors[0] * 65535.0);
				style->bg[0].green = style->bg[1].green = style->bg[2].green = style->bg[3].green = (guint16) (colors[1] * 65535.0);
				style->bg[0].blue = style->bg[1].blue = style->bg[2].blue = style->bg[3].blue = (guint16) (colors[2] * 65535.0);
				break;
		}
		gtk_widget_set_style(GTK_WIDGET(periodic->priv->buttons[i]), style);
		g_object_unref(style);
	}
}
