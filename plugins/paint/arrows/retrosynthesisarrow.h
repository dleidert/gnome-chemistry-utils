// -*- C++ -*-

/* 
 * GChemPaint arrows plugin
 * retrosynthesisarrow.h 
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

#ifndef GCHEMPAINT_RETROSYNTHESIS_ARROW_H
#define GCHEMPAINT_RETROSYNTHESIS_ARROW_H

#include <gcp/arrow.h>

extern gcu::TypeId RetrosynthesisArrowType;

class gcpRetrosynthesisStep;
class gcpRetrosynthesis;

class gcpRetrosynthesisArrow: public gcp::Arrow
{
public:
	gcpRetrosynthesisArrow (gcpRetrosynthesis *rs);
	virtual ~gcpRetrosynthesisArrow ();
	
	virtual xmlNodePtr Save (xmlDocPtr xml);
	virtual bool Load (xmlNodePtr);
	virtual void Add (GtkWidget* w);
	virtual void Update (GtkWidget* w);
	virtual void SetSelected (GtkWidget* w, int state);
	void SetStartStep (gcpRetrosynthesisStep *Step) {m_Start = Step;}
	gcpRetrosynthesisStep* GetStartStep () {return m_Start;}
	void SetEndStep (gcpRetrosynthesisStep *Step) {m_End = Step;}
	gcpRetrosynthesisStep* GetEndStep () {return m_End;}

private:
	gcpRetrosynthesisStep *m_Start, *m_End;
};

#endif	// GCHEMPAINT_RETROSYNTHESIS_ARROW_H
