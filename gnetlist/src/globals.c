/* gEDA - GNU Electronic Design Automation
 * gnetlist - GNU Netlist 
 * Copyright (C) 1998 Ales V. Hvezda
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <stdio.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#include <guile/gh.h>

#include <libgeda/defines.h>
#include <libgeda/struct.h>
#include <libgeda/colors.h>
#include <libgeda/globals.h>
#include <libgeda/prototype.h>

#include "../include/globals.h"
#include "../include/prototype.h"


char rc_filename[256]; /* size is hack */  

/* color stuff */
GdkColormap *colormap; 
GdkVisual *visual; 

/* colors */
GdkColor white;
GdkColor black;
GdkColor red;
GdkColor green;
GdkColor blue;
GdkColor cyan;
GdkColor yellow;
GdkColor grey;
GdkColor grey90;
GdkColor darkgreen;
GdkColor darkred;
GdkColor darkyellow;
GdkColor darkcyan;
GdkColor darkblue;
GdkColor darkgrey; 


int logfile_fd=-1;
int do_logging=TRUE;
int logging_dest=LOG_WINDOW;

/* these are required by libgeda */
void (*arc_draw_func)() = o_arc_recalc;
void (*box_draw_func)() = o_box_recalc;
void (*circle_draw_func)() = o_circle_recalc;
void (*complex_draw_func)() = o_complex_recalc;
void (*line_draw_func)() = o_line_recalc;
void (*net_draw_func)() = o_net_recalc;
void (*ntext_draw_func)() = o_ntext_recalc;
void (*pin_draw_func)() = o_pin_recalc;
void (*select_func)() = NULL;
/* void (*pin_conn_recalc_func)() = o_pin_conn_recalc_only;
void (*net_conn_recalc_func)() = o_net_conn_recalc_only;old CONN stuff */
void (*x_log_update_func)() = NULL;


/* netlist specific variables */
NETLIST *netlist_head=NULL;
char *guile_proc=NULL; 


/* command line arguments */
int verbose_mode=FALSE;
int interactive_mode=FALSE;
int quiet_mode=FALSE;

/* what kind of netlist are we generating? see define.h for #defs */
int netlist_mode=gEDA;
char *output_filename=NULL;

