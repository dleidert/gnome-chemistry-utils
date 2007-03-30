// -*- C++ -*-

/* 
 * GChemPaint library
 * reaction-arrow.h 
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

#ifndef GCHEMPAINT_REACTION_ARROW_H
#define GCHEMPAINT_REACTION_ARROW_H

#include "arrow.h"

namespace gcp {

class ReactionStep;
class Reaction;

class ReactionArrow: public Arrow
{
public:
	ReactionArrow (Reaction* react, unsigned Type = SimpleArrow);
	virtual ~ReactionArrow ();
	
	virtual xmlNodePtr Save (xmlDocPtr xml);
	virtual bool Load (xmlNodePtr);
	virtual void Add (GtkWidget* w);
	virtual void Update (GtkWidget* w);
	void SetStartStep (ReactionStep *Step) {m_Start = Step;}
	ReactionStep* GetStartStep () {return m_Start;}
	void SetEndStep (ReactionStep *Step) {m_End = Step;}
	ReactionStep* GetEndStep () {return m_End;}
	void RemoveStep (ReactionStep *Step);

private:
	unsigned m_Type;
	bool m_TypeChanged;
	ReactionStep *m_Start, *m_End;
};

}	//	namespace gcp

#endif	//	GCHEMPAINT_REACTION_ARROW_H
