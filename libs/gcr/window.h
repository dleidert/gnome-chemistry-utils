/* 
 * GCrystal library
 * window.h
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCR_WINDOW_H
#define GCR_WINDOW_H

#include <gcu/macros.h>
#include <gcu/window.h>

namespace gcu {
class Application;
}

namespace gcr {

class Document;

class Window: public gcu::Window
{
public:
	Window (gcu::Application *app);
	virtual ~Window ();

	virtual void Destroy ();
/*!\var m_Document
The gcr::Document displayed in this window.
*/
/*!\fn GetApplication()
@return the gcr::Document displayed in this window.
*/
GCU_PROT_PROP (Document*, Document)
/*!\var m_Application
The gcu::Application owning this window.
*/
/*!\fn GetApplication()
@return the gcu::Application owning this window.
*/
GCU_PROT_PROP (gcu::Application*, Application)
};

}

#endif	//	GCR_WINDOW_H
