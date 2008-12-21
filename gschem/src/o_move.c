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

#include <stdio.h>

#include <libgeda/libgeda.h>

#include "../include/gschem_struct.h"
#include "../include/globals.h"
#include "../include/prototype.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_move_start(GSCHEM_TOPLEVEL *w_current, int x, int y)
{
  TOPLEVEL *toplevel = w_current->toplevel;
  if ( geda_list_get_glist( toplevel->page_current->selection_list ) != NULL) {

    /* Save the current state. When rotating the selection when moving,
       we have to come back to here */
    o_undo_savestate(w_current, UNDO_ALL);
    w_current->last_drawb_mode = -1;
    w_current->event_state = MOVE;

    w_current->last_x = w_current->start_x = fix_x(toplevel, x);
    w_current->last_y = w_current->start_y = fix_y(toplevel, y);

    o_erase_selected(w_current);

    o_drawbounding(w_current,
                   geda_list_get_glist( toplevel->page_current->selection_list ),
                   x_get_darkcolor(w_current->bb_color), TRUE);

    if (w_current->netconn_rubberband) {
      o_move_prep_rubberband(w_current);
      o_move_stretch_rubberband(w_current);
    }

    o_select_move_to_place_list(w_current);
    w_current->inside_action = 1;
  }
}

#define SINGLE     0
#define COMPLEX    1

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 *  \note
 *  type can be SINGLE or COMPLEX
 *  which basically controls if this is a single object or a complex
 */
void o_move_end_lowlevel(GSCHEM_TOPLEVEL *w_current, OBJECT * list, int type,
			 int diff_x, int diff_y,
			 GList** other_objects, GList** connected_objects)
{
  TOPLEVEL *toplevel = w_current->toplevel;
  OBJECT *o_current;
  OBJECT *object;

  o_current = list;
  while (o_current != NULL) {

    object = o_current;
    switch (object->type) {

      case (OBJ_NET):
      case (OBJ_BUS):
      case (OBJ_PIN):
        /* save the other objects and remove object's connections */
        *other_objects = s_conn_return_others(*other_objects, object);
        s_conn_remove(toplevel, object);

        /* do the actual translation */
        o_translate_world(toplevel, diff_x, diff_y, object);
        s_conn_update_object(toplevel, object);
        *connected_objects = s_conn_return_others(*connected_objects, object);
        break;

      default:
        o_translate_world(toplevel, diff_x, diff_y, object);
        break;
    }

    if (type == COMPLEX) {
      o_current = o_current->next;
    } else {
      o_current = NULL;
    }
  }
  
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_move_end(GSCHEM_TOPLEVEL *w_current)
{
  TOPLEVEL *toplevel = w_current->toplevel;
  GList *s_current = NULL;
  OBJECT *object;
  int diff_x, diff_y;
  int lx, ly;
  int sx, sy;
  int left, top, right, bottom;
  GList *other_objects = NULL;
  GList *connected_objects = NULL;
  GList *rubbernet_objects = NULL; 
  GList *rubbernet_other_objects = NULL;
  GList *rubbernet_connected_objects = NULL;

  object = o_select_return_first_object(w_current);

  if (!object) {
    /* actually this is an error condition hack */
    w_current->inside_action = 0;
    i_set_state(w_current, SELECT);
    return;
  }


  SCREENtoWORLD(toplevel, w_current->last_x, w_current->last_y,
                &lx, &ly);
  SCREENtoWORLD(toplevel, w_current->start_x, w_current->start_y,
                &sx, &sy);
  lx = snap_grid(toplevel, lx);
  ly = snap_grid(toplevel, ly);
  sx = snap_grid(toplevel, sx);
  sy = snap_grid(toplevel, sy);

  diff_x = lx - sx;
  diff_y = ly - sy;

  if (w_current->netconn_rubberband)
  {
    o_move_end_rubberband(w_current, diff_x, diff_y,
                          &rubbernet_objects, &rubbernet_other_objects,
                          &rubbernet_connected_objects);
  }

  o_drawbounding(w_current,
                 geda_list_get_glist( toplevel->page_current->selection_list ),
                 x_get_darkcolor(w_current->bb_color), FALSE);

  s_current = geda_list_get_glist( toplevel->page_current->selection_list );

  while (s_current != NULL) {

    object = (OBJECT *) s_current->data;

    if (object == NULL) {
      fprintf(stderr, _("ERROR: NULL object in o_move_end!\n"));
      exit(-1);
    }


    switch (object->type) {
      case (OBJ_NET):
      case (OBJ_PIN):
      case (OBJ_BUS):
      case (OBJ_LINE):
      case (OBJ_BOX):
      case (OBJ_PICTURE):
      case (OBJ_CIRCLE):
      case (OBJ_ARC):
      case (OBJ_TEXT):
        o_move_end_lowlevel(w_current, object, SINGLE, diff_x, diff_y,
                            &other_objects, &connected_objects);
        break;

      case (OBJ_COMPLEX):
      case (OBJ_PLACEHOLDER):

        if (scm_hook_empty_p(move_component_hook) == SCM_BOOL_F &&
            object != NULL) {
          scm_run_hook(move_component_hook,
                       scm_cons (g_make_attrib_smob_list
                                 (w_current, object), SCM_EOL));
        }

        /* this next section of code is from */
        /* o_complex_world_translate_world */
        object->complex->x = object->complex->x + diff_x;
        object->complex->y = object->complex->y + diff_y;

        o_move_end_lowlevel(w_current, object->complex->prim_objs,
                            COMPLEX, diff_x, diff_y,
                            &other_objects, &connected_objects);


        world_get_object_list_bounds(toplevel, object->complex->prim_objs,
			       &left, &top, &right, &bottom);

        object->w_left = left;
        object->w_top = top;
        object->w_right = right;
        object->w_bottom = bottom;

        break;
    }

    s_current = g_list_next(s_current);
  }

  /* Remove the undo saved in o_move_start */
  o_undo_remove_last_undo(w_current);

  /* Draw the objects that were moved (and connected/disconnected objects) */
  o_draw_selected(w_current);
  o_cue_undraw_list(w_current, other_objects);
  o_cue_draw_list(w_current, other_objects);
  o_cue_undraw_list(w_current, connected_objects);
  o_cue_draw_list(w_current, connected_objects);

  /* Draw the connected nets/buses that were also changed */
  o_draw_list(w_current, rubbernet_objects);
  o_cue_undraw_list(w_current, rubbernet_objects);
  o_cue_draw_list(w_current, rubbernet_objects);
  o_cue_undraw_list(w_current, rubbernet_other_objects);
  o_cue_draw_list(w_current, rubbernet_other_objects);
  o_cue_undraw_list(w_current, rubbernet_connected_objects);
  o_cue_draw_list(w_current, rubbernet_connected_objects);
 
  toplevel->page_current->CHANGED = 1;
  o_undo_savestate(w_current, UNDO_ALL);

  g_list_free(other_objects);
  g_list_free(connected_objects);
  g_list_free(rubbernet_objects);
  g_list_free(rubbernet_other_objects);
  g_list_free(rubbernet_connected_objects);

  g_list_free(toplevel->page_current->complex_place_list);
  toplevel->page_current->complex_place_list = NULL;
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
int o_move_return_whichone(OBJECT * object, int x, int y)
{
  if (object->line->x[0] == x && object->line->y[0] == y) {
    return (0);
  }

  if (object->line->x[1] == x && object->line->y[1] == y) {
    return (1);
  }

  fprintf(stderr,
          _("DOH! tried to find the whichone, but didn't find it!\n"));
  return (-1);
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_move_check_endpoint(GSCHEM_TOPLEVEL *w_current, OBJECT * object)
{
  TOPLEVEL *toplevel = w_current->toplevel;
  GList *cl_current;
  CONN *c_current;
  int whichone;

  if (!object)
  return;

  if (object->type != OBJ_NET && object->type != OBJ_PIN &&
      object->type != OBJ_BUS) {
    fprintf(stderr,
            _("Got a non line object in o_move_check_endpoint\n"));
    return;
  }

  cl_current = object->conn_list;
  while (cl_current != NULL) {

    c_current = (CONN *) cl_current->data;

    if (c_current->other_object != NULL) {

      /* really make sure that the object is not selected */
      if (c_current->other_object->saved_color == -1 &&
          c_current->other_object->selected == FALSE) {

        if (c_current->type == CONN_ENDPOINT ||
            (c_current->type == CONN_MIDPOINT &&
             c_current->other_whichone != -1)) {

          whichone =
            o_move_return_whichone(c_current->other_object,
                                   c_current->x, c_current->y);

#if DEBUG
          printf
            ("FOUND: %s type: %d, whichone: %d, x,y: %d %d\n",
             c_current->other_object->name, c_current->type,
             whichone, c_current->x, c_current->y);
#endif

#if DEBUG
          printf("other x,y: %d %d\n", c_current->x,
                 c_current->y);
          printf("type: %d return: %d real: [ %d %d ]\n",
                 c_current->type, whichone, c_current->whichone,
                 c_current->other_whichone);
#endif

          if (whichone >= 0 && whichone <= 1) {
            toplevel->page_current->stretch_tail =
              s_stretch_add(toplevel->page_current->
                            stretch_head,
                            c_current->other_object,
                            c_current, whichone);
            
            if (c_current->other_object->type == OBJ_NET || 
                c_current->other_object->type == OBJ_BUS) {
              o_erase_single(w_current, c_current->other_object);
            }
          }

        }
      }
    }
    cl_current = g_list_next(cl_current);
  }

}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_move_prep_rubberband(GSCHEM_TOPLEVEL *w_current)
{
  TOPLEVEL *toplevel = w_current->toplevel;
  GList *s_current;
  OBJECT *object;
  OBJECT *o_current;

  s_stretch_remove_most(toplevel,
                        toplevel->page_current->stretch_head);
  toplevel->page_current->stretch_tail =
  toplevel->page_current->stretch_head;

#if DEBUG
  printf("\n\n\n");
  s_stretch_print_all(toplevel->page_current->stretch_head);
  printf("\n\n\n");
#endif

  s_current = geda_list_get_glist( toplevel->page_current->selection_list );
  while (s_current != NULL) {
    object = (OBJECT *) s_current->data;
    if (object) {
      switch (object->type) {
        case (OBJ_NET):
        case (OBJ_PIN):
        case (OBJ_BUS):
          o_move_check_endpoint(w_current, object);
          break;

        case (OBJ_COMPLEX):
        case (OBJ_PLACEHOLDER):
          o_current = object->complex->prim_objs;
          while (o_current != NULL) {

            if (o_current->type == OBJ_PIN) {
              o_move_check_endpoint(w_current, o_current);
            }

            o_current = o_current->next;
          }

          break;

      }
    }
    s_current = g_list_next(s_current);
  }

#if DEBUG
  printf("\n\n\n\nfinished building scretch list:\n");
  s_stretch_print_all(toplevel->page_current->stretch_head);
#endif
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
int o_move_zero_length(OBJECT * object)
{
#if DEBUG
  printf("x: %d %d y: %d %d\n",
         object->line->x[0], object->line->x[1],
         object->line->y[0], object->line->y[1]);
#endif

  if (object->line->x[0] == object->line->x[1] &&
      object->line->y[0] == object->line->y[1]) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_move_end_rubberband(GSCHEM_TOPLEVEL *w_current, int world_diff_x,
			   int world_diff_y,
			   GList** objects,
			   GList** other_objects, GList** connected_objects)
{
  TOPLEVEL *toplevel = w_current->toplevel;
  STRETCH *s_current, *s_next;
  OBJECT *object;
  int x, y;
  int whichone;
  GList *iter;

  /* save a list of objects the stretched objects
     are connected to before we move them. */
  for (s_current = toplevel->page_current->stretch_head->next;
       s_current != NULL; s_current = s_current->next) {
    object = s_current->object;

    if (object != NULL &&
        (object->type == OBJ_NET ||
         object->type == OBJ_BUS)) {
      *other_objects = s_conn_return_others (*other_objects, object);
    }
  }

  /* skip over head */
  s_current = toplevel->page_current->stretch_head->next;

  while (s_current != NULL) {
    
    /* Store this now, since we may delete the current item */
    s_next = s_current->next;

    object = s_current->object;
    if (object) {
      whichone = s_current->whichone;

      switch (object->type) {
        
        case (OBJ_NET):
          
          /* remove the object's connections */
          s_conn_remove(toplevel, object);

          x = object->line->x[whichone];
          y = object->line->y[whichone];

#if DEBUG
          printf("OLD: %d, %d\n", x, y);
          printf("diff: %d, %d\n", world_diff_x, world_diff_y);
#endif

          x = x + world_diff_x;
          y = y + world_diff_y;

#if DEBUG
          printf("NEW: %d, %d\n", x, y);
#endif

          object->line->x[whichone] = x;
          object->line->y[whichone] = y;

          if (o_move_zero_length(object)) {
            o_delete_net(w_current, object);
            s_stretch_remove (toplevel->page_current->stretch_head, object);
            toplevel->page_current->stretch_tail =
              s_stretch_return_tail (toplevel->page_current->stretch_head);
            *other_objects = g_list_remove_all (*other_objects, object);
          } else {
            o_net_recalc(toplevel, object);
            s_tile_update_object(toplevel, object);
            s_conn_update_object(toplevel, object);
            *objects = g_list_append(*objects, object);
          }

          break;

        case (OBJ_PIN):

          /* not valid to do with pins */

          break;

        case (OBJ_BUS):

          /* remove the object's connections */
          s_conn_remove(toplevel, object);

          x = object->line->x[whichone];
          y = object->line->y[whichone];

#if DEBUG
          printf("OLD: %d, %d\n", x, y);
          printf("diff: %d, %d\n", world_diff_x, world_diff_y);
#endif
          x = x + world_diff_x;
          y = y + world_diff_y;

#if DEBUG
          printf("NEW: %d, %d\n", x, y);
#endif
          object->line->x[whichone] = x;
          object->line->y[whichone] = y;

          if (o_move_zero_length(object)) {
            o_delete_bus(w_current, object);
            s_stretch_remove (toplevel->page_current->stretch_head, object);
            toplevel->page_current->stretch_tail =
              s_stretch_return_tail (toplevel->page_current->stretch_head);
            *other_objects = g_list_remove_all (*other_objects, object);
          } else {
            o_bus_recalc(toplevel, object);
            s_tile_update_object(toplevel, object);
            s_conn_update_object(toplevel, object);
            *objects = g_list_append(*objects, object);
          }

          break;
      }
    }
    
    s_current = s_next;
  }

  /* save a list of objects the stretched objects
     are now connected to after we moved them. */
  for (iter = *objects; iter != NULL; iter = g_list_next (iter)) {
    object = iter->data;
    *connected_objects = s_conn_return_others (*connected_objects, object);
  }

#if DEBUG
  /*! \bug FIXME: moved_objects doesn't exist? */
  /*printf("%d\n", g_list_length(*moved_objects));*/
  printf("%d\n", g_list_length(*other_objects));
  printf("%d\n", g_list_length(*connected_objects));
#endif
}

/*! \todo Finish function documentation!!!
 *  \brief
 *  \par Function Description
 *
 */
void o_move_stretch_rubberband(GSCHEM_TOPLEVEL *w_current)
{
  TOPLEVEL *toplevel = w_current->toplevel;
  STRETCH *s_current;
  OBJECT *object;
  int diff_x, diff_y;
  int whichone;

  diff_x = w_current->last_x - w_current->start_x;
  diff_y = w_current->last_y - w_current->start_y;


  /* skip over head */
  s_current = toplevel->page_current->stretch_head->next;
  while (s_current != NULL) {

    object = s_current->object;
    if (object) {
      whichone = s_current->whichone;
      switch (object->type) {
        case (OBJ_NET):
          o_net_draw_xor_single(w_current,
                                diff_x, diff_y, whichone, object);
          break;

        case (OBJ_BUS):
          o_bus_draw_xor_single(w_current,
                                diff_x, diff_y, whichone, object);
          break;
      }
    }
    s_current = s_current->next;
  }
}
