#include <ctype.h>

#include "defs.h"


int is_pathend(char);
int is_space(char);
int is_newcmd(char);

extern int LINES, COLS, INPUT_RETURN_MODE;
int INPUT_RETURN_MODE;

void call_proc(DIR_LIST *dl)
{
    int     i, _input_flag, _errflag, j, x, tag, expand, expanded;
    int     PMODE;
    char   *prompt_str, c, *t, *s;
    char   *str;
    char    tmp[1024], *exec_str, *p;
    int    *tagged;

    tagged = malloc(sizeof(int) * dl->fl->no_of_files);
    for (i = 0 ; i < dl->fl->no_of_files ; i++)
	tagged[i] = dl->tagged[i];

    tag = TRUE;
    j = _input_flag = _errflag = 0;
    for (x = 0 ; x < Proc->pl[Proc->now].exec_cnt ; x++) {
	exec_str = NULL;
	expand = FALSE;
	expanded = FALSE;
	str = strenv(Proc->pl[Proc->now].exec[x].str);
	PMODE = Proc->pl[Proc->now].exec[x].mode;
	p = str;
	while (*p) {
	    if (*p == '%' && *p+1 != '%' && *p+1 != 0) {
		expand = TRUE;
		switch(*++p) {
		case 'n':
		    exec_str = dstrcat(exec_str, dl->fl->fl[dl->flpos].name);
		    exec_str = dstrcat(exec_str, " ");
		    expanded = TRUE;
		    p++;
		    break;
	  
		case 't':
		    if (!dl->no_of_files_tagged) {
			put_msg(NEED_TAGGED_FILES);
			if (exec_str != NULL)
			    free(exec_str);
			free(str);
			free(tagged);
			if (_input_flag)
			    put_proc_help();
			return;
		    }
		    if (Proc->pl[Proc->now].exec[x].list == NULL) {
			for (i = 0 ; i < dl->fl->no_of_files ; i++) {
			    if (tagged[i]) {
				exec_str = dstrcat(exec_str, dl->fl->fl[i].name);
				exec_str = dstrcat(exec_str, " ");
				tagged[i] = 0;
				expanded = TRUE;
			    }
			}
		    } else {
			for (i = 0 ; i < dl->fl->no_of_files ; i++) {
			    for (j = 0 ; Proc->pl[Proc->now].exec[x].list[j] != NULL ; j++) {
				if (tagged[i] && (match(dl->fl->fl[i].name, Proc->pl[Proc->now].exec[x].list[j]))) {
				    exec_str = dstrcat(exec_str, dl->fl->fl[i].name);
				    exec_str = dstrcat(exec_str, " ");
				    tagged[i] = 0;
				    expanded = TRUE;
				}
			    }
			}
		    }
		    p++;
		    break;
	
		case 'f':
		    if (dl->no_of_files_tagged) {
			if (Proc->pl[Proc->now].exec[x].list == NULL) {
			    for (i = 0 ; i < dl->fl->no_of_files ; i++) {
				if (tagged[i]) {
				    exec_str = dstrcat(exec_str, "\"");
				    exec_str = dstrcat(exec_str, dl->fl->fl[i].name);
				    exec_str = dstrcat(exec_str, "\" ");
				    tagged[i] = 0;
				    expanded = TRUE;
				}
			    }
			} else {
			    for (i = 0 ; i < dl->fl->no_of_files ; i++) {
				for (j = 0 ; Proc->pl[Proc->now].exec[x].list[j] != NULL ; j++) {
				    if (tagged[i] && (match(dl->fl->fl[i].name, Proc->pl[Proc->now].exec[x].list[j]))) {
					exec_str = dstrcat(exec_str, "\"");
					exec_str = dstrcat(exec_str, dl->fl->fl[i].name);
					exec_str = dstrcat(exec_str, "\" ");
					tagged[i] = 0;
					expanded = TRUE;
				    }
				}
			    }
			}
		    } else {
			if (Proc->pl[Proc->now].exec[x].list == NULL) {
			    if (tag) {
				exec_str = dstrcat(exec_str, "\"");
				exec_str = dstrcat(exec_str, dl->fl->fl[dl->flpos].name);
				exec_str = dstrcat(exec_str, "\" ");
				tag = FALSE;
				expanded = TRUE;
			    }
			} else {
			    for (j = 0 ; Proc->pl[Proc->now].exec[x].list[j] != NULL ; j++) {
				if (tag && match(dl->fl->fl[dl->flpos].name, Proc->pl[Proc->now].exec[x].list[j])) {
				    exec_str = dstrcat(exec_str, "\"");
				    exec_str = dstrcat(exec_str, dl->fl->fl[dl->flpos].name);
				    exec_str = dstrcat(exec_str, "\" ");
				    tag = FALSE;
				    expanded = TRUE;
				}
			    }
			}
		    }
		    p++;
		    break;
	
		case 'p':
		    if (*(p + 1) != '{' || (!strchr(p + 2, '}'))) {
			put_proc_help();
			put_msg(P_NEEDS_PARENTHESES);
			if (exec_str != NULL)
			    free(exec_str);
			free(tagged);
			free(str);
			return;
		    }
		    prompt_str = malloc(strlen(str)+1);
		    j = 0;
		    p += 2;
		    while (*p != '}')
			prompt_str[j++] = *p++;
		    p++;
		    prompt_str[j] = 0;
		    gotoxy(0, 1);
		    clrtoeol();
		    _input_flag = 1;
		    s = input(prompt_str);
		    free(prompt_str);
		    if (!s && INPUT_RETURN_MODE != I_ENTER) {
			put_proc_help();
			if (exec_str != NULL)
			    free(exec_str);
			free(tagged);
			free(str);
			return;
		    }
		    t = strenv(s);
		    if (s)
			free(s);
		    exec_str = dstrcat(exec_str, t);
		    exec_str = dstrcat(exec_str, " ");
		    if (t)
			free(t);
		    expanded = TRUE;
		    break; 
		}
	    } else {
		tmp[0] = *p++;
		tmp[1] = 0;
		exec_str = dstrcat(exec_str, tmp);
	    }
	}

	if (expand == FALSE || (expand == TRUE && expanded == TRUE)) {
	    if (PMODE & P__SURE) {
		gotoxy(0, 1);
		_input_flag = 1;
		fputs(ARE_YOU_SURE_PROMPT, stdout);
		clrtoeol();
		while ((c = _ikey()) != KEY_FOR_NO && c != KEY_FOR_YES)
		    ;
		if (c == KEY_FOR_NO) {
		    put_proc_help();
		    if (exec_str != NULL)
			free(exec_str);
		    free(tagged);
		    free(str);
		    return;
		}
	    }	
      
	    if (PMODE & P__CLS)
		clear();
	    else 
		gotoxy(0, LINES - 1);
      
	    chdir(dl->cwd);
      
	    /* for example, gnu-emacs looks in the enviromentvariable PWD. 
	       this is for such a case like gnu-emacs */
	    {
		char *pwd = malloc(strlen(dl->cwd) + 5);
	
		sprintf(pwd, "PWD=%s", dl->cwd);
		putenv(pwd);
		free(pwd);
	    }
    
	    _errflag = system(exec_str);
    
	    for (i = 0 ; i < dl->fl->no_of_files ; i++)
		dl->tagged[i] = 0;
	    dl->no_of_files_tagged = 0;
      
	    if (PMODE & P__UPDATE) {
		dl = dl_newdir(dl, NEWDIR_KEEP);
		dl = dl_layout(dl);
		if (!(PMODE & P__CLS) && !_errflag)
		    put_dir(dl);
		if (PMODE & P__SURE)
		    put_proc_help();
	    }
      
	    if (PMODE & P__WAIT || _errflag) {
		gotoxy(0, LINES-1);
		fputs(PRESS_KEY_MSG, stdout);
		fflush(stdout);
		_ikey();
	    }
      
	    if ((_input_flag && PMODE | P__CLS) && !_errflag)
		put_proc_help();  
      
	    if (PMODE & P__CLS || _errflag)
		put_screen();
	    else
		put_msg("");
	}
	free(str);
	if (exec_str != NULL)
	    free(exec_str);
    }
    free(tagged);
}
