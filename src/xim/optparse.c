// -*- coding:unix; mode:c++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
/* Unikey Vietnamese Input Method
 * Copyright (C) 2004 Pham Kim Long
 * Contact:
 *   longcz@yahoo.com
 *   UniKey project: http://unikey.sf.net
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include "optparse.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

#define OPT_COMMENT_CHAR '#'

//--------------------------------------------------
static int parseLine(char *line, char **name, char **value)
{
  char *p, *mark;
  char ch;

  if (line == 0)
    return 0;

  // get rid of comment
  p = strchr(line, OPT_COMMENT_CHAR);
  if (p)
    *p = 0;

  //get option name
  for (p=line; *p == ' '; p++);
  if (*p == 0)
    return 0;

  *name = p;
  mark = 0; //mark the last non-space character
  while ((ch=*p) != '=' && ch!=0) {
    if (ch != ' ')
      mark = p;
    p++;
  }

  if (ch == 0 && mark == 0)
    return 0;
  *(mark+1) = 0; //terminate name with a null character

  //get option value
  p++;
  while (*p == ' ') p++;
  if (*p == 0)
    return 0;

  *value = p;
  mark = p;
  while (*p) { //strip trailing spaces
    if (*p != ' ')
      mark = p;
    p++;
  }
  *++mark = 0;
  return 1;
}

//----------------------------------------------------
int parseValue(OptItem *info, void *rec, const char *strValue)
{
  char *addr = ((char *)rec)+info->offset;
  switch (info->type) {
  case BoolOpt:
    {
      int v;

      if (strcasecmp(strValue, "no") == 0 ||
	 strcasecmp(strValue, "false") == 0 ||
	 strcasecmp(strValue, "0") == 0)
	v = 0;
      else
      if (strcasecmp(strValue, "yes") == 0 ||
	 strcasecmp(strValue, "true") == 0 ||
	 strcasecmp(strValue, "1") == 0)
	v = 1;
      else
	return 0;

      *(long *)addr = v;
    }
    break;
  case StrOpt:
    {
      char **ppStr = (char **)addr;
      if (*ppStr)
	free(*ppStr);
      *ppStr = strdup(strValue);
    }
    break;
  case LookupOpt:
    {
      long v;
      OptMap *p;
      for (p = info->lookup; p->name; p++) {
	if (strcasecmp(p->name, strValue) == 0) {
	  v = p->value;
	  break;
	}
      }
      if (!p->name)
	return 0;
      *(long *)addr = v;
    }
    break;
  case LongOpt:
  default:
    *(long *)addr = strtol(strValue, 0, 0);
  }

  return 1;
}

//----------------------------------------------------
int ParseOptFile(const char *fileName, void *optRec, OptItem *optList, int count)
{
  FILE *f;
  char *buf, *name, *value;
  int bufSize, len, i;

  f = fopen(fileName, "r");
  if (f == 0) {
    fprintf(stderr, "Failed to open file: %s\n", fileName);
    return 0;
  }

  bufSize = 256;
  buf = (char *)malloc(bufSize);

  while (!feof(f)) {
  /* FreeBSD doesn't have getline, so don't use this
    if ((len = getline(&buf, &bufSize, f)) == -1)
      break;
  */
    if (fgets(buf, bufSize, f) == 0)
      break;

    len = strlen(buf);
    if (len == 0)
      break;

    if (buf[len-1] == '\n')
      buf[len-1] = 0;
    if (parseLine(buf, &name, &value)) {
      for (i=0; i<count; i++) {
	if (strcasecmp(optList[i].name, name) == 0) {
	  parseValue(&optList[i], optRec, value);
	  break;
	}
      }
    }
  }
  free(buf);
  fclose(f);
  return 1;
}

//----------------------------------------------------
int ParseExpandFileName(const char *name, char **expandedName)
{
  char *tmp, *path, *homeDir;
  struct passwd *user;
  int bufSize;

  if (name == 0 || name[0] != '~') 
    return 0;

  tmp = strdup(name);
  if (!tmp)
    return 0;

  if (tmp[1] == '/') {
    homeDir = getenv("HOME");
    path = tmp+1;
  }
  else if (tmp[1] == 0) {
    homeDir = getenv("HOME");
    path = "";
  }
  else {
    char *p = strchr(tmp+1, '/');
  
    if (p) 
      *p = 0;
    user = getpwnam(tmp+1);
    
    if (user)
      homeDir = user->pw_dir;
    else {
      free(tmp);
      return 0;
    }

    if (p) {
      *p = '/'; //restore previously overwritten slash
      path = p;
    }
    else path="";
  }

  bufSize = strlen(homeDir) + strlen(path) + 1;
  *expandedName = (char *)malloc(bufSize);
  if (!*expandedName) {
    free(tmp);
    return 0;
  }

  strcpy(*expandedName, homeDir);
  strcat(*expandedName, path);
  free(tmp);
  return 1;
}

//----------------------------------------------------
void writeValue(FILE *f, OptItem *optInfo, void *rec)
{
  void *addr = ((char *)rec)+optInfo->offset;

  fputs(optInfo->comment, f);

  switch (optInfo->type) {

  case BoolOpt:
    fprintf(f, "%s = %s\n\n", 
	    optInfo->name, 
	    (*(long *)addr) ? "Yes" : "No");
    break;

  case StrOpt:
    if (*(char **)addr)
      fprintf(f, "%s = %s\n\n", optInfo->name,
	     *(char **)addr);
    else {
      //      fprintf(stderr, "strange\n");
      fprintf(f, "#%s = \n\n", optInfo->name);
    }

    break;

  case LookupOpt:
    {
      OptMap *p;
      long v = *(int *)addr;

      for (p = optInfo->lookup; p->name && p->value != v; p++);
      if (p->name)
	fprintf(f, "%s = %s\n\n", optInfo->name, p->name);
    }
    break;
  case LongOpt:
  default:
    fprintf(f, "%s = %ld\n\n", optInfo->name, *(long *)addr);
  }
}

//----------------------------------------------------
int ParseWriteOptFile(const char *fileName, const char *header,
		    void *optRec, OptItem *optList, int count)
{
  FILE *f;
  int i;

  f = fopen(fileName, "w");
  if (!f)
    return 0;

  fputs(header, f);
  for (i=0; i<count; i++) {
    writeValue(f, &optList[i], optRec);
  }

  fclose(f);
  return 1;
}
