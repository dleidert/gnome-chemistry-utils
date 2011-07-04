/*
 * GChemPaint templates plugin
 * gtkcombotoolitem.h
 *
 * Copyright (C) 2004-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GTK_COMBO_TOOl_ITEM_H
#define GTK_COMBO_TOOl_ITEM_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_TOOL_COMBO            (gtk_tool_combo_get_type ())
#define GTK_TOOL_COMBO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_TOOL_COMBO, GtkToolCombo))
#define GTK_TOOL_COMBO_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_TOOL_COMBO, GtkToolComboClass))
#define GTK_IS_TOOL_COMBO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_TOOL_COMBO))
#define GTK_IS_TOOL_COMBO_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), GTK_TYPE_TOOL_COMBO))
#define GTK_TOOL_COMBO_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), GTK_TYPE_TOOL_COMBO, GtkToolComboClass))

typedef struct _GtkToolCombo        GtkToolCombo;
typedef struct _GtkToolComboClass   GtkToolComboClass;


struct _GtkToolCombo
{
	GtkToolItem parent;
	GtkComboBox* box;
};

struct _GtkToolComboClass
{
  GtkToolItemClass parent_class;
};

GType        gtk_tool_combo_get_type       (void);

GtkToolItem* gtk_tool_combo_new_with_model (GtkTreeModel* model);


G_END_DECLS

#endif  /* GTK_COMBO_TOOl_ITEM_H */
