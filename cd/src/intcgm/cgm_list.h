#ifndef __CGM_LIST_H__
#define __CGM_LIST_H__

/* Actually it is an array... */

typedef struct {
  void* *data;     
  int alloc_size;
  int used_size;        
} tList;

tList* cgm_list_new   (void);
void   cgm_list_delete(tList * list);
void   cgm_list_append(tList* list, void* data); /* data will be release when removed */
void   cgm_list_del   (tList* list, int index);  /* index starts at 1 */
void*  cgm_list_get   (tList* list, int index);

#endif
