// -*- C++ -*-

/*
 * GChemPaint templates plugin
 * category.cc
 *
 * Copyright (C) 2006-2007 Jean Bréfort <jean.brefort@normalesup.org>
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
#include "category.h"
#include "templatetree.h"

map<string, gcpTemplateCategory*> TemplateCategories;
set<string> categories;

gcpTemplateCategory::gcpTemplateCategory (string &name)
{
	m_Name = name;
	categories.insert (name);
	TemplateCategories[name] = this;
}

gcpTemplateCategory::gcpTemplateCategory (char const *name)
{
	m_Name = name;
	categories.insert (m_Name);
	TemplateCategories[m_Name] = this;
}

gcpTemplateCategory::~gcpTemplateCategory ()
{
}

void gcpTemplateCategory::AddTemplate (gcpTemplate *temp)
{
	m_Templates[temp] = NULL;
}
