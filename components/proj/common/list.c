/********************************************************************************************************
 * @file     list.c 
 *
 * @brief    for TLSR chips
 *
 * @author	 telink
 * @date     Sep. 30, 2010
 *
 * @par      Copyright (c) 2010, Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *           
 *			 The information contained herein is confidential and proprietary property of Telink 
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms 
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai) 
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in. 
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this 
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided. 
 *           
 *******************************************************************************************************/

// suppress all lint messages for this file
//lint --e{*}

#include "list.h"

#define NULL 0

struct list {
  struct list *next;
};

/*---------------------------------------------------------------------------*/
/**
 * Initialize a list.
 *
 * This function initalizes a list. The list will be empty after this
 * function has been called.
 *
 * \param list The list to be initialized.
 */
void
list_init(list_t list)
{
  *list = NULL;
}
/*---------------------------------------------------------------------------*/
/**
 * Get a pointer to the first element of a list.
 *
 * This function returns a pointer to the first element of the
 * list. The element will \b not be removed from the list.
 *
 * \param list The list.
 * \return A pointer to the first element on the list.
 *
 * \sa list_tail()
 */
void *
list_head(list_t list)
{
  return *list;
}
/*---------------------------------------------------------------------------*/
/**
 * Duplicate a list.
 *
 * This function duplicates a list by copying the list reference, but
 * not the elements.
 *
 * \note This function does \b not copy the elements of the list, but
 * merely duplicates the pointer to the first element of the list.
 *
 * \param dest The destination list.
 * \param src The source list.
 */
void
list_copy(list_t dest, list_t src)
{
  *dest = *src;
}
/*---------------------------------------------------------------------------*/
/**
 * Get the tail of a list.
 *
 * This function returns a pointer to the elements following the first
 * element of a list. No elements are removed by this function.
 *
 * \param list The list
 * \return A pointer to the element after the first element on the list.
 *
 * \sa list_head()
 */
void *
list_tail(list_t list)
{
  struct list *l;

  if(*list == NULL) {
    return NULL;
  }

  for(l = (struct list*)(*list); l->next != NULL; l = l->next);

  return l;
}
/*---------------------------------------------------------------------------*/
/**
 * Add an item at the end of a list.
 *
 * This function adds an item to the end of the list.
 *
 * \param list The list.
 * \param item A pointer to the item to be added.
 *
 * \sa list_push()
 *
 */
void
list_add(list_t list, void *item)
{
  struct list *l;

  ((struct list *)item)->next = NULL;

  l = (struct list*)list_tail(list);

  if(l == NULL) {
    *list = item;
  } else {
    l->next = (struct list*)item;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Add an item to the start of the list.
 */
void
list_push(list_t list, void *item)
{
  /*  struct list *l;*/

  ((struct list *)item)->next = (struct list*)*list;
  *list = item;
}
/*---------------------------------------------------------------------------*/
/**
 * Remove the last object on the list.
 *
 * This function removes the last object on the list and returns it.
 *
 * \param list The list
 * \return The removed object
 *
 */
void *
list_chop(list_t list)
{
  struct list *l, *r;

  if(*list == NULL) {
    return NULL;
  }
  if(((struct list *)*list)->next == NULL) {
    l = (struct list*)*list;
    *list = NULL;
    return l;
  }

  for(l = (struct list*)*list; l->next->next != NULL; l = l->next);

  r = l->next;
  l->next = NULL;

  return r;
}
/*---------------------------------------------------------------------------*/
/**
 * Remove the first object on a list.
 *
 * This function removes the first object on the list and returns a
 * pointer to the list.
 *
 * \param list The list.
 * \return The new head of the list.
 */
/*---------------------------------------------------------------------------*/
void *
list_pop(list_t list)
{
  if(*list != NULL) {
    *list = ((struct list *)*list)->next;
  }

  return *list;
}
/*---------------------------------------------------------------------------*/
/**
 * Remove a specific element from a list.
 *
 * This function removes a specified element from the list.
 *
 * \param list The list.
 * \param item The item that is to be removed from the list.
 *
 */
/*---------------------------------------------------------------------------*/
void
list_remove(list_t list, void *item)
{
  struct list *l, *r;

  if(*list == NULL) {
    return;
  }

  r = NULL;
  for(l = (struct list*)*list; l != NULL; l = l->next) {
    if(l == item) {
      if(r == NULL) {
	/* First on list */
	*list = l->next;
      } else {
	/* Not first on list */
	r->next = l->next;
      }
      l->next = NULL;
      return;
    }
    r = l;
  }
}
/*---------------------------------------------------------------------------*/
/**
 * Get the length of a list.
 *
 * This function counts the number of elements on a specified list.
 *
 * \param list The list.
 * \return The length of the list.
 */
/*---------------------------------------------------------------------------*/
int
list_length(list_t list)
{
  struct list *l;
  int n = 0;

  for(l = (struct list*)*list; l != NULL; l = l->next) {
    ++n;
  }

  return n;
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Insert an item after a specified item on the list
 * \param list The list
 * \param previtem The item after which the new item should be inserted
 * \param newitem  The new item that is to be inserted
 * \author     Adam Dunkels
 *
 *             This function inserts an item right after a specified
 *             item on the list. This function is useful when using
 *             the list module to ordered lists.
 *
 *             If previtem is NULL, the new item is placed at the
 *             start of the list.
 *
 */
void
list_insert(list_t list, void *previtem, void *newitem)
{
  if(previtem == NULL) {
    list_push(list, newitem);
  } else {

    ((struct list *)newitem)->next = ((struct list *)previtem)->next;
    ((struct list *)previtem)->next = (struct list*)newitem;
  }
}
/*---------------------------------------------------------------------------*/
/** @} */
