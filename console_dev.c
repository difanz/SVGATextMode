/*  SVGATextMode -- An SVGA textmode manipulation/enhancement tool
 *
 *  console_dev.c -- console device base name discovery
 *
 *  Copyright (C) 2000  Ron Lee <ron@debian.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "messages.h"

static char console_device_name[64] = "";

void GetConsoleDevice(void)
{
	const char	device_list[][10] = { "/dev/vc/", "/dev/tty", "" };
	struct stat	st;
	char		dev[64];
	int		i = 0;

	while( device_list[i][0] != '\0' ) {
		snprintf(dev, sizeof(dev), "%s0", device_list[i]);
		if( !stat(dev, &st) ) {
			strncpy(console_device_name, device_list[i], sizeof(console_device_name));
			return;
		}
		++i;
	}
	PERROR(("Failed to determine console device name\n"));
}

const char *ConsoleDevice(const char *number)
{
	static char device[64];

	if( console_device_name[0] == '\0' )
		GetConsoleDevice();

	snprintf(device, sizeof(device), "%s%s", console_device_name, number);

	return device;
}

