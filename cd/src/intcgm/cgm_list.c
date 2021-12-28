#include <stdlib.h>
#include "cgm_list.h"


tList *cgm_list_new(void)
{
  tList *l =(tList *) malloc(sizeof(tList));

  l->alloc_size = 8;
  l->data =(void **)malloc(l->alloc_size*sizeof(void*));
  l->used_size = 0;

  return l;
}

void cgm_list_delete(tList * l)
{
  int i;
  for(i=0; i< l->used_size; ++i)
    free(l->data[i]);  /* release data also */
  free(l->data);
  free(l);
}

void cgm_list_append(tList *l, void *data)
{
  if(l == NULL)
    return;

  if(l->used_size == l->alloc_size)
  {
    l->alloc_size += 32;
    l->data =(void **)realloc(l->data,l->alloc_size*sizeof(void*));
  }

  l->data[l->used_size] = data;
  l->used_size++;
}

void cgm_list_del(tList *l, int index)
{
  int i;

  if(l == NULL || l->used_size == 0 || index < 0)
    return;

  if(index < 1)
    index=1;

  if(index > l->used_size)
    index=l->used_size;

  --index;           /* index starts at 1, shift to 0 */

  l->used_size--;

  free(l->data[index]);  /* release data */

  for(i=index; i<l->used_size; ++i)
    l->data[i]=l->data[i+1];
}

void* cgm_list_get(tList *l, int index)
{
  if(l == NULL || index <= 0)
    return NULL;

  return index > l->used_size ? NULL : l->data[index-1];
}

