#include <stdlib.h>
#include <string.h>

#ifndef rindex
#define rindex strrchr
#endif

/* dirname.c -- return all but the last element in a path
   Copyright (C) 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Return the leading directories part of PATH,
   allocated with malloc.  If out of memory, return 0. */

char *dirname (char *path)
{
  char *newpath;
  char *slash;

  slash = rindex (path, '/');
  if (slash == 0)
    return strdup (".");

  newpath = malloc (strlen (path) + 1);
  if (newpath == 0)
    return 0;
  strcpy (newpath, path);
  slash += newpath - path;
  /* Remove any trailing slashes and final element. */
  while (slash > newpath && *slash == '/')
    --slash;
  slash[1] = 0;
  return newpath;
}


/* basename.c -- return the last element in a path
   Copyright (C) 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */


/* Return NAME with any leading path stripped off.  */

#ifndef HAVE_BASENAME
char *basename (char *name)
{
  char *base;

  base = rindex (name, '/');
  return base ? base + 1 : name;
}
#endif

