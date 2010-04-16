#include "trie.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef unsigned char uchar;

TRIE_NODE *trie_alloc()
{
  return calloc(1, sizeof(TRIE_NODE));
}

void trie_rfree(TRIE_NODE *node)
{
  for (int i=0; i<256; i++)
    if (node->links[i]!=NULL)
      trie_rfree(node->links[i]);
  free(node);
}

TRIE_NODE *trie_add(TRIE_NODE *node, const char *word)
{
  if (word[0]==0)
    return node;

  TRIE_NODE *next = node->links[(uchar)word[0]];
  if (next==NULL)
    next = trie_alloc();

  node->links[(uchar)word[0]] = trie_add(next, word+1);
  return node;
}

//======================================

void trie_print(TRIE_NODE *node)
{
  static char current[256] = "";
  char out[256];
  strcpy(out, current);
  if (strlen(out)==0)
    strcpy(out, "<NULL>");

  int length = strlen(current);
  for (int i=0; i<256; i++)
    if (node->links[i]!=NULL)
    {
      current[length] = i;
      current[length+1] = 0;
      printf("\"%s\"-> \"%s\" [label=\"%c\"];\n", out, current, (char)i);
      trie_print(node->links[i]);
      current[length] = 0; 
    }
}

//======================================

