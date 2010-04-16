#include "trie.h"
#include "common/strbuf.h"

/** A compressed TRIE **/

#define CTRIE_WLEN 256

typedef struct _CTRIE_NODE
{
  unsigned int position;
  unsigned int value;
  unsigned int n_links;
  struct 
  {
    STRBUF *str;
    struct _CTRIE_NODE *next;
  } links[256];
} CTRIE_NODE;

/** Allocate an empty ctrie node
 */
CTRIE_NODE *ctrie_alloc();

/** Free a ctrie node
 */
void ctrie_free(CTRIE_NODE *node);

/** Free a ctrie recursively
 */
void ctrie_rfree(CTRIE_NODE *node);

/** Build a non-compressed ctrie from a trie
 */
CTRIE_NODE *ctrie_from_trie(TRIE_NODE *trie);

/** Compress a ctrie
 */
CTRIE_NODE *ctrie_compact(CTRIE_NODE *node);

/** Print a ctrie as a sequence of statements in "dot" language
 */
void ctrie_print(CTRIE_NODE *node);

