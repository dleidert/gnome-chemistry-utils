/* 
 * Gnome Chemisty Utils
 * moz-plugin.c 
 *
 * Copyright (C) 2002-2005 Jean Bréfort <jean.brefort@normalesup.org>
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

#include <config.h>
#include "npapi.h"
#ifdef HAVE_NPFUNCTIONS_H
#   include "npfunctions.h"
#else
#   include "npupp.h"
#endif
#include <unistd.h>
#include <string.h>
#include <glib.h>

static NPNetscapeFuncs mozilla_funcs;
static int pid = 0;
static int to_pipe = 0;

typedef struct {
	
	char *url;	/* The URL that this instance displays */
	
	char *mime_type;	/* The mime type that this instance displays */
	
	int width, height;	/* The size of the display area */
	
	unsigned long moz_xid;	/* The XID of the window mozilla has for us */
	
/*	int argc;	
	char **args;
	pthread_t thread;*/
	NPP instance;

	NPStream *m_Stream;
	bool m_ExpectingStream;
} ChemPlugin;

static NPError ChemNew (NPMIMEType mime_type, NPP instance,
#ifdef HAVE_NPFUNCTIONS_H
				 G_GNUC_UNUSED uint16_t mode, uint16_t argc, char *argn[], char *argv[],
#else
				 G_GNUC_UNUSED uint16 mode, uint16 argc, char *argn[], char *argv[],
#endif
				 G_GNUC_UNUSED NPSavedData *saved)
{
	ChemPlugin *plugin;
	char buf [32];
	int i;
  
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	instance->pdata = mozilla_funcs.memalloc (sizeof (ChemPlugin));
	plugin = (ChemPlugin *) instance->pdata;

	if (plugin == NULL)
		return NPERR_OUT_OF_MEMORY_ERROR;
	memset (plugin, 0, sizeof (ChemPlugin));

	plugin->instance = instance;

	if (pid == 0) {
		int p[2];
		char *argv[2];
		argv[0] = LIBEXECDIR"/chem-viewer";
		argv[1] = NULL;
		
		if (pipe (p) < 0) {
			perror ("pipe creation");
			return NPERR_INVALID_INSTANCE_ERROR;
		}	
		if ((pid = fork()) < 0) {
			perror ("fork");
			return NPERR_INVALID_INSTANCE_ERROR;
		} else if (pid > 0) {
			close (p[0]);
			to_pipe = p[1];
		} else {
			close (p[1]);
			if (p[0] != STDIN_FILENO){
				if (dup2 (p[0], STDIN_FILENO) != STDIN_FILENO) {
					perror("dup2 (stdin)");
				}
				close (p[0]);
			}
			if (execvp (argv[0], argv) < 0) {
				perror ("execvp");
			}
		}
	}
	write (to_pipe, "new\n", 4);
	snprintf (buf, 32, "%p\n", instance);
	write (to_pipe, buf, strlen (buf));
	write (to_pipe, mime_type, strlen ((char*) mime_type));
	write (to_pipe, "\n", 1);
	i = 0;
	while (i < argc && strcmp (argn[i++], "PARAM"));
	for (; i < argc; i++) {
		if (argn[i] == NULL || argv[i] == NULL)
			break;
		write (to_pipe, argn[i], strlen (argn[i]));
		write (to_pipe, "\n", 1);
		write (to_pipe, argv[i], strlen (argv[i]));
		write (to_pipe, "\n", 1);
	}
	write (to_pipe, "end\n", 4);
	plugin->m_Stream = NULL;
	plugin->m_ExpectingStream = true;
	return NPERR_NO_ERROR;
}

static NPError ChemDestroy (NPP instance, G_GNUC_UNUSED NPSavedData **save)
{
	ChemPlugin *plugin;
	char buf[32];

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	
	plugin = (ChemPlugin *) instance->pdata;
	if (plugin == NULL)
		return NPERR_NO_ERROR;

	write (to_pipe, "kill\n", 5);
	snprintf (buf, 32, "%p\n", instance);
	write (to_pipe, buf, strlen (buf));

	mozilla_funcs.memfree (instance->pdata);
	instance->pdata = NULL;
	return NPERR_NO_ERROR;
}

static NPError ChemSetWindow (NPP instance, NPWindow *window)
{
	char buf[32];
	write (to_pipe, "win\n", 4);
	snprintf (buf, 32, "%p\n", instance);
	write (to_pipe, buf, strlen (buf));
	snprintf (buf, 32, "%p\n", window->window);
	write (to_pipe, buf, strlen (buf));
	return NPERR_NO_ERROR;
}

static NPError ChemNewStream (NPP instance, G_GNUC_UNUSED NPMIMEType type, G_GNUC_UNUSED NPStream *stream,
#ifdef HAVE_NPFUNCTIONS_H
		      G_GNUC_UNUSED NPBool seekable, uint16_t *stype)
#else
		      G_GNUC_UNUSED NPBool seekable, uint16 *stype)
#endif
{
	if (!stream || !stream->url)
		return NPERR_GENERIC_ERROR;

	ChemPlugin *plugin;

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	plugin = (ChemPlugin *) instance->pdata;
	if (plugin == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;	

	if (plugin->m_Stream || !plugin->m_ExpectingStream)
		return mozilla_funcs.destroystream (instance, stream, NPRES_DONE);

	plugin->m_ExpectingStream = false;
	plugin->m_Stream = stream;

	*stype = NP_ASFILEONLY;

	return NPERR_NO_ERROR;
}

static NPError ChemDestroyStream (G_GNUC_UNUSED NPP instance, G_GNUC_UNUSED NPStream* stream, G_GNUC_UNUSED NPReason reason)
{
	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	ChemPlugin *plugin = (ChemPlugin *) instance->pdata;
	if (!plugin->m_Stream || plugin->m_Stream != stream)
		return NPERR_GENERIC_ERROR;
	plugin->m_Stream = NULL;
	return NPERR_NO_ERROR;
}

static void ChemPrint (G_GNUC_UNUSED NPP instance, G_GNUC_UNUSED NPPrint *platformPrint)
{
// TODO: implement !!!
}

static void ChemStreamAsFile (NPP instance, G_GNUC_UNUSED NPStream *stream, const char *fname)
{
	char buf[32];
	write (to_pipe, "file\n", 5);
	snprintf (buf, 32, "%p\n", instance);
	write (to_pipe, buf, strlen (buf));
	write (to_pipe, fname, strlen (fname));
	write (to_pipe, "\n", 1);
}

NPError NP_GetValue (G_GNUC_UNUSED void *future, NPPVariable variable, void *value)
{
	switch (variable) {
	case NPPVpluginNameString:
		*((char **) value) = "Gnome Chemistry Utils";
		break;
	case NPPVpluginDescriptionString:
		*((char **) value) =
			"Gnome Chemistry Utils "VERSION". "
			"Chemical structures display.";
		break;
	default:
		return NPERR_GENERIC_ERROR;
	}
	return NPERR_NO_ERROR;
}

char *NP_GetMIMEDescription (void)
{
	return ("chemical/x-xyz:xyz:XYZ Coordinate Format;"
			"chemical/x-mdl-molfile:mol:MDL Molfile;"
			"chemical/x-pdb:pdb,ent:Protein DataBank;"
			"chemical/x-jcamp-dx:dx,jdx:JCAMP Spectroscopic Data Exchange Format;"
			"application/x-gcrystal:gcrystal:Crystalline structure model;"
			"chemical/x-cif:cif:Crystallographic Information File;"
			"application/x-gchempaint:gchempaint:2D chemical structures"
			"chemical/x-cdx:cdx:ChemDraw binary CDX format;"
			"chemical/x-cdxml:cdxml:ChemDraw CDXML format;");
}

/* This is called to initialise the plugin
 */
NPError NP_Initialize(NPNetscapeFuncs *mozFuncs, NPPluginFuncs *pluginFuncs) {
	if (mozFuncs == NULL || pluginFuncs == NULL)
		return NPERR_INVALID_FUNCTABLE_ERROR;
	
	if ((mozFuncs->version >> 8) > NP_VERSION_MAJOR)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;
	if (mozFuncs->size < sizeof (NPNetscapeFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;
	if (pluginFuncs->size < sizeof (NPPluginFuncs))
		return NPERR_INVALID_FUNCTABLE_ERROR;

	memcpy (&mozilla_funcs, mozFuncs, sizeof (NPNetscapeFuncs));
	pluginFuncs->version    = (NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR;
	pluginFuncs->size       = sizeof (NPPluginFuncs);
#ifdef HAVE_NPFUNCTIONS_H
	pluginFuncs->newp       = (NPP_NewProcPtr)ChemNew;
	pluginFuncs->destroy    = (NPP_DestroyProcPtr) ChemDestroy;
	pluginFuncs->setwindow  = (NPP_SetWindowProcPtr) ChemSetWindow;
	pluginFuncs->newstream  = (NPP_NewStreamProcPtr) ChemNewStream;
	pluginFuncs->destroystream  = (NPP_DestroyStreamProcPtr) ChemDestroyStream;
	pluginFuncs->asfile     = (NPP_StreamAsFileProcPtr) ChemStreamAsFile;
#else
	pluginFuncs->newp       = NewNPP_NewProc (ChemNew);
	pluginFuncs->destroy    = NewNPP_DestroyProc (ChemDestroy);
	pluginFuncs->setwindow  = NewNPP_SetWindowProc (ChemSetWindow);
	pluginFuncs->newstream  = NewNPP_NewStreamProc (ChemNewStream);
	pluginFuncs->destroystream  = NewNPP_DestroyStreamProcPtr (ChemDestroyStream);
	pluginFuncs->asfile     = NewNPP_StreamAsFileProc (ChemStreamAsFile);
#endif
	pluginFuncs->writeready = NULL;
	pluginFuncs->write      = NULL;
#ifdef HAVE_NPFUNCTIONS_H
	pluginFuncs->print      = (NPP_PrintProcPtr) ChemPrint;
#else
	pluginFuncs->print      = NewNPP_PrintProc (ChemPrint);
#endif
	pluginFuncs->urlnotify  = NULL;
	pluginFuncs->event      = NULL;
#ifdef OJI
	pluginFuncs->javaClass  = NULL;
#endif

	return NPERR_NO_ERROR;
}

NPError	NP_Shutdown(void)
{
	/* stop the server and close all resources */
	write (to_pipe, "halt\n", 5);
	pid = 0;
	close (to_pipe);
	to_pipe = 0;
	return NPERR_NO_ERROR;
}
