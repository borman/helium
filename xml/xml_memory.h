#ifndef XML_MEMORY_H
#define XML_MEMORY_H

#include "xml_common.h"

XMLNode *XML_AllocNode();
XMLAttr *XML_AllocAttr();

void XML_DestroyTree(XMLNode *root);

#endif // XML_MEMORY_H

