/* 
 * Gnome Chemisty Utils
 * tests/testbabelserver.c 
 *
 * Copyright (C) 2010-2011 Jean Bréfort <jean.brefort@normalesup.org>
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
	struct stat statbuf;
	if (stat ("/tmp/babelsocket", &statbuf)) {
		char *args[] = {LIBEXECDIR"/babelserver", NULL};
		GError *error = NULL;
		g_spawn_async (NULL, (char **) args, NULL, 0, NULL, NULL, NULL, &error);
		if (error) {
			g_error_free (error);
			error = NULL;
		}
		while (stat ("/tmp/babelsocket", &statbuf));
	}
	int babelsocket = socket (AF_UNIX, SOCK_STREAM, 0);
	if (babelsocket == -1) {
		perror ("Could not create the socket");
		return FALSE;
	}
	struct sockaddr_un adr_serv;
	adr_serv.sun_family = AF_UNIX;
	strcpy (adr_serv.sun_path, "/tmp/babelsocket");
	if (connect (babelsocket, (const struct sockaddr*) &adr_serv, sizeof (struct sockaddr_un)) == -1) {
		perror ("Connexion failed");
		return FALSE;
	}
	char const *buf = "-i chemical/x-xyz -o chemical/x-inchi ";
	write (babelsocket, buf, strlen (buf));
	write (babelsocket, buf, strlen (buf));
	buf = "5\n\nC       0       0       0\nH       0       1.093   0\nH       1.030490282     -0.364333333    0\nH       -0.515245141    -0.364333333    0.892430763\nH       -0.515245141    -0.364333333    -0.892430763";
	char *size = g_strdup_printf ("-l %u -D", strlen (buf));
	write (babelsocket, size, strlen (size));
	write (babelsocket, buf, strlen (buf));
	
	// TODO: write the code
	while (1); // for now don't close the socket
	return 0;
}
