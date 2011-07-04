// -*- C++ -*-

/*
 * GChemPaint templates plugin
 * category.h
 *
 * Copyright (C) 2006-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_TEMPLATE_CATEGORY_H
#define GCHEMPAINT_TEMPLATE_CATEGORY_H

#include <map>
#include <set>
#include <string>

using namespace std;

class gcpTemplate;
namespace gcp {
	class WidgetData;
}

class gcpTemplateCategory
{
public:
	gcpTemplateCategory (string &name);
	gcpTemplateCategory (char const *name);
	~gcpTemplateCategory ();

	void AddTemplate (gcpTemplate *temp);
	gcp::WidgetData *GetData (gcpTemplate *temp) {return m_Templates[temp];}

private:
	string m_Name;
	map<gcpTemplate*, gcp::WidgetData*> m_Templates;
};

extern map<string, gcpTemplateCategory*> TemplateCategories;
extern set<string> categories;

#endif	//	GCHEMPAINT_TEMPLATE_CATEGORY_H
