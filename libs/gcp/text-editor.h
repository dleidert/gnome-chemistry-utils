// -*- C++ -*-

/*
 * GChemPaint library
 * text-editor.h
 *
 * Copyright (C) 2009 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef GCHEMPAINT_TEXT_EDITOR_H
#define GCHEMPAINT_TEXT_EDITOR_H

/*!\file*/
namespace gcp {

/*!
@brief Text editor.

Virtual class used to edit texts. The goal is to get a notification when the
selection changes.
*/
class TextEditor
{
public:
/*!
The method call by the framework when the edited text selection changes. This
methid must be implemented by derived classes.
*/
	virtual void SelectionChanged () = 0;
};

}   //  namespace gcp

#endif  //  GCHEMPAINT_TEXT_EDITOR_H
