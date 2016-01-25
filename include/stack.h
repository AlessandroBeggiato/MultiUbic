/*
 * stack.h
 * A general purpose stack of pointers
 *  Created on: 27/giu/2014
 *      Author: francesco
 *
 *
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

#ifndef STACK_H_
#define STACK_H_

struct _node {
  void * info;
  struct _node * next;
};

struct stack {
  struct _node * head; // head of the stack
  unsigned int size; // number of elements in the stack
};


/**
 * Allocate and return a new stack
 */
struct stack * stack_alloc();
/**
 * Deallocate all information of the the stack and the stack itself
 */
void stack_destroy(struct stack * s );
/**
 * Test if there is information in the stack
 */
int isEmpty(struct stack * s);

/**
 * Push new information in the stack
 */
void push(struct stack * s, void * i);

/**
 * Get the head of the stack and remove the first element
 */
void * pop(struct stack * s);

/**
 * Get the head of the stack without removing the first element
 */
void * head(struct stack * s);

#endif /* STACK_H_ */
