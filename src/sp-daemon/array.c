#include "main.h"
#include <string.h>

struct _Array{
   void *ptr;
   size_t size;
   unsigned int len;
};

#define ACCESS(arr, i) arr->ptr + (i)*arr->size

Array*
array_new(size_t size)
{
   Array *result;

   result = calloc(1, sizeof(Array));
   result->size = size;

   return result;
}

void*
array_add(Array *arr)
{
   arr->len++;
   arr->ptr = realloc(arr->ptr, arr->len*arr->size);

   return ACCESS(arr, arr->len - 1);
}

void*
array_get(Array *arr, unsigned int i)
{
   if (i >= arr->len) return NULL;

   return ACCESS(arr, i);
}

void
array_del(Array *arr, unsigned int i)
{
   size_t s;
   if (i >= arr->len) return;

   s = (arr->len - i - 1)*arr->size;
   memmove(ACCESS(arr, i), ACCESS(arr, i + 1), s);

   arr->len --;
   arr->ptr = realloc(arr->ptr, arr->len*arr->size);
}

unsigned int
array_len_get(Array *arr)
{
   return arr->len;
}

void
array_free(Array *arr)
{
   if (arr->ptr)
     free(arr->ptr);

   free(arr);
}
