#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>





char **wordlist(char *str, char **arg, int *cnt, int (*trunc)(char c))
{
    char *tmp = malloc(strlen(str)+1);
    int i = 0, j = 0;
  
    if (arg == NULL) {
	arg = malloc(sizeof(char *));
	arg[0] = NULL;
    }
  
    while (*str) {
	while((*trunc)(*str) && *str && *str != '"') 
	    str++;

    

	if (*str == '\0') 
	    goto start;

	while (!(*trunc)(*str) && *str && *str != '"')
	    tmp[i++] = *str++;
	if (*str == '"') {
	    str++;
	    while(*str && *str != '"') {
		if (*str == '\\' && *str+1 != 0)
		    str++;
		tmp[i++] = *str++;
	    }
	    str++;
	}
	tmp[i] = 0;
	if (arg[j] == NULL) 
	    arg[j] = malloc(i + 1); 
	else 
	    arg[j] = realloc(arg[j], i + 1);
	strcpy(arg[j++], tmp);
	i = 0;

    start:

	if (j > *cnt) {
	    arg = realloc(arg, sizeof(char *) * (j + 1)); 
	    arg[j] = NULL;
	}
    }
    if (j < *cnt) {
	for (i = j ; i < *cnt ; i++) {
	    free(arg[i]);
	    arg[i] = NULL;
	}
	arg = realloc(arg, sizeof(char *) * (j + 1));
    }
    *cnt = j;

    return arg;
}


/**********************************************************************/


/*
 * frees a char array (char **); last element must be NULL
 */

char **free_char_array(char **a)
{
    int i;

    if (a == NULL)
	return NULL;
  
    for (i = 0 ; a[i] != NULL ; i++) {
	free(a[i]);
    }
    free(a);
    a = NULL;

    return NULL;
}


/**************************************************************************/

int is_space(char c)
{
    return isspace(c);
}


/**************************************************************************/

int is_newcmd(char c)
{
    return c==';'?1:0;
}

/**************************************************************************/

int is_pathend(char c)
{
    return c == ':' || c == ' ' ? 1 : 0;
}


/**************************************************************************/



int parser(char *cmd, char **cmd_list, char ***arglist, int *argcount)
{
    char **arg = *arglist;
    int cnt = *argcount;
    register int i;

    if ( cmd[0] == '\0' )
	return -2; 

    arg = wordlist(cmd, arg, &cnt, is_space);

    for (i = 0 ; cmd_list[i][0] != 0 ; i++ ) {
	if (!strcmp(arg[0],cmd_list[i])) {
	    *arglist = arg;
	    *argcount = cnt;
	    return i;
	}
    }
    *arglist = arg;
    *argcount = cnt;
    return -1;    /* Falsches Kommando */
}
