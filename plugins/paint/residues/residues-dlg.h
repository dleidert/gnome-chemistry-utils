// -*- C++ -*-

/*
 * GChemPaint library
 * residues-dlg.h
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef GCP_RESIDUE_DLG_H
#define GCP_RESIDUE_DLG_H

#include <gcu/dialog.h>
#include <gcp/target.h>

using namespace std;
using namespace gcu;

namespace gcp {
class Application;
class Document;
}

class gcpResiduesDlg: public Dialog, public gcp::Target
{
public:
	gcpResiduesDlg (gcp::Application *App);
	virtual ~gcpResiduesDlg ();
	
	void Add ();
	void Remove ();
	bool Close ();
	
private:
};

#endif	//	GCP_RESIDUE_DLG_H
