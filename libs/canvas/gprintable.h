/* 
 * gprintable.h 
 *
 * Copyright (C) 2003 Jean Bréfort <jean.brefort@normalesup.org>
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
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef _INC_G_PRINTABLE_H
#define _INC_G_PRINTABLE_H

#include <glib-object.h>
#include <libgnomeprint/gnome-print.h>
#include <libxml/tree.h>

G_BEGIN_DECLS

typedef struct _GPrintable GPrintable;
typedef struct _GPrintableIface GPrintableIface;

struct _GPrintableIface {
  GTypeInterface interface;

  /* virtual functions */
  void (*print)       (GPrintable *, GnomePrintContext*);
  void (*export_svg)  (GPrintable *, xmlDocPtr, xmlNodePtr);
};

#define G_TYPE_PRINTABLE (g_printable_get_type())
#define G_PRINTABLE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), G_TYPE_PRINTABLE, GPrintable))
#define G_IS_PRINTABLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),  G_TYPE_PRINTABLE))
#define G_PRINTABLE_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE((obj), G_TYPE_PRINTABLE, GPrintableIface))

GType    g_printable_get_type              (void);

void     g_printable_print             (GPrintable *gprintable, GnomePrintContext *pc);
void     g_printable_export_svg        (GPrintable *gprintable, xmlDocPtr doc, xmlNodePtr node);

G_END_DECLS

#endif	/*_INC_G_PRINTABLE_H*/
