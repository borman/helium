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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../util/string_util.h"
#include "xml_common.h"
#include "xml_parser.h"
#include "xml_memory.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define MAX_TAGNAME_LENGTH 1024
#define MAX_ATTRNAME_LENGTH 1024
#define MAX_ATTRVALUE_LENGTH 1024

/*
 * Char classes
 * (From XML RFC, http://www.w3.org/TR/2008/REC-xml-20081126/#NT-Name )
 * (Only implementing ASCII subset)
 * S             ::=    (#x20 | #x9 | #xD | #xA)+
 * NameStartChar ::=    ":" | [A-Z] | "_" | [a-z] 
 * NameChar      ::=    NameStartChar | "-" | "." | [0-9]
 * Name          ::=    NameStartChar (NameChar)*
 */

static inline int is_space(const char c)
{
  return	c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

static inline int is_name_start_char(const char c)
{
  return c == ':' || c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline int is_name_char(const char c)
{
  return is_name_start_char(c) || c == '-' || c == '.' || (c >= '0' && c <= '9');
}

/* Opens a tag whose name is in TagName field
 * Attributes will be pushed to this tag
 * Called at first opening '<'
 */
static void XML_BeginTag(XML_CONTEXT *ctx)
{
  ctx->CurrentTag = XML_AllocNode();
  ctx->CurrentTag->name = strdup(ctx->TagName);
}

/* Completes the tag and pushes it into the stack as an open one
 * Tags encountered later will be in this tag's subtree
 * Called at '>' of the start-tag
 */
static void XML_PushTag(XML_CONTEXT *ctx)
{
  if (ctx->History[ctx->pHistory])
    ctx->History[ctx->pHistory]->next = ctx->CurrentTag;
  else if (ctx->pHistory>0)
    ctx->History[ctx->pHistory-1]->subnode = ctx->CurrentTag;
  else
    ctx->Root = ctx->CurrentTag;
  ctx->History[ctx->pHistory] = ctx->CurrentTag;

  ctx->pHistory++;
  ctx->History[ctx->pHistory] = NULL;
  ctx->CurrentTag = NULL;
}

/* Pops the tag whose name is in TagName field
 * The tag is considered closed from this moment
 * Called at '>' of the end-tag
 */
static void XML_PopTag(XML_CONTEXT *ctx)
{
  ctx->pHistory--;
}

/* Convenience function
 * Pushes a start-tag, pops an end-tag, push-pops an empty tag
 */
static inline void XML_FinishTag(XML_CONTEXT *ctx)
{
  if (ctx->TagState == TS_EMPTY || ctx->TagState == TS_START)
    XML_PushTag(ctx);
  if (ctx->TagState == TS_EMPTY || ctx->TagState == TS_END)
    XML_PopTag(ctx);
}

/* Pushes an attribute key-value pair into current tag
 * Called at the end of an attribute pair
 */
static void XML_PushAttribute(XML_CONTEXT *ctx)
{
  XMLAttr *attr = XML_AllocAttr();
  attr->name = strdup(ctx->AttrName);
  attr->value = strdup(ctx->AttrValue);
  attr->next = ctx->CurrentTag->attr;
  ctx->CurrentTag->attr = attr;
}

XML_CONTEXT *XML_CreateContext()
{
  XML_CONTEXT *ctx = malloc(sizeof(XML_CONTEXT));

  ctx->TagName = malloc(MAX_TAGNAME_LENGTH);
  ctx->AttrName = malloc(MAX_ATTRNAME_LENGTH);
  ctx->AttrValue = malloc(MAX_ATTRVALUE_LENGTH);

  ctx->CurrentTag = NULL;
  ctx->Root = NULL;
  ctx->pHistory = 0;
  ctx->History[0] = NULL;

  ctx->MSState = MS_BEGIN;
  ctx->TagState = TS_INDEFINITE;

  return ctx;
}

void XML_DestroyContext(XML_CONTEXT *ctx)
{
  free(ctx->TagName);
  free(ctx->AttrName);
  free(ctx->AttrValue);
  free(ctx);
}

void XML_Decode(XML_CONTEXT *ctx, char *buf, int size)
{
  char c_as_string[2]=".\0";
  int i = 0;

  while (ctx->MSState!=MS_ERROR && i<size)
  {
    const char c = buf[i];
    c_as_string[0]=c;

    switch (ctx->MSState)
    {
    /* "   <root-tag ..."
     *    -^
     */
    case MS_BEGIN:
      if (c == '<')
        ctx->MSState = MS_BEGINTAG;
      /* else if (is_space(c)) 
       * skip char */
      else if (!is_space(c))
        ctx->MSState = MS_ERROR;
      break;

    /* "<tag ..."
     * "<? ..."
     * "</tag ..."
     *  -^
     */
    case MS_BEGINTAG:
      strcpy(ctx->AttrName,"");
      if (is_name_start_char(c))
      {
        ctx->MSState = MS_TAGNAME;
        ctx->TagState = TS_START;
        strcpy(ctx->TagName, c_as_string);
      }
      else if (c == '?')
        ctx->MSState = MS_PROCESSING_INSTRUCTION;
      else if (c == '/')
      {
        ctx->TagState = TS_END;
        ctx->MSState = MS_SLASHTAG;
      }
      else
        ctx->MSState = MS_ERROR;
      break;

    /* "<? ... ?>
     *   -^
     */
    case MS_PROCESSING_INSTRUCTION:
      if (c == '?')
        ctx->MSState = MS_ENDTAG;
      /* else
       *   skip char */
      break;

    /* "</tag ..."
     *   -^
     */
    case MS_SLASHTAG:
      if (is_name_start_char(c))
      {
        ctx->MSState = MS_TAGNAME;
        strcpy(ctx->TagName, c_as_string);
      }
      else
        ctx->MSState = MS_ERROR;
      break;

    /* "<sometag ..."
     * "<tag/> ..."
     * "<tag> ..."
     *     -^
     */
    case MS_TAGNAME:
      if (is_name_char(c))
        strcat(ctx->TagName, c_as_string);
      else if (is_space(c))
      {
        ctx->MSState = MS_ENDTAGNAME;
        XML_BeginTag(ctx);
      }
      else if (c == '/')
      {
        XML_BeginTag(ctx);

        ctx->MSState = MS_ENDTAG;
        ctx->TagState = TS_EMPTY;
      }
      else if (c == '>')
      {
        ctx->MSState = MS_TEXT;
        XML_BeginTag(ctx);
        XML_FinishTag(ctx);
      }
      else
        ctx->MSState = MS_ERROR;
      break;

    /* "<tag a='b' ..."
     * "<tag /> ..."
     * "<tag > ..."
     * "<tag   ..."
     *      -^
     */
    case MS_ENDTAGNAME:
      if (is_name_start_char(c))
      {
        ctx->MSState = MS_ATTRIBNAME;
        strcpy(ctx->AttrName,c_as_string);
      }
      else if (c == '/')
      {
        ctx->MSState = MS_ENDTAG;
        ctx->TagState = TS_EMPTY;
      }
      else if (c == '>')
      {
        ctx->MSState = MS_TEXT;
        XML_FinishTag(ctx);
      }
      else if (!is_space(c))
        ctx->MSState = MS_ERROR;
      break;

    /* "attribute='value'..."
     * "attr='value'..."
     * "attr ='value' ..."
     *     -^
     */
    case MS_ATTRIBNAME:
      if (is_name_char(c))
        strcat(ctx->AttrName, c_as_string);
      else if (c == '=')
        ctx->MSState = MS_ENDEQUALLY;
      else if (is_space(c))
        ctx->MSState = MS_ENDATTRIBNAME;
      else
        ctx->MSState = MS_ERROR;
      break;

    /* "attr    = ..."
     * "attr = ..."
     *      -^
     */
    case MS_ENDATTRIBNAME:
      if (c == '=')
        ctx->MSState = MS_ENDEQUALLY;
      /* else if (is_space(c))
       *   skip char */
      else if (!is_space(c))
        ctx->MSState = MS_ERROR;
      break;

    /* "attr='value' ..."
     * "attr=  'value' ..."
     *      -^
     */
    case MS_ENDEQUALLY:
      if ((c == '\"')||(c == '\''))
      {
        ctx->MSState = MS_ATTRIBVALUE;
        strcpy(ctx->AttrValue,"");
      }
      /* else if (is_space(c))
       *   skip char */
      else if (!is_space(c))
        ctx->MSState = MS_ERROR;
      break;

    /* "attr='abcabcabcabcba ..."
     * "attr='ab' ..."
     *         -^
     */
    case MS_ATTRIBVALUE:
      if ((c == '\"')||(c == '\''))
      {
        ctx->MSState = MS_ENDTAGNAME;
        XML_PushAttribute(ctx);
      }
      else
        strcat(ctx->AttrValue, c_as_string);
      break;

    /* "... > ..."
     *     -^
     */
    case MS_ENDTAG:
      if (c == '>')
      {
        ctx->MSState = MS_TEXT;
        XML_FinishTag(ctx);
      }
      else
        ctx->MSState = MS_ERROR;
      break;

    /* "... > ... < ..."
     *        -^
     */
    case MS_TEXT:
      if (c == '<')
      {
        ctx->MSState = MS_BEGINTAG;
        ctx->TagState = TS_INDEFINITE;
      }
      break;
    }
    i++;
  }
}


/*
  Получить значение атрибута по его имени
IN: char* req_attr_name - название атрибута
    XMLAttr* attr_list  - список атрибутов тега
OUT: значение атрибута или NULL
*/
char* XML_Get_Attr_Value(char* req_attr_name, XMLAttr* attr_list)
{
  XMLAttr* attr_Ex = attr_list;
  while(attr_Ex)
  {
    if(attr_Ex->name)
      if(!strcmp(req_attr_name, attr_Ex->name)) return attr_Ex->value;
    attr_Ex = attr_Ex->next;
  }
  return NULL;
}

/*
  Получить дочерний узел из списка дочерних узлов по его имени
IN: XMLNode* node         - родительский узел
    char* req_node_name   - имя требуемого узла
OUT: дочерний узел или NULL
*/
XMLNode* XML_Get_Child_Node_By_Name(XMLNode* node, char* req_node_name)
{
  XMLNode* nodeEx = node->subnode;
  while(nodeEx)
  {
    if(nodeEx->name)
      if(!strcmp(req_node_name, nodeEx->name)) return nodeEx;
    nodeEx = nodeEx->next;
  }
  return NULL;
}

/*
  Получить дочерний узел из списка дочерних узлов по его имени,
  при условии, что существует заданный атрибут с заданным значением
IN: XMLNode* node         - родительский узел
    char* req_node_name   - имя требуемого узла
    char* req_attr_name   - имя атрибута
    char* req_attr_velue  - значение атрибута
OUT: дочерний узел или NULL
*/
XMLNode* XML_Get_Child_Node_By_Name_And_Attr(XMLNode* node, char* req_node_name, char* req_attr_name, char* req_attr_velue)
{
  XMLNode* nodeEx = node->subnode;
  while(nodeEx)
  {
    if(nodeEx->name)
      if(!strcmp(req_node_name, nodeEx->name))    // Если найден нод с нужным именем
      {
        char *attr_val = XML_Get_Attr_Value(req_attr_name, nodeEx->attr);
        if(attr_val)    // Если есть требуемый атрибут
        {
          if(!strcmp(attr_val, req_attr_velue))return nodeEx;
        }
      }
    nodeEx = nodeEx->next;
  }
  return NULL;
}

//EOL,EOF
