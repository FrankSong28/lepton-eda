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
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <libgeda/libgeda.h>

#include "../include/gschem_struct.h"
#include "../include/globals.h"
#include "../include/prototype.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

#include "../include/x_preview.h"

#define OVER_ZOOM_FACTOR 0.1

extern int mouse_x, mouse_y;


enum {
  PROP_FILENAME=1,
  PROP_BUFFER,
  PROP_ACTIVE
};

static GObjectClass *preview_parent_class = NULL;


static void preview_class_init (PreviewClass *class);
static void preview_init       (Preview *preview);
static void preview_set_property (GObject *object,
                                  guint property_id,
                                  const GValue *value,
                                  GParamSpec *pspec);
static void preview_get_property (GObject *object,
                                  guint property_id,
                                  GValue *value,
                                  GParamSpec *pspec);
static void preview_dispose (GObject *self);


/*! \brief Completes initialitation of the widget after realization.
 *  \par Function Description
 *  This function terminates the initialization of preview's GSCHEM_TOPLEVEL
 *  and TOPLEVEL environments after the widget has been realized.
 *
 *  It creates a preview page in the TOPLEVEL environment.
 *
 *  \param [in] widget    The preview widget.
 *  \param [in] user_data Unused user data.
 */
static void
preview_callback_realize (GtkWidget *widget,
                          gpointer user_data)
{
  Preview *preview = PREVIEW (widget);
  GSCHEM_TOPLEVEL *preview_w_current = preview->preview_w_current;
  TOPLEVEL *preview_toplevel = preview_w_current->toplevel;
  PAGE *preview_page;

  preview_w_current->window = preview_w_current->drawing_area->window;
  gtk_widget_grab_focus (preview_w_current->drawing_area);

  preview_toplevel->width  = preview_w_current->drawing_area->allocation.width;
  preview_toplevel->height = preview_w_current->drawing_area->allocation.height;
  preview_w_current->win_width  = preview_toplevel->width;
  preview_w_current->win_height = preview_toplevel->height;

  preview_w_current->backingstore = gdk_pixmap_new (
    preview_w_current->window,
    preview_w_current->drawing_area->allocation.width,
    preview_w_current->drawing_area->allocation.height, -1);

  x_window_setup_gc (preview_w_current);

  preview_page = s_page_new (preview_toplevel, "unknown");
  s_page_goto (preview_toplevel, preview_page);

  x_repaint_background(preview_w_current);

  preview_toplevel->DONT_REDRAW = 0;

  a_zoom_extents(preview_w_current,
                 preview_page->object_head,
                 A_PAN_DONT_REDRAW);

  o_redraw_all(preview_w_current);

}

/*! \brief Redraws the view when widget is exposed.
 *  \par Function Description
 *  It redraws the preview pixmap every time the widget is exposed.
 *
 *  \param [in] widget    The preview widget.
 *  \param [in] event     The event structure.
 *  \param [in] user_data Unused user data.
 *  \returns FALSE to propagate the event further.
 */
static gboolean
preview_callback_expose (GtkWidget *widget,
                         GdkEventExpose *event,
                         gpointer user_data)
{
  Preview *preview = PREVIEW (widget);
  GSCHEM_TOPLEVEL *preview_w_current = preview->preview_w_current;

  gdk_draw_pixmap(widget->window,
                  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                  preview_w_current->backingstore,
                  event->area.x, event->area.y,
                  event->area.x, event->area.y,
                  event->area.width, event->area.height);

  return FALSE;
}

/*! \brief Handles the press on a mouse button.
 *  \par Function Description
 *  It handles the user inputs.
 *
 *  Three action are available: zoom in, pan and zoom out on preview display.
 *
 *  \param [in] widget    The preview widget.
 *  \param [in] event     The event structure.
 *  \param [in] user_data Unused user data.
 *  \returns FALSE to propagate the event further.
 */
static gboolean
preview_callback_button_press (GtkWidget *widget,
                               GdkEventButton *event,
                               gpointer user_data)
{
  Preview *preview = PREVIEW (widget);
  GSCHEM_TOPLEVEL *preview_w_current = preview->preview_w_current;

  if (!preview->active) {
    return TRUE;
  }

  switch (event->button) {
      case 1: /* left mouse button: zoom in */
        a_zoom (preview_w_current, ZOOM_IN, HOTKEY,
                A_PAN_DONT_REDRAW);
        o_redraw_all (preview_w_current);
        break;
      case 2: /* middle mouse button: pan */
        a_pan (preview_w_current, mouse_x, mouse_y);
        break;
      case 3: /* right mouse button: zoom out */
        a_zoom (preview_w_current, ZOOM_OUT, HOTKEY,
                A_PAN_DONT_REDRAW);
        o_redraw_all (preview_w_current);
        break;
  }
  
  return FALSE;
}

/*! \brief Handles the displacement of the pointer.
 *  \par Function Description
 *  This function temporary saves the position of the mouse pointer
 *  over the preview widget.
 *
 *  This position can be later used when the user press a button of
 *  the mouse (see <B>preview_callback_button_press()</B>).
 *
 *  \param [in] widget    The preview widget.
 *  \param [in] event     The event structure.
 *  \param [in] user_data Unused user data.
 *  \returns FALSE to propagate the event further.
 */
static gboolean
preview_callback_motion_notify (GtkWidget *widget,
                                GdkEventMotion *event,
                                gpointer user_data)
{
  Preview *preview = PREVIEW (widget);
  
  if (!preview->active) {
    return TRUE;
  }
  
  mouse_x = (int)event->x;
  mouse_y = (int)event->y;

  return FALSE;
}

/*! \brief Updates the preview widget.
 *  \par Function Description
 *  This function update the preview: if the preview is active and a
 *  filename has been given, it opens the file and display
 *  it. Otherwise it display a blank page.
 *
 *  \param [in] preview The preview widget.
 */
static void
preview_update (Preview *preview)
{
  GSCHEM_TOPLEVEL *preview_w_current = preview->preview_w_current;
  TOPLEVEL *preview_toplevel = preview_w_current->toplevel;
  int left, top, right, bottom;
  int width, height;

  if (preview_toplevel->page_current == NULL) {
    return;
  }
  
  /* delete old preview, create new page */
  /* it would be better to just resets current page - Fix me */
  s_page_delete (preview_toplevel, preview_toplevel->page_current);
  s_page_goto (preview_toplevel, s_page_new (preview_toplevel, "preview"));
  
  if (preview->active) {
    g_assert ((preview->filename == NULL) || (preview->buffer == NULL));
    if (preview->filename != NULL) {
      /* open up file in current page */
      f_open_flags (preview_toplevel, preview->filename,
                    F_OPEN_RC | F_OPEN_RESTORE_CWD, NULL);
      /* test value returned by f_open... - Fix me */
      /* we should display something if there an error occured - Fix me */
    }
    if (preview->buffer != NULL) {

      /* Load the data buffer */
      preview_toplevel->page_current->object_tail = (OBJECT *) 
        o_read_buffer (preview_toplevel, 
                       preview_toplevel->page_current->object_tail, 
                       preview->buffer, -1, "Preview Buffer");
      preview_toplevel->page_current->object_tail = (OBJECT *) 
        return_tail(preview_toplevel->page_current->object_head); 

      /* Is this needed? */
      if (preview_toplevel->net_consolidate == TRUE) {	
              o_net_consolidate(preview_toplevel);
      }
    }
  }

  if (world_get_object_list_bounds (preview_toplevel,
                                    preview_toplevel->page_current->object_head,
                                    &left, &top,
                                    &right, &bottom)) {
    /* Clamp the canvas size to the extents of the page being previewed */
    width = right - left;
    height = bottom - top;
    preview_toplevel->init_left   = left  - ((double)width * OVER_ZOOM_FACTOR);
    preview_toplevel->init_right  = right + ((double)width * OVER_ZOOM_FACTOR);
    preview_toplevel->init_top    = top    - ((double)height * OVER_ZOOM_FACTOR);
    preview_toplevel->init_bottom = bottom + ((double)height * OVER_ZOOM_FACTOR);
  }

  /* display current page (possibly empty) */
  a_zoom_extents (preview_w_current,
                  preview_toplevel->page_current->object_head,
                  A_PAN_DONT_REDRAW);
  o_redraw_all (preview_w_current);
  
}

GType
preview_get_type ()
{
  static GType preview_type = 0;
  
  if (!preview_type) {
    static const GTypeInfo preview_info = {
      sizeof(PreviewClass),
      NULL, /* base_init */
      NULL, /* base_finalize */
      (GClassInitFunc) preview_class_init,
      NULL, /* class_finalize */
      NULL, /* class_data */
      sizeof(Preview),
      0,    /* n_preallocs */
      (GInstanceInitFunc) preview_init,
    };
                
    preview_type = g_type_register_static (GTK_TYPE_DRAWING_AREA,
                                           "Preview",
                                           &preview_info, 0);
  }
  
  return preview_type;
}

static void
preview_class_init (PreviewClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  preview_parent_class = g_type_class_peek_parent (klass);
  
  gobject_class->set_property = preview_set_property;
  gobject_class->get_property = preview_get_property;
  gobject_class->dispose      = preview_dispose;

  g_object_class_install_property (
    gobject_class, PROP_FILENAME,
    g_param_spec_string ("filename",
                         "",
                         "",
                         NULL,
                         G_PARAM_READWRITE));
  g_object_class_install_property (
    gobject_class, PROP_BUFFER,
    g_param_spec_string ("buffer",
                         "",
                         "",
                         NULL,
                         G_PARAM_WRITABLE));
  g_object_class_install_property(
    gobject_class, PROP_ACTIVE,
    g_param_spec_boolean ("active",
                          "",
                          "",
                          FALSE,
                          G_PARAM_READWRITE));

        
}

static gboolean
preview_event_configure (GtkWidget         *widget,
                         GdkEventConfigure *event,
                         gpointer           user_data)
{
  gboolean retval;
  int save_redraw;
  GSCHEM_TOPLEVEL *preview_w_current = PREVIEW (widget)->preview_w_current;
  PAGE     *preview_page = preview_w_current->toplevel->page_current;

  save_redraw = preview_w_current->toplevel->DONT_REDRAW;
  preview_w_current->toplevel->DONT_REDRAW = 1;
  retval = x_event_configure (widget, event, preview_w_current);
  preview_w_current->toplevel->DONT_REDRAW = save_redraw;
  if (preview_page != NULL) {
    /* If we have an empty page without object, just redraw the background */
    if (preview_page->object_head == NULL
	|| preview_page->object_head->next == NULL) {
      x_repaint_background(preview_w_current);
    } else {
      a_zoom_extents(preview_w_current, preview_page->object_head, 0);
    }
  }
  return retval;
}


static gboolean
preview_event_scroll (GtkWidget *widget,
                      GdkEventScroll *event,
                      GSCHEM_TOPLEVEL *w_current)
{
  if (!PREVIEW (widget)->active) {
    return TRUE;
  }
  return x_event_scroll (widget, event, PREVIEW (widget)->preview_w_current);
}

static void
preview_init (Preview *preview)
{
  struct event_reg_t {
    gchar *detailed_signal;
    GCallback c_handler;
  } drawing_area_events[] = {
    { "realize",              G_CALLBACK (preview_callback_realize)       },
    { "expose_event",         G_CALLBACK (preview_callback_expose)        },
    { "button_press_event",   G_CALLBACK (preview_callback_button_press)  },
    { "motion_notify_event",  G_CALLBACK (preview_callback_motion_notify) },
    { "configure_event",      G_CALLBACK (preview_event_configure)        },
    { "scroll_event",         G_CALLBACK (preview_event_scroll)           },
    { NULL,                   NULL                                        }
  }, *tmp;
  GSCHEM_TOPLEVEL *preview_w_current;
  preview_w_current = gschem_toplevel_new ();
  preview_w_current->toplevel = s_toplevel_new ();

  i_vars_set (preview_w_current);

  /* be sure to turn off scrollbars */
  preview_w_current->scrollbars_flag = FALSE;

  /* be sure to turn off the grid */
  preview_w_current->grid = FALSE;

  /* preview_w_current windows don't have toolbars */
  preview_w_current->handleboxes = FALSE;
  preview_w_current->toolbars    = FALSE;

  preview_w_current->toplevel->width  = 160;
  preview_w_current->toplevel->height = 120;
  preview_w_current->win_width  = preview_w_current->toplevel->width;
  preview_w_current->win_height = preview_w_current->toplevel->height;

  preview_w_current->drawing_area = GTK_WIDGET (preview);
  preview->preview_w_current = preview_w_current;

  g_object_set (GTK_WIDGET (preview),
                "width-request",  preview_w_current->toplevel->width,
                "height-request", preview_w_current->toplevel->height,
                NULL);

  preview->active   = FALSE;
  preview->filename = NULL;
  preview->buffer   = NULL;
  
  gtk_widget_set_events (GTK_WIDGET (preview), 
                         GDK_EXPOSURE_MASK | 
                         GDK_POINTER_MOTION_MASK |
                         GDK_BUTTON_PRESS_MASK);
  for (tmp = drawing_area_events; tmp->detailed_signal != NULL; tmp++) {
    g_signal_connect (preview,
                      tmp->detailed_signal,
                      tmp->c_handler,
                      NULL);
  }
  
}

static void
preview_set_property (GObject *object,
                      guint property_id,
                      const GValue *value,
                      GParamSpec *pspec)
{
  Preview *preview = PREVIEW (object);
  GSCHEM_TOPLEVEL *preview_w_current = preview->preview_w_current;

  g_assert (preview_w_current != NULL);
  
  switch(property_id) {
      case PROP_FILENAME:
        g_free (preview->filename);
        g_free (preview->buffer);
        preview->filename = g_strdup (g_value_get_string (value));
        preview_update (preview);
        break;

      case PROP_BUFFER:
        g_free (preview->filename);
        g_free (preview->buffer);
        preview->buffer = g_strdup (g_value_get_string (value));
        preview_update (preview);
        break;

      case PROP_ACTIVE:
        preview->active = g_value_get_boolean (value);
        preview_update (preview);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }

}

static void
preview_get_property (GObject *object,
                      guint property_id,
                      GValue *value,
                      GParamSpec *pspec)
{
  Preview *preview = PREVIEW (object);
  GSCHEM_TOPLEVEL *preview_w_current = preview->preview_w_current;

  switch(property_id) {
      case PROP_FILENAME:
        g_assert (preview_w_current != NULL);
        /* return the filename of the current page in toplevel */
        g_value_set_string (value,
                            preview_w_current->toplevel->page_current->page_filename);
        break;
      case PROP_ACTIVE:
        g_value_set_boolean (value, preview->active);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }

}

static void
preview_dispose (GObject *self)
{
  Preview *preview = PREVIEW (self);
  GSCHEM_TOPLEVEL *preview_w_current = preview->preview_w_current;

  if (preview_w_current != NULL) {
    preview_w_current->drawing_area = NULL;

    o_attrib_free_current (preview_w_current->toplevel);
    o_complex_free_filename (preview_w_current->toplevel);

    if (preview_w_current->backingstore) {
      gdk_pixmap_unref (preview_w_current->backingstore);
    }

    x_window_free_gc (preview_w_current);
    
    s_toplevel_delete (preview_w_current->toplevel);
    g_free (preview_w_current);

    preview->preview_w_current = NULL;
  }
    
  G_OBJECT_CLASS (preview_parent_class)->dispose (self);
  
}



