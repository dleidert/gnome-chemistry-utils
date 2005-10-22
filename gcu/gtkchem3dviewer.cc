/* 
 * Gnome Chemisty Utils
 * gtkchem3dviewer.c 
 *
 * Copyright (C) 2003-2004
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
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
#include "gtkchem3dviewer.h"
#include "matrix.h"
#include "element.h"
#include <libgnomevfs/gnome-vfs.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>
#ifdef HAVE_GTKGLAREA
#	include <gtkgl/gtkglarea.h>
#else
#	include <gtk/gtkgl.h>
#endif
#include <sstream>
#include <libintl.h>
#include <locale.h>
#define _(String) gettext(String)
#include <openbabel/mol.h>
#include <openbabel/obconversion.h>

/* Attribute list for gtkglarea widget. Specifies a
     list of Boolean attributes and enum/integer
     attribute/value pairs. The last attribute must be
     GDK_GL_NONE. See glXChooseVisual manpage for further
     explanation.
*/

static GdkGLConfig *glconfig = NULL;

enum {
	PROP_0,
	PROP_DISPLAY3D,
	PROP_BGCOLOR
};
GType
gtk_display3d_get_type (void)
{
  static GType etype = 0;
  if (etype == 0) {
    static const GEnumValue values[] = {
      { BALL_AND_STICK, "BALL_AND_STICK", "ball&stick" },
      { SPACEFILL, "SPACEFILL", "spacefill" },
      { 0, NULL, NULL }
    };
    etype = g_enum_register_static ("Dispay3D", values);
  }
  return etype;
}

using namespace std;
using namespace OpenBabel;
using namespace gcu;

typedef struct _GtkChem3DViewerPrivate
{
	guint glList;
	OBMol Mol;
	gdouble lastx, lasty;
	bool Init;
	gdouble Angle, Radius, MaxDist;
	gdouble psi, theta, phi;
	gdouble height, width, near, far;
	Matrix Euler;
	GtkWidget* widget;
	//background color
	float Blue, Red, Green, Alpha;
	Display3DMode display3d;
} GtkChem3DViewerPrivate;

static GtkBinClass *parent_class = NULL;

static void gtk_chem3d_viewer_class_init (GtkChem3DViewerClass  *klass);
static void gtk_chem3d_viewer_init(GtkChem3DViewer *viewer);
static void gtk_chem3d_viewer_finalize(GObject* object);
static void gtk_chem3d_viewer_update(GtkChem3DViewer *viewer);
static void gtk_chem3d_viewer_set_property(GObject *object, guint property_id,
						const GValue *value, GParamSpec *pspec);
static void gtk_chem3d_viewer_get_property(GObject *object, guint property_id,
						GValue *value, GParamSpec *pspec);

static bool on_init(GtkWidget* widget, GtkChem3DViewer *viewer) 
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	if (gdk_gl_drawable_gl_begin(gldrawable, glcontext))
	{
	    glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_COLOR_MATERIAL);
		float shiny = 25.0, spec[4] = {1.0, 1.0, 1.0, 1.0};
		glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &shiny);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
		gtk_chem3d_viewer_update(viewer);
		viewer->priv->Init = true;
    }
	return true;
}

static bool on_reshape(GtkWidget* widget, GdkEventConfigure *event, GtkChem3DViewer *viewer) 
{
	float fAspect;
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	if (gdk_gl_drawable_gl_begin(gldrawable, glcontext))
    {
		if (widget->allocation.height)
		{
			fAspect = (GLfloat)widget->allocation.width/(GLfloat)widget->allocation.height;
			if (fAspect == 0.0) fAspect = 1.0;
		}
		else	// don't divide by zero, not that we should ever run into that...
			fAspect = 1.0f;
		double x = viewer->priv->MaxDist;
		if (x == 0) x = 1;
		viewer->priv->Radius = (float) (x / sin(viewer->priv->Angle / 360 * M_PI)) ;
		glViewport(0,0, widget->allocation.width, widget->allocation.height);
		if (fAspect > 1.0)
		{
			viewer->priv->height = x * (1 - tan(viewer->priv->Angle / 360 * M_PI));
			viewer->priv->width = viewer->priv->height * fAspect;
		}
		else
		{
			viewer->priv->width = x * (1 - tan(viewer->priv->Angle / 360 * M_PI));
			viewer->priv->height = viewer->priv->width / fAspect;
		}
		viewer->priv->near = viewer->priv->Radius - x;
		viewer->priv->far = viewer->priv->Radius + x;
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
		glFrustum(- viewer->priv->width, viewer->priv->width, - viewer->priv->height, viewer->priv->height, viewer->priv->near , viewer->priv->far);
	    glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(0, 0, - viewer->priv->Radius);
	}
	return true;
}

static bool on_draw(GtkWidget *widget, GdkEventExpose *event, GtkChem3DViewer *viewer) 
{
	/* Draw only last expose. */
	if (event->count > 0) return true;
	GdkGLContext *glcontext = gtk_widget_get_gl_context(widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(widget);
	if (gdk_gl_drawable_gl_begin(gldrawable, glcontext))
    {
		glClearColor(viewer->priv->Red, viewer->priv->Green, viewer->priv->Blue, viewer->priv->Alpha);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		if  (viewer->priv->glList)
		{
			glPushMatrix();
			glRotated(viewer->priv->psi, 0.0, 1.0, 0.0);
			glRotated(viewer->priv->theta, 0.0, 0.0, 1.0);
			glRotated(viewer->priv->phi, 0.0, 1.0, 0.0);
			glCallList(viewer->priv->glList);
			glPopMatrix();
		}
	/* Swap backbuffer to front */
		gdk_gl_drawable_swap_buffers(gldrawable);
    }
	return true;
}

static bool on_motion(GtkWidget *widget, GdkEventMotion *event, GtkChem3DViewer *viewer) 
{
	gint x, y;
	GdkModifierType state;
	
	if (event->is_hint)
		gdk_window_get_pointer(event->window, &x, &y, &state);
	else
	{
	    x = (gint) event->x;
	    y = (gint) event->y;
	    state = (GdkModifierType) event->state;
	}
	if (state & GDK_BUTTON1_MASK)
	{
		if ((x == viewer->priv->lastx) && (y == viewer->priv->lasty)) return true;
		gdouble dx = x - viewer->priv->lastx, dy = y - viewer->priv->lasty;
		gdouble z = sqrt(dx*dx + dy*dy);
		Matrix Mat(0, (dy > 0) ? - acos(dx/z) : acos(dx/z), z * 0.00349065850398866, rotation);
		viewer->priv->Euler = Mat * viewer->priv->Euler;
		viewer->priv->Euler.Euler(viewer->priv->psi, viewer->priv->theta, viewer->priv->phi);
		viewer->priv->psi /= 0.0174532925199433;
		viewer->priv->theta /= 0.0174532925199433;
		viewer->priv->phi /= 0.0174532925199433;
		viewer->priv->lastx = x;
		viewer->priv->lasty = y;
		gtk_widget_queue_draw_area(widget, 0, 0, widget->allocation.width, widget->allocation.height);
	}
	return true;
}

static bool on_pressed(GtkWidget *widget, GdkEventButton *event, GtkChem3DViewer *viewer) 
{
	if (event->button == 1) {
		// beginning of drag, reset mouse position
		viewer->priv->lastx = event->x;
		viewer->priv->lasty = event->y;
		return true;
	}
	return false;
}

static void on_destroyed(GtkWidget *widget, GtkChem3DViewer *viewer)
{
}


static void on_size(GtkWidget *w, GtkAllocation *allocation, gpointer data)
{
	if (GTK_BIN(w)->child && GTK_WIDGET_VISIBLE (GTK_BIN(w)->child))
		gtk_widget_size_allocate (GTK_BIN(w)->child, allocation);
}

extern "C"
{

GType
gtk_chem3d_viewer_get_type (void)
{
	static GType chem3d_viewer_type = 0;
  
	if (!chem3d_viewer_type)
	{
		static const GTypeInfo chem3d_viewer_info =
		{
			sizeof (GtkChem3DViewerClass),
			NULL,           /* base_init */
			NULL,           /* base_finalize */
			(GClassInitFunc) gtk_chem3d_viewer_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (GtkChem3DViewer),
			0,              /* n_preallocs */
			(GInstanceInitFunc) gtk_chem3d_viewer_init,
		};

		chem3d_viewer_type = g_type_register_static (GTK_TYPE_BIN, "GtkChem3DViewer", &chem3d_viewer_info, (GTypeFlags)0);
	}
  
	return chem3d_viewer_type;
}

GtkWidget* gtk_chem3d_viewer_new(gchar *uri)
{
	GtkChem3DViewer* viewer = (GtkChem3DViewer*)g_object_new(GTK_TYPE_CHEM3D_VIEWER, NULL);
	g_signal_connect(G_OBJECT(viewer), "size_allocate", GTK_SIGNAL_FUNC(on_size), NULL);
	gtk_chem3d_viewer_set_uri (viewer, uri);
	return GTK_WIDGET(viewer);
}

} //extern "C"

void gtk_chem3d_viewer_class_init(GtkChem3DViewerClass  *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	parent_class = (GtkBinClass*)gtk_type_class(gtk_bin_get_type());
	
	gobject_class->finalize = gtk_chem3d_viewer_finalize;
	gobject_class->set_property = gtk_chem3d_viewer_set_property;
	gobject_class->get_property = gtk_chem3d_viewer_get_property;
	
	g_object_class_install_property (
		gobject_class,
		PROP_DISPLAY3D,
		g_param_spec_enum("display3d",
						"3D display mode",
						"Mode used to display the model",
						GTK_DISPLAY_3D,
						BALL_AND_STICK,
						(GParamFlags)G_PARAM_READWRITE));
        g_object_class_install_property
                (gobject_class,
                 PROP_BGCOLOR,
                 g_param_spec_string ("bgcolor",
				      _("Background Color"),
				      _("Color used to paint the background"),
                                      "black",
                                      (GParamFlags)(G_PARAM_READABLE | G_PARAM_WRITABLE)));
}

void gtk_chem3d_viewer_init(GtkChem3DViewer *viewer)
{
	g_return_if_fail (GTK_IS_CHEM3D_VIEWER(viewer));
	viewer->priv = new GtkChem3DViewerPrivate;
	/* Create new OpenGL widget. */
	if (glconfig == NULL)
	{
		/* Check if OpenGL is supported. */
		if (!gdk_gl_query_extension())
		{
			g_print("\n*** OpenGL is not supported.\n");
			exit(1);
		}
	
		/* Configure OpenGL-capable visual. */
	
		/* Try double-buffered visual */
		glconfig = gdk_gl_config_new_by_mode(GdkGLConfigMode(GDK_GL_MODE_RGB |
											GDK_GL_MODE_DEPTH |
											GDK_GL_MODE_DOUBLE));
		if (glconfig == NULL)
		{
			g_print("*** Cannot find the double-buffered visual.\n");
			  exit(1);
		}
	}
	/* create new OpenGL widget */
	viewer->priv->widget = GTK_WIDGET(gtk_drawing_area_new());
	
	/* Set OpenGL-capability to the widget. */
	gtk_widget_set_gl_capability(viewer->priv->widget,
                                glconfig,
                                NULL,
                                TRUE,
                                GDK_GL_RGBA_TYPE);


	viewer->priv->Angle = 10;
	viewer->priv->psi = 0.0;
	viewer->priv->theta = 0.0;
	viewer->priv->phi = 0.0;
	Matrix m(0, 0, 0, euler);
	viewer->priv->Euler = m;
// Set background to white
	viewer->priv->Red = viewer->priv->Green = viewer->priv->Blue = 0.;
	viewer->priv->Alpha = 1.0;
// Set Ball and Stick mode by default
	viewer->priv->display3d = BALL_AND_STICK;
// Events for widget must be set before X Window is created
	gtk_widget_set_events(GTK_WIDGET(viewer->priv->widget),
			GDK_EXPOSURE_MASK|
			GDK_BUTTON_MOTION_MASK|
			GDK_POINTER_MOTION_HINT_MASK|
			GDK_BUTTON_PRESS_MASK);

// Connect signal handlers
// Do initialization when widget has been realized.
	g_signal_connect(G_OBJECT(viewer->priv->widget), "realize",
		     GTK_SIGNAL_FUNC(on_init), viewer);
// When window is resized viewport needs to be resized also.
	g_signal_connect(G_OBJECT(viewer->priv->widget), "configure_event",
		     GTK_SIGNAL_FUNC(on_reshape), viewer);
// Redraw image when exposed. 
	g_signal_connect(G_OBJECT(viewer->priv->widget), "expose_event",
		     GTK_SIGNAL_FUNC(on_draw), viewer);
// When moving mouse 
  g_signal_connect (G_OBJECT(viewer->priv->widget), "motion_notify_event",
		      GTK_SIGNAL_FUNC(on_motion), viewer);
// When a mouse button is pressed
  g_signal_connect (G_OBJECT(viewer->priv->widget), "button_press_event",
		      GTK_SIGNAL_FUNC(on_pressed), viewer);
// When a widget is destroyed
	g_signal_connect (G_OBJECT(viewer->priv->widget), "destroy", GTK_SIGNAL_FUNC(on_destroyed), viewer);

	gtk_widget_show(GTK_WIDGET(viewer->priv->widget));
	gtk_container_add(GTK_CONTAINER(viewer), viewer->priv->widget);
	gtk_widget_show_all(GTK_WIDGET(viewer));
	viewer->priv->Init = false;
}

void gtk_chem3d_viewer_finalize(GObject* object)
{
	((GObjectClass*)parent_class)->finalize(object);
	GtkChem3DViewer* viewer = GTK_CHEM3D_VIEWER(object);
	delete viewer->priv;
}

void gtk_chem3d_viewer_set_uri (GtkChem3DViewer * viewer, gchar *uri)
{
	g_return_if_fail (GTK_IS_CHEM3D_VIEWER (viewer));
	g_return_if_fail (uri);
	GnomeVFSHandle *handle;
	GnomeVFSFileInfo *info = gnome_vfs_file_info_new ();
	GnomeVFSResult result = gnome_vfs_open (&handle, uri, GNOME_VFS_OPEN_READ);
	if (result != GNOME_VFS_OK) {
		gnome_vfs_file_info_unref (info);
		return;
	}
	gnome_vfs_get_file_info_from_handle (handle, info,
		(GnomeVFSFileInfoOptions)(GNOME_VFS_FILE_INFO_GET_MIME_TYPE |
							GNOME_VFS_FILE_INFO_FORCE_SLOW_MIME_TYPE));
	gchar *buf = new gchar[info->size + 1];
	GnomeVFSFileSize n;
	gnome_vfs_read (handle, buf, info->size, &n);
	buf[info->size] = 0;
	if (n == info->size)
		gtk_chem3d_viewer_set_data (viewer, buf, info->mime_type);
	gnome_vfs_file_info_unref (info);
	delete [] buf;
	g_free (handle);
}

void gtk_chem3d_viewer_set_data(GtkChem3DViewer * viewer, const gchar *data, const gchar* mime_type)
{
	istringstream is(data);
	viewer->priv->Mol.Clear ();
	char *old_num_locale = g_strdup(setlocale(LC_NUMERIC, NULL));
	setlocale(LC_NUMERIC, "C");
	OBConversion Conv;
	OBFormat* pInFormat = Conv.FormatFromMIME(mime_type);
	Conv.SetInAndOutFormats(pInFormat, pInFormat);
	Conv.Read(&viewer->priv->Mol,&is);
	setlocale(LC_NUMERIC, old_num_locale);
	if (viewer->priv->Init) gtk_chem3d_viewer_update(viewer);
	g_free(old_num_locale);
}

void gtk_chem3d_viewer_update(GtkChem3DViewer *viewer)
{
	GdkGLContext *glcontext = gtk_widget_get_gl_context(viewer->priv->widget);
	GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(viewer->priv->widget);
	if (gdk_gl_drawable_gl_begin(gldrawable, glcontext))
 	{
		if (viewer->priv->glList) glDeleteLists(viewer->priv->glList,1);
		viewer->priv->glList = glGenLists(1);
		glNewList(viewer->priv->glList, GL_COMPILE);
		std::vector< OBNodeBase * >::iterator i;
		OBAtom* atom = viewer->priv->Mol.BeginAtom(i);
		guint Z;
		gdouble R, w, x, y, z, x0, y0, z0, dist;
		x0 = y0 = z0 = 0.0;
		const gdouble* color;
		while (atom)
		{
			Z = atom->GetAtomicNum();
			x0 += atom->GetX();
			y0 += atom->GetY();
			z0 += atom->GetZ();
			atom = viewer->priv->Mol.NextAtom(i);
		}
		x0 /= viewer->priv->Mol.NumAtoms();
		y0 /= viewer->priv->Mol.NumAtoms();
		z0 /= viewer->priv->Mol.NumAtoms();
		atom = viewer->priv->Mol.BeginAtom(i);
		GLUquadricObj *quadObj ;
		dist = 0;
		while (atom)
		{
			Z = atom->GetAtomicNum();
			R = etab.GetVdwRad(Z);
			if (viewer->priv->display3d == BALL_AND_STICK) R*= 0.2;
			x = atom->GetX() - x0;
			y = atom->GetY() - y0;
			z = atom->GetZ() - z0;
			color = gcu_element_get_default_color(Z);
			if ((w = sqrt(x * x + y * y + z * z)) > dist - R)
				dist = w + R;
			glPushMatrix() ;
			glTranslated(x, y, z) ;
			glColor3d(color[0], color[1], color[2]) ;
			quadObj = gluNewQuadric() ;
			gluQuadricDrawStyle(quadObj, GL_FILL);
			gluQuadricNormals(quadObj, GL_SMOOTH) ;
			gluSphere(quadObj, R, 20, 10) ;
			gluDeleteQuadric(quadObj) ;
			glPopMatrix() ;
			atom = viewer->priv->Mol.NextAtom(i);
		}
		viewer->priv->MaxDist = dist * 1.05;
		std::vector< OBEdgeBase * >::iterator j;
		OBBond* bond = viewer->priv->Mol.BeginBond(j);
		double x1, y1, z1, arot, xrot, yrot;
		if (viewer->priv->display3d == BALL_AND_STICK) while(bond)
		{
			atom = bond->GetBeginAtom();
			x = atom->GetX() - x0;
			y = atom->GetY() - y0;
			z = atom->GetZ() - z0;
			atom = bond->GetEndAtom();
			x1 = atom->GetX() - x0 - x;
			y1 = atom->GetY() - y0 - y;
			z1 = atom->GetZ() - z0 - z;
			dist = sqrt(x1 * x1 + y1 * y1 + z1 * z1);
			w = sqrt(x1 * x1 + y1 * y1);
			if (w > 0)
			{
				xrot = - y1 / w ;
				yrot = x1 / w ;
				arot = atan2(w, z1) * 90 / 1.570796326794897 ;
			}
			else
			{
				xrot = 0;
				if (z1 > 0) yrot = arot = 0.0;
				else
				{
					yrot = 1.0;
					arot = 180.0;
				}
			}
			glPushMatrix();
			glTranslated(x, y, z);
			glRotated(arot, xrot, yrot, 0.0f);
			glColor3f(0.75, 0.75, 0.75);
			quadObj = gluNewQuadric();
			gluQuadricDrawStyle(quadObj, GL_FILL);
			gluQuadricNormals(quadObj, GL_SMOOTH);
			gluCylinder(quadObj, 0.12, 0.12, dist, 20, 10);
			gluDeleteQuadric(quadObj);
			glPopMatrix();
			bond = viewer->priv->Mol.NextBond(j);
		}
		glEndList();
	}
	on_reshape(viewer->priv->widget, NULL, viewer);
}

static void gtk_chem3d_viewer_get_property (GObject *object, guint property_id,
				     GValue *value, GParamSpec *pspec)
{
	GtkChem3DViewer *viewer = GTK_CHEM3D_VIEWER(object);

	switch (property_id) {
	case PROP_DISPLAY3D:
		g_value_set_enum (value, viewer->priv->display3d);
		break;
	case PROP_BGCOLOR:
		{
			int r = (int) (viewer->priv->Red * 255.), g = (int) (viewer->priv->Green * 255.), b = (int) (viewer->priv->Blue * 255.);
			if ((r ==0) && (g == 0) && (b == 0))
				g_value_set_string (value, "black");
			else if ((r ==255) && (g == 255) && (b == 255))
				g_value_set_string (value, "white");
			else
			{
				char buf[10];
				g_snprintf(buf, sizeof(buf), "#%2x%2x%2x", r, g, b);
				g_value_set_string(value, buf);
			}
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void gtk_chem3d_viewer_set_property (GObject *object, guint property_id,
				     const GValue *value, GParamSpec *pspec)
{
	GtkChem3DViewer *viewer = GTK_CHEM3D_VIEWER(object);

	switch (property_id) {
		case PROP_DISPLAY3D:
			viewer->priv->display3d = (Display3DMode)g_value_get_enum (value);
			break;
		case PROP_BGCOLOR:
			{
				const gchar* str = g_value_get_string(value);
				if (!strcmp(str, "black"))
					viewer->priv->Red = viewer->priv->Green = viewer->priv->Blue = 0.;
				else if (!strcmp(str, "white"))
					viewer->priv->Red = viewer->priv->Green = viewer->priv->Blue = 1.;
				else
				{
					if ((strlen(str) != 7) || (*str != '#'))
					{
						g_warning("Unrecognized color: %s\n", str);
						break;
					}
					int r, g, b;
					r = strtoul(str + 1, NULL, 16);
					b = r &0xff;
					 viewer->priv->Blue = (float) b / 255.;
					r >>= 8;
					g = r &0xff;
					 viewer->priv->Green = (float) g / 255.;
					r >>=8;
					 viewer->priv->Red = (float) r / 255.;
				}
			}
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
			break;
	}
	gtk_chem3d_viewer_update(viewer);
}
