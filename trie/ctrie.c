#include "ctrie.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

CTRIE_NODE *ctrie_alloc()
{
  return calloc(1, sizeof(CTRIE_NODE));
}

void ctrie_free(CTRIE_NODE *node)
{
  for (int i=0; i<node->n_links; i++)
    strbuf_free(node->links[i].str);
  free(node);
}

void ctrie_rfree(CTRIE_NODE *node)
{
  for (unsigned int i=0; i<node->n_links; i++)
    ctrie_rfree(node->links[i].next);
  ctrie_free(node);
}

CTRIE_NODE *ctrie_from_trie(TRIE_NODE *trie)
{
  CTRIE_NODE *ctrie = ctrie_alloc();
  ctrie->value = trie->value;
  for (int c=0; c<256; c++)
    if (trie->links[c]!=NULL)
    {
      unsigned int n = ctrie->n_links;
      ctrie->n_links++;

      ctrie->links[n].str = strbuf_alloc(STRBUF_SIZE_SMALL);
      ctrie->links[n].str->d[0] = (char)c;
      ctrie->links[n].str->d[1] = 0;
      ctrie->links[n].next = ctrie_from_trie(trie->links[c]);
    }
  return ctrie;
}

CTRIE_NODE *ctrie_compact(CTRIE_NODE *node)
{
  for (unsigned int i=0; i<node->n_links; i++)
  {
    while (node->links[i].next->n_links==1 && node->links[i].next->value==0)
    {
      CTRIE_NODE *tmp = node->links[i].next;
      strbuf_cat(node->links[i].str, tmp->links[0].str);
      node->links[i].next = tmp->links[0].next;
      ctrie_free(tmp);
    }
    ctrie_compact(node->links[i].next);
  }
  return node;
}


//======================================


void ctrie_print(CTRIE_NODE *node)
{
  static char current[256] = "";
  char out[256];
  strcpy(out, current);
  if (strlen(out)==0)
    strcpy(out, "<NULL>");

  printf("\"%s\" [label=\"%s [%u]\", fillcolor=\"%s\", style=\"filled\"];\n", 
      out, out, node->value, (node->value)==0?"white":"#AAFFAA");

  int length = strlen(current);
  for (unsigned int i=0; i<node->n_links; i++)
  {
    strcat(current, node->links[i].str->d);
    printf("\"%s\"-> \"%s\" [label=\"%s\"];\n", out, current, node->links[i].str->d);
    ctrie_print(node->links[i].next);
    current[length] = 0; 
  }
}


