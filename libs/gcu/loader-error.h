// -*- C++ -*-

/*
 * Gnome Chemistry Utils
 * gcu/loader-error.h
 *
 * Copyright (C) 2002-2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_LOADER_ERROR_H
#define GCU_LOADER_ERROR_H

/*!\file*/

#include <stdexcept>

namespace gcu {

/*!\class loader_error gcu/loader-error.h
@brief Loader exception.

Exception class derived from std::exception used for errors encountered
when parsing a formula.
*/

class LoaderError: public std::exception
{
public:
/*! Takes a character string describing the error
*/
    explicit LoaderError (const std::string&  __arg);

/*!
The destructor.
*/
    virtual ~LoaderError () throw ();

/*! Returns a C-style character string describing the general cause of
 *  the current error (the same string passed to the constructor).
*/
    virtual const char* what () const throw ();

private:
	std::string m_msg, m_id;
};

}	//	namespace gcu

#endif	//	GCU_LOADER_ERROR_H