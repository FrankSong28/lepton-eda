/*******************************************************************************/
/*                                                                             */
/* Setup - version 0.2.1                                                       */
/*                                                                             */
/* Copyright (C) 2002 Piotr Miarecki, sp9rve@eter.ariadna.pl                   */
/*                                                                             */
/* This program is free software; you can redistribute it and/or               */
/* modify it under the terms of the GNU General Public License                 */
/* as published by the Free Software Foundation version 2.                     */
/*                                                                             */
/* This program is distributed in the hope that it will be useful,             */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of              */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               */
/* GNU General Public License for more details.                                */
/*                                                                             */
/* You should have received a copy of the GNU General Public License           */
/* along with this program; if not, write to the Free Software                 */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */
/*                                                                             */
/*******************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "global.h"



/* static variables */
static FILE *fp = NULL;

/* static functions */
static int ConfigReadLine(char *szLine);



/*
	Functions to open and close config file
*/

int ConfigOpen(char *szName)
{
	fp = fopen(szName, "r");
	if (fp == NULL)
		return FAILURE;
	
	return SUCCESS;
}


int ConfigClose(void)
{
	int iReturn;
	
	iReturn = fclose(fp);
	if (iReturn != 0)
		return FAILURE;
	
	fp = NULL;
	
	return SUCCESS;
}



/*
	Functions to select sections: first, next and specified by name
*/

int ConfigSectionFirst(char *szName)
{
	rewind(fp);
	
	return ConfigSectionNext(szName);
}


int ConfigSectionNext(char *szName)
{
	int iFound = 0, iResult, i;
	char szLine[TEXTLEN];

	/* read lines to find the first section name */
	while (!feof(fp) && iFound == 0)
	{
		/* read line */
		iResult = ConfigReadLine(szLine);
		if (iResult != SUCCESS)
			return FAILURE;

		/* check if section */
		if (szLine[0] == '[')
		{
			iFound = 1;
			break;
		}
	}
	
	/* return with failure if no section found */
	if (iFound == 0)
		return FAILURE;
	
	/* get a name of the section */
	for (i = 1; i < strlen(szLine) && !isspace(szLine[i]) && szLine[i] != ']'; i ++)
		szName[i - 1] = szLine[i];
	szName[i - 1] = 0;
	
	return SUCCESS;
}


int ConfigSection(char *szName)
{
	int iFound = 0, iResult, i;
	char szLine[TEXTLEN], szPattern[TEXTLEN];
	
	rewind(fp);
	sprintf(szPattern, "[%s]", szName);
	for (i = 0; i < strlen(szPattern); i ++)
		szPattern[i] = toupper(szPattern[i]);
		
	
	while (!feof(fp) && iFound == 0)
	{
		iResult = ConfigReadLine(szLine);
		if (iResult != SUCCESS)
			return FAILURE;
		for (i = 0; i < strlen(szLine); i ++)
			szLine[i] = toupper(szLine[i]);

		if (strncmp(szPattern, szLine, strlen(szPattern)) == 0)
		{
			iFound = 1;
			break;
		}
	}
	
	if (iFound == 0)
		return FAILURE;
	
	return SUCCESS;
}



/*
	Functions to read parameters: next
*/

int ConfigGetNext(char *szName, char *szValue)
{
	int iFound = 0, iResult, i, j = 0;
	char szLine[TEXTLEN];
	
	while (!feof(fp) && iFound == 0)
	{
		iResult = ConfigReadLine(szLine);
		if (iResult != SUCCESS)
			return FAILURE;

		if (strlen(szLine) == 0)
			continue;
		if (szLine[0] == '#')
			continue;
		if (szLine[0] == '[')
			break;
		if (isascii(szLine[0]))
			iFound = 1;
		break;
	}
	
	if (iFound == 0)
		return FAILURE;

	for (i = 0; i < strlen(szLine) && isalnum(szLine[i]); i ++)
		szName[i] = toupper(szLine[i]);
	szName[i] = 0;
	for (; i < strlen(szLine) && (szLine[i] == '=' || isspace(szLine[i])); i ++)
		;
	for (; i < strlen(szLine); i ++)
		szValue[j ++] = szLine[i];
	szValue[j] = 0;
	
	return SUCCESS;
}



/*
	Static functions
*/

static int ConfigReadLine(char *szLine)
{
	int c = 0, j = 0;
	
	while (!feof(fp) && j < TEXTLEN)
	{
		c = fgetc(fp);
		
		if (c == '\n' || c == '\r')
		{
			//j = 0;
			break;
		}
		
		szLine[j ++] = (char) c;
	}
	
	while (!feof(fp) && (c == '\n' || c == '\r'))
		c = fgetc(fp);
	ungetc(c, fp);
	szLine[j] = 0;
	
	return SUCCESS;
}
