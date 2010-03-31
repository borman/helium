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
