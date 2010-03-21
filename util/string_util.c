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

#include "string_util.h"

#include <assert.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif
/////////////////////////////////////////// Разный стафф для замены спецсимволов

// Структура, описывающая, что на что менять
typedef struct
{
  char mask[7];
  char replace;
}REPL_ARRAY;

// Сами замены и их количество
const unsigned short Repl_chars_count = 5;
REPL_ARRAY Repl_chars[] = {{"&apos;\0",0x27},
                           {"&quot;\0",'"'},
                           {"&lt;\0\0\0", '<'},
                           {"&gt;\0\0\0", '>'},
                           {"&amp;\0\0", '&'},
};

/*
    Получить спецсимвол по его маске
IN: mask_begin - строка символов
    out_ofs - число, к которому прибавится длина обработанной последовательности
OUT: out_ofs - смещение в строке, откуда начинаются необработанные данные
    <return> - спецсимвол
*/
char GetSpecialSym(char *mask_begin, size_t *out_ofs)
{
  unsigned short i=0;
  size_t replen;
  char rep_ex[10];
  if(*mask_begin!='&')return *(mask_begin);
  for(i=0;i<Repl_chars_count;i++)
  {
    replen = strlen(Repl_chars[i].mask);  // Определяем длину очередной маски
    memset(rep_ex, 0, 10);
    strncpy(rep_ex,mask_begin,replen);    // Копируем строку такой длины с текущей позиции
    if(!strcmp(rep_ex,Repl_chars[i].mask))// Если совпало с очередной маской
    {
      *out_ofs+=replen-1;                   // Увеличиваем обработанную длину на длину маски
      return Repl_chars[i].replace;       // Возвращаем символ для замены
    }
  }
  return *(mask_begin);       //  Масок не нашлось, возвращаем как есть
}

int GetSpecialSymMaskN(char sym)
{
  int i;

  for(i=0; i<Repl_chars_count; i++)
    if(Repl_chars[i].replace==sym)
      return i;
  return -1;
}

char * Replace_Special_Syms(char * unrep_str)
{
  unsigned int unrep_len=strlen(unrep_str);
  char *rep_buffer = malloc(unrep_len+1);
  memset(rep_buffer, 0, unrep_len+1);
  char tmp=0;
  unsigned int rpl_c=0;
  size_t j;
  for(j=0;j<unrep_len;j++)
  {
    tmp = *(unrep_str+j);
    tmp = GetSpecialSym(unrep_str+j,&j);
    //ShowMSG(1,(int)"fnd");
    rep_buffer[rpl_c++]=tmp;
    assert(rpl_c < unrep_len + 1);
  }
  rep_buffer = realloc(rep_buffer,rpl_c+1);
  return rep_buffer;
}

char * Mask_Special_Syms(const char * unrep_str)
{
  unsigned int unrep_len = strlen(unrep_str);
  unsigned int rep_buffer_size = unrep_len*2+16;
  char *rep_buffer = malloc(rep_buffer_size);
  unsigned int c_pos = 0, i;
  for(i=0; i<unrep_len; i++)
  {
    int n=GetSpecialSymMaskN(unrep_str[i]);
    if(n!=-1)
    {
      strcpy(rep_buffer+c_pos, Repl_chars[n].mask);
      c_pos += strlen(Repl_chars[n].mask);
    }
    else
    {
      rep_buffer[c_pos++] = unrep_str[i];
    }
    if (c_pos+10>rep_buffer_size)
    {
      rep_buffer_size *= 2;
      rep_buffer = realloc(rep_buffer, rep_buffer_size);
    }
  }
  rep_buffer[c_pos] = '\0';
  rep_buffer = realloc(rep_buffer, c_pos+1);
  return rep_buffer;
}

char*
getBareJid(char *full_jid)
{
    char *slash;
    char *bare_jid;
    
    slash = strstr(full_jid, "/");
    if(!slash)
	return (NULL);
    size_t jid_len = (size_t) (slash - full_jid);
    if(!(bare_jid = malloc(jid_len + 1))) {
	perror("Cannot allocate memory for JID");
	return (NULL);
    }
    memcpy(bare_jid, full_jid, jid_len);
    bare_jid[jid_len] = '\0';
    return (bare_jid);
}
