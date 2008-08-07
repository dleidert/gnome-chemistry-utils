/* 
 * Gnome Chemistry Utils GOffice component
 * gogcpapp.h
 *
 * Copyright (C) 2005-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef GCP_GOFFICE_APP_H
#define GCP_GOFFICE_APP_H

#include <gcp/application.h>
#include <gogcuapp.h>
#include <map>
#include <string>

class GOGcpApplication: public gcp::Application, public GOGcuApplication
{
public:
	GOGcpApplication ();
	virtual ~GOGcpApplication ();

	virtual GtkWindow* GetWindow();
	virtual void ToggleMenu (const std::string& menuname, bool active);

	gcu::Document *ImportDocument (const std::string& mime_type, const char* data, int length);
	GtkWindow *EditDocument (GOGChemUtilsComponent *gogcu);

	void OnFileNew (char const *Theme = NULL);
	void OnFileClose ();

	void NoMoreDocsEvent () {;}

private:
	std::map<gcp::Document *, gpointer> m_Windows;
};

#endif	// GCP_GOFFICE_APP_H
