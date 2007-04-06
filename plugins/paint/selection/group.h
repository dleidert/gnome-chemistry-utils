/* 
 * GChemPaint selection plugin
 * group.h
 *
 * Copyright (C) 2004-2007 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCHEMPAINT_GROUP_H
#define GCHEMPAINT_GROUP_H

#include <gcu/object.h>

using namespace gcu;

extern TypeId GroupType;

typedef enum {
	GCP_ALIGN_NORMAL,
	GCP_ALIGN_TOP,
	GCP_ALIGN_MID_HEIGHT,
	GCP_ALIGN_BOTTOM,
	GCP_ALIGN_LEFT,
	GCP_ALIGN_CENTER,
	GCP_ALIGN_RIGHT
} gcpAlignType;

class gcpGroup: public Object
{
public:
	gcpGroup ();
	virtual ~gcpGroup ();

	virtual bool BuildContextualMenu (GtkUIManager *UIManager, Object *object, double x, double y);
	virtual bool Load (xmlNodePtr);
	virtual xmlNodePtr Save (xmlDocPtr xml);
	virtual bool OnSignal (SignalId Signal, Object *Child);
	void SetAligned (gcpAlignType type);
	void UnAlign ();
	bool GetAlignType (gcpAlignType& align);
	void SetPadding (double padding);
	void UnSpace ();
	bool GetPadding (double& padding);
	virtual void Transform2D (Matrix2D& m, double x, double y);
	virtual double GetYAlign ();

private:
	void Align ();
	void Space ();

private:
	gcpAlignType m_AlignType;
	double m_Padding;
	bool m_Align, m_Spaced;
};

#endif	// GCHEMPAINT_GROUP_H
