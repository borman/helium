/*
 * XML unit tester
 *
 * Reads xml input from file and prints its tree
 */

#include "xml/xml_parser.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define MAXFILESIZE 65536

void dump_xmltree(XMLNode *node, int level) {
  if (node==NULL) 
    return;

  char *name = node->name;
  if (name==NULL)
    name = "<NULL>";
  
  printf("%s {\n", name);
  dump_xmltree(node->subnode, level+1);
  printf("}");

  dump_xmltree(node->next, level); 
}

char file_buffer[MAXFILESIZE];

int main(int argc, char **argv) {
  if (argc!=2) {
    printf("Usage: %s <file.xml>\n", argv[0]);
    return 1;
  }

  FILE *input = fopen(argv[1], "r");
  if (input==NULL) {
    perror(strerror(errno));
    return 1;
  }

  size_t n_read = fread(file_buffer, sizeof(char), MAXFILESIZE, input);
  if (ferror(input)) {
    perror(strerror(errno));
    return 1;
  }
  fclose(input);

  XML_CONTEXT ctx;
  XMLNode *tree = XMLDecode(&ctx, file_buffer, n_read);
  printf("parsed, tree null: %d\n", tree==NULL);
  dump_xmltree(tree, 0);
 
  DestroyTree(&ctx, tree);
  return 0;
}
