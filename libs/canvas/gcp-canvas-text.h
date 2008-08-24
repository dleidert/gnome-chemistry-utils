/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02111-1307, USA.
 */
/*
  @NOTATION@
 */
/* Text item type for GnomeCanvas widget
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 *
 * Author: Federico Mena <federico@nuclecu.unam.mx>
 * Port to Pango co-done by Gergõ Érdi <cactus@cactus.rulez.org>
 */

#ifndef GNOME_CANVAS_TEXT_EXT_H
#define GNOME_CANVAS_TEXT_EXT_H

#include <libgnomecanvas/gnome-canvas-text.h>

typedef struct _GnomeCanvasTextExt      GnomeCanvasTextExt;
typedef struct _GnomeCanvasTextExtClass GnomeCanvasTextExtClass;

G_BEGIN_DECLS
#define GNOME_TYPE_CANVAS_TEXT_EXT            (gnome_canvas_text_get_type ())
#define GNOME_CANVAS_TEXT_EXT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GNOME_TYPE_CANVAS_TEXT_EXT, GnomeCanvasTextExt))
#define GNOME_CANVAS_TEXT_EXT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_TEXT_EXT, GnomeCanvasTextExtClass))
#define GNOME_IS_CANVAS_TEXT_EXT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GNOME_TYPE_CANVAS_TEXT_EXT))
#define GNOME_IS_CANVAS_TEXT_EXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_TEXT_EXT))
#define GNOME_CANVAS_TEXT_EXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GNOME_TYPE_CANVAS_TEXT_EXT, GnomeCanvasTextExtClass))


struct _GnomeCanvasTextExt {
	GnomeCanvasText text;

	gboolean rendered;
};

struct _GnomeCanvasTextExtClass {
	GnomeCanvasTextClass parent_class;
};

GType gnome_canvas_text_ext_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
