#ifndef STRBUF_H
#define STRBUF_H

#include <stdlib.h>

typedef enum
{
  STRBUF_DYNAMIC
} STRBUF_TYPE;

typedef enum
{
  STRBUF_SIZE_SMALL = 16,
  STRBUF_SIZE_MEDIUM = 256
} STRBUF_SIZE;

typedef struct
{
  STRBUF_TYPE type;
  size_t size;
  char *d;
} STRBUF;


/**
 * Allocate a string buffer of size at least \a size.
 *
 * String buffer data is assigned a zero-length string.
 */
STRBUF *strbuf_alloc(size_t size);

/**
 * Free memory occupied by \a buf
 */
void strbuf_free(STRBUF *buf);

/**
 * Append a char to \a buf. Resize when needed.
 */
void strbuf_append(STRBUF *buf, char c);

/**
 * If *buf is NULL, allocate memory; append \a c
 */
static inline void strbuf_xappend(STRBUF **buf, char c, size_t size)
{
  if (*buf==NULL)
    *buf = strbuf_alloc(size);
  strbuf_append(*buf, c);
}

/**
 * Append src to dest. Returns dest.
 */
STRBUF *strbuf_cat(STRBUF *dest, const STRBUF *src);

#endif // STRBUF_H

