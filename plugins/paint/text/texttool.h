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

#include <gccv/structs.h>
#include <gcp/tool.h>
#include <gcp/text-editor.h>
#include <gcu/macros.h>
#include <goffice/gtk/go-color-selector.h>
#include <list>
#include <map>

namespace gccv {
	class Text;
}
using namespace std;

class gcpTextTool: public gcp::Tool, public gcp::TextEditor
{
public:
	gcpTextTool (gcp::Application *App, string Id = "Text");
	virtual ~gcpTextTool ();

	bool OnClicked ();
	void OnDrag ();
	bool Deactivate ();
	void Activate ();
	bool OnKeyPress (GdkEventKey *event);
	bool NotifyViewChange ();
	bool DeleteSelection ();
	bool CopySelection (GtkClipboard *clipboard);
	bool CutSelection (GtkClipboard *clipboard);
	bool PasteSelection (GtkClipboard *clipboard);
	bool OnReceive (GtkClipboard *clipboard, GtkSelectionData *data, int type);
	bool OnUndo ();
	bool OnRedo ();
	void PushNode (xmlNodePtr node);
	void OnGetData (GtkClipboard *clipboard, GtkSelectionData *selection_data,  guint info);
	GtkWidget *GetPropertyPage ();
	void OnSelectFamily (GtkTreeSelection *selection);
	void OnSelectFace (GtkTreeSelection *selection);
	void OnSelectSize (int size);
	void OnSizeChanged ();
	void SetSizeFull (bool update_list);
	void UpdateTagsList ();
	unsigned GetIndex ();
	void OnUnderlineChanged (unsigned underline);
	void OnStriketroughToggled (bool strikethrough);
	void OnPositionChanged (int position);
	void OnForeColorChanged (GOColor color);
	char const *GetHelpTag () {return "text";}
	void SelectionChanged ();

protected:
	virtual bool Unselect ();

private:
	void BuildTagsList ();
	void SelectBestFontFace ();

protected:
	gccv::Text* m_Active;
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
	gulong m_UnderlineSignal, m_StrikethroughSignal, m_ForeSignal, m_RiseSignal;

GCU_PROP (std::string, FamilyName)
GCU_PROP (PangoStyle, Style)
GCU_PROP (PangoWeight, Weight)
GCU_PROP (PangoStretch, Stretch)
GCU_PROP (PangoVariant, Variant)
GCU_PROP (PangoUnderline, Underline)
GCU_PROP (int, Size)
GCU_PROP (int, Rise)
GCU_PROP (bool, Strikethrough)
GCU_PROP (gccv::TextPosition, Position)
GCU_PROP (GOColor, Color);
};

#endif	//GCHEMPAINT_TEXT_TOOL_H
