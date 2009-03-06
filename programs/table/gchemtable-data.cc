/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-data.cc
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "config.h"
#include "gchemtable-data.h"
#include <gcu/element.h>
#include <map>
#include <string>
#include <goffice/math/go-math.h>
#include <gsf/gsf-impl-utils.h>
#include <glib/gi18n.h>

using namespace std;
static map <string, GOData *> GctScalars, GctVectors, GctMatrices;

static  GOData *
gct_data_dup (GOData const *src)
{
	GOData *dst = GO_DATA (g_object_ref (G_OBJECT (src)));

	return dst;
}

static gboolean
gct_data_eq (GOData const *data_a, GOData const *data_b)
{
	return FALSE;
}

static GOFormat *
gct_data_preferred_fmt (GOData const *dat)
{

	return NULL;
}

static char *
gct_data_as_str (GOData const *dat)
{
	return NULL;
}

static  gboolean
gct_data_from_str (GOData *dat, char const *str)
{
	return FALSE;
}

/*******************************************/
// scalar data

struct _GctDataScalar {
	GODataScalar	 base;
	void (*loader) (double *value);
	bool loaded;
	double data;
};

typedef GODataScalarClass GctDataScalarClass;

static GObjectClass *scalar_parent_klass;

static double
gct_data_scalar_get_value (GODataScalar *dat)
{
	return 0.;
}

static char const *
gct_data_scalar_get_str (GODataScalar *dat)
{
	return NULL;
}

static void
gct_data_scalar_class_init (GObjectClass *gobject_klass)
{
	GODataClass *godata_klass = (GODataClass *) gobject_klass;
	GODataScalarClass *scalar_klass = (GODataScalarClass *) gobject_klass;

	scalar_parent_klass = (GObjectClass *) g_type_class_peek_parent (gobject_klass);
	godata_klass->dup = gct_data_dup;
	godata_klass->eq = gct_data_eq;
	godata_klass->preferred_fmt	= gct_data_preferred_fmt;
	godata_klass->as_str = gct_data_as_str;
	godata_klass->from_str = gct_data_from_str;
	scalar_klass->get_value = gct_data_scalar_get_value;
	scalar_klass->get_str = gct_data_scalar_get_str;
}

static void
gct_data_scalar_init (GObject *obj)
{
}

GSF_CLASS (GctDataScalar, gct_data_scalar,
	   gct_data_scalar_class_init, gct_data_scalar_init,
	   GO_DATA_SCALAR_TYPE)

void gct_data_scalar_new (char const *name, void (*loader) (double *value))
{
	GctDataScalar *sc = GCT_DATA_SCALAR (g_object_new (GO_DATA_SCALAR_TYPE, NULL));
	sc->loader = loader;
	GctScalars[name] = GO_DATA (sc);

}

GOData *gct_data_scalar_get_from_name (char const *name)
{
	map <string, GOData *>::iterator it = GctScalars.find (name);
	return (it != GctScalars.end ())? GO_DATA (g_object_ref (G_OBJECT ((*it).second))): NULL;
}

/*******************************************/
// vector data

struct _GctDataVector {
	GODataVector	 base;
	void (*loader) (double **, int *);
	bool loaded;
	double *data;
};

typedef GODataVectorClass GctDataVectorClass;

static GObjectClass *vector_parent_klass;

static void
gct_data_vector_load_len (GODataVector *dat)
{
	GctDataVector *vec = GCT_DATA_VECTOR (dat);
	if (!vec)
		return;
	if (!vec->loaded)
		vec->loader (&vec->data, &dat->len);
	vec->loaded = true;
	dat->base.flags |= GO_DATA_VECTOR_LEN_CACHED;
}

static void
gct_data_vector_load_values (GODataVector *dat)
{
	GctDataVector *vec = GCT_DATA_VECTOR (dat);
	if (!vec)
		return;
	if (!vec->loaded)
		vec->loader (&vec->data, &dat->len);
	vec->loaded = true;
	dat->values = vec->data;
	if (dat->len > 0) {
		int i = 0;
		while (!go_finite (vec->data[i++]) && i < dat->len);
		if (i < dat->len)
			dat->minimum = dat->maximum = vec->data[i];
		for (; i < dat->len; i++)
			if (go_finite (vec->data[i])) {
				if (vec->data[i] < dat->minimum)
					dat->minimum = vec->data[i];
				if (vec->data[i] > dat->maximum)
					dat->maximum = vec->data[i];
			}
	}
	dat->base.flags |= GO_DATA_CACHE_IS_VALID;
}

static double
gct_data_vector_get_value (GODataVector *dat, unsigned i)
{
	GctDataVector *vec = GCT_DATA_VECTOR (dat);
	if (!vec)
		return NULL;
	if (!vec->loaded)
		vec->loader (&vec->data, &dat->len);
	vec->loaded = true;
	return ((int) i < dat->len)? vec->data[i]: go_nan;
}

static char *
gct_data_vector_get_str (GODataVector *dat, unsigned i)
{
	GctDataVector *vec = GCT_DATA_VECTOR (dat);
	if (!vec)
		return NULL;
	if (!vec->loaded)
		vec->loader (&vec->data, &dat->len);
	vec->loaded = true;
	return ((int) i < dat->len)? g_strdup_printf ("%g", vec->data[i]): NULL;
}

void
gct_data_vector_finalize (GObject *obj)
{
	GctDataVector *vec = GCT_DATA_VECTOR (obj);
	if (!vec)
		return;
	if (vec->data)
		delete [] vec->data;
}

static void
gct_data_vector_class_init (GObjectClass *gobject_klass)
{
	GODataClass *godata_klass = (GODataClass *) gobject_klass;
	GODataVectorClass *vector_klass = (GODataVectorClass *) gobject_klass;

	gobject_klass->finalize = gct_data_vector_finalize;
	vector_parent_klass = (GObjectClass *) g_type_class_peek_parent (gobject_klass);
	godata_klass->dup = gct_data_dup;
	godata_klass->eq = gct_data_eq;
	godata_klass->preferred_fmt	= gct_data_preferred_fmt;
	godata_klass->as_str = gct_data_as_str;
	godata_klass->from_str = gct_data_from_str;
	vector_klass->load_len = gct_data_vector_load_len;
	vector_klass->load_values = gct_data_vector_load_values;
	vector_klass->get_value = gct_data_vector_get_value;
	vector_klass->get_str = gct_data_vector_get_str;
}

static void
gct_data_vector_init (GObject *obj)
{
}

GSF_CLASS (GctDataVector, gct_data_vector,
	   gct_data_vector_class_init, gct_data_vector_init,
	   GO_DATA_VECTOR_TYPE)

void gct_data_vector_new (char const *name, void (*loader) (double **values, int *length))
{
	GctDataVector *vec = GCT_DATA_VECTOR (g_object_new (GCT_DATA_VECTOR_TYPE, NULL));
	vec->loader = loader;
	GctVectors[name] = GO_DATA (vec);
}

GOData *gct_data_vector_get_from_name (char const *name)
{
	map <string, GOData *>::iterator it = GctVectors.find (name);
	return (it != GctVectors.end ())? GO_DATA (g_object_ref (G_OBJECT ((*it).second))): NULL;
}

char const *gct_data_vector_get_first (GOData **data, gpointer *closure)
{
	g_return_val_if_fail (closure && (*closure == NULL), NULL);
	if (GctVectors.empty ())
		return NULL;
	map <string, GOData *>::iterator *it = new map <string, GOData *>::iterator;
	*it = GctVectors.begin ();
	*closure = it;
	if (data)
		*data = GO_DATA (g_object_ref (G_OBJECT ((**it).second)));
	return (**it).first.c_str ();
}

char const *gct_data_vector_get_next (GOData **data, gpointer *closure)
{
	g_return_val_if_fail (closure, NULL);
	map <string, GOData *>::iterator *it = reinterpret_cast <map <string, GOData *>::iterator *> (*closure);
	++*it;
	if (*it == GctVectors.end ()) {
		delete it;
		*closure = NULL;
		return NULL;
	}
	if (data)
		*data = GO_DATA (g_object_ref (G_OBJECT ((**it).second)));
	return (**it).first.c_str ();
}

/*******************************************/
// matrix data

struct _GctDataMatrix {
	GODataMatrix	 base;
	void (*loader) (double **, int *, int *);
	bool loaded;
	double *data;
};

typedef GODataMatrixClass GctDataMatrixClass;

static GObjectClass *matrix_parent_klass;

static void
gct_data_matrix_load_size (GODataMatrix *dat)
{
}

static void
gct_data_matrix_load_values (GODataMatrix *dat)
{
}

static double
gct_data_matrix_get_value (GODataMatrix *dat, unsigned i, unsigned j)
{
	return go_nan;
}

static char *
gct_data_matrix_get_str (GODataMatrix *dat, unsigned i, unsigned j)
{
	return NULL;
}

static void
gct_data_matrix_class_init (GObjectClass *gobject_klass)
{
	GODataClass *godata_klass = (GODataClass *) gobject_klass;
	GODataMatrixClass *matrix_klass = (GODataMatrixClass *) gobject_klass;

	matrix_parent_klass = (GObjectClass *) g_type_class_peek_parent (gobject_klass);
	godata_klass->dup = gct_data_dup;
	godata_klass->eq = gct_data_eq;
	godata_klass->preferred_fmt	= gct_data_preferred_fmt;
	godata_klass->as_str = gct_data_as_str;
	godata_klass->from_str = gct_data_from_str;
	matrix_klass->load_size = gct_data_matrix_load_size;
	matrix_klass->load_values = gct_data_matrix_load_values;
	matrix_klass->get_value = gct_data_matrix_get_value;
	matrix_klass->get_str = gct_data_matrix_get_str;
}

static void
gct_data_matrix_init (GObject *obj)
{
}

GSF_CLASS (GctDataMatrix, gct_data_matrix,
	   gct_data_matrix_class_init, gct_data_matrix_init,
	   GO_DATA_MATRIX_TYPE)

void gct_data_matrix_new (char const *name, void (*loader) (double **values, int *cols, int *rows))
{
	GctDataMatrix *mat = GCT_DATA_MATRIX (g_object_new (GCT_DATA_MATRIX_TYPE, NULL));
	mat->loader = loader;
	GctMatrices[name] = GO_DATA (mat);
}

GOData *gct_data_matrix_get_from_name (char const *name)
{
	map <string, GOData *>::iterator it = GctMatrices.find (name);
	return (it != GctMatrices.end ())? GO_DATA (g_object_ref (G_OBJECT ((*it).second))): NULL;
}

/******************************************************************************/
//	Data loaders

static void loadZ (double **values, int *len)
{
	*values = new double[MAX_ELT];
	for (int i = 1; i <= MAX_ELT; i++)
		(*values)[i - 1] = (gcu::Element::GetElement (i))? i: go_nan;
	*len = MAX_ELT;
}

static void loadMass (double **values, int *len)
{
	*values = new double[118];
	for (int i = 1; i <= 118; i++) {
		gcu::Element *elt = gcu::Element::GetElement (i);
		(*values)[i - 1] = (elt)? elt->GetWeight ()->GetAsDouble (): go_nan;
	}
	*len = MAX_ELT;
}

static void loadPaulingENeg (double **values, int *len)
{
	*values = new double[MAX_ELT];
	GcuElectronegativity en;
	en.scale = "Pauling";
	for (int i = 1; i <= MAX_ELT; i++) {
		en.Z = i;
		(*values)[i - 1] = (gcu_element_get_electronegativity (&en))?
							en.value.value: go_nan;
	}
	*len = MAX_ELT;
}

static void loadIE1 (double **values, int *len)
{
	*values = new double[MAX_ELT];
	GcuDimensionalValue const *val;
	for (int i = 1; i <= MAX_ELT; i++) {
		gcu::Element *elt = gcu::Element::GetElement (i);
		val = (elt)? elt->GetIonizationEnergy (1): NULL;
		(*values)[i - 1] = (val)? val->value: go_nan;
	}
	*len = MAX_ELT;
}

static void loadIE2 (double **values, int *len)
{
	*values = new double[MAX_ELT];
	GcuDimensionalValue const *val;
	for (int i = 1; i <= MAX_ELT; i++) {
		gcu::Element *elt = gcu::Element::GetElement (i);
		val = (elt)? elt->GetIonizationEnergy (2): NULL;
		(*values)[i - 1] = (val)? val->value: go_nan;
	}
	*len = MAX_ELT;
}

static void loadIE3 (double **values, int *len)
{
	*values = new double[MAX_ELT];
	GcuDimensionalValue const *val;
	for (int i = 1; i <= MAX_ELT; i++) {
		gcu::Element *elt = gcu::Element::GetElement (i);
		val = (elt)? elt->GetIonizationEnergy (3): NULL;
		(*values)[i - 1] = (val)? val->value: go_nan;
	}
	*len = MAX_ELT;
}

static void loadEA (double **values, int *len)
{
	*values = new double[MAX_ELT];
	GcuDimensionalValue const *val;
	for (int i = 1; i <= MAX_ELT; i++) {
		gcu::Element *elt = gcu::Element::GetElement (i);
		val = (elt)? elt->GetElectronAffinity (1): NULL;
		(*values)[i - 1] = (val)? val->value: go_nan;
	}
	*len = MAX_ELT;
}

static void loadCovRad (double **values, int *len)
{
	*values = new double[MAX_ELT];
	GcuAtomicRadius r;
	r.type = GCU_COVALENT;
	r.charge = 0;
	r.scale = NULL;
	r.cn = -1;
	r.spin = GCU_N_A_SPIN;
	for (int i = 1; i <= MAX_ELT; i++) {
		r.Z = i;
		gcu::Element *elt = gcu::Element::GetElement (i);
		(*values)[i - 1] = (elt && elt->GetRadius (&r))? r.value.value: go_nan;
	}
	*len = MAX_ELT;
}

static void loadVdWRad (double **values, int *len)
{
	*values = new double[MAX_ELT];
	GcuAtomicRadius r;
	r.type = GCU_VAN_DER_WAALS;
	r.charge = 0;
	r.scale = NULL;
	r.cn = -1;
	r.spin = GCU_N_A_SPIN;
	for (int i = 1; i <= MAX_ELT; i++) {
		r.Z = i;
		gcu::Element *elt = gcu::Element::GetElement (i);
		(*values)[i - 1] = (elt && elt->GetRadius (&r))? r.value.value: go_nan;
	}
	*len = MAX_ELT;
}

static void loadMetRad (double **values, int *len)
{
	*values = new double[MAX_ELT];
	GcuAtomicRadius r;
	r.type = GCU_METALLIC;
	r.charge = 0;
	r.scale = NULL;
	r.cn = -1;
	r.spin = GCU_N_A_SPIN;
	for (int i = 1; i <= MAX_ELT; i++) {
		r.Z = i;
		gcu::Element *elt = gcu::Element::GetElement (i);
		(*values)[i - 1] = (elt && elt->GetRadius (&r))? r.value.value: go_nan;
	}
	*len = MAX_ELT;
}

static void loadFP (double **values, int *len)
{
	*values = new double[MAX_ELT];
	gcu::Value const *prop;
	for (int i = 1; i <= MAX_ELT; i++) {
		gcu::Element *elt = gcu::Element::GetElement (i);
		prop = (elt)? elt->GetProperty ("meltingpoint"): NULL;
		(*values)[i - 1] = (prop)? prop->GetAsDouble (): go_nan;
	}
	*len = MAX_ELT;
}

static void loadEP (double **values, int *len)
{
	*values = new double[MAX_ELT];
	gcu::Value const *prop;
	for (int i = 1; i <= MAX_ELT; i++) {
		gcu::Element *elt = gcu::Element::GetElement (i);
		prop = (elt)? elt->GetProperty ("boilingpoint"): NULL;
		(*values)[i - 1] = (prop)? prop->GetAsDouble (): go_nan;
	}
	*len = MAX_ELT;
}

/******************************************************************************/
//	Initialization and destruction of generic data

void gct_data_init ()
{
	gct_data_vector_new (_("Atomic number"), loadZ);
	gct_data_vector_new (_("Atomic mass"), loadMass);
	gct_data_vector_new (_("Pauling electronegativity"), loadPaulingENeg);
	gct_data_vector_new (_("First ionization energy"), loadIE1);
	gct_data_vector_new (_("Second ionization energy"), loadIE2);
	gct_data_vector_new (_("Third ionization energy"), loadIE3);
	gct_data_vector_new (_("Electronic affinity"), loadEA);
	gct_data_vector_new (_("Covalent radius"), loadCovRad);
	gct_data_vector_new (_("Van der Waals radius"), loadVdWRad);
	gct_data_vector_new (_("Metallic radius"), loadMetRad);
	gct_data_vector_new (_("Fusion temperature"), loadFP);
	gct_data_vector_new (_("Ebullition temperature"), loadEP);
}

void gct_data_clear ()
{
	map <string, GOData *>::iterator it, end;
	end = GctScalars.end ();
	for (it = GctScalars.begin (); it != end; it++)
		g_object_unref ((*it).second);
	GctScalars.clear ();
	end = GctVectors.end ();
	for (it = GctVectors.begin (); it != end; it++)
		g_object_unref ((*it).second);
	GctVectors.clear ();
	end = GctMatrices.end ();
	for (it = GctMatrices.begin (); it != end; it++)
		g_object_unref ((*it).second);
	GctMatrices.clear ();
}
