// -*- C++ -*-

/* 
 * Gnome Chemistry Utils
 * gcu/application.h
 *
 * Copyright (C) 2005-2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_APPLICATION_H
#define GCU_APPLICATION_H

#include "dialog-owner.h"
#include "structs.h"
#include "object.h"
#include <list>
#include <map>
#include <set>
#include <string>
#include <gcu/macros.h>

/*!\file*/
namespace gcu {

class Document;
class Dialog;
struct option_data;
class TypeDesc;
class CmdContext;
class UIManager;

typedef struct {
	std::string name;
	std::string uri;
} Database;

#define GCU_CONF_DIR "gchemutils"

/*!\class Application gcu/application.h
This class is a base class for applications. It provides some basic services.
*/
class Application: virtual public DialogOwner
{
friend class Document;
friend class Dialog;
public:
/*!
@param name the name of the application.
@param datadir where data for the application are stored.
@param help_name the name to use for the help file (with .xml extension). 
If NULL, the name parameter is used.
@param icon_name the name to use for the default icon of all windows. If NULL,
the help_name or name parameters will be used.

The datadir variable is used to build the full path to the help file:
"file://"+datadir+"/gnome/help/"+name+"/"+LANG+"/"+name".xml".
*/
	Application (std::string name, std::string datadir = DATADIR, char const *help_name = NULL, char const *icon_name = NULL, CmdContext *cc = NULL);
/*!
The destructor.
*/
	virtual ~Application ();

/*!
@param s an optional tag in the help file.
Displays the help file using the file browser retrieved from GConf using the
"/desktop/gnome/applications/help_viewer/exec" key. If a tag is given, it will
be added to the help uri.
For an application named "myapp" and a tag "mytag", the uri will be:
helpfilename\#myapp-mytag.
*/
	void OnHelp (std::string s = "");
/*!
@return true if both a help browser and a help file are available, false otherwise.
*/
	bool HasHelp ();
/*!
@return the name of the application. Mostly useful to set the icon name of a new window.
*/
	std::string const &GetName () const {return Name;}

/*!
@return a GtkWindow if any. Should be overloaded by children classes.
*/
	virtual GtkWindow * GetWindow () {return NULL;}

/*!
@param filename the uri of the file.
@param mime_type the mime type of the file if known.
@param bSave true if saving, and false if loading.
@param window the current top level window.
@param pDoc an optional document.

Called by the FileChooser when a file name has been selected. This method does
nothing in the parent class and must be implemented in children classes
if they use the FileChooser.
	
@return true if no error occured.
*/
	virtual bool FileProcess (G_GNUC_UNUSED const gchar* filename, G_GNUC_UNUSED const gchar* mime_type, G_GNUC_UNUSED bool bSave, G_GNUC_UNUSED GtkWindow *window, G_GNUC_UNUSED Document *pDoc = NULL)
		{return false;}

/*!
@return the path to the current directory.
*/
	char const* GetCurDir () {return CurDir.c_str ();}

/*!
@param dir the path to the new current directory.
*/
	void SetCurDir (char const* dir);

/*!
@param dir the path to the new current directory.
*/
	void SetCurDir (std::string const &dir);

/*!
@param uri the uri to display.

Displays uri in the default web browser if any.
*/
	void ShowURI (std::string& uri);

/*!
@param uri the uri to the package bugs database. Default value is PACKAGE_BUGREPORT.

Opens the bugs web page in the default browser if any.
*/
	void OnBug (char const *uri = PACKAGE_BUGREPORT)
		{std::string s (uri); ShowURI (s);}

/*!
@param uri the uri to the main web page of the program. Default value is
"http://gchemutils.nongnu.org/".
*/
	void OnWeb (char const *uri = "http://gchemutils.nongnu.org/")
		{std::string s (uri); ShowURI (s);}

/*!
@param MailAddress the mail adress to which a message will be sent. Defaults to the
		Gnome Chemistry Utils main list.

Creates a new empty message using the default mail if any.
*/
	void OnMail (char const *MailAddress = "mailto:gchemutils-main@nongnu.org");

/*!
Attempts to open the \#gchemutils channel at irc.gimp.net.
*/
	void OnLiveAssistance ();

/*!
@return a std::map of the supported pixbuf formats. Keys are the mime type names.
*/
	std::map<std::string, GdkPixbufFormat*> &GetSupportedPixbufFormats () {return m_SupportedPixbufFormats;}

/*!
@param filename the file name.
@param mime_type the file mime type.

A default extension is appended to filename if none is detected.

@return the GdkPixbuf name associated to mime_type or NULL if the file type
is not supported by GdkPixbuf.
*/
	char const *GetPixbufTypeName (std::string& filename, char const *mime_type);

/*!
@param uri the uri of the document to load.
@param mime_type the mime type of the document.
@param Doc the document instance which will contain the loaded data.

This method loads a document using the appropriate gcu::Loader class
instance.
@return the found \a ContentType ot ContentTypeUnknown if an error occured.
*/
	ContentType Load (std::string const &uri, const char *mime_type, Document* Doc, const char *options = NULL);
	             
/*!
@param input a GsfInput.
@param mime_type the mime type of the document.
@param Doc the document instance which will contain the loaded data.

This method loads a document using the appropriate gcu::Loader class
instance.
@return the found \a ContentType ot ContentTypeUnknown if an error occured.
*/
	ContentType Load (GsfInput *input, const char *mime_type, Document* Doc, const char *options = NULL);
	            
/*!
@param uri the uri to which the document should be saved.
@param mime_type the mime type of the document.
@param Obj the object instance which contains the data to be saved.
@param type the type of the data to be saved (see gcu::ContentType).

This method saves the document using the appropriate gcu::Loader class
instance.
@return true if no error occurred.
*/
	bool Save (std::string const &uri, const char *mime_type, Object const *Obj, ContentType type, const char *options = NULL);
	            
/*!
@param output a GsfOutput.
@param mime_type the mime type of the document.
@param Obj the object instance which contains the data to be saved.
@param type the type of the data to be saved (see gcu::ContentType).

This method saves the document using the appropriate gcu::Loader class
instance using \a output as target.
@return true if no error occurred.
*/
	bool Save (GsfOutput *output, const char *mime_type, Object const *Obj, ContentType type, const char *options = NULL);

/*!
Virtual method used to create documents. Default behavior does nothing and returns NULL.
@return the newly created document or NULL.
*/
	virtual Document *CreateNewDocument () {return NULL;}

/*!
Method used to retrieve the base configuration node.
@return the base configuration node.
*/
	static GOConfNode *GetConfDir ();

/*!
Retrieves the icon name that should be set to every window owned by the application.
@return the icon name for the appication.
*/
	std::string const &GetIconName () {return IconName;}

/*!
@param entries: the entries to register.
@param translation_domain: the entries to register.

Adds new command line options. Typically called from a plugin. The new
options are added to the main group.
*/
	void RegisterOptions (GOptionEntry const *entries, char const *translation_domain = GETTEXT_PACKAGE);

/*!
@param context: a GOptionContext

Adds all registered options to the context. This should be called once
just after creating the application and before parsing options.
*/
	void AddOptions (GOptionContext *context);
/*!
@return a dummy Application instance which might be used when there is no other
Application available.
*/
	static Application *GetDefaultApplication ();
	static Application *GetApplication (char const *name);
	static Application *GetApplication (std::string &name);

	// Object creation related methods
/*!
@param TypeName the name of the new type.
@param CreateFunc a pointer to a function returning a pointer to a new object of the new type.
@param id the Id of the type to create if a standard one or OtherType for a new type. In this last case, this parameter
can be omitted.

This method is used to register a new type derived from Object.
@return the Id of the new type.
*/
	TypeId AddType (std::string TypeName, Object* (*CreateFunc) (), TypeId id = OtherType);

/*!
@param TypeName the name of the new type.
@param parent the parent of the newly created object or NULL. if NULL, the parameter can be omitted.

Used to create an object of type name TypeName. The AddType() method must have been called with the same
TypeName parameter. if parent is given and not NULL, the new Object will be a child of parent.
It will also be given a default Id.

@return a pointer to the newly created Object or NULL if the Object could not be created.
*/
	Object* CreateObject (const std::string& TypeName, Object* parent = NULL);
/*!
@param type1 the TypeId of the first class in the rule
@param rule the new rule value
@param type2 the TypeId of the second class in the rule

Adds a rule.
*/
	void AddRule (TypeId type1, RuleId rule, TypeId type2);
/*!
@param type1 the name of the first class in the rule
@param rule the new rule value
@param type2 the name of the second class in the rule

Adds a rule.
*/
	void AddRule (const std::string& type1, RuleId rule, const std::string& type2);
/*!
@param type the TypeId of a class
@param rule a RuleId value

@return the set of rules correponding to the RuleId value for this class.
*/
	 const std::set<TypeId>& GetRules (TypeId type, RuleId rule);

/*!
@param type the name of a class
@param rule a RuleId value

@return the set of rules correponding to the RuleId value for this class.
*/
	const std::set<TypeId>& GetRules (const std::string& type, RuleId rule);

/*!
@param Id the TypeId of a class
@param Label the string to display in a contextual menu

Used to give a label for contextual menus used when the creation of an instance of
the class seems possible.
*/
	void SetCreationLabel (TypeId Id, std::string Label);

/*!
@param Id the TypeId of a class

@return the string defined by SetCreationLabel.
*/
	const std::string& GetCreationLabel (TypeId Id);

/*!
@param uim the UIManager to populate.
@param object the Object on which occured the mouse click.
@param x x coordinate of the mouse click.
@param y y coordinate of the mouse click.

This method is called to build a contextual menu for the object. It is called by Object::BuildContextualMenu, so
it should not be necessary to call it directly.
@return true if something is added to the UIManager, false otherwise.
*/
	bool BuildObjectContextualMenu (Object *target, UIManager *uim, Object *object, double x, double y);

/*!
@param Id the TypeId of the Object derived class
@param cb the BuildMenuCb callback to call when building the menu.

adds a callback for modifying the contextual menu of objects of type Id.
*/
	void AddMenuCallback (TypeId Id, BuildMenuCb cb);
	            
/*!
@param TypeName the name of a class

@return the string defined by SetCreationLabel.
*/
	const std::string& GetCreationLabel (const std::string& TypeName);

	TypeDesc const *GetTypeDescription (TypeId Id);
	CmdContext *GetCmdContext ();


/*!
@param input a source GsfInput.
@param mime_type the mime type of the document.
@param options options to pass to OpenBabel.
 
This method converts the source to CML.
@return the converted text as a newly allocate string or NULL.
*/
	char* ConvertToCML (std::string const &uri, const char *mime_type, const char *options = NULL);
	char* ConvertToCML (GsfInput *input, const char *mime_type, const char *options = NULL);

/*!
@param cml: the CML string to convert.
@param uri the uri of the document to which the document will be saved.
@param mime_type the mime type of the document.
@param options options to pass to OpenBabel.
 
This method converts CML to a target.
*/
	void ConvertFromCML (const char *cml, std::string const &uri, const char *mime_type, const char *options = NULL);

/*!
@param cml: the CML string to convert.
@param output a target GsfOutput.
@param mime_type the mime type of the document.
@param options options to pass to OpenBabel.
 
This method converts CML to a target.
*/
	void ConvertFromCML (const char *cml, GsfOutput *output, const char *mime_type, const char *options = NULL);
	std::list < Database > const &GetDatabases (char const *classname) {return m_Databases[classname];}

protected:
	void RegisterBabelType (const char *mime_type, const char *type);
	virtual void CreateDefaultCmdContext () {}

private:
	void AddDocument (Document *Doc) {m_Docs.insert (Doc);}
	void RemoveDocument (Document *Doc);
	int OpenBabelSocket ();
	char const *MimeToBabelType (char const *mime_type);

private:
	std::string Name;
	std::string HelpName;
	std::string HelpBrowser;
	std::string HelpFilename;
	std::string CurDir;
	std::string IconName;
	static GOConfNode *m_ConfDir;
	std::list <option_data> m_Options;
	std::map <TypeId, TypeDesc> m_Types;
	std::map <std::string, std::string> m_BabelTypes;

protected:

/*!
This method is called by the framework when all the documents have been removed from
the set of opened documents. The default behavior is to do nothing
Derived class might overide this method to change this.
*/
	virtual void NoMoreDocsEvent () {}
/*!
std::map of the supported pixbuf formats. Keys are the mime type names.
*/
	std::map<std::string, GdkPixbufFormat*> m_SupportedPixbufFormats;

	CmdContext *m_CmdContext;

private:
	std::map < std::string, std::list <Database> >m_Databases;
/*!\var m_Docs
The currently opened documents.
*/
/*!\fn GetDocs()
@return the set of currently opened documents.
*/
GCU_PROT_PROP (std::set <Document*>, Docs)
/*!\fn GetScreenResolution()
@return the current screen resolution.
*/
GCU_RO_PROP (unsigned, ScreenResolution)
/*!\fn SetImageResolution(unsigned ImageResolution)
@param ImageResolution the new image resolution.

Sets the image resolution used when exporting a pixmap.
Applications can use either the image resolution or the width and height to select
an exported image size, but not both.
*/
/*!\fn GetImageResolution()
@return the current image resolution used on export.
*/
/*!\fn GetRefImageResolution()
@return the current image resolution used on export as a reference.
*/
GCU_PROP (unsigned, ImageResolution)
/*!\fn SetImageWidth(unsigned Width)
@param Width the new image width.

Sets the image width used when exporting a pixmap.
Applications can use either the image resolution or the width and height to select
an exported image size, but not both.
*/
/*!\fn GetImageWidth()
@return the current image width used on export.
*/
/*!\fn GetRefImageWidth()
@return the current image width used on export as a reference.
*/
GCU_PROP (unsigned, ImageWidth)
/*!\fn SetImageHeight(unsigned Height)
@param Height the new image height.

Sets the image height used when exporting a pixmap.
Applications can use either the image resolution or the width and height to select
an exported image size, but not both.
*/
/*!\fn GetImageHeight()
@return the current image height used on export.
*/
/*!\fn GetRefImageHeight()
@return the current image height used on export as a reference.
*/
GCU_PROP (unsigned, ImageHeight)
/*!\fn SetTransparentBackground(bool transparent)
@param transparent whether an export image should have a transparent background.

Sets the transparency of the exported image when possible.
*/
/*!\fn GetTransparentBackground()
@return whether exported image have a transparent background.
*/
/*!\fn GetRefTransparentBackground()
@return whether exported image have a transparent background as a reference.
*/
GCU_PROP (bool, TransparentBackground)
};

}	// namespace gcu

#endif // GCU_APPLICATION_H
