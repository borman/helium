#include "trie/trie.h"
#include "trie/ctrie.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

/**
 * Wordlist-to-binary-tree converter
 * Converts a list of words into 2 C header files
 * <const.h>: word constant definitions
 * <data.h>: a serialized TRIE.
 *
 * For serialized TRIE format, see dict/dict.c
 */

static char *ident(char *word)
{
  size_t length = strlen(word);
  for (unsigned int i=0; i<length; i++)
  {
    word[i] = toupper(word[i]);
  }
  int s = 0, d = 0;
  while (s<length)
  {
    if (!isalpha(word[s]))
    {
      word[d] = '_';
      while (!isalpha(word[s]) && s<length)
        s++;
    }
    else
    {
      word[d] = word[s];
      s++;
    }
    d++;
  }
  word[d+1] = 0;
  return word;
}

static void ctrie_set_positions(CTRIE_NODE *node, unsigned int *current_position)
{
  assert(node!=NULL);
  assert(current_position!=NULL);

  node->s_position = *current_position;
  node->s_size = 1/*value*/ + 1/*n_links*/ + node->n_links/*links*/;
  assert(node->n_links<256);
  for (unsigned int i=0; i<node->n_links; i++)
    node->s_size += 2+strlen(node->links[i].str->d)+1;
  *current_position += node->s_size;

  for (unsigned int i=0; i<node->n_links; i++)
    ctrie_set_positions(node->links[i].next, current_position);
}

static void ctrie_print_serialized(CTRIE_NODE *node, unsigned int indent, FILE *file)
{
#define XCHAR "\\x%02X"
  for (unsigned int i=0; i<indent; i++)
    fputc(' ', file);
  fputc('"', file);
  
  /* Write word code: u8 */ 
  assert(node->value < 256);
  fprintf(file, XCHAR, node->value);

  /* Write number of links: u8 */
  assert(node->n_links < 256);
  fprintf(file, XCHAR, node->n_links);
  
  /* Write link offsets: u8 * node->n_links */
  unsigned int offsets[256];
  unsigned int current_offset = node->n_links;
  for (unsigned int i=0; i<node->n_links; i++)
  {
    assert(current_offset<256);
    offsets[i] = current_offset;
    current_offset += 2+strlen(node->links[i].str->d)+1/*delta-string*/ - 1/*jumptable offset*/;
  }
  for (unsigned int i=0; i<node->n_links; i++)
    fprintf(file, XCHAR, offsets[i]);
  fputs("\"\n", file);

  /* Write links: u16_le + <ASCIIZ-string> */
  for (unsigned int i=0; i<node->n_links; i++)
  {
    unsigned int link_offset = node->links[i].next->s_position;
    assert(link_offset<65536);
    for (unsigned int i=0; i<indent+2; i++)
      fputc(' ', file);
    fprintf(file, "/*->*/ \"" XCHAR XCHAR "\" \"%s\\x00\"\n", 
      link_offset&0xff, link_offset>>8, node->links[i].str->d);
  }

  for (unsigned int i=0; i<node->n_links; i++)
    ctrie_print_serialized(node->links[i].next, indent+2, file);
#undef XCHAR
}

int main(int argc, char **argv)
{ 
  if (argc!=3)
  {
    printf("Usage: %s <constants.h> <data.h>\n", argv[0]);
    return 1;
  }

  FILE *f_const, *f_data;

  f_const = fopen(argv[1], "w");
  if (f_const==NULL)
    goto L_FILE_ERROR;

  f_data = fopen(argv[2], "w");
  if (f_data==NULL)
    goto L_FILE_ERROR;

  /* Build an uncompressed TRIE while writing word codes */
  fputs(
    "#ifndef XMPP_DICT_H\n"
    "#define XMPP_DICT_H\n"
    "/** XMPP TOKEN CODES\n"
    "  * AUTO-GENERATED, DO NOT EDIT\n"
    " **/\n"
    "\n"
    "typedef enum\n{\n  SS1_NONE = 0,\n", 
    f_const);

  char word[512];
  TRIE_NODE *trie = trie_alloc();
  unsigned int index = 1;
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
    {
      fprintf(f_const, "  SS_%s = %u,\n", ident(word), index);
      index++;
    }
  }

  fputs(
    "  SS1_DICT_END\n"
    "} XMPP_DICT;\n"
    "#endif // XMPP_DICT_H\n", 
    f_const);

  /* Compress TRIE */
  CTRIE_NODE *ctrie = ctrie_compact(ctrie_from_trie(trie));
  trie_rfree(trie);

  /* Serialize CTRIE */
  fputs(
    "/** XMPP TOKEN TRIE\n"
    "  * AUTO-GENERATED, DO NOT EDIT\n"
    " **/\n"
    "\n"
    "static const char xmpp_trie[] =\n", 
    f_data);

  unsigned int pos = 0;
  ctrie_set_positions(ctrie, &pos);
  ctrie_print_serialized(ctrie, 2, f_data);
  ctrie_rfree(ctrie);

  fputs("/* TRIE END */;\n", f_data);

  fclose(f_const);
  fclose(f_data);

  return 0;

L_FILE_ERROR:
  printf("ERROR: %s\n", strerror(errno));
  return 1;
}

