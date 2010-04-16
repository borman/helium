#include "trie/trie.h"
#include "trie/ctrie.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main()
{
  char word[256];
  TRIE_NODE *trie = trie_alloc();
  unsigned int index;
  while (scanf("%s", word)>0)
  {
    if (word[0]=='#') /* Skip comments */
    {
      getline(NULL, NULL, stdin);
      continue;
    }
    int ret;
    trie = trie_add(trie, word, index, &ret);
    if (ret)
      index++;
  }
  CTRIE_NODE *ctrie = ctrie_compact(ctrie_from_trie(trie));
  trie_rfree(trie);

  printf("digraph Trie {\n");
  ctrie_print(ctrie);
  printf("}\n");
  ctrie_rfree(ctrie);

  return 0;
}

