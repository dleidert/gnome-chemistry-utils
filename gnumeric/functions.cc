/*
 * GChemUtils Gnumeric plugin
 * functions.cc
 *
 * Copyright (C) 2005-2013 Jean Bréfort <jean.brefort@normalesup.org>
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

static bool isotopes_loaded = false;

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
gnm_float value_get_as_float		 (GnmValue const *v);
GnmValue *value_new_float            (gnm_float f);
GnmValue *value_new_string           (char const *str);
static GnmFuncHelp const help_molarmass[] = {
    { GNM_FUNC_HELP_NAME, N_("MOLARMASS:molar mass of a chemical entity")},
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

static GnmFuncHelp const help_monoisotopicmass[] = {
    { GNM_FUNC_HELP_NAME, N_("MONOISOTOPICMASS:monoisotopic mass of a chemical entity")},
    { GNM_FUNC_HELP_ARG, N_("formula:the input chemical formula such as \"CCl4\"")},
	{ GNM_FUNC_HELP_DESCRIPTION, N_("MONOISOTOPICMASS calculates a monoisotopic mass associated with the given @{formula}.") },
	{ GNM_FUNC_HELP_EXAMPLES, N_("=monoisotopicmass(\"CCl4\")") },
	{ GNM_FUNC_HELP_END, NULL }
};

static GnmValue *
gnumeric_monoisotopicmass (GnmFuncEvalInfo *ei, GnmValue const * const *argv)
{
	GnmValue *res;
	gcu::Formula *f = NULL;

	if (!isotopes_loaded) {
		gcu::Element::LoadIsotopes ();
		isotopes_loaded = true;
	}

	try {
		f = new gcu::Formula (value_peek_string (argv[0]));
		gcu::IsotopicPattern pattern;
		f->CalculateIsotopicPattern (pattern);
		gcu::SimpleValue v = pattern.GetMonoMass ();
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

static GnmFuncHelp const help_chemcomposition[] = {
    { GNM_FUNC_HELP_NAME, N_("CHEMCOMPOSITION:mass percent of a given element inside a chemical entity")},
    { GNM_FUNC_HELP_ARG, N_("formula:the input chemical formula such as \"CCl4\"")},
    { GNM_FUNC_HELP_ARG, N_("element:an element symbol \"C\"")},
	{ GNM_FUNC_HELP_DESCRIPTION, N_("CHEMCOMPOSITION calculates the mass percent of an element inside the given @{formula}.") },
	{ GNM_FUNC_HELP_EXAMPLES, N_("=chemcomposition(\"CCl4\",\"C\")") },
	{ GNM_FUNC_HELP_END, NULL }
};

static GnmValue *
gnumeric_chemcomposition (GnmFuncEvalInfo *ei, GnmValue const * const *argv)
{
	GnmValue *res;
	gcu::Formula *f = NULL;
	char const *elt = value_peek_string (argv[1]);
	if (!elt || !*elt)
		return value_new_error_std (ei->pos, GNM_ERROR_VALUE);
	int num = gcu_element_get_Z (elt);
	if (num == 0)
		return value_new_error_std (ei->pos, GNM_ERROR_VALUE);

	try {
		f = new gcu::Formula (value_peek_string (argv[0]));
		bool artificial;
		double weight = f->GetMolecularWeight (artificial).GetAsDouble ();
		std::map < int, int > elts = f->GetRawFormula ();
		std::map < int, int >::iterator i = elts.find (num);
		int stoich = i == elts.end ()? 0: (*i).second;
		
		res = value_new_float (round (gcu_element_get_weight (num) * stoich / weight * 10000.) / 100.); // round to the second decimal
	}
	catch (gcu::parse_error &e) {
		res = value_new_error_std (ei->pos, GNM_ERROR_VALUE);
	}
	if (f)
		delete f ;
	return res;
}

static GnmFuncHelp const help_elementnumber[] = {
    { GNM_FUNC_HELP_NAME, N_("ELEMENTNUMBER:element number")},
    { GNM_FUNC_HELP_ARG, N_("symbol:an element symbol such as \"C\"")},
	{ GNM_FUNC_HELP_DESCRIPTION, N_("ELEMENTNUMBER returns the element number for the given @{symbol}.") },
	{ GNM_FUNC_HELP_EXAMPLES, N_("=elementnumber(\"C\")") },
	{ GNM_FUNC_HELP_END, NULL }
};

static GnmValue *
gnumeric_elementnumber (GnmFuncEvalInfo *ei, GnmValue const * const *argv)
{
	char const *elt = value_peek_string (argv[0]);
	if (!elt || !*elt)
		return value_new_error_std (ei->pos, GNM_ERROR_VALUE);
	int num = gcu_element_get_Z (elt);
	return num? value_new_float (num): value_new_error_std (ei->pos, GNM_ERROR_VALUE);
}

static GnmFuncHelp const help_elementsymbol[] = {
    { GNM_FUNC_HELP_NAME, N_("ELEMENTSYMBOL:symbol of an element")},
    { GNM_FUNC_HELP_ARG, N_("number:the element number such as 12")},
	{ GNM_FUNC_HELP_DESCRIPTION, N_("ELEMENTSYMBOL returns the symbol for the given element @{number}.") },
	{ GNM_FUNC_HELP_EXAMPLES, N_("=elementsymbol(12)") },
	{ GNM_FUNC_HELP_END, NULL }
};

static GnmValue *
gnumeric_elementsymbol (GnmFuncEvalInfo *ei, GnmValue const * const *argv)
{
	double val = value_get_as_float (argv[0]);
	int Z = floor (val);
	if (val != Z)
		return value_new_error_std (ei->pos, GNM_ERROR_VALUE);
	char const *symbol = gcu_element_get_symbol (Z);
	return (symbol)? value_new_string (symbol): value_new_error_std (ei->pos, GNM_ERROR_VALUE);
}

const GnmFuncDescriptor Chemistry_functions[] = {

        { N_("molarmass"),       "s",
			help_molarmass, gnumeric_molarmass, NULL, NULL, NULL,
			GNM_FUNC_SIMPLE, GNM_FUNC_IMPL_STATUS_COMPLETE, GNM_FUNC_TEST_STATUS_NO_TESTSUITE},
        { N_("monoisotopicmass"),       "s",
			help_monoisotopicmass, gnumeric_monoisotopicmass, NULL, NULL, NULL,
			GNM_FUNC_SIMPLE, GNM_FUNC_IMPL_STATUS_COMPLETE, GNM_FUNC_TEST_STATUS_NO_TESTSUITE},
        { N_("chemcomposition"),       "ss",
			help_chemcomposition, gnumeric_chemcomposition, NULL, NULL, NULL,
			GNM_FUNC_SIMPLE, GNM_FUNC_IMPL_STATUS_COMPLETE, GNM_FUNC_TEST_STATUS_NO_TESTSUITE},
        { N_("elementnumber"),       "s",
			help_elementnumber, gnumeric_elementnumber, NULL, NULL, NULL,
			GNM_FUNC_SIMPLE, GNM_FUNC_IMPL_STATUS_COMPLETE, GNM_FUNC_TEST_STATUS_NO_TESTSUITE},
        { N_("elementsymbol"),       "f",
			help_elementsymbol, gnumeric_elementsymbol, NULL, NULL, NULL,
			GNM_FUNC_SIMPLE, GNM_FUNC_IMPL_STATUS_COMPLETE, GNM_FUNC_TEST_STATUS_NO_TESTSUITE},


        {NULL, NULL, NULL, NULL, NULL, NULL, NULL,
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

