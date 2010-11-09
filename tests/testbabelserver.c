/* 
 * Gnome Chemisty Utils
 * tests/testbabelserver.c 
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include <glib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

int main ()
{
	char *args[] = {"babelserver", NULL};
	GError *error = NULL;
	g_spawn_async (NULL, (char **) args, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL,&error);
	if (error) {
		g_error_free (error);
		error = NULL;
	}
	struct stat statbuf;
	while (stat ("/tmp/babelsocket", &statbuf));
	int babelsocket = socket (AF_UNIX, SOCK_STREAM, 0);
	if (babelsocket == -1) {
		perror("Could not create the socket");
		return FALSE;
	}
	struct sockaddr_un adr_serv;
	adr_serv.sun_family = AF_UNIX;
	strcpy (adr_serv.sun_path, "/tmp/babelsocket");
	if (connect (babelsocket, (const struct sockaddr*) &adr_serv, sizeof (struct sockaddr_un)) == -1) {
		perror ("Connexion failed");
		return FALSE;
	}
	// TODO: write the code
	return 0;
}