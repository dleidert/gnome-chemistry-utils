/* 
 * Gnome Chemisty Utils
 * moz-plugin.c 
 *
 * Copyright (C) 2002-2005
 *
 * Developed by Jean Bréfort <jean.brefort@normalesup.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include <config.h>
#include "npapi.h"
#include "npupp.h"

static NPNetscapeFuncs mozilla_funcs;

static NPError ChemNew (NPMIMEType mime_type, NPP instance,
				 uint16 mode, uint16 argc, char *argn[], char *argv[],
				 NPSavedData *saved)
{
	return NPERR_NO_ERROR;
}

static NPError ChemDestroy (NPP instance, NPSavedData **save)
{
	return NPERR_NO_ERROR;
}

static NPError ChemSetWindow (NPP instance, NPWindow *window)
{
	return NPERR_NO_ERROR;
}

static NPError ChemNewStream (NPP instance, NPMIMEType type, NPStream *stream,
		      NPBool seekable, uint16 *stype)
{
	return NPERR_NO_ERROR;
}

static NPError ChemDestroyStream (NPP instance, NPStream *stream, NPError reason)
{
	return NPERR_NO_ERROR;
}

static int32 ChemWriteReady (NPP instance, NPStream *stream)
{
	return 0;
}

static void ChemPrint (NPP instance, NPPrint *platformPrint)
{
}

static int32 ChemWrite (NPP instance, NPStream *stream, int32 offset,
			  int32 len, void *buffer)
{
	return 0;
}

static void ChemStreamAsFile (NPP instance, NPStream *stream, const char *fname)
{
}

NPError ChemURLNotify (NPP instance, const char* url, const char* window, void* notifyData)
{
	return NPERR_NO_ERROR;
}

NPError NP_GetValue (void *future, NPPVariable variable, void *value)
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
	return ("chemical/x-xyz:XYZ Coordinate Format");
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
	pluginFuncs->newp       = NewNPP_NewProc (ChemNew);
	pluginFuncs->destroy    = NewNPP_DestroyProc (ChemDestroy);
	pluginFuncs->setwindow  = NewNPP_SetWindowProc (ChemSetWindow);
	pluginFuncs->newstream  = NewNPP_NewStreamProc (ChemNewStream);
	pluginFuncs->destroystream = NewNPP_DestroyStreamProc (ChemDestroyStream);
	pluginFuncs->asfile     = NewNPP_StreamAsFileProc (ChemStreamAsFile);
	pluginFuncs->writeready = NewNPP_WriteReadyProc (ChemWriteReady);
	pluginFuncs->write      = NewNPP_WriteProc (ChemWrite);
	pluginFuncs->print      = NewNPP_PrintProc (ChemPrint);
	pluginFuncs->urlnotify  = NewNPP_URLNotifyProc (ChemURLNotify);
	pluginFuncs->event      = NULL;
#ifdef OJI
	pluginFuncs->javaClass  = NULL;
#endif

	return NPERR_NO_ERROR;
}

NPError	NP_Shutdown(void)
{
	return NPERR_NO_ERROR;
}
