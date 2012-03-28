/*
 * Gnome Chemisty Utils
 * spectrumview.cc
 *
 * Copyright (C) 2007-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#include "config.h"
#include "spectrumdoc.h"
#include "spectrumview.h"
#include <gsf/gsf-output-gio.h>
#include <glib/gi18n-lib.h>
#include <cmath>
#include <map>
#include <string>

using namespace std;

namespace gcugtk
{

class SpectrumViewPrivate
{
public:
	static void OnSize (GtkWidget* w, GtkAllocation *allocation, SpectrumView *view);
};

void SpectrumViewPrivate::OnSize (G_GNUC_UNUSED GtkWidget* w, GtkAllocation *allocation, SpectrumView *view)
{
	view->m_Width = allocation->width;
	view->m_Height = allocation->height;
}

static void on_min_changed (SpectrumView *view)
{
	view->OnMinChanged ();
}

static void on_max_changed (SpectrumView *view)
{
	view->OnMaxChanged ();
}

static void on_ymin_changed (SpectrumView *view)
{
	view->OnYMinChanged ();
}

static void on_ymax_changed (SpectrumView *view)
{
	view->OnYMaxChanged();
}

static void on_xrange_changed (SpectrumView *view)
{
	view->OnXRangeChanged ();
}

static void on_yrange_changed (SpectrumView *view)
{
	view->OnYRangeChanged ();
}

SpectrumView::SpectrumView (SpectrumDocument *pDoc)
{
	m_Doc = pDoc;
	m_Widget = go_graph_widget_new (NULL);
	g_signal_connect (G_OBJECT (m_Widget), "size_allocate", G_CALLBACK (SpectrumViewPrivate::OnSize), this);
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
	/* Create a series for the plot */
	m_Series = gog_plot_new_series (plot);
	m_OptionBox = gtk_grid_new ();
	g_object_set (G_OBJECT (m_OptionBox),
	              "orientation", GTK_ORIENTATION_VERTICAL,
	              "margin-left", 6,
	              "margin-top", 6,
	              "margin-right", 6,
	              NULL);
	GtkGrid *grid = GTK_GRID (m_OptionBox);
	if (gtk_check_version (3, 2, 0)) {
		gtk_grid_set_column_spacing (GTK_GRID (grid), 12);
		gtk_grid_set_row_spacing (grid, 6);
	} else {
		gtk_grid_set_row_spacing (GTK_GRID (grid), 12);
		gtk_grid_set_row_spacing (grid, 6);
	}
	GtkWidget *w = gtk_label_new (_("Minimum X value:"));
	gtk_grid_attach (grid, w, 0, 0, 1, 1);
	xminbtn = GTK_SPIN_BUTTON (gtk_spin_button_new_with_range (0., 1., 0.1));
	minsgn = g_signal_connect_swapped (xminbtn, "value-changed", G_CALLBACK (on_min_changed), this);
	gtk_grid_attach (grid, GTK_WIDGET (xminbtn), 1, 0, 1, 1);
	w = gtk_label_new (_("Maximum X value:"));
	gtk_grid_attach (grid, w, 2, 0, 1, 1);
	xmaxbtn = GTK_SPIN_BUTTON (gtk_spin_button_new_with_range (0., 1., 0.1));
	maxsgn = g_signal_connect_swapped (xmaxbtn, "value-changed", G_CALLBACK (on_max_changed), this);
	gtk_grid_attach (grid, GTK_WIDGET (xmaxbtn), 3, 0, 1, 1);
	xrange = GTK_RANGE (gtk_scrollbar_new (GTK_ORIENTATION_HORIZONTAL, NULL));
	g_object_set (G_OBJECT (xrange), "hexpand", true, NULL);
	gtk_widget_set_sensitive (GTK_WIDGET (xrange), false);
	xrangesgn = g_signal_connect_swapped (xrange, "value-changed", G_CALLBACK (on_xrange_changed), this);
	gtk_grid_attach (grid, GTK_WIDGET (xrange), 4, 0, 1, 1);
	w = gtk_label_new (_("Minimum Y value:"));
	gtk_grid_attach (grid, w, 0, 1, 1, 1);
	yminbtn = GTK_SPIN_BUTTON (gtk_spin_button_new_with_range (0., 1., 0.1));
	yminsgn = g_signal_connect_swapped (yminbtn, "value-changed", G_CALLBACK (on_ymin_changed), this);
	gtk_grid_attach (grid, GTK_WIDGET (yminbtn), 1, 1, 1, 1);
	w = gtk_label_new (_("Maximum Y value:"));
	gtk_grid_attach (grid, w, 2, 1, 1, 1);
	ymaxbtn = GTK_SPIN_BUTTON (gtk_spin_button_new_with_range (0., 1., 0.1));
	ymaxsgn = g_signal_connect_swapped (ymaxbtn, "value-changed", G_CALLBACK (on_ymax_changed), this);
	gtk_grid_attach (grid, GTK_WIDGET (ymaxbtn), 3, 1, 1, 1);
	yrange = GTK_RANGE (gtk_scrollbar_new (GTK_ORIENTATION_HORIZONTAL, NULL));
	gtk_widget_set_sensitive (GTK_WIDGET (yrange), false);
	yrangesgn = g_signal_connect_swapped (yrange, "value-changed", G_CALLBACK (on_yrange_changed), this);
	gtk_grid_attach (grid, GTK_WIDGET (yrange), 4, 1, 1, 1);
	m_ExtraWidget = NULL;
}

SpectrumView::~SpectrumView ()
{
	g_signal_handler_disconnect (xminbtn, minsgn);
	g_signal_handler_disconnect (xmaxbtn, maxsgn);
	g_signal_handler_disconnect (yminbtn, yminsgn);
	g_signal_handler_disconnect (ymaxbtn, ymaxsgn);
	g_signal_handler_disconnect (xrange, xrangesgn);
	g_signal_handler_disconnect (yrange, yrangesgn);
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
	else if (target == GOG_AXIS_Y) {
		double l = log10 (fabs (max - min));
		int n = (l < 3)? rint (3 - l): 0;
		ystep = pow (10., -n);
		g_signal_handler_block (yminbtn, yminsgn);
		g_signal_handler_block (ymaxbtn, ymaxsgn);
		gtk_spin_button_set_range (yminbtn, min, max);
		gtk_spin_button_set_range (ymaxbtn, min, max);
		gtk_spin_button_set_increments (yminbtn, ystep, 100 * ystep);
		gtk_spin_button_set_increments (ymaxbtn, ystep, 100 * ystep);
		gtk_range_set_increments (yrange, ystep, 100 * ystep);
		gtk_range_set_inverted (yrange, !inverted);
		g_signal_handler_block (yrange, yrangesgn);
		gtk_range_set_value (yrange, 0.);
		gtk_widget_set_sensitive (GTK_WIDGET (yrange), false);
		g_signal_handler_unblock (yrange, yrangesgn);
		gtk_spin_button_set_value (yminbtn, min);
		gtk_spin_button_set_value (ymaxbtn, max);
		gtk_spin_button_set_digits (yminbtn, n);
		gtk_spin_button_set_digits (ymaxbtn, n);
		g_signal_handler_unblock (yminbtn, yminsgn);
		g_signal_handler_unblock (ymaxbtn, ymaxsgn);
		ymin = min;
		ymax = max;
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
	g_object_set (G_OBJECT (label), "allow-markup", TRUE, NULL);
	gog_dataset_set_dim (GOG_DATASET (label), 0, data, NULL);
	gog_object_add_by_name (axis, "Label", label);

}

void SpectrumView::ShowAxis (GogAxisType target, G_GNUC_UNUSED bool show)
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

void SpectrumView::OnYMinChanged ()
{
	double min = gtk_spin_button_get_value (yminbtn);
	double max = gtk_spin_button_get_value (ymaxbtn);
	if (max <= min) {
		double step;
		gtk_spin_button_get_increments (yminbtn, &step, NULL);
		min = max - step;
		g_signal_handler_block (yminbtn, yminsgn);
		gtk_spin_button_set_value (yminbtn, min);
		g_signal_handler_unblock (yminbtn, yminsgn);
	}
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	GSList *axes = gog_chart_get_axes (chart, GOG_AXIS_Y);
	GogAxis *axis = GOG_AXIS (axes->data);
	gog_axis_set_bounds (axis, min, max);
	g_signal_handler_block (yrange, yrangesgn);
	if (max - min < ymax - ymin) {
		gtk_range_set_range (yrange, 0., ymax - ymin - max + min);
		gtk_range_set_value (yrange, min - ymin);
		gtk_widget_set_sensitive (GTK_WIDGET (yrange), true);
	} else {
		gtk_range_set_value (yrange, 0.);
		gtk_widget_set_sensitive (GTK_WIDGET (yrange), false);
	}
	g_signal_handler_unblock (yrange, yrangesgn);
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

void SpectrumView::OnYMaxChanged ()
{
	double min = gtk_spin_button_get_value (yminbtn);
	double max = gtk_spin_button_get_value (ymaxbtn);
	if (max <= min) {
		double step;
		gtk_spin_button_get_increments (ymaxbtn, &step, NULL);
		max = min + step;
		g_signal_handler_block (ymaxbtn, ymaxsgn);
		gtk_spin_button_set_value (ymaxbtn, max);
		g_signal_handler_unblock (ymaxbtn, ymaxsgn);
	}
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	GSList *axes = gog_chart_get_axes (chart, GOG_AXIS_Y);
	GogAxis *axis = GOG_AXIS (axes->data);
	gog_axis_set_bounds (axis, min, max);
	g_signal_handler_block (yrange, yrangesgn);
	if (max - min < ymax - ymin) {
		gtk_range_set_range (yrange, 0., ymax - ymin - max + min);
		gtk_range_set_value (yrange, min - xmin);
		gtk_widget_set_sensitive (GTK_WIDGET (yrange), true);
	} else {
		gtk_range_set_value (yrange, 0.);
		gtk_widget_set_sensitive (GTK_WIDGET (yrange), false);
	}
	g_signal_handler_unblock (yrange, yrangesgn);
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

void SpectrumView::OnYRangeChanged ()
{
	double max = gtk_spin_button_get_value (ymaxbtn) - gtk_spin_button_get_value (yminbtn);
	double min = ymin + gtk_range_get_value (yrange);
	max += min;
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	GSList *axes = gog_chart_get_axes (chart, GOG_AXIS_Y);
	GogAxis *axis = GOG_AXIS (axes->data);
	gog_axis_set_bounds (axis, min, max);
	g_signal_handler_block (yminbtn, yminsgn);
	gtk_spin_button_set_value (yminbtn, min);
	g_signal_handler_unblock (yminbtn, yminsgn);
	g_signal_handler_block (ymaxbtn, ymaxsgn);
	gtk_spin_button_set_value (ymaxbtn, max);
	g_signal_handler_unblock (ymaxbtn, ymaxsgn);
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

void SpectrumView::AddToOptionBox (GtkWidget *w)
{
	gtk_grid_attach (GTK_GRID (m_OptionBox), w, 0, 2, 5, 1);
	m_ExtraWidget = w;
}

void SpectrumView::DestroyExtraWidget ()
{
	if (m_ExtraWidget) {
		gtk_widget_destroy (m_ExtraWidget);
		m_ExtraWidget = NULL;
	}
}

}	//	namespace gcu
