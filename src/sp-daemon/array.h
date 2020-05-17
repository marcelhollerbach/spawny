#ifndef ARRAY_H
#define ARRAY_H

#include <stdlib.h>

typedef struct _Array Array;

Array *
array_new(size_t size);
void *
array_add(Array *arr);
void *
array_get(Array *arr, unsigned int i);
void
array_del(Array *arr, unsigned int i);
unsigned int
array_len_get(Array *arr);
void
array_free(Array *arr);

#endif

#define ARR_DEFINE_NEW(TYPE)                                                   \
   static inline Array *array_##TYPE##_new(void)                               \
   {                                                                           \
      return array_new(sizeof(TYPE));                                          \
   }

#define ARR_DEFINE_ADD(TYPE)                                                   \
   static inline TYPE *array_##TYPE##_add(Array *arr) { return array_add(arr); }
#define ARR_DEFINE_GET(TYPE)                                                   \
   static inline TYPE *array_##TYPE##_get(Array *arr, unsigned int i)          \
   {                                                                           \
      return array_get(arr, i);                                                \
   }

#define ARR_DEFINE_DEL(TYPE)                                                   \
   static inline void array_##TYPE##_del(Array *arr, unsigned int i)           \
   {                                                                           \
      array_del(arr, i);                                                       \
   }

#define ARRAY_API(TYPE)                                                        \
   ARR_DEFINE_NEW(TYPE)                                                        \
   ARR_DEFINE_ADD(TYPE)                                                        \
   ARR_DEFINE_GET(TYPE)                                                        \
   ARR_DEFINE_DEL(TYPE)
