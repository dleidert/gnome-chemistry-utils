/*
 * Gnome Chemisty Utils
 * tests/testbabelserver.c
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#include "config.h"
#include <glib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

int main ()
{
	struct stat statbuf;
	char inbuf[256], *start = NULL;
	int index, cur, length;
	char *usr = getenv ("USER");
	char *path = malloc (strlen ("/tmp/babelsocket-") + strlen (usr) + 1);
	strcpy (path, "/tmp/babelsocket-");
	strcat (path, usr);
	if (strlen (path) >= 107) { //WARNING: don't know if this is portable
		puts ("path too long");
		return -1;
	}
	if (stat (path, &statbuf)) {
		char *args[] = {LIBEXECDIR"/babelserver", NULL};
		GError *error = NULL;
		g_spawn_async (NULL, (char **) args, NULL, 0, NULL, NULL, NULL, &error);
		if (error) {
			g_error_free (error);
			error = NULL;
			free (path);
			return -1;
		}
		while (stat (path, &statbuf));
	}
	int babelsocket = socket (AF_UNIX, SOCK_STREAM, 0);
	if (babelsocket == -1) {
		perror ("Could not create the socket");
		free (path);
		return -1;
	}
	struct sockaddr_un adr_serv;
	adr_serv.sun_family = AF_UNIX;
	strcpy (adr_serv.sun_path, path);
	free (path);
	if (connect (babelsocket, (const struct sockaddr*) &adr_serv, sizeof (struct sockaddr_un)) == -1) {
		perror ("Connexion failed");
		return -1;
	}
	char const *buf = "-i xyz -o inchi ";
	write (babelsocket, buf, strlen (buf));
	buf = "5\n\nC       0       0       0\nH       0       1.093   0\nH       1.030490282     -0.364333333    0\nH       -0.515245141    -0.364333333    0.892430763\nH       -0.515245141    -0.364333333    -0.892430763";
	char *size = g_strdup_printf ("-l %u -D", strlen (buf));
	write (babelsocket, size, strlen (size));
	write (babelsocket, buf, strlen (buf));
	while (1) {
		if ((cur = read (babelsocket, inbuf + index, 255 - index))) {
			index += cur;
			inbuf[index] = 0;
			if (start == NULL) {
				if ((start = strchr (inbuf, ' '))) {
					length = strtol (inbuf, NULL, 10);
					start++;
				}
			}
			if (index - (start - inbuf) == length) {
				printf ("answer is: %s\n", start);
				break;
			}
		}
	}
	close (babelsocket);
	return 0;
}
