// -*- C++ -*-

/* 
 * OpenBabel server
 * babelserv.cc 
 *
 * Copyright (C) 2010 Jean Bréfort <jean.brefort@normalesup.org>
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
#include <errno.h>
#include <locale.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

int listening_socket;
time_t timeout = 1800;
time_t endtime;

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
	strcpy (address.sun_path, "/tmp/babelsocket");

	/* bind the socket */
	if (bind (listening_socket, (struct sockaddr*) &address, sizeof(address)) == -1) {
		perror ("socket attachment failed");
		close (listening_socket);
		unlink (address.sun_path);
		return -2;
	}

	if (listen (listening_socket, 16) == -1) {
		perror ("listen");
		close (listening_socket);
		unlink (address.sun_path);
		return -3;
	}

	endtime = time (NULL) + timeout;
	struct pollfd fds;
	fds.fd = listening_socket;
	fds.events = POLLIN;
	fds.revents = 0;
	struct sockaddr_in fromend;
	unsigned lng_address;
	int service_socket;

	while (time (NULL) < endtime) {
		if (poll (&fds, 1, 1000) && fds.revents == POLLIN) {
			service_socket = accept (listening_socket, (struct sockaddr*) &fromend, &lng_address);
			if (service_socket == -1 && errno == EINTR)	// a signal was received
				continue ;
			if (service_socket == -1) {	// fatal error
				perror ("accept") ;
				return -4;
			}
			// TODO: start listening the client
		}
	}

	close (listening_socket);
	unlink (address.sun_path);
	return 0;
}
