/* gEDA - GPL Electronic Design Automation
 * libgeda - gEDA's library
 * Copyright (C) 1998-2010 Ales Hvezda
 * Copyright (C) 1998-2010 gEDA Contributors (see ChangeLog for details)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
/*! \file geda_box.h
 *
 *  \brief Low-level mathematical functions for boxes
 */

typedef struct _GedaBox GedaBox;
typedef struct _GedaBox BOX;

struct _GedaBox
{
  /* upper is considered the origin */
  int upper_x, upper_y; /* world */
  int lower_x, lower_y;
};

GedaBox*
geda_box_new ();

void
geda_box_free (GedaBox *box);

void
geda_box_calculate_bounds (const GedaBox *box, GedaBounds *bounds);

double
geda_box_shortest_distance (GedaBox *box, int x, int y, int solid);
