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
#include <cstdlib>

#define bufsize 128 // should be large enough

enum {
	STEP_INIT,
	STEP_INIT_OPTION,
	STEP_OPTION_IN,
	STEP_OPTION_OUT,
	STEP_SIZE,
	STEP_DATA
};

BabelSocket::BabelSocket (int socket):
m_Socket (socket),
m_Index (0),
m_Cur (0),
m_Start (0),
m_Size (bufsize),
m_WaitSpace (false),
m_Step (STEP_INIT)
{
	m_InBuf = new char[m_Size + 8];
}

BabelSocket::~BabelSocket ()
{
	if (m_Socket)
		close (m_Socket);
	delete m_InBuf;
}

size_t BabelSocket::Read ()
{
	size_t res = read (m_Socket, m_InBuf + m_Index, m_Size - m_Index);
	m_Index += res;
	m_InBuf[m_Index] = 0;
	while (m_Cur < m_Index) {
		if (!m_WaitSpace && m_Step != STEP_DATA) {
			while (m_InBuf[m_Cur] == ' ')
				m_Cur++;
			m_Start = m_Cur;
			m_WaitSpace = true;
		}
		switch (m_Step) {
		case STEP_INIT:
			if (m_InBuf[m_Cur] != '-') // error
				return -1;
			m_Step = STEP_INIT_OPTION;
			m_Cur++;
			break;
		case STEP_INIT_OPTION: {
			char option = m_InBuf[m_Cur];
			if (!option)
				break;
			switch (option) {
			case 'i':
				m_Step = STEP_OPTION_IN;
				break;
			case 'o':
				m_Step = STEP_OPTION_OUT;
				break;
			case 'l':
				m_Step = STEP_SIZE;
				break;
			case 'D':
				m_Step = STEP_DATA;
				memmove (m_InBuf, m_InBuf + 2, m_Index + 1);
				m_Index -= 2;
				break;
			default:
				break;
			}
			m_WaitSpace = false;
			m_Cur++;
			m_Start = m_Cur;
			break;
		}
		case STEP_OPTION_IN:
			while (m_InBuf[m_Cur] != ' ' && m_Cur < m_Index)
				m_Cur++;
			if (m_InBuf[m_Cur] == ' ') {
				m_InBuf[m_Cur] = 0;
				if (strcmp (m_InBuf + m_Start, "chemical/x-inchi")) {
					OpenBabel::OBFormat *format = m_Conv.FormatFromMIME (m_InBuf + m_Start);
					if (format)
						m_Conv.SetInFormat (format);
					else
						return -1;
				} else
					m_Conv.SetInFormat ("inchi");
				FinishOption ();
			}
			break;
		case STEP_OPTION_OUT:
			while (m_InBuf[m_Cur] != ' ' && m_Cur < m_Index)
				m_Cur++;
			if (m_InBuf[m_Cur] == ' ') {
				m_InBuf[m_Cur] = 0;
				if (strcmp (m_InBuf + m_Start, "chemical/x-inchi")) {
					OpenBabel::OBFormat *format = m_Conv.FormatFromMIME (m_InBuf + m_Start);
					if (format)
						m_Conv.SetOutFormat (format);
					else
						return -1;
				} else
					m_Conv.SetOutFormat ("inchi");
				FinishOption ();
			}
			break;
		case STEP_SIZE:
			while (m_InBuf[m_Cur] != ' ' && m_Cur < m_Index)
				m_Cur++;
			if (m_InBuf[m_Cur] == ' ') {
				char *end;
				m_InBuf[m_Cur] = 0;
				m_Size = strtoul (m_InBuf + m_Start, &end, 10);
				if (end && *end)
					return -1; // invalid size
				FinishOption ();
				if (m_Size > bufsize) {
					char *new_buf = new char[m_Size + 256]; // we need a bit more, at least because of the "-D"
					memcpy (new_buf, m_InBuf, m_Index);
					delete [] m_InBuf;
					m_InBuf = new_buf;
				}
				return res; // force more read
			}
			break;
		case STEP_DATA:
			if (m_Index == m_Size) {
				std::istringstream is (m_InBuf);
				std::ostringstream os, l;
				m_Conv.Convert (&is, &os);
				l << os.str ().length () << " ";
				write (m_Socket, l.str ().c_str (), l.str ().length ());
				write (m_Socket, os.str ().c_str (), os.str ().length ());
				return -1;
			} else
				return res;
		default:
			break;
		}
	}
	return res;
}

void BabelSocket::FinishOption ()
{
	m_Cur++;
	if (m_Cur < m_Index)
			memmove (m_InBuf, m_InBuf + m_Cur, m_Index - m_Cur);
	m_WaitSpace = false;
	m_Index -= m_Cur;
	m_Cur = m_Start = 0;
	m_Step = STEP_INIT;
}
