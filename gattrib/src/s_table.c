/* gEDA - GPL Electronic Design Automation
 * gattrib -- gEDA component and net attribute manipulation using spreadsheet.
 * Copyright (C) 2003 -- 2004 Stuart D. Brorson.
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
 * This file holds fcns involved in manipulating the TABLE structure, 
 * which is subsidiary to SHEET_DATA.  TABLE is a 2 dimensional array 
 * of structs; each struct corresponds to the data about an element
 * in a single cell of the spreadsheet.
 *------------------------------------------------------------------*/

#include <config.h>

#include <stdio.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#include <math.h>

/*------------------------------------------------------------------
 * Gattrib specific includes
 *------------------------------------------------------------------*/
#include <libgeda/libgeda.h>       /* geda library fcns  */
#include "../include/struct.h"     /* typdef and struct declarations */
#include "../include/prototype.h"  /* function prototypes */
#include "../include/globals.h"

/* ===================  Public Functions  ====================== */

/*------------------------------------------------------------------
 * This fcn is the table creator.  It returns a pointer to 
 * an initialized TABLE struct.  As calling args, it needs
 * the number of rows and cols to allocate.  The table is a
 * dynamically allocated 2D array of structs.  To access data in
 * a cell in the table, you reference (for example):
 * ((sheet_data->comp_table)[i][j]).attrib_value
 * (Parens used only for clarity.  It works without parens.)
 *------------------------------------------------------------------*/
TABLE **s_table_new(int rows, int cols)
{
  TABLE **new_table;
  int i, j;

  /* Here I am trying to create a 2 dimensional array of structs */
  new_table = (TABLE **) malloc(rows*sizeof(TABLE *));
  for (i = 0; i < rows; i++) {
    new_table[i] = (TABLE *) malloc(cols * sizeof(TABLE));
    /* Note that I should put some checks in here to verify that 
     * malloc worked correctly. */
  }

  /* Now pre-load the table with NULLs */
  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++) {
      (new_table[i][j]).attrib_value = NULL;
      (new_table[i][j]).row_name = NULL;
      (new_table[i][j]).col_name = NULL;
      (new_table[i][j]).row = i;
      (new_table[i][j]).col = j;
      (new_table[i][j]).visibility = 3; /* both name & value visible */
    }
  }

  return (new_table);

}


/*------------------------------------------------------------------
 * This fcn destroys the old table.  Use it after reading in a new
 * page to get rid of the old table before building a new one.
 *------------------------------------------------------------------*/
void s_table_destroy(TABLE **table, int row_count, int col_count)
{
  int i, j;
  for (i = 0; i < row_count; i++) {
    for (j = 0; j < row_count; j++) {
      free( (table[i][j]).attrib_value );
    }

  }
  return;
}



/*------------------------------------------------------------------
 * This fcn returns the index number when given a STRING_LIST and a 
 * string to match.  It finds the index
 * number by iterating through the master  list.
 *------------------------------------------------------------------*/
int s_table_get_index(STRING_LIST *local_list, char *local_string) {
  int count = 0;
  STRING_LIST *list_element;

#ifdef DEBUG
  printf("In s_table_get_index, examining %s to see if it is in the list.\n", local_string);
#endif


  list_element = local_list;
  while (list_element != NULL) {
    if (strcmp(list_element->data, local_string) == 0) {
      return count;
    }
    count++;
    list_element = list_element->next;
  }
  /* If we are here, it is by mistake */
  fprintf(stderr, "s_table_get_index passed a string not in master list!.\n");  
  exit(-1);
}


/*------------------------------------------------------------------
 * This fcn iterates over adds all objects found on this page looking
 * for components.  When it finds a component, it finds all component
 * attribs and sticks them in the TABLE.
 *------------------------------------------------------------------*/
void s_table_add_toplevel_comp_items_to_comp_table(OBJECT *start_obj) {
  OBJECT *o_current;
  gchar *temp;
  gchar *temp_uref;
  int row, col;
  gchar *attrib_text;
  gchar *attrib_name;
  gchar *attrib_value;
  ATTRIB *a_current;

  if (verbose_mode) {
    printf("- Starting internal component TABLE creation\n");
  }

#ifdef DEBUG
  fflush(stderr);
  fflush(stdout);
  printf("=========== Just entered  s_table_add_toplevel_comp_items_to_comp_table!  ==============\n");
#endif

  /* -----  Iterate through all objects found on page  ----- */
  o_current = start_obj;
  while (o_current != NULL) {

#ifdef DEBUG
      printf("   ---> In s_table_add_toplevel_comp_items_to_comp_table, examining o_current->name = %s\n", o_current->name);
#endif

    /* -----  Now process objects found on page  ----- */
    if ( (o_current->type == OBJ_COMPLEX) &&
          o_current->attribs &&
         !o_attrib_search_component(o_current, "graphical") ) {
                                                              
      /* ---- Don't process part if it lacks a refdes ----- */
      temp_uref = o_attrib_search_name_single(o_current, "refdes", NULL);
      if (temp_uref) {

#if DEBUG
        printf("      In s_table_add_toplevel_comp_items_to_comp_table, found component on page. Refdes = %s\n", temp_uref);
#endif
        verbose_print(" C");

        /* Having found a component, we loop over all attribs in this
         * component, and stick them
         * into cells in the table. */
        a_current = o_current->attribs;
        while (a_current != NULL) {
          if (a_current->object->type == OBJ_TEXT
              && a_current->object->text != NULL) {  /* found an attribute */
            /* may need to check more thoroughly here. . . . */
            attrib_text = u_basic_strdup(a_current->object->text->string);
            attrib_name = u_basic_breakup_string(attrib_text, '=', 0);
            attrib_value = u_basic_breakup_string(attrib_text, '=', 1);
            if (strcmp(attrib_name, "refdes") != 0) {
              /* Don't include "refdes" */
               
              /* Get row and col where to put this attrib */
              row = s_table_get_index(sheet_head->master_comp_list_head, temp_uref);
              col = s_table_get_index(sheet_head->master_comp_attrib_list_head, attrib_name);
#if DEBUG
              printf("       In s_table_add_toplevel_comp_items_to_comp_table, about to add row %d, col %d, attrib_value = %s\n",
                     row, col, attrib_value);
              printf(" . . . current address of attrib_value cell is [%p]\n", &((sheet_head->component_table)[row][col]).attrib_value);
#endif
              /* Is there a compelling reason for me to put this into a separate fcn? */
              ((sheet_head->component_table)[row][col]).row = row;
              ((sheet_head->component_table)[row][col]).col = col;
              ((sheet_head->component_table)[row][col]).row_name = u_basic_strdup(temp_uref);
              ((sheet_head->component_table)[row][col]).col_name = u_basic_strdup(attrib_name);
              ((sheet_head->component_table)[row][col]).attrib_value = u_basic_strdup(attrib_value);
            }
            free(attrib_name);
            free(attrib_text);
            free(attrib_value);
          }
          a_current = a_current->next;
           
        }  /* while (a_current != NULL) */
        free(temp_uref);
      }  /* if (temp_uref) */
    }    /* if (o_current->type == OBJ_COMPLEX)  */
 
    o_current = o_current->next;  /* iterate to next object on page */
 
  }  /* while o_current != NULL */
 
  verbose_done();
 
}
 
 
/*------------------------------------------------------------------
 * This fcn iterates over adds all items found on this page looking
 * for nets and adds them individually to the net table.  Looping over
 * objects occurs here.
 *------------------------------------------------------------------*/
void s_table_add_toplevel_net_items_to_net_table(OBJECT *start_obj) {
  OBJECT *o_current;
  char *temp_netname;
  int row, col;
  char *attrib_text;
  char *attrib_name;
  char *attrib_value;
  ATTRIB *a_current;
 
  /* -----  Iterate through all objects found on page  ----- */
  o_current = start_obj;
  while (o_current != NULL) {
 
    /* -----  Now process objects found on page  ----- */
    if (o_current->type == OBJ_NET) {
#if DEBUG
      fflush(stderr);
      fflush(stdout);
      printf("In s_table_add_toplevel_net_items_to_net_table, Found net on page\n");
#endif
      verbose_print(" N");
 
      /* Having found a net, we stick it into the table. */
      a_current = o_current->attribs;
      while (a_current != NULL) {
        if (a_current->object->type == OBJ_TEXT
            && a_current->object->text != NULL) {  /* found an attribute */
          /* may need to check more thoroughly here. . . . */
          attrib_text = u_basic_strdup(a_current->object->text->string);
          attrib_name = u_basic_breakup_string(attrib_text, '=', 0);
          attrib_value = u_basic_breakup_string(attrib_text, '=', 1);
          if (strcmp(attrib_name, "netname") != 0) {
            /* Don't include "netname" */
             
            /* Get row and col where to put this attrib */
            row = s_table_get_index(sheet_head->master_net_list_head, temp_netname);
            col = s_table_get_index(sheet_head->master_net_attrib_list_head, attrib_name);
#if DEBUG
            fflush(stderr);
            fflush(stdout);
            printf("In s_table_add_toplevel_net_items_to_net_table, about to add row %d, col %d, attrib_value = %s\n",
                   row, col, attrib_value);
            printf(" . . . current address of attrib_value cell is [%p]\n", &((sheet_head->net_table)[row][col]).attrib_value);
#endif
            /* Is there a compelling reason for me to put this into a separate fcn? */
            ((sheet_head->net_table)[row][col]).row = row;
            ((sheet_head->net_table)[row][col]).col = col;
            ((sheet_head->net_table)[row][col]).row_name = u_basic_strdup(temp_netname);
            ((sheet_head->net_table)[row][col]).col_name = u_basic_strdup(attrib_name);
            ((sheet_head->net_table)[row][col]).attrib_value = u_basic_strdup(attrib_value);
          }
          free(attrib_name);
          free(attrib_text);
          free(attrib_value);
        }
        a_current = a_current->next;
 
      }  /* while (a_current != NULL) */
      free(temp_netname);
 
    }    /*--- if (o_current->type == OBJ_NET)   ---*/
       
 
    o_current = o_current->next;  /* iterate to next object on page */
  }  /* while o_current != NULL */
 
  verbose_done();
 
#if DEBUG
  fflush(stderr);
  fflush(stdout);
  printf("In s_table_add_toplevel_net_items_to_net_table -- we are about to return\n");
#endif
 
}
 


/*------------------------------------------------------------------
 * This fcn iterates over adds all items found on this page
 * looking for pins.  WHen it finds a pin, it gathers all
 * pin attribs and sticks them into the pin table. 
 *------------------------------------------------------------------*/
void s_table_add_toplevel_pin_items_to_pin_table(OBJECT *start_obj) {
  OBJECT *o_current;
  OBJECT *o_lower_current;;
  gchar *temp;
  gchar *temp_uref;
  gchar *pinnumber;
  gchar *row_label;
  int row, col;
  gchar *attrib_text;
  gchar *attrib_name;
  gchar *attrib_value;
  ATTRIB *pin_attrib;

  if (verbose_mode) {
    printf("- Starting internal pin TABLE creation\n");
  }

#ifdef DEBUG
  printf("=========== Just entered  s_table_add_toplevel_pin_items_to_pin_table!  ==============\n");
#endif

  /* -----  Iterate through all objects found on page  ----- */
  o_current = start_obj;
  while (o_current != NULL) {

#ifdef DEBUG
      printf("   ---> In s_table_add_toplevel_pin_items_to_pin_table, examining o_current->name = %s\n", o_current->name);
#endif

    /* -----  Now process objects found on page  ----- */
    if ( (o_current->type == OBJ_COMPLEX) &&
          o_current->attribs &&
         !o_attrib_search_component(o_current, "graphical") ) {
                                                              
      /* ---- Don't process part if it lacks a refdes ----- */
      temp_uref = o_attrib_search_name_single(o_current, "refdes", NULL);
      if (temp_uref) {

	/* -----  Now iterate through lower level objects looking for pins.  ----- */
	o_lower_current = o_current->complex->prim_objs;
	while (o_lower_current != NULL) {

	  if (o_lower_current->type == OBJ_PIN) {
	    /* -----  Found a pin.  First get its pinnumber.  then get attrib head and loop on attribs.  ----- */
	    pinnumber = o_attrib_search_name_single(o_lower_current, "pinnumber", NULL);
	    row_label = u_basic_strdup_multiple(temp_uref, ":", pinnumber, NULL);

#if DEBUG
        printf("      In s_table_add_toplevel_pin_items_to_pin_table, examining pin %s\n", row_label);
#endif

	    pin_attrib = o_lower_current->attribs;
	    while (pin_attrib != NULL) {
	      if (pin_attrib->object->type == OBJ_TEXT
		  && pin_attrib->object->text != NULL) {  /* found an attribute */
		attrib_text = u_basic_strdup(pin_attrib->object->text->string);
		attrib_name = u_basic_breakup_string(attrib_text, '=', 0);
		attrib_value = u_basic_breakup_string(attrib_text, '=', 1);
 
		if (strcmp(attrib_name, "pinnumber") != 0) {
		  /* Don't include "pinnumber" because it is already in other master list */

		  /* Get row and col where to put this attrib */
		  row = s_table_get_index(sheet_head->master_pin_list_head, row_label);
		  col = s_table_get_index(sheet_head->master_pin_attrib_list_head, attrib_name);
#if DEBUG
		  printf("       In s_table_add_toplevel_pin_items_to_pin_table, about to add row %d, col %d, attrib_value = %s\n",
			 row, col, attrib_value);
		  printf(" . . . current address of attrib_value cell is [%p]\n", &((sheet_head->component_table)[row][col]).attrib_value);
#endif
		  /* Is there a compelling reason for me to put this into a separate fcn? */
		  ((sheet_head->pin_table)[row][col]).row = row;
		  ((sheet_head->pin_table)[row][col]).col = col;
		  ((sheet_head->pin_table)[row][col]).row_name = u_basic_strdup(row_label);
		  ((sheet_head->pin_table)[row][col]).col_name = u_basic_strdup(attrib_name);
		  ((sheet_head->pin_table)[row][col]).attrib_value = u_basic_strdup(attrib_value);
		}
		free(attrib_name);
		free(attrib_text);
		free(attrib_value);
	      }
	      pin_attrib = pin_attrib->next;
           
	    }  /* while (pin_attrib != NULL) */
	    free(pinnumber);
	    free(row_label);
	  }

	  o_lower_current = o_lower_current->next;
	}    /*  while (o_lower_current != NULL)  */
      }

      free(temp_uref);
    }

    o_current = o_current->next;  /* iterate to next object on page */
	
  }  /* while o_current != NULL */
 
  verbose_done();
}


/*------------------------------------------------------------------
 * This fcn through the spreadsheet, extracts the attribs from
 * the cells, and places them back into TABLE.  This is the
 * first step in saving out a project.
 *------------------------------------------------------------------*/
int s_table_gtksheet_to_all_tables() {

  int num_rows;
  int num_cols;
  STRING_LIST *master_row_list;
  STRING_LIST *master_col_list;
  TABLE **local_table;
  GtkSheet *local_gtk_sheet;     

  /* First handle component sheet */
  num_rows = sheet_head->comp_count;
  num_cols = sheet_head->comp_attrib_count;
  local_gtk_sheet = sheets[0];
  master_row_list = sheet_head->master_comp_list_head;
  master_col_list = sheet_head->master_comp_attrib_list_head;
  local_table = sheet_head->component_table;

  s_table_gtksheet_to_table(local_gtk_sheet, master_row_list, 
		       master_col_list, local_table,
		       num_rows, num_cols);

#if 0
  /* Next handle net sheet */
  num_rows = sheet_head->net_count;
  num_cols = sheet_head->net_attrib_count;
  local_gtk_sheet = sheets[1];
  master_row_list = sheet_head->master_net_list_head;
  master_col_list = sheet_head->master_net_attrib_list_head;
  local_table = sheet_head->net_table;

  s_table_gtksheet_to_table(local_gtk_sheet, master_row_list, 
		       master_col_list, local_table,
		       num_rows, num_cols);
#endif

  /* Finally, handle component pin sheet */
  num_rows = sheet_head->pin_count;
  num_cols = sheet_head->pin_attrib_count;
  local_gtk_sheet = sheets[2];
  master_row_list = sheet_head->master_pin_list_head;
  master_col_list = sheet_head->master_pin_attrib_list_head;
  local_table = sheet_head->pin_table;

  s_table_gtksheet_to_table(local_gtk_sheet, master_row_list, 
		       master_col_list, local_table,
		       num_rows, num_cols);

  return;
}


/* ===================  Private Functions  ====================== */
/*------------------------------------------------------------------
 * This fcn does the actual heaving lifting of looping 
 * through the spreadsheet, extracting the attribs from
 * the cells, and placing them back into TABLE.  This is the
 * first step in saving out a project.
 *------------------------------------------------------------------*/
int s_table_gtksheet_to_table(GtkSheet *local_gtk_sheet, STRING_LIST *master_row_list, 
			 STRING_LIST *master_col_list, TABLE **local_table,
			 int num_rows, int num_cols) 
{
  int row, col;

  STRING_LIST *row_list_item;
  gchar *row_title;

  STRING_LIST *col_list_item;
  gchar *col_title;
  
  gchar *attrib_value;

  row_list_item = master_row_list;
  for (row = 0; row < num_rows; row++) {
    row_title = (gchar *) u_basic_strdup(row_list_item->data);

    col_list_item = master_col_list;
    for (col = 0; col < num_cols; col++) {
      col_title = (gchar *) u_basic_strdup(col_list_item->data);

      /* get value of attrib in cell  */
      attrib_value = (gchar *) gtk_sheet_cell_get_text(GTK_SHEET(local_gtk_sheet), row, col);

#if 0
      if (strlen(attrib_value) == 0) {
	/* free(attrib_value);  */   /* sometimes we have spurious, zero length strings creep */
	attrib_value = NULL;    /* into the GtkSheet                                     */
      }
#endif


#ifdef DEBUG
      printf("In s_table_update_table, found attrib_value = %s in cell row=%d, col=%d\n", 
	     attrib_value, row, col);
#endif

      /* first handle attrib value in cell */
      if ( local_table[row][col].attrib_value != NULL) {
	free( local_table[row][col].attrib_value );
      }
      if (attrib_value != NULL) {
	local_table[row][col].attrib_value = (gchar *) u_basic_strdup(attrib_value);
      } else {
	local_table[row][col].attrib_value = NULL;
      }

      /* next handle name of row (also held in TABLE cell) */
      if ( local_table[row][col].row_name != NULL) {
	free( local_table[row][col].row_name );
      }
      if (row_title != NULL) {
	local_table[row][col].row_name = (gchar *) u_basic_strdup(row_title);
      } else {
	local_table[row][col].row_name = NULL;
      }

      /* finally handle name of col */
      if ( local_table[row][col].col_name != NULL) {
	free( local_table[row][col].col_name );
      }
      if (col_title != NULL) {
	local_table[row][col].col_name = (gchar *) u_basic_strdup(col_title);
      } else {
	local_table[row][col].col_name = NULL;
      }

      /* get next col list item and then iterate. */
      col_list_item = col_list_item->next;
    }

    row_list_item = row_list_item->next;
  }

  return;
}

