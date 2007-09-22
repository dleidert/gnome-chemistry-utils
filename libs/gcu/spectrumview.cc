/* 
 * Gnome Chemisty Utils
 * spectrumview.cc
 *
 * Copyright (C) 2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <goffice/graph/gog-label.h>
#include <goffice/graph/gog-object.h>
#include <goffice/graph/gog-plot.h>
#include <goffice/graph/gog-series.h>
#include <goffice/graph/gog-style.h>
#include <goffice/gtk/go-graph-widget.h>

namespace gcu
{

SpectrumView::SpectrumView (SpectrumDocument *pDoc)
{
	m_Doc = pDoc;
	m_Widget = go_graph_widget_new (NULL);
	GogGraph *graph = go_graph_widget_get_graph (GO_GRAPH_WIDGET (m_Widget));
	/* Add a title */
	GogLabel *label = (GogLabel *) g_object_new (GOG_LABEL_TYPE, NULL);
	gog_object_add_by_name (GOG_OBJECT (graph), "Title", GOG_OBJECT (label));
	/* Get the chart created by the widget initialization */
	GogChart *chart = go_graph_widget_get_chart (GO_GRAPH_WIDGET (m_Widget));
	/* Create a pie plot and add it to the chart */
	GogPlot *plot = (GogPlot *) gog_plot_new_by_name ("GogXYPlot");
	g_object_set (plot, "default-style-has-markers", false, NULL);
	gog_object_add_by_name (GOG_OBJECT (chart), "Plot", GOG_OBJECT (plot));
	/* Create a series for the plot and populate it with some simple data */
	m_Series = gog_plot_new_series (plot);
}

SpectrumView::~SpectrumView ()
{
}

}	//	namespace gcu
