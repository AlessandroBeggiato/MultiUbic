/*
 * stack.c
 * Implementantion of a general purpose stack of pointers
 *  Created on: 27/giu/2014
 *      Author: francesco
 * Copyright (C) 2014 Francesco Burato
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stack.h"
#include "glue.h"

/**
 * Allocate and return a new stack
 */
struct stack * stack_alloc() {
  struct stack * newstack = gl_malloc(sizeof(struct stack));
  newstack->size = 0;
  newstack->head = 0;
  return newstack;
}

/**
 * Deallocate all information of the the stack and the stack itself
 */
void stack_destroy(struct stack * s ) {
  struct _node * current = s->head, *temp ;
  // Pop all the nodes from the stack
  while(current) {
    temp = current->next;
    gl_free(current);
    current = temp;
  }
  // delete stack s from memory
  gl_free(s);
}
/**
 * Test if there is information in the stack
 */
int isEmpty(struct stack * s) {
  return s->size == 0;
}

/**
 * Push new information in the stack
 */
void push(struct stack * s, void * i) {
  //create new node with information i
  struct _node * newhead = gl_malloc(sizeof(struct _node));
  newhead->info = i;
  newhead->next = s->head;
  s->size++;
  // set current head of the stack to newly created node
  s->head = newhead;
}

/**
 * Get the head of the stack and remove the first element
 */
void * pop(struct stack * s) {
  void * information;
  struct _node * oldhead;
  if(s->head) {
    // save the information and the old head of the stack
    information = s->head->info;
    oldhead = s->head;
    // move the head to the next node
    s->head = s->head->next;
    // delete the old head
    gl_free(oldhead);
    s->size--;
    return information;
  } else
    return NULL;
}

/**
 * Get the head of the stack without removing the first element
 */
void * head(struct stack * s) {
  if(s->head)
    return s->head->info;
  else
    return NULL;
}
