#include "strbuf.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

static unsigned int n_allocs = 0;
static unsigned int n_reallocs = 0;
static unsigned int n_frees = 0;
static unsigned int max_used = 0;

static inline size_t next_size(size_t size)
{
  return size+128;
}

STRBUF *strbuf_alloc(size_t size)
{
  assert(size>0);

  STRBUF *buf = malloc(sizeof(STRBUF));
  buf->type = STRBUF_DYNAMIC;
  buf->size = size;
  buf->d = malloc(size);
  buf->d[0] = 0;

  n_allocs++;
  if (n_allocs-n_frees>max_used)
    max_used = n_allocs-n_frees;

  return buf;
}

void strbuf_free(STRBUF *buf)
{
  if (buf == NULL)
    return;

  free(buf->d);
  free(buf);
  n_frees++;

  /** Debug
   * TODO: attach a debugging unit
   */
  //fprintf(stderr, "[strbuf] a=%u f=%u r=%u b=%u\n", n_allocs, n_frees, n_reallocs, max_used);
}

void strbuf_append(STRBUF *buf, char c)
{
  assert(buf != NULL);

  size_t length = strlen(buf->d);
  if (length+2 > buf->size) /* Grow buffer if needed */
  {
    buf->size = next_size(buf->size);
    buf->d = realloc(buf->d, buf->size);
    n_reallocs++;
  }

  buf->d[length] = c;
  buf->d[length+1] = 0;
}

STRBUF *strbuf_cat(STRBUF *dest, const STRBUF *src)
{
  assert(dest!=NULL);
  assert(src!=NULL);

  size_t dest_length = strlen(dest->d);
  size_t src_length = strlen(src->d);
  if (dest_length+src_length+1 > dest->size) /* Grow */
  {
    while (dest_length+src_length+1 > dest->size)    
      dest->size = next_size(dest->size);
    dest->d = realloc(dest->d, dest->size);
    n_reallocs++;
  }

  strcat(dest->d, src->d);
  return dest;
}

