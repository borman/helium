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

#define MS_BEGIN 1						///< Начало парсинга
#define MS_BEGINTAG 2					///< Начался тег
#define MS_MIDDLETAG 3					///< Середина тега, но еще до имени
#define MS_TAGNAME 4						///< Парсим имя тега
#define MS_ENDTAGNAME 5					///< Закончили парсить имя тега
#define MS_ATTRIBNAME 6					///< Парсим имя аттрибута
#define MS_ENDATTRIBNAME 7				///< Прочитали имя аттрибута
#define MS_ENDEQUALLY 8					///< Прошли знак = между именем аттритута и значением
#define MS_ATTRIBVALUE 9					///< Парсим значение аттрибута (после ")
#define MS_ENDTAG 10						///< Конец тега (для самозакрывающегося после /)
#define MS_TEXT 11						///< Текст


#define TS_INDEFINITE 1					///< Неопределенный, т.к. тег только начали парсить (если начали)
#define TS_NORMAL 2						///< Обычный (не)закрывающийся. Точно не декларация
#define TS_CLOSE 3						///< Закрывающийся
#define TS_EMPTY 4						///< Сам и закрывается
#define TS_DECLARATION 5					///< Декларация, что это XML. Парсер аттрибуты игнорирует

#endif
