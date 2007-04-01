// -*- C++ -*-

/* 
 * GChemPaint text plugin
 * texttool.h 
 *
 * Copyright (C) 2002-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_TEXT_TOOL_H
#define GCHEMPAINT_TEXT_TOOL_H

#include <gcp/tool.h>
#include <gcu/macros.h>
#include <goffice/gtk/go-color-selector.h>
#include <list>

using namespace std;

class gcpTextTool: public gcp::Tool
{
public:
	gcpTextTool (gcp::Application *App, string Id = "Text");
	virtual ~gcpTextTool ();

	virtual bool OnClicked ();
	virtual bool Deactivate ();
	virtual void Activate ();
	virtual bool OnEvent (GdkEvent *event);
	virtual bool NotifyViewChange ();
	virtual bool DeleteSelection ();
	virtual bool CopySelection (GtkClipboard *clipboard);
	virtual bool CutSelection (GtkClipboard *clipboard);
	virtual bool PasteSelection (GtkClipboard *clipboard);
	virtual bool OnReceive (GtkClipboard *clipboard, GtkSelectionData *data, int type);
	virtual bool OnUndo ();
	virtual bool OnRedo ();
	void PushNode (xmlNodePtr node);
	void OnGetData (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info);
	GtkWidget *GetPropertyPage ();
	void OnSelectFamily (GtkTreeSelection *selection);
	void OnSelectFace (GtkTreeSelection *selection);
	void OnSelectSize (int size);
	void OnSizeChanged ();
	void SetSizeFull (bool update_list);
	void UpdateAttributeList ();
	int GetIndex ();
	void OnUnderlineChanged (unsigned underline);
	void OnStriketroughToggled (bool strikethrough);
	void OnPositionChanged (int position);
	void OnForeColorChanged (GOColor color);
	char const *GetHelpTag () {return "text";}

protected:
	virtual bool Unselect ();

private:
	void BuildAttributeList ();
	void SelectBestFontFace ();

protected:
	GnomeCanvasPango* m_Active;
	list<xmlNodePtr> m_UndoList, m_RedoList;
	xmlNodePtr m_CurNode, m_InitNode;

private:
	bool m_bUndo;
	PangoFontDescription *m_FontDesc, *m_DefaultFontDesc;
	GtkListStore *m_FamilyList, *m_FaceList, *m_SizeList;
	GtkTreeView *m_FamilyTree;
	GtkTreeView *m_FacesTree;
	GtkTreeView *m_SizesTree;
	GtkEntry *m_SizeEntry;
	GOSelector *m_ColorSelector;
	map<string, PangoFontFamily*> m_Families;
	map<string, PangoFontFace*> m_Faces;
	guint m_FamilySignal, m_FaceSignal, m_SizeSignal;
	gpointer m_FamilySel, m_FaceSel, m_SizeSel;
	GtkComboBox *m_UnderlineBox;
	GtkToggleButton *m_StrikethroughBtn;
	GtkSpinButton *m_RiseButton;
	bool m_Dirty;
	gulong m_SelSignal, m_UnderlineSignal, m_StrikethroughSignal, m_ForeSignal, m_RiseSignal;

GCU_PROP (char const *, FamilyName)
GCU_PROP (PangoStyle, Style)
GCU_PROP (PangoWeight, Weight)
GCU_PROP (PangoStretch, Stretch)
GCU_PROP (PangoVariant, Variant)
GCU_PROP (PangoUnderline, Underline)
GCU_PROP (int, Size)
GCU_PROP (int, Rise)
GCU_PROP (bool, Strikethrough)
GCU_PROP (GOColor, Color);
};

#endif	//GCHEMPAINT_TEXT_TOOL_H
