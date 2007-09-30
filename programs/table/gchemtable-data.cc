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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include "config.h"
#include "gchemtable-data.h"
#include <goffice/math/go-math.h>
#include <gsf/gsf-impl-utils.h>

static  GOData *
gct_data_dup (GOData const *src)
{
	GOData *dst = GO_DATA (g_object_new (G_OBJECT_TYPE (src), NULL));

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

/*******************************************/
// vector data

struct _GctDataVector {
	GODataVector	 base;
};

typedef GODataVectorClass GctDataVectorClass;

static GObjectClass *vector_parent_klass;

static void
gct_data_vector_load_len (GODataVector *dat)
{
}

static void
gct_data_vector_load_values (GODataVector *dat)
{
}

static double
gct_data_vector_get_value (GODataVector *dat, unsigned i)
{
	return go_nan;
}

static char *
gct_data_vector_get_str (GODataVector *dat, unsigned i)
{
	return NULL;
}

static void
gct_data_vector_class_init (GObjectClass *gobject_klass)
{
	GODataClass *godata_klass = (GODataClass *) gobject_klass;
	GODataVectorClass *vector_klass = (GODataVectorClass *) gobject_klass;

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

/*******************************************/
// matrix data

struct _GctDataMatrix {
	GODataMatrix	 base;
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
