#include "strbuf.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

static unsigned int n_allocs = 0;
static unsigned int n_reallocs = 0;
static unsigned int n_frees = 0;

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
  return buf;
}

void strbuf_free(STRBUF *buf)
{
  if (buf == NULL)
    return;

  free(buf->d);
  free(buf);
  n_frees++;

  fprintf(stderr, "[strbuf] %u/%u/%u\n", n_allocs, n_frees, n_reallocs);
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

