/*
 * Gnome Crystal
 * grid.h
 *
 * Copyright (C) 2010-2012 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCR_GRID_H
#define GCR_GRID_H

/*!\file
@brief Grid widget.

The grid widget used in GCrystal dialogs to display lists of atoms, cleavages,
or lines.
*/

#include <gtk/gtk.h>

G_BEGIN_DECLS

/*!\return the GType associated to GcrGrid */
#define GCR_TYPE_GRID	(gcr_grid_get_type ())
/*!
Casts \a obj to a GcrGrid * pointer.
@return a pointer to the GcrGrid * or NULL if \a obj does not point to
a GcrGrid widget.
*/
#define GCR_GRID(obj)	(G_TYPE_CHECK_INSTANCE_CAST((obj), GCR_TYPE_GRID, GcrGrid))
/*!
@return TRUE if \a obj points to a GcrGrid widget, FALSE otherwise.
*/
#define GCR_IS_GRID(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), GCR_TYPE_GRID))

/*!\struct GcrGrid
@brief Grid widget.

The grid widget used in GCrystal dialogs to display lists of atoms, cleavages,
or lines.

Functions related to the GcrGrid Widget are described in the gcrgrid.h page.
*/
typedef struct _GcrGrid GcrGrid;

GType      gcr_grid_get_type (void);

/*!
@param col_title the first column title.
@param col_type the first column data type.

Builds a new GcrGrid widget.
The arguments must include titles and types for each column in the grid.
Supported types are G_TYPE_INT, G_TYPE_UINT, G_TYPE_DOUBLE, G_TYPE_STRING,
and G_TYPE_BOOLEAN. Strings can't be edited in this version.
@return a pointer to the new widget.
*/
GtkWidget *gcr_grid_new (char const *col_title, GType col_type,...);

/*!
@param grid a GcrGrid.
@param row the row.
@param column the column.

Retrieves the integer value stored in the cell. If the column type is not
G_TYPE_INT, a critical will be emitted and the function will return 0.
@return the integer value stored in the cell defined by \a row and \a column.
*/
int gcr_grid_get_int (GcrGrid *grid, unsigned row, unsigned column);

/*!
@param grid a GcrGrid.
@param row the row.
@param column the column.

Retrieves the unsigned integer value stored in the cell. If the column type is not
G_TYPE_UINT, a critical will be emitted and the function will return 0.
@return the unsigned integer value stored in the cell defined by \a row and \a column.
*/
unsigned gcr_grid_get_uint (GcrGrid *grid, unsigned row, unsigned column);

/*!
@param grid a GcrGrid.
@param row the row.
@param column the column.

Retrieves the floating point value stored in the cell. If the column type is not
G_TYPE_DOUBLE, a critical will be emitted and the function will return NAN.
@return the double value stored in the cell defined by \a row and \a column.
*/
double gcr_grid_get_double (GcrGrid *grid, unsigned row, unsigned column);

/*!
@param grid a GcrGrid.
@param row the row.
@param column the column.

Retrieves the string stored in the cell. If the column type is not
G_TYPE_DOUBLE, a critical will be emitted and the function will return NULL.
@return the string stored in the cell defined by \a row and \a column.
*/
char const *gcr_grid_get_string (GcrGrid *grid, unsigned row, unsigned column);

/*!
@param grid a GcrGrid.
@param row the row.
@param column the column.

Retrieves the boolean value stored in the cell. If the column type is not
G_TYPE_BOOLEAN, a critical will be emitted and the function will return false.
@return the boolean value stored in the cell defined by \a row and \a column.
*/
bool gcr_grid_get_boolean (GcrGrid *grid, unsigned row, unsigned column);

/*!
@param grid a GcrGrid.
@param row the row.
@param column the column.
@param value the new value.

Sets the integer value stored in the cell. If the column type is not
G_TYPE_INT, a critical will be emitted and the cell content will be unchanged.
*/
void gcr_grid_set_int (GcrGrid *grid, unsigned row, unsigned column, int value);

/*!
@param grid a GcrGrid.
@param row the row.
@param column the column.
@param value the new value.

Sets the unsigned integer value stored in the cell. If the column type is not
G_TYPE_UINT, a critical will be emitted and the cell content will be unchanged.
*/
void gcr_grid_set_uint (GcrGrid *grid, unsigned row, unsigned column, unsigned value);

/*!
@param grid a GcrGrid.
@param row the row.
@param column the column.
@param value the new value.

Sets the floating point value stored in the cell. If the column type is not
G_TYPE_DOUBLE, a critical will be emitted and the cell content will be unchanged.
*/
void gcr_grid_set_double (GcrGrid *grid, unsigned row, unsigned column, double value);

/*!
@param grid a GcrGrid.
@param row the row.
@param column the column.
@param value the new value.

Sets the string stored in the cell. If the column type is not
G_TYPE_STRING, a critical will be emitted and the cell content will be unchanged.
*/
void gcr_grid_set_string (GcrGrid *grid, unsigned row, unsigned column, char const *value);

/*!
@param grid a GcrGrid.
@param row the row.
@param column the column.
@param value the new value.

Sets the boolean value stored in the cell. If the column type is not
G_TYPE_BOOLEAN, a critical will be emitted and the cell content will be unchanged.
*/
void gcr_grid_set_boolean (GcrGrid *grid, unsigned row, unsigned column, bool value);

/*!
@param grid a GcrGrid.

Adds a new row to the grid. The \a grid argument must be followed by the values to fill
inside the cells.
@return the new row index.
*/
unsigned gcr_grid_append_row (GcrGrid *grid,...);

/*!
@param grid a GcrGrid.
@param row the row.

Deletes a row from the grid.
*/
void gcr_grid_delete_row (GcrGrid *grid, unsigned row);

/*!
@param grid a GcrGrid.

Deletes all selected rows from the grid.
*/
void gcr_grid_delete_selected_rows (GcrGrid *grid);

/*!
@param grid a GcrGrid.

Deletes all rows from the grid.
*/
void gcr_grid_delete_all (GcrGrid *grid);

/*!
@param grid a GcrGrid.

Selects all rows in the grid.
*/
void gcr_grid_select_all (GcrGrid *grid);

/*!
@param grid a GcrGrid.
@param column the column.
@param chars the maximum number of characters to display.
@param editable whether data in the column can be edited.

Changes the properties for the column.
*/
void gcr_grid_customize_column (GcrGrid *grid, unsigned column, unsigned chars, bool editable);

/*!
@param grid a GcrGrid.
@param allow boolean.

Sets whether multiple rows selection is allowed for the grid.
*/
void gcr_grid_set_allow_multiple_selection (GcrGrid *grid, bool allow);

/*!
@param grid a GcrGrid.
@param row the row to select.

Adds the row to the selection if multiple selection is allowed or just select the
row and replace the previous selection.
*/
void gcr_grid_add_row_to_selection (GcrGrid *grid, unsigned row);

/*!
@param grid a GcrGrid.
@param row a row index.

Unselects a row.
*/
void gcr_grid_unselect_row (GcrGrid *grid, unsigned row);


/*!
@param i a row index.
@param user_data user data.

Callback for gcr_grid_for_each_selected().
*/
typedef void (*GridCb) (unsigned i, void *user_data);

/*!
@param grid a GcrGrid.
@param cb callback.
@param user_data user data.

Executes \a cb for each selected row.
*/
void gcr_grid_for_each_selected (GcrGrid *grid, GridCb cb, void *user_data);
G_END_DECLS

#endif	//	GCR_GRID_H
