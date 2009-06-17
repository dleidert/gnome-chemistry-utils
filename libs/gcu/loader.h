// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/loader.h
 *
 * Copyright (C) 2007-2008 Jean Bréfort <jean.brefort@normalesup.org>
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
#ifdef GOFFICE_HAS_GLOBAL_HEADER
#   include <goffice/goffice.h>
#else
#   include <goffice/app/io-context.h>
#endif
#include <gsf/gsf-input.h>
#include <gsf/gsf-output.h>

/*!\file*/
namespace gcu {

class Document;
class Loader;

/*!\struct LoaderStruct gcu/loader.h
	 Stores data for loaders. They are created when Loader::Init is called for
	 the first time. Using Loader::GetFirstLoader and Loader::GetNextLoader is
	 the way used to access the list of available loaders and what they provide.
*/
typedef struct {
/*!
The loader instance associated to a mime type. Might be NULL if not in use.
*/
	Loader *loader;
/*!
If true, the loader can read files of the mime type.
*/
	bool read;
/*!
If true, the loader can write files of the mime type.
*/
	bool write;
/*!
If true, the mime type is able to store 2D chemical structures.
*/
	bool supports2D;
/*!
If true, the mime type is able to store 3D chemical structures.
*/
	bool supports3D;
/*!
If true, the mime type is able to store crystal structures.
*/
	bool supportsCrystals;
/*!
If true, the mime type is able to store spectral data.
*/
	bool supportsSpectra;
} LoaderStruct;

/*!\class Loader gcu/loader.h
\brief class used to implement serialization engines.

This class is used to load and save files of various types, using the
gcu::Object framework. Derived classes should be implemented in plugins, using
the goffice plugins API. When Loader::Init is called, the framework collects
informations about the services exposed by each plugin in XML files. For the CDX
file loader plugin, the corresponding plugin.xml.in file content is:

\code
<?xml version="1.0" encoding="UTF-8"?>
<plugin id="GCULoader_cdxml">
	<information>
		<_name>Loader : cdxml</_name>
		<_description>Chemdraw XML files loader.</_description>
	</information>
	<loader type="Gnumeric_Builtin:module">
		<attribute name="module_file" value="cdxml"/>
	</loader>
	<services>
		<service type="chemical_loader" id="GCULoader_cdxml">
			<mime_type name="chemical/x-cdxml" capabilities="r" scope="2"/>
			<information>
				<_description>Chemdraw XML files loader</_description>
			</information>
		</service>
	</services>
</plugin>
\endcode

In the present context, the important node is the mime type related one. Its
attributes are:
 - name: the mime type.
 - capabilities: what is supported: r for reading, w for writing. Both "rw" and
 "wr" are valid.
 - scope: 2 and 3 mean 2D and 3D structures, repectively, c means crystal
 structure, and s, spectra . Any combination might be used.
 
Other fields are standard in the goffice world. The plugin is loaded
only when needed.

Each plugin should implement at least one derived class and a static instance
of this class. The CDX loader has:

\code

class CDXLoader: public gcu::Loader
{
public:
	CDXLoader ();
	virtual ~CDXLoader ();

	bool Read (Document *doc, GsfInput *in, char const *mime_type, IOContext *io);
	bool Write (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io);

private:
	...
};

...
static CDXLoader loader;

\endcode

*/
class Loader
{
public:
/*!
The constructor. Derived class need to call Loader::AddMimeType for each
mime type they support.
*/
	Loader ();
/*!
The destructor Derived class should call Loader::RemoveMimeType for each
mime type they support.
*/
	virtual ~Loader ();

	// static methods
/*!
Initializes the load/save engines system. This must be called before trying to use
any loader (no one will be available before, anyway).
*/
	static void Init ();
/*!
@param it a std::map iterator.

Must be called to access the first LoaderStruct instance. When successful
the iterator can be used to retrieve both the mime type ((*it).first), and the
corresponding LoaderStruct ((*it).second)
@return true if successful, false otherwise.
*/
	static bool GetFirstLoader (std::map<std::string, LoaderStruct>::iterator &it);
/*!
@param it a std::map iterator initialized by Loader::GetFirstLoader

Gets the next LoderStruct and its associated mime type.
@return true if successful, false otherwise.
*/
	static bool GetNextLoader (std::map<std::string, LoaderStruct>::iterator &it);
/*!
@param mime_type a mime type.
@return the Loader instance able to read the mime type if any.
*/
	static Loader *GetLoader (char const *mime_type);
/*!
@param mime_type a mime type.
@return the Loader instance able to write the mime type if any.
*/
	static Loader *GetSaver (char const *mime_type);

	// virtual methods
/*!
@param doc the gcu::Document being read.
@param in a GsfInput (see the libgsf documentation at
http://library.gnome.org/devel/gsf/stable/gsf-Input-from-unstructured-files.html#GsfInput).
@param mime_type the mime type of the data.
@param io a GOffice IOContext.

This function must be overloaded by a derived class able to read. Default
implementation just return false.
@return true on success, false otherwise.
*/
	virtual bool Read (Document *doc, GsfInput *in, char const *mime_type, IOContext *io);
/*!
@param doc the gcu::Document being written.
@param out a GsfOutput (see the libgsf documentation at
http://library.gnome.org/devel/gsf/stable/gsf-Output-to-unstructured-files.html#GsfOutput).
@param mime_type the mime type of the data.
@param io a GOffice IOContext.

This function must be overloaded by a derived class able to write. Default
implementation just return false.
@return true on success, false otherwise.
*/
	virtual bool Write (Document *doc, GsfOutput *out, char const *mime_type, IOContext *io);

protected:
/*!
@param mime_type a mime type.

Registers a mime type and stores the instance calling it as the Loader for this mime type.
*/
	void AddMimeType (const char *mime_type);
/*!
@param mime_type a mime type.

Unregisters a mime type.
*/
	void RemoveMimeType (const char *mime_type);

private:
	static bool Inited;

protected:
/*!
The list of supported mime types.
*/
	std::list<std::string> MimeTypes;
};

}

#endif	//	GCU_LOADER_H
