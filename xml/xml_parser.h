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

#define MAX_XML_TREE_DEPTH 100

typedef struct
{
  STRBUF *TagName;
  STRBUF *AttrName;
  STRBUF *AttrValue;

  short TagState;
  short MSState;

  XMLNode *Root;
  XMLNode *History[MAX_XML_TREE_DEPTH];
  int pHistory;

  XMLNode *CurrentTag;
} XML_CONTEXT;

XML_CONTEXT *XML_CreateContext();
void XML_DestroyContext(XML_CONTEXT *ctx);

// Декодировать поток
void XML_Decode(XML_CONTEXT *ctx, char *buf, int size);

////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  MS_BEGIN,					// Document start
  MS_BEGINTAG,					// Tag open bracket
  MS_PROCESSING_INSTRUCTION, // Processing Instruction, "<? ... ?>"
  MS_SLASHTAG,					// End-tag's slash
  MS_TAGNAME,						// Start-tag's or empty-tag's name
  MS_ENDTAGNAME,					// Whitespace after tag name
  MS_ATTRIBNAME,				// Attribute name
  MS_ENDATTRIBNAME,				// Whitespace after attribute name
  MS_ENDEQUALLY,					// Equality sign in an attribute pair
  MS_ATTRIBVALUE,					// Attribute value inside quotes
  MS_ENDTAG,					// Tag close bracket
  MS_TEXT,						// Tag text
  MS_ERROR  // Parser error
} XML_PARSER_STATE;

typedef enum
{
  TS_INDEFINITE,					// Неопределенный, т.к. тег только начали парсить (если начали)
  TS_START,						// Обычный (не)закрывающийся. Точно не декларация
  TS_END,						// Закрывающийся
  TS_EMPTY						// Сам и закрывается
} TAG_TYPE;

#endif
