/* 
 * GChemPaint library
 * fontsel.h 
 *
 * Copyright (C) 2006-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCP_FONT_SEL_H
#define GCP_FONT_SEL_H

#include <glib-object.h>

/*!\file*/

G_BEGIN_DECLS

typedef struct _GcpFontSel GcpFontSel;

#define GCP_FONT_SEL_TYPE     (gcp_font_sel_get_type ())
#define GCP_FONT_SEL(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCP_FONT_SEL_TYPE, GcpFontSel))
#define IS_GCP_FONT_SEL(obj)  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GCP_FONT_SEL_TYPE))

GType gcp_font_sel_get_type (void);

G_END_DECLS

#endif	/* GCP_FONT_SEL_H */
