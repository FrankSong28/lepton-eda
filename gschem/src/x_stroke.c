/* gEDA - GPL Electronic Design Automation
 * gschem - gEDA Schematic Capture
 * Copyright (C) 1998-2007 Ales Hvezda
 * Copyright (C) 1998-2007 gEDA Contributors (see ChangeLog for details)
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

#include <math.h>

#include <libgeda/libgeda.h>

#include "../include/gschem_struct.h"
#include "../include/x_states.h"
#include "../include/prototype.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

typedef struct st_stroke_point STROKE_POINT;

struct st_stroke_point {
        int x, y;
        STROKE_POINT *next;
};

static STROKE_POINT *stroke_points = NULL;

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_stroke_add_point(GSCHEM_TOPLEVEL *w_current, int x, int y)
{
  STROKE_POINT *new_point;

  new_point = (STROKE_POINT *) g_malloc(sizeof(STROKE_POINT));

  new_point->x = x;
  new_point->y = y;

  if (stroke_points == NULL) {
    stroke_points = new_point;
    stroke_points->next = NULL;
  } else {
    new_point->next = stroke_points;
    stroke_points = new_point;
  }

  gdk_gc_set_foreground(w_current->gc,
                        x_get_color(w_current->stroke_color));

  gdk_draw_point(w_current->backingstore, w_current->gc, x, y);
  o_invalidate_rect (w_current, x, y, x, y);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 *  \note
 *  traverse list as well as free each point as you go along
 */
void x_stroke_erase_all(GSCHEM_TOPLEVEL *w_current)
{
  STROKE_POINT *temp;

  while(stroke_points != NULL) {

#if DEBUG
    printf("%d %d\n", stroke_points->x, stroke_points->y);
#endif

    /* was xor, wasn't working out... see above note */
    gdk_gc_set_foreground(
                          w_current->gc,
                          x_get_color(w_current->toplevel->background_color));

    gdk_draw_point(w_current->backingstore, w_current->gc,
                   stroke_points->x, stroke_points->y);
    o_invalidate_rect (w_current, stroke_points->x, stroke_points->y,
                                  stroke_points->x, stroke_points->y);

    temp = stroke_points;
    stroke_points = stroke_points->next;
    g_free(temp);
  }

  stroke_points = NULL;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void x_stroke_free_all(void)
{
  STROKE_POINT *temp;

  while(stroke_points != NULL) {
#if DEBUG
    printf("%d %d\n", stroke_points->x, stroke_points->y);
#endif

    temp = stroke_points;
    stroke_points = stroke_points->next;
    g_free(temp);
  }

  stroke_points = NULL;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 *  \note
 *  this is the function that does the actual work of the strokes
 *  by executing the right guile function which is associated with the stroke
 */
int x_stroke_search_execute(char *sequence)
{
  gchar *guile_string; 
  SCM eval;

  guile_string = g_strdup_printf("(eval-stroke \"%s\")", sequence);

  eval = g_scm_c_eval_string_protected (guile_string);
  g_free(guile_string);

  return (SCM_FALSEP (eval)) ? 0 : 1;
}
