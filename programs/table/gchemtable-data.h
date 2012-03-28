/*
 * Gnome Chemistry Utils
 * programs/gchemtable-data.h
 *
 * Copyright (C) 2007-2009 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMTABLE_DATA_H
#define GCHEMTABLE_DATA_H

#include <goffice/goffice.h>

G_BEGIN_DECLS

typedef struct _GctDataScalar GctDataScalar;

#define GCT_TYPE_DATA_SCALAR	(gct_data_scalar_get_type ())
#define GCT_DATA_SCALAR(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GCT_TYPE_DATA_SCALAR, GctDataScalar))
#define GCT_IS_DATA_SCALAR(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GCT_TYPE_DATA_SCALAR))

GType gct_data_scalar_get_type ();
void gct_data_scalar_new (char const *name, void (*loader) (double *value));
GOData *gct_data_scalar_get_from_name (char const *name);

typedef struct _GctDataVector GctDataVector;

#define GCT_TYPE_DATA_VECTOR	(gct_data_vector_get_type ())
#define GCT_DATA_VECTOR(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GCT_TYPE_DATA_VECTOR, GctDataVector))
#define GCT_IS_DATA_VECTOR(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GCT_TYPE_DATA_VECTOR))

GType gct_data_vector_get_type ();
void gct_data_vector_new (char const *name, void (*loader) (double **values, int *length));
GOData *gct_data_vector_get_from_name (char const *name);
char const *gct_data_vector_get_first (GOData **data, gpointer *closure);
char const *gct_data_vector_get_next (GOData **data, gpointer *closure);

typedef struct _GctDataMatrix GctDataMatrix;

#define GCT_TYPE_DATA_MATRIX	(gct_data_matrix_get_type ())
#define GCT_DATA_MATRIX(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GCT_TYPE_DATA_MATRIX, GctDataMatrix))
#define GCT_IS_DATA_MATRIX(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GCT_TYPE_DATA_MATRIX))

GType gct_data_matrix_get_type ();
void gct_data_matrix_new (char const *name, void (*loader) (double **values, int *cols, int *rows));
GOData *gct_data_matrix_get_from_name (char const *name);

void gct_data_init (void);
void gct_data_clear (void);

G_END_DECLS

#endif	/*	GCHEMTABLE_DATA_H	*/


