// -*- C++ -*-

/* 
 * GChemPaint templates plugin
 * templatetool.cc 
 *
 * Copyright (C) 2004-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "templatetool.h"
#include "templatetree.h"
#include <gcp/application.h>
#include <gcp/document.h>
#include <gcp/theme.h>
#include <gcp/view.h>
#include <gcp/widgetdata.h>
#include <gccv/canvas.h>
#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include <cmath>

using namespace gcu;

xmlDocPtr xml;

class gcpNewTemplateToolDlg: public Dialog
{
public:
	gcpNewTemplateToolDlg (gcp::Application* App);
	virtual ~gcpNewTemplateToolDlg ();

	virtual bool Apply ();
	void SetTemplate (xmlNodePtr node);
	
private:
	gcpTemplate *temp;
	gcp::Document *pDoc;
	gcp::WidgetData* pData;
	xmlNodePtr m_node;
	GtkEntry *category_entry;
};

gcpTemplateTool::gcpTemplateTool (gcp::Application* App): gcp::Tool (App, "Templates")
{
	m_Template = NULL;
	xml = xmlNewDoc ((const xmlChar*) "1.0");
}

gcpTemplateTool::~gcpTemplateTool ()
{
	xmlFreeDoc (xml);
}

bool gcpTemplateTool::OnClicked ()
{
	gcp::Document* pDoc = m_pView->GetDoc ();
	gcpNewTemplateToolDlg *dlg = (gcpNewTemplateToolDlg*) m_pApp->GetDialog ("new_template");
	if (dlg) {
		m_pObject = m_pObject->GetMolecule ();
		if (m_pObject) {
			xmlNodePtr node = m_pObject->Save (xml);
			if (node) {
				char *buf = g_strdup_printf ("%g", pDoc->GetTheme ()->GetBondLength ());
				xmlNewProp (node, (const xmlChar*) "bond-length", (const xmlChar*) buf);
				g_free (buf);
				dlg->SetTemplate (node);
				gdk_window_raise (GTK_WIDGET (dlg->GetWindow ())->window);
			}
		}
		return false;
	}
	if (!m_Template)
		return false;
	pDoc->AddData (m_Template->node);
	m_pObject = m_pData->SelectedObjects.front ();
	if (m_Template->bond_length != 0.) { // if not, there is no bond...
		double r = pDoc->GetBondLength () / m_Template->bond_length;
		if (fabs (r - 1.) > .0001) { 
			Matrix2D m (r, 0., 0., r);
			// FIXME: this would not work for reactions
			m_pObject->Transform2D (m, 0., 0.);
			m_pView->Update (m_pObject);
		}
	}
	gccv::Rect rect;
	double dx, dy;
	pDoc->AbortOperation ();
	m_pData->GetSelectionBounds (rect);
	dx = m_x0 - (rect.x0 + rect.x1) / 2.;
	dy = m_y0 - (rect.y0 + rect.y1) / 2.;
	m_x0 -= dx;
	m_y0 -= dy;
	m_pData->MoveSelectedItems (dx, dy);
	return true;
}

void gcpTemplateTool::OnDrag ()
{
	double dx = m_x - m_x1, dy = m_y - m_y1;
	m_x1 = m_x;
	m_y1 = m_y;
	m_pData->MoveSelectedItems (dx, dy);
}

void gcpTemplateTool::OnRelease ()
{
	gcp::Document* pDoc = m_pView->GetDoc ();
	double dx = m_x - m_x0, dy = m_y - m_y0;
	m_pData->MoveSelectedItems (-dx, -dy);
	m_pData->MoveSelection(dx, dy);
	pDoc->PopOperation (); 
	m_pData->UnselectAll ();
	gcp::Operation* pOp = pDoc->GetNewOperation (gcp::GCP_ADD_OPERATION);
	pOp->AddObject (m_pObject);
	pDoc->FinishOperation ();
}

enum {
	NAME_COLUMN,
	NUM_COLUMNS
};

static void on_template_changed (GtkComboBox *combo, gcpTemplateTool *tool)
{
	tool->OnChanged (combo);
}

static void on_size (GtkWidget *widget, GtkAllocation *allocation, gcpTemplateTool *tool)
{
	tool->OnPreviewSize (allocation);
}

static void on_add_template (GtkWidget *w, gcpTemplateTool *tool)
{
	tool->OnAddTemplate ();
}

static void on_delete_template (GtkWidget *w, gcpTemplateTool *tool)
{
	tool->OnDeleteTemplate ();
}

GtkWidget *gcpTemplateTool::GetPropertyPage ()
{
	GladeXML *xml = glade_xml_new (GLADEDIR"/templates.glade", "templates", GETTEXT_PACKAGE);
	gcpTemplateTree *tree = (gcpTemplateTree*) m_pApp->GetTool ("TemplateTree");
	if (!tree)
		return NULL;
	GtkComboBox *combo = GTK_COMBO_BOX (glade_xml_get_widget (xml, "templates-combo"));
	gtk_combo_box_set_model (combo, tree->GetModel ());
	GtkCellRenderer* renderer = (GtkCellRenderer*) gtk_cell_renderer_text_new ();
	g_object_set (renderer, "xalign", 0.0, NULL);
	gtk_cell_layout_clear (GTK_CELL_LAYOUT (combo));
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo), renderer, true);
	gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combo),
								renderer, "text",
								NAME_COLUMN);
	gtk_combo_box_set_active (combo, 0);
	g_signal_connect (G_OBJECT (combo), "changed", G_CALLBACK (on_template_changed), this);
	m_DeleteBtn = glade_xml_get_widget (xml, "delete");
	g_signal_connect (m_DeleteBtn, "clicked", G_CALLBACK (on_delete_template), this);
	gtk_widget_set_sensitive (m_DeleteBtn, false);
	GtkWidget *w = glade_xml_get_widget (xml, "add");
	g_signal_connect (w, "clicked", G_CALLBACK (on_add_template), this);
	m_Book = GTK_NOTEBOOK (glade_xml_get_widget (xml, "book"));
	g_signal_connect (m_Book, "size-allocate", G_CALLBACK (on_size), this);
	
	return glade_xml_get_widget (xml, "templates");
}

void gcpTemplateTool::OnChanged (GtkComboBox *combo)
{
	gcpTemplateTree *tree = (gcpTemplateTree*) m_pApp->GetTool ("TemplateTree");
	if (!tree)
		return;
	GtkTreeIter iter;
	if (gtk_combo_box_get_active_iter (combo, &iter)) {
		GtkTreePath *path = gtk_tree_model_get_path ((GtkTreeModel*) tree->GetModel (), &iter);
		char* path_string = gtk_tree_path_to_string (path);
		m_Template = tree->GetTemplate (path_string);
		int page;
		if (m_Template) {
			if (!m_Template->doc) {
				m_Template->doc = new gcp::Document (NULL, false);
				gcp::Theme *pTheme = m_Template->doc->GetTheme ();
				m_Template->doc->SetEditable (false);
				m_Template->data =  (gcp::WidgetData*) g_object_get_data (G_OBJECT (m_Template->doc->GetView ()->CreateNewWidget ()), "data");
				m_Template->doc->AddData (m_Template->node);
				m_Template->data->UnselectAll ();
				m_Template->data->GetObjectBounds (m_Template->doc, &m_Template->rect);
				m_Template->doc->Move (-m_Template->rect.x0 / pTheme->GetZoomFactor (), -m_Template->rect.y0 / pTheme->GetZoomFactor ());
				m_Template->doc->GetView ()->Update (m_Template->doc);
				m_Template->bond_length = 140.;
				page = -1;
			} else
				page = gtk_notebook_page_num (m_Book, m_Template->w);
			if (page < 0) {
				m_Template->data =  (gcp::WidgetData*) g_object_get_data (G_OBJECT (m_Template->doc->GetView ()->CreateNewWidget ()), "data");
				m_Template->data->GetObjectBounds (m_Template->doc, &m_Template->rect);
				m_Template->w = gtk_scrolled_window_new (NULL, NULL);
				gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (m_Template->w), GTK_SHADOW_NONE);
				gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (m_Template->w), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
				gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW (m_Template->w), m_Template->doc->GetWidget ());
				gtk_widget_show_all (m_Template->w);
				gtk_notebook_append_page (m_Book, m_Template->w, NULL);
				page = gtk_notebook_page_num (m_Book, m_Template->w);
			}
			GtkWidget *wd = gtk_notebook_get_nth_page (m_Book, 0);
			// Why 4 and not 2 in following lines?
			double w = (wd->allocation.width - 4 * m_Template->w->style->xthickness) / (m_Template->rect.x1 - m_Template->rect.x0);
			double h = (wd->allocation.height - 4 * m_Template->w->style->ythickness) / (m_Template->rect.y1 - m_Template->rect.y0);
			if (w < 1. || h < 1.) {
				m_Template->data->Zoom = MIN (w, h);
			}
			gtk_notebook_set_current_page (m_Book, page);
			gtk_widget_set_sensitive (m_DeleteBtn, m_Template->writeable);
		} else {
			// a category has been selected
			m_Template = NULL;
			GtkTreeModel *model = tree->GetModel ();
			char const *name;
			gtk_tree_model_get (model, &iter, 0, &name, -1);
			gtk_widget_set_sensitive (m_DeleteBtn, false);
			gtk_notebook_set_current_page (m_Book, 0);
			gtk_widget_set_sensitive (m_DeleteBtn, false);
		}
		g_free (path_string);
		gtk_tree_path_free (path);		
	}
}

void gcpTemplateTool::OnPreviewSize (GtkAllocation *allocation)
{
	if (m_Template) {
		double w = (allocation->width - 4 * m_Template->w->style->xthickness) / (m_Template->rect.x1 - m_Template->rect.x0);
		double h = (allocation->height - 4 * m_Template->w->style->ythickness) / (m_Template->rect.y1 - m_Template->rect.y0);
		m_Template->data->Zoom = (w < 1. || h < 1.)? MIN (w, h): 1.;
		m_Template->doc->GetView ()->GetCanvas ()->SetZoom (m_Template->data->Zoom);
	}
}

void gcpTemplateTool::OnAddTemplate ()
{
	new gcpNewTemplateToolDlg (m_pApp);
}

void gcpTemplateTool::OnDeleteTemplate ()
{
	gcpTemplateTree *tree = (gcpTemplateTree*) m_pApp->GetTool ("TemplateTree");
	if (!tree)
		return;
	string key = m_Template->category + "/" + m_Template->name;
	if (Templates[key] != m_Template) {
		int i = 0;
		char* str = g_strdup_printf ("%d", i);
		while (Templates[key + str] != m_Template) {
			g_free (str);
			str = g_strdup_printf ("%d", ++i);
		}
		key += str;
		g_free (str);
	}
	tree->DeleteTemplate (key);
	m_Template = NULL;
	gtk_notebook_set_current_page (m_Book, 0);
}

gcpNewTemplateToolDlg::gcpNewTemplateToolDlg (gcp::Application* App):
	Dialog(App, GLADEDIR"/new-template.glade", "new_template", App)
{
	m_node = NULL;
	if (!xml) {
		delete this;
		return;
	}
	pDoc = new gcp::Document (reinterpret_cast<gcp::Application*> (m_App), true);
	pDoc->SetEditable (false);
	GtkWidget* w;
	GtkScrolledWindow* scroll = (GtkScrolledWindow*) glade_xml_get_widget (xml, "scrolledcanvas");
	gtk_scrolled_window_add_with_viewport (scroll, w = pDoc->GetView ()->CreateNewWidget ());
	pData = (gcp::WidgetData*) g_object_get_data (G_OBJECT (w), "data");
	/* build the categories list */
	GtkListStore *model = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING);
	GtkTreeIter iter;
	set<string>::iterator it = categories.begin (), end = categories.end ();
	for (; it != end; it++) {
		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
			  NAME_COLUMN, (*it).c_str(),
			  -1);
	}
	w = gtk_combo_box_entry_new_with_model (GTK_TREE_MODEL (model), NAME_COLUMN);
	g_object_unref (model);
	gtk_table_attach_defaults (GTK_TABLE (glade_xml_get_widget (xml, "table1")), w, 1, 2, 1, 2);
	gtk_widget_show (w);
	category_entry = GTK_ENTRY (gtk_bin_get_child (GTK_BIN (w)));
}

gcpNewTemplateToolDlg::~gcpNewTemplateToolDlg ()
{
	if (m_node){
		xmlUnlinkNode (m_node);
		xmlFreeNode (m_node);
	}
}

bool gcpNewTemplateToolDlg::Apply ()
{
	const char* name = gtk_entry_get_text ((GtkEntry*) glade_xml_get_widget (xml, "name"));
	const char* category = gtk_entry_get_text (category_entry);
	if (!m_node || (*name == 0) || (*category == 0)) {
		char* msg;
		if (!m_node)
			msg = _("Please provide an object.");
		else if (*name == 0)
			msg = _("Please give a name.");
		else
			msg = _("Please choose a category.");
		GtkWidget* message = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, 
															msg);
		g_signal_connect_swapped (G_OBJECT (message), "response", G_CALLBACK (gtk_widget_destroy), G_OBJECT (message));
		gtk_window_set_icon_name (GTK_WINDOW (message), "gchempaint");
		gtk_widget_show (message);
		return false;
	}
	gcpTemplate *temp = new gcpTemplate ();
	temp->node = m_node;
	temp->writeable = true;
	temp->name = name;
	temp->category = category;
	if (!user_templates) {
		user_templates = xmlNewDoc ((xmlChar*) "1.0");
		xmlDocSetRootElement (user_templates,  xmlNewDocNode (user_templates, NULL, (xmlChar*) "templates", NULL));
		char* filename = g_strconcat (getenv ("HOME"), "/.gchempaint/templates/templates.xml", NULL);
		user_templates->URL = xmlStrdup ((xmlChar*) filename);
		g_free (filename);
	}
	xmlNodePtr node = xmlNewDocNode (user_templates, NULL, (xmlChar*) "template", NULL);
	xmlNodePtr child = xmlNewDocNode (user_templates, NULL, (xmlChar*)"category", (xmlChar*) category);
	xmlAddChild (node, child);
	child = xmlNewDocNode (user_templates, NULL, (xmlChar*)"name", (xmlChar*) name);
	xmlAddChild (node, child);
	xmlUnlinkNode (m_node);
	xmlAddChild (node, m_node);
	set<string>::iterator it = categories.find (category);
	if (it == categories.end()) categories.insert (category);
	string key = temp->name;
	if (TempbyName[key]) {
		int i = 0;
		char* str = g_strdup_printf ("%d", i);
		while (TempbyName[key + str]) {
			g_free (str);
			str = g_strdup_printf ("%d", ++i);
		}
		key += str;
		g_free (str);
	}
	TempbyName[key] = temp;
	key = string ((char*) category) + "/" + (char*) name;
	if (Templates[key]) {
		int i = 0;
		char* str = g_strdup_printf ("%d", i);
		while (Templates[key + str]) {
			g_free (str);
			str = g_strdup_printf ("%d", ++i);
		}
		key += str;
		g_free (str);
	}
	Templates[key] = temp;
	m_node = NULL;
	xmlAddChild (user_templates->children, node);
	xmlIndentTreeOutput = true;
	xmlKeepBlanksDefault (0);
	xmlSaveFormatFile ((char*) user_templates->URL, user_templates, true);
	gcpTemplateTree *tree = (gcpTemplateTree*)
			reinterpret_cast<gcp::Application*> (m_App)->GetTool ("TemplateTree");
	if (tree)
		tree->AddTemplate (key);
	return true;
}

void gcpNewTemplateToolDlg::SetTemplate (xmlNodePtr node)
{
	map<string, Object*>::iterator it;
	Object* obj = pDoc->GetFirstChild(it);
	if (obj) pDoc->Remove (obj);
	pDoc->PopOperation ();
	if (m_node) {
		xmlUnlinkNode (m_node);
		xmlFreeNode (m_node);
	}
	pDoc->AddData (node);
	gccv::Rect rect;
	char *buf = (char*) xmlGetProp (node, (const xmlChar*) "bond-length");
	double r = 140. / strtod (buf, NULL);
	xmlFree (buf);
	if (fabs (r - 1.) > .0001) { 
		Matrix2D m (r, 0., 0., r);
		// FIXME: this would not work for reactions
		pDoc->Transform2D (m, 0., 0.);
		pDoc->GetView ()->Update (pDoc);
	}
	while (gtk_events_pending ())
		gtk_main_iteration ();
	pDoc->AbortOperation ();
	pData->GetSelectionBounds (rect);
	pData->MoveSelection (- rect.x0, - rect.y0);
	pDoc->PopOperation ();
	pData->UnselectAll ();
	xmlUnlinkNode (node);
	xmlFreeNode (node);
	obj = pDoc->GetFirstChild (it);
	m_node = obj->Save (::xml);
}
