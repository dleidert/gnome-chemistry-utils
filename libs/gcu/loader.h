// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/loader.h
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

#ifndef GCU_LOADER_H
#define GCU_LOADER_H

#include <list>
#include <map>
#include <string>
#include <goffice/app/io-context.h>
#include <gsf/gsf-input.h>
#include <gsf/gsf-output.h>

namespace gcu {

class Document;
class Loader;

typedef struct {
	Loader *loader;
	bool read;
	bool write;
	bool supports2D;
	bool supports3D;
	bool supportsCrystals;
} LoaderStruct;

class Loader
{
public:
	Loader ();
	virtual ~Loader ();

	// static methods
	static void Init ();
	static bool GetFirstLoader (std::map<std::string, LoaderStruct>::iterator &it);
	static bool GetNextLoader (std::map<std::string, LoaderStruct>::iterator &it);
	static Loader *GetLoader (char const *mime_type);
	static Loader *GetSaver (char const *mime_type);

	// virtual methods
	virtual bool Read (Document *doc, GsfInput *in, char const *mime_type, IOContext *io);
	virtual bool Write (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io);

protected:
	void AddMimeType (const char *mime_type);
	void RemoveMimeType (const char *mime_type);

private:
	static bool Inited;

protected:
	std::list<std::string> MimeTypes;
};

}

#endif	//	GCU_LOADER_H
