/* 
 * Gnome Chemisty Utils
 * spectrumview.cc
 *
 * Copyright (C) 2007-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "spectrumdoc.h"
#include "spectrumview.h"
#include <goffice/data/go-data-simple.h>
#include <goffice/graph/gog-data-set.h>
#include <goffice/graph/gog-graph.h>
#include <goffice/graph/gog-label.h>
#include <goffice/graph/gog-object.h>
#include <goffice/graph/gog-plot.h>
#include <goffice/graph/gog-series.h>
#ifndef GOG_TYPE_LABEL
#   define GOG_TYPE_LABEL GOG_LABEL_TYPE
#   include <goffice/graph/gog-style.h>
#   define GOStyle GogStyle
#   define go_styled_object_get_style gog_styled_object_get_style
#   define GO_STYLED_OBJECT GOG_STYLED_OBJECT
#else
#   include <goffice/utils/go-style.h>
#   include <goffice/utils/go-styled-object.h>
#endif
#include <goffice/gtk/go-graph-widget.h>
#include <goffice/utils/go-image.h>
#include <gsf/gsf-output-gio.h>
#include <glib/gi18n-lib.h>
#include <cmath>
#include <map>
#include <string>

using namespace std;

namespace gcu
{

static void on_min_changed (SpectrumView *view)
{
	view->OnMinChanged ();
}

static void on_max_changed (SpectrumView *view)
{
	view->OnMaxChanged ();
}

static void on_xrange_changed (SpectrumView *view)
{
	view->OnXRangeChanged ();
}
	
SpectrumView::SpectrumView (SpectrumDocument *pDoc)
{
	m_Doc = pDoc;
	m_Widget = go_graph_widget_new (NULL);
	GogGraph *graph = go_graph_widget_get_graph (GO_GRAPH_WIDGET (m_Widget));
	/* Add a title */
	GogLabel *label = (GogLabel *) g_object_new (GOG_TYPE_LABEL, NULL);
	gog_object_add_by_name (GOG_OBJECT (graph), "Title", GOG_OBJECT (label));
	/* Get the chart created by the widget initialization */
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	/* Create a scatter plot and add it to the chart */
	GogPlot *plot = (GogPlot *) gog_plot_new_by_name ("GogXYPlot");
	g_object_set (plot, "default-style-has-markers", false, NULL);
	gog_object_add_by_name (GOG_OBJECT (chart), "Plot", GOG_OBJECT (plot));
	/* Create a series for the plot and populate it with some simple data */
	m_Series = gog_plot_new_series (plot);
	m_OptionBox = gtk_vbox_new (false, 5);
	GtkWidget *box = gtk_hbox_new (false, 5);
	GtkWidget *w = gtk_label_new (_("Minimum X value:"));
	gtk_box_pack_start (GTK_BOX (box), w, false, false, 0);
	xminbtn = GTK_SPIN_BUTTON (gtk_spin_button_new_with_range (0., 1., 0.1));
	minsgn = g_signal_connect_swapped (xminbtn, "value-changed", G_CALLBACK (on_min_changed), this);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (xminbtn), false, false, 0);
	w = gtk_label_new (_("Maximum X value:"));
	gtk_box_pack_start (GTK_BOX (box), w, false, false, 0);
	xmaxbtn = GTK_SPIN_BUTTON (gtk_spin_button_new_with_range (0., 1., 0.1));
	maxsgn = g_signal_connect_swapped (xmaxbtn, "value-changed", G_CALLBACK (on_max_changed), this);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (xmaxbtn), false, false, 0);
	xrange = GTK_RANGE (gtk_hscrollbar_new (NULL));
	gtk_widget_set_sensitive (GTK_WIDGET (xrange), false);
	xrangesgn = g_signal_connect_swapped (xrange, "value-changed", G_CALLBACK (on_xrange_changed), this);
	gtk_box_pack_start (GTK_BOX (box), GTK_WIDGET (xrange), true, true, 0);
	gtk_box_pack_start (GTK_BOX (m_OptionBox), box, false, false, 0);
}

SpectrumView::~SpectrumView ()
{
}

void SpectrumView::SetAxisBounds (GogAxisType target, double min, double max, bool inverted)
{
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	GSList *axes = gog_chart_get_axes (chart, target);
	GogAxis *axis = GOG_AXIS (axes->data);
	gog_axis_set_bounds (axis, min, max);
	g_object_set (axis, "invert-axis", inverted, NULL);
	if (target == GOG_AXIS_X) {
		double l = log10 (fabs (max - min));
		int n = (l < 3)? rint (3 - l): 0;
		xstep = pow (10., -n);
		g_signal_handler_block (xminbtn, minsgn); 
		g_signal_handler_block (xmaxbtn, maxsgn); 
		gtk_spin_button_set_range (xminbtn, min, max); 
		gtk_spin_button_set_range (xmaxbtn, min, max); 
		gtk_spin_button_set_increments (xminbtn, xstep, 100 * xstep); 
		gtk_spin_button_set_increments (xmaxbtn, xstep, 100 * xstep);
		gtk_range_set_increments (xrange, xstep, 100 * xstep);
		gtk_range_set_inverted (xrange, !inverted);
		g_signal_handler_block (xrange, xrangesgn);
		gtk_range_set_value (xrange, 0.);
		gtk_widget_set_sensitive (GTK_WIDGET (xrange), false);
		g_signal_handler_unblock (xrange, xrangesgn);
		gtk_spin_button_set_value (xminbtn, min);
		gtk_spin_button_set_value (xmaxbtn, max);
		gtk_spin_button_set_digits (xminbtn, n);
		gtk_spin_button_set_digits (xmaxbtn, n);
		g_signal_handler_unblock (xminbtn, minsgn); 
		g_signal_handler_unblock (xmaxbtn, maxsgn); 
		xmin = min;
		xmax = max;
	}
}

void SpectrumView::SetAxisLabel (GogAxisType target, char const *unit)
{
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	GSList *axes = gog_chart_get_axes (chart, target);
	GogObject *axis = GOG_OBJECT (axes->data);
	GOData *data = go_data_scalar_str_new (unit, false);
	GogObject *label = gog_object_get_child_by_name (axis, "Label");
	if (label) {
		// remove the old label if any
		gog_object_clear_parent (label);
		g_object_unref (label);
	}
	label = GOG_OBJECT (g_object_new (GOG_TYPE_LABEL, NULL));
	gog_dataset_set_dim (GOG_DATASET (label), 0, data, NULL);
	gog_object_add_by_name (axis, "Label", label);
	
}

void SpectrumView::ShowAxis (GogAxisType target, bool show)
{
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	GSList *axes = gog_chart_get_axes (chart, target);
	GogObject *axis = GOG_OBJECT (axes->data);
	g_object_set (G_OBJECT (axis), "major-tick-labeled", false, NULL);
	GOStyle *style = go_styled_object_get_style (GO_STYLED_OBJECT (axis));
	style->line.dash_type = GO_LINE_NONE;
	style->line.auto_dash = false;
}

void SpectrumView::Render (cairo_t *cr, double width, double height)
{
	gog_graph_render_to_cairo (go_graph_widget_get_graph (GO_GRAPH_WIDGET (m_Widget)), cr, width, height);
}

void SpectrumView::OnMinChanged ()
{
	double min = gtk_spin_button_get_value (xminbtn);
	double max = gtk_spin_button_get_value (xmaxbtn);
	if (max <= min) {
		double step;
		gtk_spin_button_get_increments (xminbtn, &step, NULL);
		min = max - step;
		g_signal_handler_block (xminbtn, minsgn); 
		gtk_spin_button_set_value (xminbtn, min);
		g_signal_handler_unblock (xminbtn, minsgn); 
	}
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	GSList *axes = gog_chart_get_axes (chart, GOG_AXIS_X);
	GogAxis *axis = GOG_AXIS (axes->data);
	gog_axis_set_bounds (axis, min, max);
	g_signal_handler_block (xrange, xrangesgn);
	if (max - min < xmax - xmin) {
		gtk_range_set_range (xrange, 0., xmax - xmin - max + min);
		gtk_range_set_value (xrange, min - xmin);
		gtk_widget_set_sensitive (GTK_WIDGET (xrange), true);
	} else {
		gtk_range_set_value (xrange, 0.);
		gtk_widget_set_sensitive (GTK_WIDGET (xrange), false);
	}
	g_signal_handler_unblock (xrange, xrangesgn);
}

void SpectrumView::OnMaxChanged ()
{
	double min = gtk_spin_button_get_value (xminbtn);
	double max = gtk_spin_button_get_value (xmaxbtn);
	if (max <= min) {
		double step;
		gtk_spin_button_get_increments (xmaxbtn, &step, NULL);
		max = min + step;
		g_signal_handler_block (xmaxbtn, maxsgn); 
		gtk_spin_button_set_value (xmaxbtn, max);
		g_signal_handler_unblock (xmaxbtn, maxsgn); 
	}
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	GSList *axes = gog_chart_get_axes (chart, GOG_AXIS_X);
	GogAxis *axis = GOG_AXIS (axes->data);
	gog_axis_set_bounds (axis, min, max);
	g_signal_handler_block (xrange, xrangesgn);
	if (max - min < xmax - xmin) {
		gtk_range_set_range (xrange, 0., xmax - xmin - max + min);
		gtk_range_set_value (xrange, min - xmin);
		gtk_widget_set_sensitive (GTK_WIDGET (xrange), true);
	} else {
		gtk_range_set_value (xrange, 0.);
		gtk_widget_set_sensitive (GTK_WIDGET (xrange), false);
	}
	g_signal_handler_unblock (xrange, xrangesgn);
}

void SpectrumView::OnXRangeChanged ()
{
	double max = gtk_spin_button_get_value (xmaxbtn) - gtk_spin_button_get_value (xminbtn);
	double min = xmin + gtk_range_get_value (xrange);
	max += min;
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	GSList *axes = gog_chart_get_axes (chart, GOG_AXIS_X);
	GogAxis *axis = GOG_AXIS (axes->data);
	gog_axis_set_bounds (axis, min, max);
	g_signal_handler_block (xminbtn, minsgn); 
	gtk_spin_button_set_value (xminbtn, min);
	g_signal_handler_unblock (xminbtn, minsgn); 
	g_signal_handler_block (xmaxbtn, maxsgn);
	gtk_spin_button_set_value (xmaxbtn, max);
	g_signal_handler_unblock (xmaxbtn, maxsgn); 
}

GogSeries *SpectrumView::NewSeries (bool new_plot)
{
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	GogPlot *plot = NULL;
	if (new_plot) {
		/* Create a scatter plot and add it to the chart */
		plot = (GogPlot *) gog_plot_new_by_name ("GogXYPlot");
		g_object_set (plot, "default-style-has-markers", false, NULL);
		gog_object_add_by_name (GOG_OBJECT (chart), "Plot", GOG_OBJECT (plot));
	} else {
		// find the first plot in the chart
		GSList *l = gog_object_get_children (GOG_OBJECT (chart), gog_object_find_role_by_name (GOG_OBJECT (chart), "Plot"));
		plot = (GogPlot*) l->data;
		g_slist_free (l);
	}
	return gog_plot_new_series (plot);
}

void SpectrumView::SaveAsImage (string const &filename, char const *mime_type, unsigned width, unsigned height) const
{
	char *fname = go_mime_to_image_format (mime_type);
	GOImageFormat format = go_image_get_format_from_name ((fname)? fname: filename.c_str ());
	if (format == GO_IMAGE_FORMAT_UNKNOWN)
		return;
	GError *error = NULL;
	GsfOutput *output = gsf_output_gio_new_for_uri (filename.c_str (), &error);
	if (error) {
		g_error_free (error);
		return;
	}
	GogGraph *graph = gog_graph_dup (go_graph_widget_get_graph (GO_GRAPH_WIDGET (m_Widget)));
	gog_graph_set_size (graph, width, height);
	gog_graph_export_image (graph, format, output, -1., -1.);
	g_object_unref (graph);
}

void SpectrumView::InvertAxis (GogAxisType target, bool inverted)
{
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	GSList *axes = gog_chart_get_axes (chart, target);
	GogAxis *axis = GOG_AXIS (axes->data);
	g_object_set (axis, "invert-axis", inverted, NULL);
}

}	//	namespace gcu
