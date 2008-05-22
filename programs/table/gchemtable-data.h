/* 
 * Gnome Chemistry Utils
 * programs/gchemtable-data.h
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

#ifndef GCHEMTABLE_DATA_H
#define GCHEMTABLE_DATA_H

#include <goffice/data/go-data-impl.h>

G_BEGIN_DECLS

typedef struct _GctDataScalar GctDataScalar;

#define GCT_DATA_SCALAR_TYPE	(gct_data_scalar_get_type ())
#define GCT_DATA_SCALAR(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GCT_DATA_SCALAR_TYPE, GctDataScalar))
#define IS_GCT_DATA_SCALAR(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GCT_DATA_SCALAR_TYPE))

GType gct_data_scalar_get_type ();
void gct_data_scalar_new (char const *name, void (*loader) (double *value));
GOData *gct_data_scalar_get_from_name (char const *name);

typedef struct _GctDataVector GctDataVector;

#define GCT_DATA_VECTOR_TYPE	(gct_data_vector_get_type ())
#define GCT_DATA_VECTOR(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GCT_DATA_VECTOR_TYPE, GctDataVector))
#define IS_GCT_DATA_VECTOR(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GCT_DATA_VECTOR_TYPE))

GType gct_data_vector_get_type ();
void gct_data_vector_new (char const *name, void (*loader) (double **values, int *length));
GOData *gct_data_vector_get_from_name (char const *name);
char const *gct_data_vector_get_first (GOData **data, gpointer *closure);
char const *gct_data_vector_get_next (GOData **data, gpointer *closure);

typedef struct _GctDataMatrix GctDataMatrix;

#define GCT_DATA_MATRIX_TYPE	(gct_data_matrix_get_type ())
#define GCT_DATA_MATRIX(obj)	(G_TYPE_CHECK_INSTANCE_CAST ((obj), GCT_DATA_MATRIX_TYPE, GctDataMatrix))
#define IS_GCT_DATA_MATRIX(o)		(G_TYPE_CHECK_INSTANCE_TYPE ((o), GCT_DATA_MATRIX_TYPE))

GType gct_data_matrix_get_type ();
void gct_data_matrix_new (char const *name, void (*loader) (double **values, int *cols, int *rows));
GOData *gct_data_matrix_get_from_name (char const *name);

void gct_data_init (void);
void gct_data_clear (void);

G_END_DECLS

#endif	/*	GCHEMTABLE_DATA_H	*/


