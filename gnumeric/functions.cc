/* 
 * GChemUtils Gnumeric plugin
 * functions.cc
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "config.h"
#include <gcu/element.h>
#include <gcu/formula.h>
#include <gcu/value.h>
#include <gnumeric.h>
#include <func.h>
#include <gnm-plugin.h>
#include <mathfunc.h>
#include <glib/gi18n-lib.h>
#include <gmodule.h>

extern "C" {

// needed to export the symbol...???
extern const GnmFuncDescriptor Chemistry_functions[];

//GNM_PLUGIN_MODULE_HEADER;
extern GOPluginModuleDepend const go_plugin_depends [] = {
	{ "goffice",	GOFFICE_API_VERSION },
	{ "gnumeric",	GNM_VERSION_FULL }
};
extern GOPluginModuleHeader const go_plugin_header =
	{ GOFFICE_MODULE_PLUGIN_MAGIC_NUMBER, G_N_ELEMENTS (go_plugin_depends) };

// add some definitions since we can't include value.h
GnmValue *value_new_error_std        (GnmEvalPos const *pos, GnmStdError err);
char const *value_peek_string		 (GnmValue const *v);
GnmValue *value_new_float            (gnm_float f);
static GnmFuncHelp const help_molarmass[] = {
    { GNM_FUNC_HELP_NAME, N_("MOLARMASS: molar mass of a chemical entity")},
    { GNM_FUNC_HELP_ARG, N_("formula:the input chemical formula such as \"CCl4\"")},
	{ GNM_FUNC_HELP_DESCRIPTION, N_("MOLARMASS calculates a molar mass associated with the given @{formula}.") },
	{ GNM_FUNC_HELP_EXAMPLES, N_("=molarmass(\"CCl4\")") },
	{ GNM_FUNC_HELP_END, NULL }
};

static GnmValue *
gnumeric_molarmass (GnmFuncEvalInfo *ei, GnmValue const * const *argv)
{
	GnmValue *res;
	gcu::Formula *f = NULL;

	try {
		f = new gcu::Formula (value_peek_string (argv[0]));
		bool artificial;
		gcu::DimensionalValue v = f->GetMolecularWeight (artificial);
		char const *s = v.GetAsString ();
		res = value_new_float (strtod (s, NULL));
	}
	catch (gcu::parse_error &e) {
		res = value_new_error_std (ei->pos, GNM_ERROR_VALUE);
	}
	if (f)
		delete f ;
	return res;
}

const GnmFuncDescriptor Chemistry_functions[] = {

        { N_("molarmass"),       "s",
			help_molarmass, gnumeric_molarmass, NULL, NULL, NULL, NULL,
			GNM_FUNC_SIMPLE, GNM_FUNC_IMPL_STATUS_COMPLETE, GNM_FUNC_TEST_STATUS_NO_TESTSUITE},


        {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			GNM_FUNC_IS_PLACEHOLDER,  GNM_FUNC_IMPL_STATUS_UNIMPLEMENTED, GNM_FUNC_TEST_STATUS_NO_TESTSUITE}
};


G_MODULE_EXPORT void
go_plugin_init (G_GNUC_UNUSED GOPlugin *plugin, G_GNUC_UNUSED GOCmdContext *cc)
{
	bindtextdomain (GETTEXT_PACKAGE, DATADIR"/locale");
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	gcu::Element::LoadBODR ();
}

}
