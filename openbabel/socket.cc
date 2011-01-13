// -*- C++ -*-

/* 
 * OpenBabel server
 * babel-socket.cc 
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

#include "config.h"
#include "socket.h"
#include <unistd.h>
#include <cstring>

#include <stdio.h>

#define bufsize 128 // should be large enough

enum {
	STEP_INIT,
	STEP_OPTION_IN,
	STEP_OPTION_OUT,
	STEP_SIZE,
	STEP_DATA
};

BabelSocket::BabelSocket (int socket):
m_Socket (socket),
m_Step (STEP_INIT)
{
	m_InBuf = new char[bufsize];
}

BabelSocket::~BabelSocket ()
{
	if (m_Socket)
		close (m_Socket);
	delete m_InBuf;
}

size_t BabelSocket::Read ()
{
	size_t res = 0;
	res = read (m_Socket, m_InBuf + m_Index, bufsize - m_Index);
	m_Index += res;
	m_InBuf[m_Index] = 0;
	if (m_Index == 0)
		return 0; // nothing has been read, should not happen
	switch (m_Step) {
	case STEP_INIT: {
		if (m_InBuf[0] != '-') // error
			return -1;
		char option = m_InBuf[1];
		if (!option)
			break;
		if (m_InBuf[2] == 0 || !strchr (m_InBuf + 3, ' ')) {
			switch (option) {
			case 'i':
				break;
			case 'o':
				break;
			case 'l':
				break;
			case 'D':
				break;
			default:
				break;
			}
			break;
		}
		break;
	}
	case STEP_OPTION_IN:
		break;
	case STEP_OPTION_OUT:
		break;
	case STEP_SIZE:
		break;
	case STEP_DATA:
		break;
	default:
		break;
	}
	return res;
}
