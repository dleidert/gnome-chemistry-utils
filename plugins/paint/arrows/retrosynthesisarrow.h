// -*- C++ -*-

/*
 * GChemPaint arrows plugin
 * retrosynthesisarrow.h
 *
 * Copyright (C) 2004-2008 Jean Bréfort <jean.brefort@normalesup.org>
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

	xmlNodePtr Save (xmlDocPtr xml) const;
	bool Load (xmlNodePtr);
	void AddItem ();
	void SetSelected (int state);
	std::string Name();
};

#endif	// GCHEMPAINT_RETROSYNTHESIS_ARROW_H
