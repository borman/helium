/*-
 * Copyright (C) 2009 Ilya Bakulin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _XML_PARSER_
#define _XML_PARSER_

#include "xml_common.h"

// For size_t
#include <string.h>

typedef struct
{
  XMLNode *XMLTree;
  XMLAttr *curattr;
  size_t *TreeBranchs;
  size_t BranchPos;
  XMLNode *current;
  XMLNode *tmp;
  XMLAttr *attribs;
  short IsClosed;
  char *TagName;
  char *AttrName;
  char *AttrValue;
  char *Text;
  short TagState;
  short MSState;
} XML_CONTEXT;


// Декодировать поток
void* XMLDecode(XML_CONTEXT *ctx, char *buf, int size);

// Уничтожить дерево
void DestroyTree(XML_CONTEXT *ctx, XMLNode *tree);

/*
  Получить значение атрибута по его имени
IN: char* req_attr_name - название атрибута
    XMLAttr* attr_list  - список атрибутов тега
OUT: значение атрибута или NULL
*/
char* XML_Get_Attr_Value(char* req_attr_name, XMLAttr* attr_list);


/*
  Получить дочерний узел из списка дочерних узлов по его имени
IN: XMLNode* node         - родительский узел
    char* req_node_name   - имя требуемого узла
OUT: дочерний узел или NULL
*/
XMLNode* XML_Get_Child_Node_By_Name(XMLNode* node, char* req_node_name);

/*
  Получить дочерний узел из списка дочерних узлов по его имени,
  при условии, что существует заданный атрибут с заданным значением
IN: XMLNode* node         - родительский узел
    char* req_node_name   - имя требуемого узла
    char* req_attr_name   - имя атрибута
    char* req_attr_velue  - значение атрибута
OUT: дочерний узел или NULL
*/
XMLNode* XML_Get_Child_Node_By_Name_And_Attr(XMLNode* node, char* req_node_name, char* req_attr_name, char* req_attr_velue);
////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  MS_BEGIN,					// Начало парсинга
  MS_BEGINTAG,					// Начался тег
  MS_PROCESSING_INSTRUCTION, // Инструкция, "<? ... ?>"
  MS_MIDDLETAG,					// Середина тега, но еще до имени
  MS_TAGNAME,						// Парсим имя тега
  MS_ENDTAGNAME,					// Закончили парсить имя тега
  MS_ATTRIBNAME,				// Парсим имя аттрибута
  MS_ENDATTRIBNAME,				// Прочитали имя аттрибута
  MS_ENDEQUALLY,					// Прошли знак = между именем аттритута и значением
  MS_ATTRIBVALUE,					// Парсим значение аттрибута (после ")
  MS_ENDTAG,					// Конец тега (для самозакрывающегося после /)
  MS_TEXT,						// Текст
  MS_ERROR  // Ошибка разбора
} XML_PARSER_STATE;

typedef enum
{
  TS_INDEFINITE,					// Неопределенный, т.к. тег только начали парсить (если начали)
  TS_NORMAL,						// Обычный (не)закрывающийся. Точно не декларация
  TS_CLOSE,						// Закрывающийся
  TS_EMPTY						// Сам и закрывается
} TAG_TYPE;

#endif
