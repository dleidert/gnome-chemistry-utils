// -*- C++ -*-

/*
 * OpenBabel server
 * socket.h
 *
 * Copyright (C) 2011 Jean Bréfort <jean.brefort@normalesup.org>
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

#ifndef GCU_BABEL_SOCKET_H
#define GCU_BABEL_SOCKET_H

#include <sys/socket.h>
#include <openbabel/obconversion.h>
#include <string>

class BabelSocket
{
public:
	BabelSocket (int socket);
	~BabelSocket ();

	size_t Read ();

private:
	void FinishOption (unsigned step);

private:
	int m_Socket;
	char *m_InBuf;
	size_t m_Index, m_Cur, m_Start, m_Size;
	bool m_WaitSpace;
	unsigned m_Step;
	std::string m_Input, m_Output;
	OpenBabel::OBConversion m_Conv;
};

#endif	//	GCU_BABEL_SOCKET_H
