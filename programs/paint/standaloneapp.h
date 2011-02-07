// -*- C++ -*-

/* 
 * GChemPaint
 * standaloneapp.h
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
 
#ifndef GCHEMPAINT_STANDALONEAPP_H
#define GCHEMPAINT_STANDALONEAPP_H

#include "gcp/application.h"
#include "gcp/document.h"

using namespace gcp;

class gcpStandaloneApp: public gcp::Application
{
public:
	gcpStandaloneApp ();
	virtual ~gcpStandaloneApp ();

	virtual GtkWindow* GetWindow ();
	void OnFileNew (char const *Theme = NULL);
	void NoMoreDocsEvent ();
	void CatchSignals (int sig_num);

private:
	GtkWidget* CreateView ();

private:
	GtkWindow* m_Window;
	GtkUIManager* m_UIManager;
	GtkNotebook* m_Book;
	GtkWidget* m_Dock; //BonoboDock
	GtkWidget* m_Bar;	//GtkStatusBar
	unsigned m_statusId;
	unsigned m_MessageId; //currently displayed message in the status bar
	unsigned m_NumDoc; //used to build the name of the action associated with the menu
	std::map <std::string, GtkRadioToolButton*> Groups;
};

#endif //GCHEMPAINT_STANDALONEAPP_H
