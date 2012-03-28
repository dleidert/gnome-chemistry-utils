/*
 * GChemPaint templates plugin
 * gtkcombotoolitem.c
 *
 * Copyright (C) 2004-2005 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "gtkcombotoolitem.h"

static void
gtk_tool_combo_class_init (G_GNUC_UNUSED GtkToolItemClass *tool_item_class)
{
}

static void
gtk_tool_combo_init (G_GNUC_UNUSED GtkToolCombo *item)
{
}

GType	gtk_tool_combo_get_type (void)
{
	static GType type = 0;
	if (type == 0) {
		static GTypeInfo const object_info = {
			sizeof (GtkToolComboClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) gtk_tool_combo_class_init,
			(GClassFinalizeFunc) NULL,
			NULL,
			sizeof (GtkToolCombo),
			0,
			(GInstanceInitFunc) gtk_tool_combo_init,
			NULL
		};
		type = g_type_register_static (GTK_TYPE_TOOL_ITEM, "GtkItemCombo",
			&object_info, 0);
	}
	return type;
}

GtkToolItem* gtk_tool_combo_new_with_model (GtkTreeModel* model)
{
 	GtkToolCombo* item = g_object_new (GTK_TYPE_TOOL_COMBO, NULL);
	GtkWidget* combo = gtk_combo_box_new_with_model (model);
	GtkCellRenderer *renderer = GTK_CELL_RENDERER (gtk_cell_renderer_text_new());
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, TRUE);
   gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo), renderer,
                                        "text", 0,
                                        NULL);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
	gtk_container_add (GTK_CONTAINER (item), combo);
	gtk_widget_show_all(combo);
	return (GtkToolItem*) item;
}
