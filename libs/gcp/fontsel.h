/*
 * GChemPaint library
 * fontsel.h
 *
 * Copyright (C) 2006-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCP_FONT_SEL_H
#define GCP_FONT_SEL_H

#include <glib-object.h>

/*!\file
Declaration of the GcpFontSel widget.
*/

G_BEGIN_DECLS

typedef struct _GcpFontSel GcpFontSel;

/*!@return the GType associated to GcpFontSel */
#define GCP_TYPE_FONT_SEL     (gcp_font_sel_get_type ())
/*!
Casts \a obj to a GcpFontSel * pointer.
@return a pointer to the GcpFontSel * or NULL if \a obj does not point to
a GcpFontSel widget.
*/
#define GCP_FONT_SEL(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj), GCP_TYPE_FONT_SEL, GcpFontSel))
/*!
\return TRUE if \a obj points to a GcpFontSel widget, FALSE otherwise.
*/
#define IS_GCP_FONT_SEL(obj)  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GCP_TYPE_FONT_SEL))

/*!@return the GType associated to GcpFontSel */
GType gcp_font_sel_get_type (void);

/*! \struct GcpFontSel gcp/fontsel.h
The GcpFontSel Widget is a font chooser widget used in GChemPaint.

\image html gcpfontsel.png

<hr>
<h2>Signals</h2>

 This widget has one signal:
- "changed": void	user_function (GcpFontSel *fc, gpointer data).
@param fc the object which received the signal.
@param data user data set when the signal handler was connected.

This signal is raised when any property changes in the widget.

<hr>
<h2>Properties</h2>
There are six properties:
- "family": char* (Read / Write).
	<br>The font family. Default is "Bitstream Vera Sans".

- "style": PangoStyle (Read / Write).
	<br>The font style. Default is PANGO_STYLE_NORMAL.

- "weight": PangoWeight (Read / Write).
	<br>The font weight. Default is PANGO_WEIGHT_NORMAL

- "variant": PangoVariant (Read / Write).
	<br>The font variant. Default is PANGO_VARIANT_NORMAL

- "stretch": PangoStretch (Read / Write).
	<br>The font stretch. Default is PANGO_STRETCH_NORMAL

- "size": int (Read / Write).
	<br>The font size expressed in pango units. Default is 12*PANGO_SCALE.

- "allow-slanted": gboolean (Construct only).
	<br>Whether to allow slanted fonts. Default is TRUE.

- "label": string (Read / Write).
	<br>The preview text. Default is the font description.
*/

G_END_DECLS

#endif	/* GCP_FONT_SEL_H */
