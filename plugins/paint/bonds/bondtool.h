// -*- C++ -*-

/* 
 * GChemPaint bonds plugin
 * bondtool.h 
 *
 * Copyright (C) 2001-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_BOND_TOOL_H
#define GCHEMPAINT_BOND_TOOL_H

#include <gcp/tool.h>

namespace gcp {
	class Atom;
	class Bond;
	class Operation;
}

class gcpBondTool: public gcp::Tool
{
public:
	gcpBondTool (gcp::Application *App, std::string ToolId = "Bond", unsigned nPoints = 2);
	virtual ~gcpBondTool ();
	
	bool OnClicked ();
	void OnDrag ();
	void OnRelease ();
	GtkWidget *GetPropertyPage ();
	void Activate ();
	void SetAngle (double angle);
	void SetLength (double length);
	char const *GetHelpTag () {return "bond";}

protected:
	virtual void Draw ();
	virtual void UpdateBond ();
	virtual void FinalizeBond ();
	virtual void SetType (gcp::Bond* pBond);

protected:
	double m_dAngle;
	gcp::Atom* m_pAtom;
	unsigned char BondOrder;

private:
	gcp::Operation *m_pOp;
	GtkSpinButton *m_LengthBtn, *m_AngleBtn;
	GtkToggleButton *m_MergeBtn;
	bool m_AutoDir;
	double m_RefAngle;
};

class gcpUpBondTool: public gcpBondTool
{
public:
	gcpUpBondTool (gcp::Application *App);
	virtual ~gcpUpBondTool ();

protected:
	virtual void Draw ();
	virtual void UpdateBond ();
	virtual void FinalizeBond ();
	virtual void SetType (gcp::Bond* pBond);
};

class gcpForeBondTool: public gcpBondTool
{
public:
	gcpForeBondTool (gcp::Application *App);
	virtual ~gcpForeBondTool ();

protected:
	virtual void Draw ();
	virtual void UpdateBond ();
	virtual void FinalizeBond ();
	virtual void SetType (gcp::Bond* pBond);
};

class gcpDownBondTool: public gcpBondTool
{
public:
	gcpDownBondTool (gcp::Application *App);
	virtual ~gcpDownBondTool ();

	void OnConfigChanged ();

protected:
	virtual void Draw ();
	virtual void UpdateBond ();
	virtual void FinalizeBond ();
	virtual void SetType (gcp::Bond* pBond);
};

class gcpSquiggleBondTool: public gcpBondTool
{
public:
	gcpSquiggleBondTool (gcp::Application *App);
	virtual ~gcpSquiggleBondTool ();

protected:
	virtual void Draw();
	virtual void UpdateBond();
	virtual void FinalizeBond();
	virtual void SetType(gcp::Bond* pBond);
};

#endif // GCHEMPAINT_BOND_TOOL_H
