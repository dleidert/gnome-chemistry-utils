/* Bpath item type for GnomeCanvas widget
 *
 * GnomeCanvas is basically a port of the Tk toolkit's most excellent canvas widget.  Tk is
 * copyrighted by the Regents of the University of California, Sun Microsystems, and other parties.
 *
 * Copyright (C) 1998,1999 The Free Software Foundation
 *
 * Authors: Federico Mena <federico@nuclecu.unam.mx>
 *          Raph Levien <raph@acm.org>
 *          Lauris Kaplinski <lauris@ximian.com>
 *          Rusty Conover <rconover@bangtail.net>
 */

#ifndef GNOME_CANVAS_BPATH_EXT_H
#define GNOME_CANVAS_BPATH_EXT_H

#include <libgnomecanvas/gnome-canvas.h>
#include "gcp-canvas-shape.h"
#include <libgnomecanvas/gnome-canvas-path-def.h>

G_BEGIN_DECLS


/* Bpath item for the canvas.
 *
 * The following object arguments are available:
 *
 * name			type			read/write	description
 * ------------------------------------------------------------------------------------------
 * bpath		GnomeCanvasPathDef *		RW		Pointer to an GnomeCanvasPathDef structure.
 *								This can be created by a call to
 *								gp_path_new() in (gp-path.h).
 */

#define GNOME_TYPE_CANVAS_BPATH_EXT            (gnome_canvas_bpath_ext_get_type ())
#define GNOME_CANVAS_BPATH_EXT(obj)            (GTK_CHECK_CAST ((obj), GNOME_TYPE_CANVAS_BPATH_EXT, GnomeCanvasBpathExt))
#define GNOME_CANVAS_BPATH_EXT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CANVAS_BPATH_EXT, GnomeCanvasBpathExtClass))
#define GNOME_IS_CANVAS_BPATH_EXT(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_CANVAS_BPATH_EXT))
#define GNOME_IS_CANVAS_BPATH_EXT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CANVAS_BPATH_EXT))


typedef struct _GnomeCanvasBpathExt GnomeCanvasBpathExt;
/*typedef struct _GnomeCanvasBpathExtPriv GnomeCanvasBpathExtPriv;*/
typedef struct _GnomeCanvasBpathExtClass GnomeCanvasBpathExtClass;

struct _GnomeCanvasBpathExt {
	GnomeCanvasShapeExt item;
	
};

struct _GnomeCanvasBpathExtClass {
	GnomeCanvasShapeExtClass parent_class;
};


/* Standard Gtk function */
GType gnome_canvas_bpath_ext_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif
