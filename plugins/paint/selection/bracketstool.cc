// -*- C++ -*-

/* 
 * GChemPaint selection plugin
 * bracketstool.cc
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
#include "bracketstool.h"
#include <gcu/message.h>
#include <gcu/ui-builder.h>
#include <gcp/application.h>
#include <glib/gi18n-lib.h>

gcpBracketsTool::gcpBracketsTool (gcp::Application* App): gcp::Tool (App, "Brackets")
{
	m_Type = GCP_BRACKET_NORMAL;
	m_Used = GCP_BRACKETS_BOTH;
}

gcpBracketsTool::~gcpBracketsTool ()
{
}

bool gcpBracketsTool::OnClicked ()
{
	return true;
}

void gcpBracketsTool::OnDrag ()
{
}

void gcpBracketsTool::OnRelease ()
{
}

GtkWidget *gcpBracketsTool::GetPropertyPage ()
{
	gcu::UIBuilder *builder= NULL;
	try {
		builder = new gcu::UIBuilder (UIDIR"/brackets.ui", GETTEXT_PACKAGE);
		GtkComboBox *box = builder->GetComboBox ("type-box");
		gtk_combo_box_set_active (box, m_Type);
		g_signal_connect (box, "changed", G_CALLBACK (gcpBracketsTool::OnTypeChanged), this);
		box = builder->GetComboBox ("used-box");
		gtk_combo_box_set_active (box, m_Used);
		g_signal_connect (box, "changed", G_CALLBACK (gcpBracketsTool::OnUsedChanged), this);

		GtkWidget *res = builder->GetRefdWidget ("brackets");
		delete builder;
		return res;
	}
	catch (std::runtime_error &e) {
		// TODO: add a one time message box
		static bool done = false;
		if (!done) {
			done = true;
			std::string mess = _("Error loading the properties widget description: \n");
			mess += e.what ();
			new gcu::Message (GetApplication (), mess, GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE);
		}
		if (builder)
			delete builder;
		return NULL;
	}
}

void gcpBracketsTool::OnTypeChanged (GtkComboBox *box, gcpBracketsTool *tool)
{
	tool->m_Type = static_cast <gcpBracketType> (gtk_combo_box_get_active (box));
}

void gcpBracketsTool::OnUsedChanged (GtkComboBox *box, gcpBracketsTool *tool)
{
	tool->m_Used = static_cast <gcpBracketsUsed> (gtk_combo_box_get_active (box));
}
