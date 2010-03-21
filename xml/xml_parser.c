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

#include "../util/string_util.h"
#include "xml_common.h"
#include "xml_parser.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/*
 * Char classes
 * (From XML RFC, http://www.w3.org/TR/2008/REC-xml-20081126/#NT-Name )
 * (Only implementing ASCII subset)
 * S             ::=    (#x20 | #x9 | #xD | #xA)+
 * NameStartChar ::=    ":" | [A-Z] | "_" | [a-z] 
 * NameChar      ::=    NameStartChar | "-" | "." | [0-9]
 * Name          ::=    NameStartChar (NameChar)*
 * */

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

char _crlf[2] = {0xd,0xa};

void EndTag(XML_CONTEXT *ctx)
{
  if((ctx->TagState == TS_NORMAL)||(ctx->TagState == TS_EMPTY))
  {
    ctx->tmp=malloc(sizeof(XMLNode));
    memset(ctx->tmp, 0, sizeof(XMLNode));
    if (!ctx->XMLTree)
    {
      ctx->XMLTree=ctx->tmp;
    }
    else
    {
      if(!ctx->IsClosed)ctx->current->next=ctx->tmp;
      else ctx->current->subnode=ctx->tmp;
    }
    ctx->tmp->name=malloc(strlen(ctx->TagName)+1);
    if(ctx->attribs)ctx->tmp->attr=ctx->attribs;
    ctx->attribs=0;
    strcpy(ctx->tmp->name,ctx->TagName);
    *(ctx->TreeBranchs+ctx->BranchPos)=(size_t)ctx->tmp;
    ctx->BranchPos++;
    ctx->current=ctx->tmp;
    ctx->curattr=0;
    ctx->IsClosed=1;
  }
  if((ctx->TagState == TS_EMPTY)||(ctx->TagState == TS_CLOSE))
  {
    if(ctx->BranchPos>0)
    {
      ctx->BranchPos--;
      ctx->current=(void*)(*(ctx->TreeBranchs+ctx->BranchPos));
    }
    ctx->IsClosed=0;
  }
}

void EndAttr(XML_CONTEXT *ctx)
{
  XMLAttr *tmp2=malloc(sizeof(XMLAttr));
  memset(tmp2, 0, sizeof(XMLAttr));
  tmp2->name=malloc(strlen(ctx->AttrName)+1);
  strcpy(tmp2->name,ctx->AttrName);
  if(strlen(ctx->AttrValue)>0)
  {
    char *newattrval = Replace_Special_Syms(ctx->AttrValue);
    tmp2->param=malloc(strlen(newattrval)+1);
    strcpy(tmp2->param,newattrval);
    free(newattrval);
  }

  if(!ctx->curattr)
  {
    ctx->attribs=tmp2;
  }
  else
  {
    ctx->curattr->next=tmp2;
  }
  ctx->curattr=tmp2;
}

void DestroyTree(XML_CONTEXT *ctx, XMLNode *tmpp)
{
  while(tmpp)
  {
    XMLAttr *ta=tmpp->attr;
    while(ta)
    {
      tmpp->attr=ta->next;
      if(ta->name)free(ta->name);
      if(ta->param)free(ta->param);
      free(ta);
      ta=tmpp->attr;
    }
    if(tmpp->subnode)
    {
      DestroyTree(ctx, tmpp->subnode);
    }
    XMLNode *tmpp2=tmpp->next;
    if(tmpp->name)free(tmpp->name);
    if(tmpp->value)free(tmpp->value);
    free(tmpp);
    tmpp=tmpp2;
  }

  ctx->XMLTree=0;
  ctx->curattr=0;
  ctx->BranchPos=0;
  ctx->current = 0;
  ctx->tmp=0;
  ctx->attribs=0;
  ctx->IsClosed=0;
}

static void Finish(XML_CONTEXT *ctx)
{
  free(ctx->TagName);
  free(ctx->AttrName);
  free(ctx->AttrValue);
  free(ctx->Text);
  free(ctx->TreeBranchs);
}

void *XMLDecode(XML_CONTEXT *ctx, char *buf, int size)
{
  ctx->TagName=malloc(size);
  ctx->AttrName=malloc(size);
  ctx->AttrValue=malloc(size);
  ctx->Text=malloc(size);
  ctx->TreeBranchs=malloc(1024);

  memset(ctx->TagName, 0, size);
  memset(ctx->AttrName, 0, size);
  memset(ctx->AttrValue, 0, size);
  memset(ctx->Text, 0, size);
  ctx->current = 0;
  ctx->MSState = MS_BEGIN;
  ctx->TagState = TS_INDEFINITE;

  char c_as_string[2]=".\0";
  int i = 0;

  while (ctx->MSState!=MS_ERROR && i<size)
  {
    const char c = buf[i];
    c_as_string[0]=c;

    switch (ctx->MSState)
    {
    case MS_BEGIN:
      if (c == '<')
        ctx->MSState = MS_BEGINTAG;
      /* else if (is_space(c)) 
       * skip char */
      else if (!is_space(c))
        ctx->MSState = MS_ERROR;
      break;

    case MS_BEGINTAG:
      strcpy(ctx->AttrName,"");
      if (is_name_start_char(c))
      {
        ctx->MSState = MS_TAGNAME;
        ctx->TagState = TS_NORMAL;
        strcpy(ctx->TagName,c_as_string);
      }
      else if (c == '?')
        ctx->MSState = MS_PROCESSING_INSTRUCTION;
      else if (c == '/')
      {
        ctx->TagState = TS_CLOSE;
        ctx->MSState = MS_MIDDLETAG;
      }
      else
        ctx->MSState = MS_ERROR;
      break;

    case MS_PROCESSING_INSTRUCTION:
      if (c == '?')
        ctx->MSState = MS_ENDTAG;
      /* else
       *   skip char */
      break;

    case MS_MIDDLETAG:
      if (is_name_start_char(c))
      {
        ctx->MSState = MS_TAGNAME;
        strcpy(ctx->TagName, c_as_string);
      }
      else
        ctx->MSState = MS_ERROR;
      break;

    case MS_TAGNAME:
      if (is_name_char(c))
        strcat(ctx->TagName, c_as_string);
      else if (is_space(c))
        ctx->MSState = MS_ENDTAGNAME;
      else if (c == '/')
      {
        ctx->MSState = MS_ENDTAG;
        ctx->TagState = TS_EMPTY;
      }
      else if (c == '>')
      {
        ctx->MSState = MS_TEXT;
        EndTag(ctx);
      }
      else
        ctx->MSState = MS_ERROR;
      break;

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
        EndTag(ctx);
      }
      else if (!is_space(c))
        ctx->MSState = MS_ERROR;
      break;

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

    case MS_ENDATTRIBNAME:
      if (c == '=')
        ctx->MSState = MS_ENDEQUALLY;
      /* else if (is_space(c))
       *   skip char */
      else if (!is_space(c))
        ctx->MSState = MS_ERROR;
      break;

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

    case MS_ATTRIBVALUE:
      if ((c == '\"')||(c == '\''))
      {
        ctx->MSState = MS_ENDTAGNAME;
        EndAttr(ctx);
      }
      else
        strcat(ctx->AttrValue, c_as_string);
      break;

    case MS_ENDTAG:
      if (c == '>')
      {
        ctx->MSState = MS_TEXT;
        EndTag(ctx);
      }
      else
        ctx->MSState = MS_ERROR;
      break;

    case MS_TEXT:
      if (c == '<')
      {
        if(strlen(ctx->Text)>0)
        {
          //ctx->tmp->value=malloc(strlen(ctx->Text)+1);
          //strcpy(ctx->tmp->value,ctx->Text);
          ctx->tmp->value = Replace_Special_Syms(ctx->Text);
        }
        strcpy(ctx->Text,"");
        ctx->MSState = MS_BEGINTAG;
        ctx->TagState = TS_INDEFINITE;
      }
      else
      {
        if ((ctx->TagState == TS_NORMAL)) 
          strcat(ctx->Text,c_as_string);
      }
      break;
    }
    i++;
  }

  Finish(ctx);
  return ctx->XMLTree;
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
      if(!strcmp(req_attr_name, attr_Ex->name)) return attr_Ex->param;
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
