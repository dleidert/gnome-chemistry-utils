// -*- C++ -*-

/*
 * OSMesa server
 * osmesaserv.cc
 *
 * Copyright (C) 2010-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
 * USA
 */

#include "config.h"
#include "socket.h"
#include <cerrno>
#include <clocale>
#include <netinet/in.h>
#include <poll.h>
#include <cstdio>
#include <sys/un.h>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <map>
#include <set>
#include <vector>

int listening_socket;
time_t timeout = 1800;
time_t endtime;
unsigned max_socket = 10;
std::map <int, OSMesaSocket *> sockets;

int main (int argc, char *argv[])
{
	int port;
	port = fork();
	if (port != 0)
	{
		if (port < 0) {
			perror("fork");
			return port;
		}
		return 0;
	}
	if ((listening_socket = socket (AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("socket creation failed");
		return -1;
	}
	struct sockaddr_un address;
	address.sun_family = AF_UNIX;
	char *usr = getenv ("USER");
	char *path = reinterpret_cast <char *> (malloc (strlen ("/tmp/babelsocket-") + strlen (usr) + 1));
	strcpy (path, "/tmp/babelsocket-");
	strcat (path, usr);
	if (strlen (path) >= 107) { //WARNING: don't know if this is portable
		puts ("path too long");
		free (path);
		return -2;
	}
	strcpy (address.sun_path, path);
	free (path);

	/* bind the socket */
	if (bind (listening_socket, (struct sockaddr*) &address, sizeof(address)) == -1) {
		perror ("socket attachment failed");
		close (listening_socket);
		unlink (address.sun_path);
		return -3;
	}

	if (listen (listening_socket, 16) == -1) {
		perror ("listen");
		close (listening_socket);
		unlink (address.sun_path);
		return -4;
	}

	endtime = time (NULL) + timeout;
	std::vector <struct pollfd> fds;
	struct pollfd _fds;
	fds.reserve (max_socket); // should be large enough
	fds.resize (1);
	fds[0].fd = listening_socket;
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	std::set <int> deleted;
	static struct sockaddr_in fromend;
	static unsigned lng_address;
	int service_socket;

	while (time (NULL) < endtime) {
		if (poll (&fds[0], fds.size (), 1000) > 0) {
			if (fds[0].revents == POLLIN) {
				service_socket = accept (listening_socket, (struct sockaddr*) &fromend, &lng_address);
				if (service_socket == -1 && errno == EINTR)	// a signal was received
					continue ;
				if (service_socket == -1) {	// fatal error
					perror ("accept") ;
					return -5;
				}
				_fds.fd = service_socket;
#ifdef POLLRDHUP
				_fds.events = POLLIN | POLLRDHUP;
#else
				_fds.events = POLLIN;
#endif
				_fds.revents = 0;
				fds.push_back (_fds);
				sockets[service_socket] = new OSMesaSocket (service_socket);
			}
			for (unsigned i = 1; i < fds.size (); i++) {
				if (fds[i].revents & POLLIN) {
					int res;
					while ((res = sockets[fds[i].fd]->Read ()) > 0);
					if (res == -1) {
						delete sockets[fds[i].fd];
						sockets.erase (fds[i].fd);
						deleted.insert (i);
					}
				}
#ifdef POLLRDHUP
				if (fds[i].revents & POLLRDHUP) {
					delete sockets[fds[i].fd];
					sockets.erase (fds[i].fd);
					deleted.insert (i);
				}
#endif
				fds[i].revents = 0;
			}
			// remove closed sockets
			if (deleted.size () > 0) {
				std::set <int>::iterator it, end = deleted.end ();
				for (it = deleted.begin (); it != end; it++)
						fds.erase (fds.begin () + *it);
				deleted.clear ();
			}
			endtime = time (NULL) + timeout; // restart time counter from now
		}
	}

	close (listening_socket);
	unlink (address.sun_path);
	return 0;
}
