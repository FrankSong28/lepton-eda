/* gEDA - GPL Electronic Design Automation
 * gattrib -- gEDA component and net attribute manipulation using spreadsheet.
 * Copyright (C) 2003 Stuart D. Brorson.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA
 */

#include <config.h>

#include <stdio.h>
#include <sys/stat.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/*------------------------------------------------------------------
 * Gattrib specific includes
 *------------------------------------------------------------------*/
#include <libgeda/libgeda.h>       /* geda library fcns  */
#include "../include/struct.h"     /* typdef and struct declarations */
#include "../include/prototype.h"  /* function prototypes */
#include "../include/globals.h"
#include "../include/i_vars.h"


/* GtkWidget *w_main; */


/* ---------------------------------------------------------------------- *
 * g_register_funcs -- this registers the Scheme functions required to use
 * gattrib.  They are mostly unnecessary, except for reading in the gattribrc
 * file at the beginning of the prog which gives the library search paths.
 * ---------------------------------------------------------------------- */
void g_register_funcs(void)
{
  /* general functions */
  gh_new_procedure0_0("quit", g_quit);
  gh_new_procedure0_0("exit", g_quit);

  /* gattrib functions */
  gh_new_procedure1_0("gattrib-version", g_rc_gattrib_version);
}

SCM g_quit(void)
{
#ifdef DEBUG
  printf("In g_quit, calling exit(0)\n");
#endif

  gattrib_quit(); 
  exit(0);
}

