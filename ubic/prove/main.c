/*
 * main.c
 *
 *  Created on: 04/apr/2014
 *      Author: francesco
 */
#include <stdio.h>
#include "bv/bv.h"
#include "nodelist.h"

void testBinary(void) {
  struct bv * arr1 = (struct bv *) gl_malloc(sizeof(struct bv))
          , * arr2 =  (struct bv *) gl_malloc(sizeof(struct bv))
          , * arr3 =  (struct bv *) gl_malloc(sizeof(struct bv));
  int i;
  const unsigned int length = 4;
  bv_init(arr1, length);
  bv_init(arr2, length);
  bv_init(arr3, length);
  for( i = 0; i < length; ++i)
    printf("arr1[%d]=%d, arr2[%d]=%d, arr3[%d]=%d\n",i,bv_get(arr1,i),i,bv_get(arr2,i),i,bv_get(arr3,i));
  for( i = 0; i < length; ++i)
    if(i%2 == 0)
      bv_set(arr1,i);
    else
      bv_set(arr2,i);
  for( i = 0; i < length; ++i)
    printf("arr1[%d]=%d, arr2[%d]=%d, arr3[%d]=%d\n",i,bv_get(arr1,i),i,bv_get(arr2,i),i,bv_get(arr3,i));
  printf("\n");
  bv_or(arr1, arr2, arr3);
  for( i = 0; i < length; ++i)
      printf("arr1[%d]=%d, arr2[%d]=%d, arr3[%d]=%d\n",i,bv_get(arr1,i),i,bv_get(arr2,i),i,bv_get(arr3,i));
  printf("\n");
  for( i = 0; i < length; ++i)
    bv_or(arr1,arr2,arr1);
  for( i = 0; i < length; ++i)
    printf("arr1[%d]=%d, arr2[%d]=%d, arr3[%d]=%d\n",i,bv_get(arr1,i),i,bv_get(arr2,i),i,bv_get(arr3,i));
  printf("\n");
  bv_zeros(arr2);
  for( i = 0; i < length; ++i)
      printf("arr1[%d]=%d, arr2[%d]=%d, arr3[%d]=%d\n",i,bv_get(arr1,i),i,bv_get(arr2,i),i,bv_get(arr3,i));
  bv_term(arr1);
  bv_term(arr2);
  bv_term(arr3);
  gl_free(arr1);
  gl_free(arr2);
  gl_free(arr3);
}

void testPointers(void) {
  const unsigned int size = 10;
  register int i;
  struct my_struct {
    struct nl * nod;
    int info;
  };
  struct my_struct * element = (struct my_struct *) malloc(sizeof(struct my_struct));
  struct my_struct * element2 = (struct my_struct *) malloc(sizeof(struct my_struct));
  struct nl * list1 = NULL;
  struct nl * list2 = NULL;
  struct nl ** array = (struct nl **) malloc(sizeof(struct nl *)*size);
  element->info=42;
  element2->info=43;
  nl_insert(&list1,element);
  nl_insert(&list2,element);
  nl_insert(&list2,element2);
  for(i = 0; i < size; ++i)
    array[i] = NULL;
  array[0] = list1;
  array[1] = list2;
}

int main(int argc, char ** argv) {
  printf("Hello Test!\n");
  // Test for the binary array
  testBinary();
  testPointers();
  return 0;
}
