/* $Id$ */

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

/*------------------------------------------------------------------
 * This file holds fcns used to display dialog boxes.  
 *------------------------------------------------------------------*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*------------------------------------------------------------------
 * Includes required to run graphical widgets.
 *------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include <glib.h>
#include <glib-object.h>


#ifdef HAVE_STRING_H
#include <string.h>
#endif


#include "gtksheet_2_2.h"
#include "gtkitementry_2_2.h"


/*------------------------------------------------------------------
 * Gattrib specific includes
 *------------------------------------------------------------------*/
#include <libgeda/libgeda.h>       /* geda library fcns  */
#include "../include/struct.h"     /* typdef and struct declarations */
#include "../include/prototype.h"  /* function prototypes */
#include "../include/globals.h"

#ifdef HAVE_LIBDMALLOC
#include <dmalloc.h>
#endif

/* ========================================================= *
 * new attrib dialog widgets
 * ========================================================= */

/* --------------------------------------------------------- *
 * This asks for the name of the attrib column to insert
 * --------------------------------------------------------- */
void x_dialog_newattrib_get_name()
{
  GtkWidget *newattrib_window;
  GtkWidget *label;
  GtkWidget *buttoncancel = NULL;
  GtkWidget *buttonok = NULL;
  GtkWidget *vbox, *action_area, *attrib_entry;

#ifdef DEBUG
    printf("In x_dialog_newattrib_get_name, creating windows.\n");
#endif

  /* Create window and set its properties */
  newattrib_window = x_dialog_create_dialog_box(&vbox, &action_area);
  gtk_window_position(GTK_WINDOW(newattrib_window),
		      GTK_WIN_POS_MOUSE);
  gtk_widget_set_size_request (GTK_WIDGET(newattrib_window), 400, 150);  
  
  gtk_window_set_title(GTK_WINDOW(newattrib_window), "Enter new attribute name");
  gtk_container_border_width(GTK_CONTAINER(newattrib_window), 5);
  
  gtk_signal_connect(GTK_OBJECT(newattrib_window),
		     "destroy", GTK_SIGNAL_FUNC(x_dialog_newattrib_close_callback),
		     GTK_WIDGET(newattrib_window) );
  
  gtk_signal_connect(GTK_OBJECT(newattrib_window),
		     "key_press_event", GTK_SIGNAL_FUNC(x_dialog_newattrib_keypress_callback),
		     GTK_WIDGET(newattrib_window) );
  gtk_window_set_modal(GTK_WINDOW(newattrib_window), TRUE);

  /*  Create a text label for the dialog window */
  label = gtk_label_new ("Enter new attribute name");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX(vbox), label, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label, 127, 50);


  /*  Create the "attrib" text entry area */
  attrib_entry = gtk_entry_new_with_max_length(1024);
  gtk_editable_select_region(GTK_EDITABLE(attrib_entry), 0,
                             -1);
  gtk_box_pack_start(GTK_BOX(vbox), attrib_entry, TRUE,
                     TRUE, 5);
  gtk_widget_set_size_request (GTK_WIDGET (attrib_entry), 400, 20);

  gtk_object_set_data(GTK_OBJECT(newattrib_window), "attrib_entry",
                      attrib_entry);  /* here we make the string "attrib_entry" point
				       * to the attrib_entry widget.  We'll use this later. */
  gtk_widget_show(attrib_entry);


  /* Now create "OK" and "cancel" buttons */
  buttonok = gtk_button_new_from_stock (GTK_STOCK_OK);
  GTK_WIDGET_SET_FLAGS(buttonok, GTK_CAN_DEFAULT); /* what does this do? */
  gtk_box_pack_start(GTK_BOX(action_area), buttonok, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttonok), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_newattrib_ok_callback), 
		     GTK_WIDGET(newattrib_window) );
  gtk_widget_show(buttonok);


  buttoncancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(action_area), buttoncancel, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttoncancel), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_newattrib_close_callback), 
		     GTK_WIDGET(newattrib_window) );
  gtk_widget_show(buttoncancel);


#ifdef DEBUG
  printf("In x_dialog_newattrib_get_name, now show window.\n");
#endif

  if (!GTK_WIDGET_VISIBLE(newattrib_window)) {
    gtk_widget_show(newattrib_window);
  }
  
  return;
}

/* --------------------------------------------------------- *
 * keypress event
 * Note that I need to add an OK and Cancel button
 * --------------------------------------------------------- */
int x_dialog_newattrib_keypress_callback(GtkWidget * widget, GdkEventKey * event,
			    GtkWidget *window)
{
  char *key_name;

  key_name = gdk_keyval_name(event->keyval);
  if ( key_name == NULL ) return FALSE;

  if (strcmp(key_name, "Escape") == 0) {
#ifdef DEBUG
    printf("In x_dialog_newattrib_keypress, trying to close window.\n");
#endif
    x_dialog_close_window(window);
    return TRUE;
  }

  return FALSE;
}


/* --------------------------------------------------------- *
 * OK button pressed -- First get text in entry box.  Then
 * return it.
 * --------------------------------------------------------- */
void x_dialog_newattrib_ok_callback(GtkWidget *buttonok, 
				   GtkWidget *window)
{
  GtkEntry *entry;
  gchar *entry_text;

  /* Retreive pointer to attrib_entry widget, then use it to get entered text. */
  entry = gtk_object_get_data(GTK_OBJECT(window), "attrib_entry");
  entry_text = g_strdup( gtk_entry_get_text(GTK_ENTRY(entry)) );

  /* Perhaps do some other checks . . . . */
  if (entry_text != NULL) {
#ifdef DEBUG
    printf("In x_dialog_newattrib_ok_callback, about to add new attrib = %s\n", entry_text);
#endif
    s_toplevel_add_new_attrib(entry_text);
    g_free(entry_text);
  }

  x_dialog_close_window(window);
  return;
}


/* --------------------------------------------------------- *
 * close window
 * --------------------------------------------------------- */
void x_dialog_newattrib_close_callback(GtkWidget *buttonclose, GtkWidget *window)
{
  x_dialog_close_window(window);
}



/* ========================================================= *
 * delete attrib column dialog widgets
 * ========================================================= */

/* --------------------------------------------------------- *
 * This fcn throws up the "detele foo, are you sure" dialog
 * box.  It offers two buttons: "yes" and "cancel" 
 * --------------------------------------------------------- */
void x_dialog_delattrib_confirm()
{
  GtkWidget *delattrib_window;
  GtkWidget *label;
  GtkWidget *buttoncancel = NULL;
  GtkWidget *buttonyes = NULL;
  GtkWidget *vbox, *action_area;
  gint mincol, maxcol;
  GtkSheet *sheet;
  gint cur_page;


#ifdef DEBUG
  printf("In x_dialog_delattrib_confirm, creating windows.\n");
#endif

  /* First verify that exactly one column is selected.  */ 

  cur_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
  sheet = GTK_SHEET(sheets[cur_page]);
  if (sheet == NULL) {
    return;
  }

  mincol = x_gtksheet_get_min_col(sheet);
  maxcol =  x_gtksheet_get_max_col(sheet);
#ifdef DEBUG
  printf("In x_dialog_delattrib_confirm, column range selected = %d, %d\n",
	 mincol, maxcol);
#endif

  if ( (mincol != maxcol) || (mincol == -1) || (maxcol == -1) ) {
    /* Improper selection -- maybe throw up error box? */
    return;
  }


  /* Create window and set its properties */
  delattrib_window = x_dialog_create_dialog_box(&vbox, &action_area);
  gtk_window_position(GTK_WINDOW(delattrib_window),
		      GTK_WIN_POS_MOUSE);
  gtk_widget_set_size_request (GTK_WIDGET(delattrib_window), 400, 150);  

  gtk_window_set_title(GTK_WINDOW(delattrib_window), "Are you sure?");
  gtk_container_border_width(GTK_CONTAINER(delattrib_window), 5);
  
  gtk_signal_connect(GTK_OBJECT(delattrib_window),
		     "destroy", GTK_SIGNAL_FUNC(x_dialog_delattrib_close_callback),
		     GTK_WIDGET(delattrib_window) );
  
  gtk_signal_connect(GTK_OBJECT(delattrib_window),
		     "key_press_event", GTK_SIGNAL_FUNC(x_dialog_delattrib_keypress_callback),
		     GTK_WIDGET(delattrib_window) );
  gtk_window_set_modal(GTK_WINDOW(delattrib_window), TRUE);

  /*  Create a text label for the dialog window */
  label = gtk_label_new ("Are you sure you want to delete this attribute?");
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX(vbox), label, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label, 127, 50);


  /* Now create "Yes" and "cancel" buttons */
  buttonyes = gtk_button_new_from_stock (GTK_STOCK_YES);
  GTK_WIDGET_SET_FLAGS(buttonyes, GTK_CAN_DEFAULT); /* what does this do? */
  gtk_box_pack_start(GTK_BOX(action_area), buttonyes, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttonyes), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_delattrib_yes_callback), 
		     GTK_WIDGET(delattrib_window) );
  gtk_widget_show(buttonyes);


  buttoncancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(action_area), buttoncancel, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttoncancel), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_delattrib_close_callback), 
		     GTK_WIDGET(delattrib_window) );
  gtk_widget_show(buttoncancel);


#ifdef DEBUG
  printf("In x_dialog_delattrib_confirm, now show window.\n");
#endif

  if (!GTK_WIDGET_VISIBLE(delattrib_window)) {
    gtk_widget_show(delattrib_window);
  }
  
  return;
}

/* --------------------------------------------------------- *
 * keypress event.  Only handle escape.
 * --------------------------------------------------------- */
int x_dialog_delattrib_keypress_callback(GtkWidget * widget, 
					 GdkEventKey * event,
					 GtkWidget *window)
{
  char *key_name;

  key_name = gdk_keyval_name(event->keyval);
  if ( key_name == NULL ) return FALSE;

  if (strcmp(key_name, "Escape") == 0) {
#ifdef DEBUG
    printf("In x_dialog_delattrib_keypress, trying to close window.\n");
#endif
    x_dialog_close_window(window);
    return TRUE;
  }

  return FALSE;
}

/* --------------------------------------------------------- *
 * close window
 * --------------------------------------------------------- */
void x_dialog_delattrib_close_callback(GtkWidget *buttonclose, 
				      GtkWidget *window)
{
  x_dialog_close_window(window);
}


/* --------------------------------------------------------- *
 * Yes button pressed -- Go ahead & delete attrib column, the
 * close window and return.
 * --------------------------------------------------------- */
void x_dialog_delattrib_yes_callback(GtkWidget *buttonyes, 
				    GtkWidget *window)
{
  /* call the fcn to actually delete the attrib column.  */
  s_toplevel_delete_attrib_col();  /* this fcn figures out
				    * which col to delete. */
#ifdef DEBUG
    printf("In x_dialog_delattrib_yes_callback, done deleting attrib col.\n");
    printf("                          now close dialog window.\n");
#endif
  x_dialog_close_window(window);
#ifdef DEBUG
    printf("In x_dialog_delattrib_yes_callback, window closed, now returning.\n");
#endif

  return;
}


/* ========================================================= *
 * Missing symbol file dialog boxes.                         *
 * ========================================================= */

/* --------------------------------------------------------- *
 * This is the "missing symbol file found on object" dialog.
 * It offers the user the chance to close the project without
 * saving because he read a schematic with a missing symbol file.
 * --------------------------------------------------------- */
void x_dialog_missing_sym()
{
  GtkWidget *missing_sym_window;
  GtkWidget *label;
  GtkWidget *buttoncontinue = NULL;
  GtkWidget *buttonabort = NULL;
  GtkWidget *vbox, *action_area;
  char *string;

#ifdef DEBUG
  printf("In x_dialog_missing_sym, creating windows.\n");
#endif

  /* Create window and set its properties */
  missing_sym_window = x_dialog_create_dialog_box(&vbox, &action_area);
  gtk_window_position(GTK_WINDOW(missing_sym_window),
		      GTK_WIN_POS_MOUSE);

  /* I want dialog to maintain focus.  Neither of these seem to work. */
  gtk_window_set_modal(GTK_WINDOW(missing_sym_window), TRUE);

  gtk_window_set_transient_for(GTK_WINDOW(missing_sym_window),
                               GTK_WINDOW(window));   /* window is gloabl main window */

  /* This works to make dialog maintain focus */
  gtk_grab_add(GTK_WIDGET(missing_sym_window));


  gtk_window_set_title(GTK_WINDOW(missing_sym_window), "Missing symbol file found for component!");
  gtk_container_border_width(GTK_CONTAINER(missing_sym_window), 5);
  
  gtk_signal_connect(GTK_OBJECT(missing_sym_window),
		     "destroy", GTK_SIGNAL_FUNC(x_dialog_missing_sym_donothing_callback),
		     GTK_WIDGET(missing_sym_window) );
  
  gtk_signal_connect(GTK_OBJECT(missing_sym_window),
		     "key_press_event", GTK_SIGNAL_FUNC(x_dialog_missing_sym_donothing_callback),
		     GTK_WIDGET(missing_sym_window) );
  gtk_window_set_modal(GTK_WINDOW(missing_sym_window), TRUE);
  
  /*  Create a text label for the dialog window */
  string =
    g_strdup_printf("Warning!  One or more components have been found with missing symbol files!\n");
  string =
    g_strdup_printf("%s\n", string);
  string =
    g_strdup_printf("%sThis probably happened because gattrib couldn't find your component libraries,\n", string);
  string =
    g_strdup_printf("%sperhaps because your gafrc or gattribrc files are misconfigured.\n", string);
  string =
    g_strdup_printf("%sChose \"Quit\" to leave gattrib and fix the problem, or\n", string);
  string =
    g_strdup_printf("%s\"Forward\" to continue working with gattrib.\n", string);

  label = gtk_label_new(string);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX(vbox), label, FALSE, FALSE, 0);

  /* Now create "Abort program" and "Continue" buttons */
  buttonabort = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  GTK_WIDGET_SET_FLAGS(buttonabort, GTK_CAN_DEFAULT); /* what does this do? */
  gtk_box_pack_start(GTK_BOX(action_area), buttonabort, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttonabort), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_missing_sym_abort_callback), 
		     GTK_WIDGET(missing_sym_window) );
  gtk_widget_show(buttonabort);


  buttoncontinue = gtk_button_new_from_stock (GTK_STOCK_GO_FORWARD);
  gtk_box_pack_start(GTK_BOX(action_area), buttoncontinue, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttoncontinue), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_missing_sym_continue_callback), 
		     GTK_WIDGET(missing_sym_window) );
  gtk_widget_show(buttoncontinue);


#ifdef DEBUG
  printf("In x_dialog_missing_sym, now show window.\n");
#endif

  if (!GTK_WIDGET_VISIBLE(missing_sym_window)) {
    gtk_widget_show(missing_sym_window);
  }
  
  return;
}

/* --------------------------------------------------------- *
 * This callback traps certain events and does nothing.  The
 * idea is to prevent default event handlers from handing the
 * events.  
 * --------------------------------------------------------- */
int x_dialog_missing_sym_donothing_callback(GtkWidget * widget, 
					 GdkEventKey * event,
					 GtkWidget *window)
{
#ifdef DEBUG
    printf("In x_dialog_missing_sym_donothing, received an event, but doing nothing.\n");
#endif
  return TRUE;
}

/* --------------------------------------------------------- *
 * continue window
 * --------------------------------------------------------- */
void x_dialog_missing_sym_continue_callback(GtkWidget *buttonclose, 
				      GtkWidget *window)
{
#ifdef DEBUG
  printf("In x_dialog_missing_sym_continue_callback, continuing program\n");
#endif
  x_dialog_close_window(window);
  return;
}


/* --------------------------------------------------------- *
 * Abort button pressed -- close the program now.
 * --------------------------------------------------------- */
void x_dialog_missing_sym_abort_callback(GtkWidget *buttonyes, 
				    GtkWidget *window)
{
#ifdef DEBUG
  printf("In x_dialog_missing_sym_abort_callback, closing program\n");
#endif
  gattrib_quit(0);
}


/* ========================================================= *
 * Unsaved data windows
 * ========================================================= */

/* --------------------------------------------------------- *
 * This is the "Unsaved data -- are you sure you want to quit?"
 * dialog box which is thrown up before the user quits.
 * --------------------------------------------------------- */
void x_dialog_unsaved_data()
{
  GtkWidget *unsaved_data_window;
  GtkWidget *label;
  GtkWidget *buttoncontinue = NULL;
  GtkWidget *buttonabort = NULL;
  GtkWidget *vbox, *action_area;
  char *string;

#ifdef DEBUG
  printf("In x_dialog_unsaved_data, creating windows.\n");
#endif

  /* Create window and set its properties */
  unsaved_data_window = x_dialog_create_dialog_box(&vbox, &action_area);
  gtk_window_position(GTK_WINDOW(unsaved_data_window),
		      GTK_WIN_POS_MOUSE);

  /* I want dialog to maintain focus.  Neither of these seem to work. */
  gtk_window_set_modal(GTK_WINDOW(unsaved_data_window), TRUE);

  gtk_window_set_transient_for(GTK_WINDOW(unsaved_data_window),
                               GTK_WINDOW(window));   /* window is gloabl main window */


  /* This works to make dialog maintain focus */
  gtk_grab_add(GTK_WIDGET(unsaved_data_window));


  gtk_widget_set_size_request (GTK_WIDGET(unsaved_data_window), 400, 150); 

  gtk_window_set_title(GTK_WINDOW(unsaved_data_window), "Unsaved data.  Really quit?");
  gtk_container_border_width(GTK_CONTAINER(unsaved_data_window), 5);
  
  gtk_signal_connect(GTK_OBJECT(unsaved_data_window),
		     "destroy", GTK_SIGNAL_FUNC(x_dialog_unsaved_data_donothing_callback),
		     GTK_WIDGET(unsaved_data_window) );
  
  gtk_signal_connect(GTK_OBJECT(unsaved_data_window),
		     "key_press_event", GTK_SIGNAL_FUNC(x_dialog_unsaved_data_donothing_callback),
		     GTK_WIDGET(unsaved_data_window) );
  

  /*  Create a text label for the dialog window */
  string =
    g_strdup_printf("Warning!  You have unsaved data in the spreadsheet!\n");
  string =
    g_strdup_printf("%sAre you sure you want to quit?  Click \"Quit\" to\n", string);
  string =
    g_strdup_printf("%squit anyway, or \"Stop\" to go back and save you work.\n", string);

  label = gtk_label_new(string);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX(vbox), label, FALSE, FALSE, 0);

  /* Now create "Abort program" and "Continue" buttons */
  buttonabort = gtk_button_new_from_stock (GTK_STOCK_STOP);
  GTK_WIDGET_SET_FLAGS(buttonabort, GTK_CAN_DEFAULT); /* what does this do? */
  gtk_box_pack_start(GTK_BOX(action_area), buttonabort, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttonabort), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_unsaved_data_abort_callback), 
		     GTK_WIDGET(unsaved_data_window) );
  gtk_widget_show(buttonabort);


  buttoncontinue = gtk_button_new_from_stock (GTK_STOCK_QUIT);
  gtk_box_pack_start(GTK_BOX(action_area), buttoncontinue, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttoncontinue), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_unsaved_data_continue_callback), 
		     GTK_WIDGET(unsaved_data_window) );
  gtk_widget_show(buttoncontinue);


#ifdef DEBUG
  printf("In x_dialog_unsaved_data, now show window.\n");
#endif

  if (!GTK_WIDGET_VISIBLE(unsaved_data_window)) {
    gtk_widget_show(unsaved_data_window);
  }
  
  return;
}

/* --------------------------------------------------------- *
 * This callback traps certain events and does nothing.  The
 * idea is to prevent default event handlers from handing the
 * events.  
 * --------------------------------------------------------- */
int x_dialog_unsaved_data_donothing_callback(GtkWidget * widget, 
					 GdkEventKey * event,
					 GtkWidget *window)
{
#ifdef DEBUG
    printf("In x_dialog_unsaved_data_donothing, received an event, but doing nothing.\n");
#endif
  return TRUE;
}

/* --------------------------------------------------------- *
 * continue window
 * --------------------------------------------------------- */
void x_dialog_unsaved_data_continue_callback(GtkWidget *buttonclose, 
				      GtkWidget *window)
{
#ifdef DEBUG
  printf("In x_dialog_unsaved_data_continue_callback, continuing to quit\n");
#endif
  x_dialog_close_window(window);
  gattrib_quit(0);
  return;
}


/* --------------------------------------------------------- *
 * Abort button pressed -- go back to save.
 * --------------------------------------------------------- */
void x_dialog_unsaved_data_abort_callback(GtkWidget *buttonyes, 
					 GtkWidget *window)
{
#ifdef DEBUG
  printf("In x_dialog_unsaved_data_abort_callback, going back\n");
#endif
  
  gtk_grab_remove(GTK_WIDGET(window));
  x_dialog_close_window(window);
}



/* ========================================================= *
 * Unimplemented feature callback
 * ========================================================= */

/* --------------------------------------------------------- *
 * This fcn informs the user that he has chosen an 
 * unimplemented feature.  It presents only an "OK" button
 * to leave.
 * --------------------------------------------------------- */
void x_dialog_unimplemented_feature()
{
  GtkWidget *unimplemented_feature_window;
  GtkWidget *label = NULL;
  GtkWidget *buttonclose = NULL;
  GtkWidget *vbox, *action_area;
  char *string;


  unimplemented_feature_window = x_dialog_create_dialog_box(&vbox, &action_area);

  gtk_window_position(GTK_WINDOW(unimplemented_feature_window),
		      GTK_WIN_POS_MOUSE);
  
  /*  gtk_widget_set_size_request (GTK_WIDGET(unimplemented_feature_window), 400, 150);  */

  gtk_window_set_title(GTK_WINDOW(unimplemented_feature_window), 
		       "Unimplemented feature!");

  gtk_container_border_width(GTK_CONTAINER(unimplemented_feature_window), 5);
  
  gtk_signal_connect(GTK_OBJECT(unimplemented_feature_window),
		     "destroy", 
		     GTK_SIGNAL_FUNC(x_dialog_close_window),
		     GTK_WIDGET(unimplemented_feature_window) );
  
  gtk_signal_connect(GTK_OBJECT(unimplemented_feature_window),
		     "key_press_event",
		     GTK_SIGNAL_FUNC(x_dialog_about_keypress_callback), /* stealing "about" fcn */
		     GTK_WIDGET(unimplemented_feature_window) );

  /* Now create text string to place in vbox area */
  string = g_strdup_printf("Unimplemented Feature!\n\n");
  label = gtk_label_new(string);
  g_free(string);
  gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 5);
  gtk_widget_show(label);
  
  string = g_strdup_printf("Sorry -- you have chosen a feature which has net been\n");
  string = g_strdup_printf("%simplemented yet. Gattrib is an open-source program which\n",
			   string);
  string = g_strdup_printf("%sI work on as a hobby.  It is still a work in progress.\n",
			   string);
  string = g_strdup_printf("%sIf you wish to contribute (perhaps by implementing this\n",
			   string);
  string = g_strdup_printf("%sfeature), please do so!  Please send patches to gattrib\n",
			   string);
  string = g_strdup_printf("%sto Stuart Brorson: sdb@cloud9.net.\n",
			   string);
  string = g_strdup_printf("%sOtherwise, just hang tight -- I'll implement this feature soon!\n",
			   string);


  label = gtk_label_new(string);
  g_free(string);
  gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 5);
  gtk_widget_show(label);
  
  /* Now create button to stick in action area */
  buttonclose = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  GTK_WIDGET_SET_FLAGS(buttonclose, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(action_area), buttonclose, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttonclose), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_about_close_callback),  /* stealing "about" fcn */
		     GTK_WIDGET(unimplemented_feature_window) );

  gtk_widget_show(buttonclose);

  if (!GTK_WIDGET_VISIBLE(unimplemented_feature_window)) {
    gtk_widget_show(unimplemented_feature_window);
  }
}



/* ========================================================= *
 * Exit announcment callback
 * ========================================================= */

/* --------------------------------------------------------- *
 * This fcn accepts a string and displays it.  It presents
 * only an "OK" button to close the box and exit gattrib.
 * --------------------------------------------------------- */
void x_dialog_exit_announcement(gchar *string, gint return_code)
{
  GtkWidget *exit_announcement_window;
  GtkWidget *label = NULL;
  GtkWidget *buttonok = NULL;
  GtkWidget *vbox, *action_area;

  exit_announcement_window = x_dialog_create_dialog_box(&vbox, &action_area);

  gtk_window_position(GTK_WINDOW(exit_announcement_window),
		      GTK_WIN_POS_MOUSE);
  
  gtk_window_set_title(GTK_WINDOW(exit_announcement_window), 
		       "Attention!");

  gtk_container_border_width(GTK_CONTAINER(exit_announcement_window), 5);
  
  /* Stick calling string into label widget */
  label = gtk_label_new(string);
  gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 5);
  gtk_widget_show(label);
  
  /* Now create OK button to stick in action area */
  buttonok = gtk_button_new_from_stock (GTK_STOCK_OK);
  GTK_WIDGET_SET_FLAGS(buttonok, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(action_area), buttonok, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttonok), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_exit_announcement_close_callback),
		     GINT_TO_POINTER( return_code ) );
  gtk_widget_show(buttonok);

  /* set this to grab ability to override other windows */
  gtk_window_set_modal (GTK_WINDOW (exit_announcement_window), TRUE);

 /* show window */
  if (!GTK_WIDGET_VISIBLE(exit_announcement_window)) {
    gtk_widget_show(exit_announcement_window);
  }
}

/* --------------------------------------------------------- *
 * OK button pressed -- 
 * --------------------------------------------------------- */
void x_dialog_exit_announcement_close_callback(GtkWidget *buttonok, 
				    gpointer return_code)
{
  gattrib_quit( GPOINTER_TO_INT(return_code) );
}



/* ========================================================= *
 * help/about dialog widgets
 * ========================================================= */

/* --------------------------------------------------------- *
 * what do to upon keypress
 * --------------------------------------------------------- */
int x_dialog_about_keypress_callback(GtkWidget * widget, GdkEventKey * event,
			    GtkWidget *window)
{
  char *key_name;

  key_name = gdk_keyval_name(event->keyval);
  if ( key_name == NULL ) return FALSE;

  if (strcmp(key_name, "Escape") == 0) {
#ifdef DEBUG
    printf("In x_dialog_about_keypress_callback, trying to close window.\n");
#endif
    x_dialog_close_window(window);
    return TRUE;
  }

  return FALSE;
}

/* --------------------------------------------------------- */
void x_dialog_about_close_callback(GtkWidget * widget, GtkWidget *window)
{
  x_dialog_close_window(window);
}


/* --------------------------------------------------------- */
void x_dialog_about_dialog()
{
  GtkWidget *about_window;
  GtkWidget *label = NULL;
  GtkWidget *buttonclose = NULL;
  GtkWidget *vbox, *action_area;
  char *string;


  about_window = x_dialog_create_dialog_box(&vbox, &action_area);
  /*   about_window =  gtk_window_new(GTK_WINDOW_POPUP);  */

  gtk_window_position(GTK_WINDOW(about_window),
		      GTK_WIN_POS_MOUSE);
  
  gtk_window_set_title(GTK_WINDOW(about_window), "About...");
  gtk_container_border_width(GTK_CONTAINER(about_window), 5);
  
  gtk_signal_connect(GTK_OBJECT(about_window),
		     "destroy", GTK_SIGNAL_FUNC(x_dialog_about_close_callback),
		     GTK_WIDGET(about_window) );
  
  gtk_signal_connect(GTK_OBJECT(about_window),
		     "key_press_event", GTK_SIGNAL_FUNC(x_dialog_about_keypress_callback),
		     GTK_WIDGET(about_window) );

  /* Now create text string to place in vbox area */
  string = g_strdup_printf("gEDA : GPL Electronic Design Automation");
  label = gtk_label_new(string);
  g_free(string);
  gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 5);
  gtk_widget_show(label);
  
  string = g_strdup_printf("This is gattrib -- gEDA's attribute editor");
  label = gtk_label_new(string);
  g_free(string);
  gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 5);
  gtk_widget_show(label);
  
  string = g_strdup_printf("Gattrib version: %s%s", PREPEND_VERSION_STRING,
						    VERSION);
  label = gtk_label_new(string);
  g_free(string);
  gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 5);
  gtk_widget_show(label);
  
  string = 
    g_strdup_printf("Gattrib is written by: Stuart Brorson (sdb@cloud9.net)\n");
  string = 
    g_strdup_printf("%swith generous helpings of code from gschem, gnetlist, \n", 
		    string);
  string =
    g_strdup_printf("%sand gtkextra, as well as support from the gEDA community.", string);
  label = gtk_label_new(string);

  g_free(string);
  gtk_box_pack_start(GTK_BOX(vbox), label, TRUE, TRUE, 5);
  gtk_widget_show(label);
  
  /* Now create button to stick in action area */
  buttonclose = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
  GTK_WIDGET_SET_FLAGS(buttonclose, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(action_area), buttonclose, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttonclose), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_about_close_callback), 
		     GTK_WIDGET(about_window) );


  gtk_widget_show(buttonclose);

  if (!GTK_WIDGET_VISIBLE(about_window)) {
    gtk_widget_show(about_window);
  }
}



/* ========================================================= *
 * Get name of file for export of CSV
 * ========================================================= */
/* --------------------------------------------------------- *
 * This asks for the filename for the export file
 * --------------------------------------------------------- */
void x_dialog_export_file()
{
  GtkWidget *export_filename_window;
  GtkWidget *label;
  GtkWidget *buttoncancel = NULL;
  GtkWidget *buttonok = NULL;
  GtkWidget *vbox, *action_area;
  GtkWidget *filename_entry;
  gchar *string;

#ifdef DEBUG
    printf("In x_dialog_get_export_filename, creating windows.\n");
#endif

  /* Create window and set its properties */
  export_filename_window = x_dialog_create_dialog_box(&vbox, &action_area);
  gtk_window_position(GTK_WINDOW(export_filename_window),
		      GTK_WIN_POS_MOUSE);
  gtk_widget_set_size_request (GTK_WIDGET(export_filename_window), 400, 150);  
  
  gtk_window_set_title(GTK_WINDOW(export_filename_window), "Export filename?");
  gtk_container_border_width(GTK_CONTAINER(export_filename_window), 5);
  
  gtk_signal_connect(GTK_OBJECT(export_filename_window),
		     "destroy", GTK_SIGNAL_FUNC(x_dialog_export_file_close_callback),
		     GTK_WIDGET(export_filename_window) );
  
  gtk_signal_connect(GTK_OBJECT(export_filename_window),
		     "key_press_event", 
		     GTK_SIGNAL_FUNC(x_dialog_export_file_keypress_callback),
		     GTK_WIDGET(export_filename_window) );
  gtk_window_set_modal(GTK_WINDOW(export_filename_window), TRUE);

  /*  Create a text label for the dialog window */



  string =
    g_strdup_printf("Enter name for export file.\n");
  string =
    g_strdup_printf("%sDon't forget to include file suffix\n", string);
  string = 
    g_strdup_printf("%s(such as .csv).\n", string);

  label = gtk_label_new(string);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_CENTER);

  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX(vbox), label, FALSE, FALSE, 0);
  gtk_widget_set_size_request (label, 127, 50);


  /*  Create the "filenname" text entry area */
  filename_entry = gtk_entry_new_with_max_length(1024);
  gtk_editable_select_region(GTK_EDITABLE(filename_entry), 0,
                             -1);
  gtk_box_pack_start(GTK_BOX(vbox), filename_entry, TRUE,
                     TRUE, 5);
  gtk_widget_set_size_request (GTK_WIDGET (filename_entry), 400, 20);

  gtk_object_set_data(GTK_OBJECT(export_filename_window), "filename_entry",
                      filename_entry);  /* here we make the string "filename_entry" point
				         * to the export_filename_entry widget.  
				         * We'll use this later. */
  gtk_widget_show(filename_entry);


  /* Now create "OK" and "cancel" buttons */
  buttonok = gtk_button_new_from_stock (GTK_STOCK_OK);
  GTK_WIDGET_SET_FLAGS(buttonok, GTK_CAN_DEFAULT); /* what does this do? */
  gtk_box_pack_start(GTK_BOX(action_area), buttonok, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttonok), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_export_file_ok_callback), 
		     GTK_WIDGET(export_filename_window) );
  gtk_widget_show(buttonok);


  buttoncancel = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
  gtk_box_pack_start(GTK_BOX(action_area), buttoncancel, FALSE, FALSE, 0);
  gtk_signal_connect(GTK_OBJECT(buttoncancel), "clicked",
		     GTK_SIGNAL_FUNC(x_dialog_export_file_close_callback), 
		     GTK_WIDGET(export_filename_window) );
  gtk_widget_show(buttoncancel);

  if (!GTK_WIDGET_VISIBLE(export_filename_window)) {
    gtk_widget_show(export_filename_window);
  }
  
  return;
}

/* --------------------------------------------------------- *
 * keypress event
 * Note that I need to add an OK and Cancel button
 * --------------------------------------------------------- */
int x_dialog_export_file_keypress_callback(GtkWidget * widget, 
					       GdkEventKey * event,
					       GtkWidget *window)
{
  char *key_name;

  key_name = gdk_keyval_name(event->keyval);
  if ( key_name == NULL ) return FALSE;

  if (strcmp(key_name, "Escape") == 0) {
#ifdef DEBUG
    printf("In x_dialog_export_file_keypress, trying to close window.\n");
#endif
    x_dialog_close_window(window);
    return TRUE;
  }

  return FALSE;
}


/* --------------------------------------------------------- *
 * OK button pressed -- First get text in entry box.  Then
 * return it.
 * --------------------------------------------------------- */
void x_dialog_export_file_ok_callback(GtkWidget *buttonok, 
				   GtkWidget *window)
{
  GtkEntry *entry;
  gchar *entry_text;

  /* Retreive pointer to filename_entry widget, then use it to get entered text. */
  entry = gtk_object_get_data(GTK_OBJECT(window), "filename_entry");
  entry_text = g_strdup( gtk_entry_get_text(GTK_ENTRY(entry)) );

  /* Perhaps do some other checks . . . . */
  if (entry_text != NULL) {
#ifdef DEBUG
    printf("In x_dialog_export_file_ok_callback, got filename = %s\n", entry_text);
#endif
    /*  XXXXX  Here's where I call the export fcn  */ 
    f_export_components(entry_text);
    g_free(entry_text);
  }

  x_dialog_close_window(window);
  return;
}


/* --------------------------------------------------------- *
 * close window
 * --------------------------------------------------------- */
void x_dialog_export_file_close_callback(GtkWidget *buttonclose, GtkWidget *window)
{
  x_dialog_close_window(window);
}








/* ========================================================= *
 * Fcns common to all dialog boxes
 * This code also stolen from gschem & adapted for gattrib. 
 * ========================================================= */

/* ---------------------------------------------------- *
 * This creates a dialog box.  It has two areas: the vbox
 * area, and the action area.  The idea is that the vbox
 * area holds text, and the action area holds buttons or
 * other active widgets.  There is a separating line between
 * the two area.  You load one or the other areas like this (for example):
 * gtk_box_pack_start(GTK_BOX(action_area), buttonclose, TRUE, TRUE, 0);
 * --------------------------------------------------------- */
GtkWidget *x_dialog_create_dialog_box(GtkWidget ** out_vbox,
                               GtkWidget ** out_action_area)
{
  GtkWidget *separator;
  GtkWidget *vbox;
  GtkWidget *action_area;
  GtkWidget *dialog;

  if (!out_vbox)
    return (NULL);

  if (!out_action_area)
    return (NULL);

  dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(dialog), vbox);
  gtk_widget_show(vbox);

  action_area = gtk_hbox_new(TRUE, 5);
  gtk_container_set_border_width(GTK_CONTAINER(action_area), 10);
  gtk_box_pack_end(GTK_BOX(vbox), action_area, FALSE, TRUE, 0);
  gtk_widget_show(action_area);

  separator = gtk_hseparator_new();
  gtk_box_pack_end(GTK_BOX(vbox), separator, FALSE, TRUE, 0);
  gtk_widget_show(separator);

  *out_vbox = vbox;
  *out_action_area = action_area;

  return (dialog);
}

/* ---------------------------------------------------- */
void x_dialog_close_window(GtkWidget * window)
{
  gtk_widget_destroy(window);
}




