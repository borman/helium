#include "dict/dict.h"
#include "sdict_d.h"

#include <stdio.h>

int main(int argc, char **argv)
{
  FILE *trie = fopen("trie.bin", "w");
  fwrite(xmpp_trie, sizeof(xmpp_trie), 1, trie);
  fclose(trie);

  char word[256];
  while (scanf("%s", word)>0)
    printf("\"%s\" -> %u\n", word, dict_match(xmpp_trie, word));
  return 0;
}

