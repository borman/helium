#include "xml_memory.h"

#include <stdlib.h>
#include <string.h>

XMLNode *XML_AllocNode() 
{
  return calloc(1, sizeof(XMLNode));
}

XMLAttr *XML_AllocAttr()
{
  return calloc(1, sizeof(XMLAttr));
}

void XML_DestroyTree(XMLNode *root)
{
  XMLAttr *attr = root->attr;
  while (attr != NULL)
  {
    XMLAttr *next = attr->next;
    strbuf_free(attr->name);
    strbuf_free(attr->value);
    free(attr);
    attr = next;
  }

  strbuf_free(root->name);
  strbuf_free(root->text);

  XMLNode *child = root->first_child;
  while (child != NULL)
  {
    XMLNode *next = child->next_sibling;
    XML_DestroyTree(child);
    child = next;
  }

  free(root);
}

