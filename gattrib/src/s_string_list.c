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
 * This file holds fcns involved in manipulating the STRING_LIST
 * structure.  STRING_LIST is basically a linked list of strings
 * (text).
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



/*------------------------------------------------------------------
 * This returns a pointer to a new STRING_LIST object.
 *------------------------------------------------------------------*/
STRING_LIST *s_string_list_new() {
  STRING_LIST *local_string_list;
  
  local_string_list = malloc(sizeof(STRING_LIST));
  local_string_list->data = NULL;
  local_string_list->next = NULL;
  local_string_list->prev = NULL;
  local_string_list->pos = -1;   /* can look for this later . . .  */
  
  return local_string_list;
}


/*------------------------------------------------------------------
 * This fcn inserts the item into a char* list.  It first cycles through the
 * list to make sure that there are no duplications. The list is assumed
 * to be a STRING_LIST:
 * struct STRING_LIST
 * {
 *   char *data;
 *   int pos;
 *   STRING_LIST *next;
 *   STRING_LIST *prev;
 * };
 *------------------------------------------------------------------*/
void s_string_list_add_item(STRING_LIST *list, int *count, char *item) {

  gchar *trial_item = NULL;
  STRING_LIST *prev;
  STRING_LIST *local_list;
  
  /* First check to see if list is empty.  Handle insertion of first item 
     into empty list separately.  (Is this necessary?) */
  if (list->data == NULL) {
#ifdef DEBUG
    printf("In s_string_list_add_item, about to place first item in list.\n");
#endif
    list->data = (gchar *) u_basic_strdup(item);
    list->next = NULL;
    list->prev = NULL;  /* this may have already been initialized. . . . */
    list->pos = (int) count;  /* This enumerates the pos on the list.  Value is reset later by sorting. */
    (*count)++;  /* increment count to 1 */
    return;
  }

  /* Otherwise, loop through list looking for duplicates */
  prev = list;
  while (list != NULL) {
    trial_item = (gchar *) u_basic_strdup(list->data);        
    if (strcmp(trial_item, item) == 0) {
      /* Found item already in list.  Just return. */
      free(trial_item);
      return;
    }
    free(trial_item);
    prev = list;
    list = list->next;
  }

  /* If we are here, it's 'cause we didn't find the item pre-existing in the list. */
  /* In this case, we insert it. */

  local_list = malloc(sizeof(STRING_LIST));  /* allocate space for this list entry */
  local_list->data = (gchar *) u_basic_strdup(item);   /* copy data into list */
  local_list->next = NULL;
  local_list->prev = prev;  /* point this item to last entry in old list */
  prev->next = local_list;  /* make last item in old list point to this one. */
  local_list->pos = (int) count;  /* This enumerates the pos on the list.  Value is reset later by sorting. */
  (*count)++;  /* increment count */
  list = local_list;
  return;

}

/*------------------------------------------------------------------
 * This fcn looks for item in the list.  It returns 1 if item is
 * present, 0 if absent.
 *------------------------------------------------------------------*/
int s_string_list_in_list(STRING_LIST *list, char *item) {

  gchar *trial_item = NULL;
  
  /* First check to see if list is empty.  If empty, return
   * 0 automatically.  (I probably don't need to handle this 
   * separately.)  */
  if (list->data == NULL) {
    return 0;
  }

  /* Otherwise, loop through list looking for duplicates */
  while (list != NULL) {
    trial_item = (gchar *) u_basic_strdup(list->data);        
    if (strcmp(trial_item, item) == 0) {
      /* Found item already in list.  return 1. */
      free(trial_item);
      return 1;
    }
    free(trial_item);
    list = list->next;
  }

  /* If we are here, it's 'cause we didn't find the item 
   * pre-existing in the list.  In this case, return 0 */
  return 0;

}



/*------------------------------------------------------------------
 * This fcn takes the master comp list sheet_head->master_comp_list_head
 * and sorts it in this order:
 * <all refdeses in alphabetical order>
 * Right now it does nothing other than fill in the "position"
 * and "length" variables.
 *------------------------------------------------------------------*/
void s_string_list_sort_master_comp_list() {
  int i = 0;
  STRING_LIST *local_list;

  /* Here's where we do the sort.  The sort is done using a fcn found on the web. */
  local_list = sheet_head->master_comp_list_head;
  local_list = listsort(local_list, 0, 1);

  /* Do this after sorting is done.  This resets the order of the individual items
   * in the list.  */
  while (local_list != NULL) {  /* make sure item is not null */
    local_list->pos = i;
    if (local_list->next != NULL) {  
      i++;
      local_list = local_list->next;
    } else {
      break;                    /* leave loop *before* iterating to NULL EOL marker */
    }
  }

  /* Now go to first item in local list and reassign list head to new first element */
  while (local_list->prev != NULL) {
    local_list = local_list->prev;
  }

  sheet_head->master_comp_list_head = local_list;

  return;
}

/*------------------------------------------------------------------
 * This fcn takes the master comp attrib list sheet_head->master_comp_attrib_list_head
 * and sorts it in this order:
 * <all refdeses in alphabetical order>
 * Right now it does nothing other than fill in the "position"
 * and "length" variables.
 *------------------------------------------------------------------*/
void s_string_list_sort_master_comp_attrib_list() {
  int i = 0;
  STRING_LIST *local_list;

  /* Here's where we do the sort */

  /*
   * Note that this sort is TBD -- it is more than just an alphabetic sort 'cause we want 
   * certain attribs to go first. 
   */
  

  /* Do this after sorting is done.  This resets the order of the individual items
   * in the list.  */
  local_list = sheet_head->master_comp_attrib_list_head;
  while (local_list != NULL) {
    local_list->pos = i;
    i++;
    local_list = local_list->next;
  }

  return;
}

/*------------------------------------------------------------------
 * This fcn takes the master net list sheet_head->master_net_list_head
 * and sorts it in this order:
 * <all nets in alphabetical order>
 *------------------------------------------------------------------*/
void s_string_list_sort_master_net_list() {
  int i = 0;
  STRING_LIST *local_list;


  /* Do this after sorting is done.  This resets the order of the individual items
   * in the list.  */
  local_list = sheet_head->master_net_list_head;
  while (local_list != NULL) {
    local_list->pos = i;
    i++;
    local_list = local_list->next;
  }

  return;
}

/*------------------------------------------------------------------
 * This fcn takes the master net attrib list sheet_head->master_net_attrib_list_head
 * and sorts it in this order:
 * value, footprint, model-name, file, <all other attribs in alphabetical order>
 *------------------------------------------------------------------*/
void s_string_list_sort_master_net_attrib_list() {
  int i = 0;
  STRING_LIST *local_list;


  /* Do this after sorting is done.  This resets the order of the individual items
   * in the list.  */
  local_list = sheet_head->master_net_attrib_list_head;
  while (local_list != NULL) {
    local_list->pos = i;
    i++;
    local_list = local_list->next;
  }

  return;
}


/*------------------------------------------------------------------
 * This fcn takes the master pin list sheet_head->master_pin_list_head
 * and sorts it in this order:
 * <all refdeses in alphabetical order>
 * Right now it does nothing other than fill in the "position"
 * and "length" variables.
 *------------------------------------------------------------------*/
void s_string_list_sort_master_pin_list() {
  int i = 0;
  STRING_LIST *local_list;

  /* Here's where we do the sort.  The sort is done using a fcn found on the web. */
  local_list = sheet_head->master_pin_list_head;
  local_list = listsort(local_list, 0, 1);

  /* Do this after sorting is done.  This resets the order of the individual items
   * in the list.  */
  while (local_list != NULL) {  /* make sure item is not null */
    local_list->pos = i;
    if (local_list->next != NULL) {  
      i++;
      local_list = local_list->next;
    } else {
      break;                    /* leave loop *before* iterating to NULL EOL marker */
    }
  }

  /* Now go to first item in local list and reassign list head to new first element */
  while (local_list->prev != NULL) {
    local_list = local_list->prev;
  }

  sheet_head->master_pin_list_head = local_list;

  return;
}

/*------------------------------------------------------------------
 * This fcn takes the master pin attrib list sheet_head->master_pin_attrib_list_head
 * and sorts it in this order:
 * <all pin attribs in alphabetical order>
 * Right now it does nothing other than fill in the "position"
 * and "length" variables.
 *------------------------------------------------------------------*/
void s_string_list_sort_master_pin_attrib_list() {
  int i = 0;
  STRING_LIST *local_list;

  /* Here's where we do the sort */

  /*
   * Note that this sort is TBD -- it is more than just an alphabetic sort 'cause we want 
   * certain attribs to go first. 
   */
  

  /* Do this after sorting is done.  This resets the order of the individual items
   * in the list.  */
  local_list = sheet_head->master_pin_attrib_list_head;
  while (local_list != NULL) {
    local_list->pos = i;
    i++;
    local_list = local_list->next;
  }

  return;
}

