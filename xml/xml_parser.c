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

#include "util/strbuf.h"
#include "xml_common.h"
#include "xml_parser.h"
#include "xml_memory.h"

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
  ctx->CurrentTag->name = ctx->TagName;
  ctx->TagName = NULL;
}

/* Completes the tag and pushes it into the stack as an open one
 * Tags encountered later will be in this tag's subtree
 * Called at '>' of the start-tag
 */
static void XML_PushTag(XML_CONTEXT *ctx)
{
  if (ctx->History[ctx->pHistory]) /* There's a tag at this level, add a sibling to it */
    ctx->History[ctx->pHistory]->next_sibling = ctx->CurrentTag;
  else if (ctx->pHistory==0)  /* Root tag */
    ctx->Root = ctx->CurrentTag;
  else if (ctx->pHistory>1) /* Stanza's subtree */
    ctx->History[ctx->pHistory-1]->first_child = ctx->CurrentTag;
  /* Stanzas are not linked to root */

  ctx->History[ctx->pHistory] = ctx->CurrentTag;

  /* stream header completed */
  if (ctx->pHistory==0 && ctx->onStreamBegin!=NULL)
    ctx->onStreamBegin(ctx->History[0]);

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

  /* level 1: stanza */
  if (ctx->pHistory==1 && ctx->onStanza!=NULL)
  {
    ctx->onStanza(ctx->History[1]);
    ctx->History[1] = NULL; /* Forget about this subtree */
  }

  /* level 0: stream header */
  if (ctx->pHistory==0 && ctx->onStreamEnd!=NULL)
    ctx->onStreamEnd(ctx->History[0]);
}

/* Convenience function
 * Pushes a start-tag, pops an end-tag, push-pops an empty tag
 */
static inline void XML_FinishTag(XML_CONTEXT *ctx)
{
  if (ctx->TagState == TS_EMPTY || ctx->TagState == TS_START)
    XML_PushTag(ctx);
  if (ctx->TagState == TS_EMPTY || ctx->TagState == TS_END)
  {
    XML_PopTag(ctx);
    if (ctx->TagState == TS_END)
    {
      XML_DestroyTree(ctx->CurrentTag);
      ctx->CurrentTag = NULL;
    }
  }
}

/* Pushes an attribute key-value pair into current tag
 * Called at the end of an attribute pair
 */
static void XML_PushAttribute(XML_CONTEXT *ctx)
{
  XMLAttr *attr = XML_AllocAttr();

  attr->name = ctx->AttrName;
  ctx->AttrName = NULL;
  attr->value = ctx->AttrValue;
  ctx->AttrValue = NULL;

  attr->next = ctx->CurrentTag->attr;
  ctx->CurrentTag->attr = attr;
}

XML_CONTEXT *XML_CreateContext()
{
  XML_CONTEXT *ctx = calloc(1, sizeof(XML_CONTEXT));

  ctx->MSState = MS_BEGIN;
  ctx->TagState = TS_INDEFINITE;

  return ctx;
}

void XML_DestroyContext(XML_CONTEXT *ctx)
{
  strbuf_free(ctx->TagName);
  strbuf_free(ctx->AttrName);
  strbuf_free(ctx->AttrValue);

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
      if (is_name_start_char(c))
      {
        ctx->MSState = MS_TAGNAME;
        ctx->TagState = TS_START;
        strbuf_xappend(&ctx->TagName, c, STRBUF_SIZE_SMALL);
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
        strbuf_xappend(&ctx->TagName, c, STRBUF_SIZE_SMALL);
        ctx->MSState = MS_TAGNAME;
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
        strbuf_xappend(&ctx->TagName, c, STRBUF_SIZE_SMALL);
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
        strbuf_xappend(&ctx->AttrName, c, STRBUF_SIZE_SMALL);
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
        strbuf_xappend(&ctx->AttrName, c, STRBUF_SIZE_SMALL);
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
        ctx->MSState = MS_ATTRIBVALUE;
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
        strbuf_xappend(&ctx->AttrValue, c, STRBUF_SIZE_SMALL);
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
      else if (ctx->pHistory>0) /* inside a tag */
        strbuf_xappend(&(ctx->History[ctx->pHistory-1]->text), c, STRBUF_SIZE_MEDIUM);
      break;
    }
    i++;
  }
}

