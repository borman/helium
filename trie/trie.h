#ifndef TRIE_H
#define TRIE_H

/** A non-compressed TRIE **/

typedef struct _TRIE_NODE
{
  unsigned int value;
  struct _TRIE_NODE *links[256];
} TRIE_NODE;


/**
 * Allocate an empty trie node
 */
TRIE_NODE *trie_alloc();

/**
 * Free a trie recursively
 */
void trie_rfree(TRIE_NODE *node);

/**
 * Add a word to a trie 
 * If added, word is mapped to value
 * ret == 1 if word is added, 0 if exists
 */
TRIE_NODE *trie_add(TRIE_NODE *node, const char *word, unsigned int value, int *ret);

/**
 * Print a trie as a list if statements in "dot" language
 */
void trie_print(TRIE_NODE *node);

int trie_mark(TRIE_NODE *root, const char *word, unsigned int value);

#endif // TRIE_H
