/*
 * XML unit tester
 *
 * Reads xml input from file and prints its tree
 */

#include "xml/xml_parser.h"
#include "xml/xml_memory.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define MAXFILESIZE 65536

void dump_xmltree(XMLNode *node, int level)
{
  char indent[40];
  for (int i=0; i<level*2; i++)
    indent[i] = ' ';
  indent[level] = '\x00';

  if (node==NULL)
    return;

  char *name = node->name->d;
  if (name==NULL)
    name = "<NULL>";

  printf("%s%s {\n", indent, name);

  XMLAttr *attr = node->attr;
  while (attr)
  {
    printf("%s \"%s\" = \"%s\"\n", indent, attr->name->d, attr->value->d);
    attr = attr->next;
  }

  dump_xmltree(node->first_child, level+1);

  printf("%s}\n", indent);

  dump_xmltree(node->next_sibling, level);
}

char file_buffer[MAXFILESIZE];

int main(int argc, char **argv)
{
  if (argc!=2)
  {
    printf("Usage: %s <file.xml>\n", argv[0]);
    return 1;
  }

  FILE *input = fopen(argv[1], "r");
  if (input==NULL)
  {
    perror(strerror(errno));
    return 1;
  }

  size_t n_read = fread(file_buffer, sizeof(char), MAXFILESIZE, input);
  if (ferror(input))
  {
    perror(strerror(errno));
    return 1;
  }
  fclose(input);

  XML_CONTEXT *ctx = XML_CreateContext();
  XML_Decode(ctx, file_buffer, n_read);
  printf("parsed, tree null: %d\n", ctx->Root==NULL);
  dump_xmltree(ctx->Root, 0);
  XML_DestroyTree(ctx->Root);
  XML_DestroyContext(ctx);

  return 0;
}
