#include "defs.h"
#include <errno.h>


extern int LINES, COLS, NO_ALDL;
extern KEYS *keytab;
extern int KEY_MAX;

void j_home(DIR_LIST *dl)
{
    if (dl->fl->no_of_files > PAGE_ENTRIES && Y_TOP_LINE != 0)
	if (Y_TOP_LINE > (Y_SIZE_SCREEN / 2) + (Y_SIZE_SCREEN / 4)) {
	    gotoxy(0,4);
	    put_lines(dl,0,Y_SIZE_SCREEN);
	} else {
	    gotoxy(dl->oldscr.x,dl->oldscr.y);
	    fputs(DL_select.cursor_spaces, stdout);
	    if (NO_ALDL == 0) {
		insert_n_lines(4,Y_TOP_LINE);
		put_lines(dl,0,Y_TOP_LINE);
	    } else {
		gotoxy(0, 4);
		put_lines(dl,0,Y_SIZE_SCREEN);
	    }
	}
    dl->screen.x = dl->position.p_cursor;
    dl->screen.y = 4;
    dl->flpos    = 0;
}



/**************************************************************************/



void j_end(DIR_LIST *dl)
{
    if (dl->fl->no_of_files > PAGE_ENTRIES && Y_MAX != Y_BOT_LINE) {
	if (Y_BOT_LINE+((Y_SIZE_SCREEN / 2) + (Y_SIZE_SCREEN / 4)) < Y_MAX) {
	    gotoxy(0,4);
	    put_lines(dl,Y_MAX - Y_SIZE_SCREEN,Y_SIZE_SCREEN);
	    clrtobot();
	} else {
	    gotoxy(dl->oldscr.x,dl->oldscr.y);
	    fputs(DL_select.cursor_spaces, stdout);
	    if (NO_ALDL == 0) {
		delete_n_lines(4, Y_MAX - Y_BOT_LINE);
		gotoxy(0, Y_SIZE_SCREEN + 4 - (Y_MAX - Y_BOT_LINE));
		put_lines(dl, Y_BOT_LINE,Y_MAX - Y_BOT_LINE);
	    } else {
		gotoxy(0, 4);
		put_lines(dl, Y_TOP_LINE, Y_SIZE_SCREEN);
	    }
	}
    }
    dl->screen.x = ENTRY_LENGTH * (MLE - (Y_MAX * MLE - (dl->fl->no_of_files-1))) +
	dl->position.p_cursor;
    dl->screen.y = dl->fl->no_of_files < PAGE_ENTRIES ? 3 + Y_MAX : LINES-2;
    dl->flpos    = dl->fl->no_of_files - 1;
}



/**************************************************************************/



void scroll_up(DIR_LIST *dl)
{
    if (dl->screen.y - 1 < 4) {
	gotoxy(dl->oldscr.x,dl->oldscr.y);
	fputs(DL_select.cursor_spaces, stdout);
	if (NO_ALDL == 0) {
	    gotoxy(0, LINES - 2);
	    clrtoeol();
	    gotoxy(0, 4);
	    insertln();
	    put_lines(dl,Y_CUR_LINE,1);
	} else {
	    gotoxy(0, 4);
	    put_lines(dl, Y_TOP_LINE, Y_SIZE_SCREEN);
	}
    } else
	dl->screen.y--;
}



/**************************************************************************/



void scroll_down(DIR_LIST *dl)
{
    if (dl->screen.y + 1 > LINES-2) {    
	gotoxy(dl->oldscr.x, dl->oldscr.y);
	fputs(DL_select.cursor_spaces, stdout);
	gotoxy(0, 4);
	if (NO_ALDL == 0) {
	    deleteln();
	    gotoxy(0, LINES - 2);
	    put_lines(dl, Y_CUR_LINE, 1);
	} else {
	    put_lines(dl, Y_TOP_LINE, Y_SIZE_SCREEN);
	} 
    } else
	dl->screen.y++;
}



/**************************************************************************/



void moves(DIR_LIST *dl)
{
    int c = -1, i;
    int errflag = 0;
    char *s, *str;
    char *fname;
    DIR_LIST *dl_tmp;
    char help_msg[80];
    char file[1024];
    struct stat st;

    for (i = 0 ; i < KEY_MAX && keytab[i].id != QX_HELP ; i++)
	;
    s = (char *)ascii2str(keytab[i].scan);
    sprintf(help_msg, "'%s' für Hilfe", s);
    put_msg(help_msg);
    free(s);

    gotoxy(dl->screen.x, dl->screen.y);
    fputs(DL_select.cursor, stdout);
    fflush(stdout);

    while (c != QX_QUIT) {

	while(!kbhit() && !DL_select.update)
	    DL_SELECT_update();

	/* periodic update? */

	if (DL_select.update) {
	    DL_select.update = 0;
	    if (dl->need_update) {
		put_dir(dl);
		dl->need_update = 0;
	    }
	    c = -1;
	} else 
	    c = igetc();

	if (errflag) {       
	    put_msg("");
	    errflag = 0;
	}
    
	switch(c) {


	case -1: /* nop */
	    break;
      
	case _C_RT:
	case C_RT:
	    if (dl->flpos + 2 > dl->fl->no_of_files) 
		break;
	    dl->flpos++;
	    if (dl->screen.x + ENTRY_LENGTH > ENTRY_LENGTH * MAX_LINE_ENTRIES - 1) {
		scroll_down(dl);
		dl->screen.x = dl->position.p_cursor;
	    } else
		dl->screen.x += ENTRY_LENGTH;
	    break;
      
      
	case _C_LF:
	case C_LF:
	    if (dl->flpos == 0) 
		break;
	    dl->flpos--;
	    if (dl->screen.x - ENTRY_LENGTH < dl->position.p_cursor) {
		scroll_up(dl);
		dl->screen.x = (ENTRY_LENGTH*(MAX_LINE_ENTRIES-1)) + dl->position.p_cursor;
	    } else
		dl->screen.x -= ENTRY_LENGTH;
	    break;
      
      
	case _C_DN:
	case C_DN:
	    if (dl->flpos + MAX_LINE_ENTRIES >= dl->fl->no_of_files)
		break;
	    dl->flpos += MAX_LINE_ENTRIES;
	    scroll_down(dl);
	    break;
      
      
	case _C_UP:
	case C_UP:
	    if (dl->flpos - MAX_LINE_ENTRIES < 0)
		break;
	    dl->flpos -= MAX_LINE_ENTRIES;
	    scroll_up(dl);
	    break;
      
      
	case _C_PGDN:
	case C_PGDN:
	    if (dl->flpos == Y_MAX)
		break;
	    if (Y_BOT_LINE + Y_SIZE_SCREEN > Y_MAX) {
		j_end(dl);
		break;
	    }
	    gotoxy(0,4);
	    put_lines(dl,Y_BOT_LINE, Y_SIZE_SCREEN);
	    dl->flpos += PAGE_ENTRIES;
	    clrtobot();
	    break;
      
      
	case _C_PGUP:
	case C_PGUP:
	    if (dl->flpos == 0)
		break;
	    if (Y_TOP_LINE - Y_SIZE_SCREEN < 0) {
		j_home(dl);
	    } else {
		gotoxy(0,4);
		put_lines(dl,Y_TOP_LINE - Y_SIZE_SCREEN, Y_SIZE_SCREEN);
		dl->flpos -= PAGE_ENTRIES;
		clrtobot(); 
	    }
	    break;
      
      
	case _C_HOME:
	case C_HOME:
	    if (dl->flpos == 0)
		break;
	    j_home(dl);
	    break;
      
      
	case _C_END:
	case C_END:
	    if (dl->flpos == dl->fl->no_of_files)
		break;
	    j_end(dl);
	    break;
      
      
	case QX_TAG:
	    dl->tagged[dl->flpos] =! dl->tagged[dl->flpos];
	    dl->no_of_files_tagged += dl->tagged[dl->flpos] ? 1 : -1;
	    gotoxy(ENTRY_LENGTH*(dl->flpos%MAX_LINE_ENTRIES)+dl->position.p_selector,
		   dl->screen.y);
	    putchar(dl->tagged[dl->flpos] ? '+' : ' ');
	    if (dl->flpos + 2 > dl->fl->no_of_files) 
		break;
	    dl->flpos++;
	    if (dl->screen.x + ENTRY_LENGTH > ENTRY_LENGTH * MAX_LINE_ENTRIES - 1){
		scroll_down(dl);
		dl->screen.x = dl->position.p_cursor;
	    } else
		dl->screen.x += ENTRY_LENGTH;
	    break;
      
      
	case QX_INC:
	    i = Proc->now;
	    if (Proc->now + 1 > Proc->max) {
		Proc->now = 0;
		if (Proc->page_max > 1) {
		    Proc->page_now = 0;
		    put_proc_page();
		    put_proc_help();
		} else 
		    put_proc_pointer(Proc->now,i);
		break;
	    } 
	    if (Proc->now + 1 > Proc->page_mem[Proc->page_now+1]-1) {
		Proc->now = Proc->page_mem[++Proc->page_now];
		put_proc_page();
		put_proc_help();
		break;
	    } 
	    put_proc_pointer(++Proc->now,i);
	    break;
      
      
	case QX_DEC:
	    i = Proc->now;
	    if (Proc->now - 1 < 0) {
		Proc->now = Proc->max;
		if (Proc->page_max > 1) {
		    Proc->page_now = Proc->page_max-1;
		    put_proc_page();
		    put_proc_help();
		} else 
		    put_proc_pointer(Proc->now,i);
		break;
	    } 
      
	    if (Proc->now - 1 < Proc->page_mem[Proc->page_now]) {
		Proc->now = Proc->page_mem[Proc->page_now--] - 1;
		put_proc_page();
		put_proc_help();
		break;
	    } 
      
	    put_proc_pointer(--Proc->now, i);
	    break;
      
      
	case QX_REFRESH:
	    put_screen();
	    break;
      
      
	case QX_CHDIR:
	    fname = dl->fl->fl[dl->flpos].name;
	    snprintf(file, sizeof(file), "%s/%s", dl->cwd, fname);
	    if (stat(file, &st) < 0) {
		put_msg(strerror(errno));
		errflag = 1;
		break;
	    }
	    if ((st.st_mode & S_IFMT) != S_IFDIR) {
		put_msg(NOT_A_DIR);
		errflag = 1;
		break;
	    }
	    if (!have_access(&dl->fl->fl[dl->flpos].stat)) {
		put_msg(PERMISSION_DENIED);
		errflag = 1;
		break;
	    }
	    if (!strcmp(fname, "..")) {
		if (rpos(dl->cwd,'/') > 0) {
		    dl->cwd[rpos(dl->cwd, '/')] = '\0';
		    dl->cwd = (char *) realloc(dl->cwd, strlen(dl->cwd) + 1);
		} else {
		    dl->cwd = (char *) realloc(dl->cwd, 2);
		    strcpy(dl->cwd, "/");
		}
	    } else {
		if (strcmp(fname, ".")) {
		    dl->cwd = (char *) realloc(dl->cwd, strlen(dl->cwd) + strlen(fname)+2);
		    if (strlen(dl->cwd) != 1)
			strcat(dl->cwd, "/");
		    strcat(dl->cwd, fname);
		}
	    }
	    DL_select.sdir[DL_select.now].wd_tbl = dl->cwd;
	    dl = (DIR_LIST *)dl_newdir(dl, NEWDIR_RESET);
	    dl = (DIR_LIST *)dl_layout(dl);
	    put_path(dl);
	    put_dir(dl);
	    break; 
      
      
	case QX_LO: case QX_LO1: case QX_LO2: case QX_LO3: case QX_LO4:
	case QX_LO5:case QX_LO6: case QX_LO7: case QX_LO8: case QX_LO9:
	    if (dl->layout_now == c - QX_LO)
		break;
	    gotoxy(0,3);
	    dl->layout_now = c - QX_LO;
	    dl = (DIR_LIST *)dl_layout(dl);
	    put_path(dl);
	    put_dir(dl);
	    break;

      
	case QX_TAGALL:
	case QX_UNTAGALL:
	    dl->no_of_files_tagged = 0;
	    for (i = 0 ; i < dl->fl->no_of_files ; i++) {
		dl->tagged[i] = c == QX_TAGALL ? TRUE : FALSE;
		dl->no_of_files_tagged += dl->tagged[i];
	    }
	    put_dir(dl);
	    break;

	case QX_TAG_INVERSE:
	    dl->no_of_files_tagged = 0;
	    for (i = 0 ; i < dl->fl->no_of_files ; i++) {
		dl->tagged[i] = !dl->tagged[i];
		dl->no_of_files_tagged += dl->tagged[i];
	    }
	    put_dir(dl);
	    break;

	case QX_MATCH:
	    gotoxy(0, 1);
	    clrtoeol();
	    s = input(GET_PATTERN_PROMPT);
	    if (!s) {
		put_proc_help();
		break;
	    }
	    dl->no_of_files_tagged = 0;
	    for (i = 0 ; i < dl->fl->no_of_files ; i++) {
		dl->tagged[i] = match(dl->fl->fl[i].name, s) ? TRUE : FALSE;
		dl->no_of_files_tagged += dl->tagged[i];
	    }
	    free(s);
	    put_proc_help();
	    put_dir(dl);
	    break;
      

	case QX_GOTO_DIR:
	case QX_GOTO_HOME:
	    if (c == QX_GOTO_DIR) {
		gotoxy(0, 1);
		clrtoeol();
		str = shortpath(dl->cwd);
		IGETDIR_FILE_MODE = I_FMDIR;
		s = inputs(DIR_PROMPT, str);
		IGETDIR_FILE_MODE = I_FMREG;
		free(str);
		if (!s) {
		    put_proc_help();
		    break;
		}
		str = strenv(s);
		free(s);
		if (str[(i = strlen(str)-1)] == '/' && i)
		    str[i] = 0;
	    } else 
		str = strdup(getenv("HOME"));
	    switch(errflag = check_dir(str)) {
	    case 1:
		put_msg(PATH_NOT_ABSOLUT);
		break;
	    case 2:
		put_msg(DIR_NOT_EXIST);
		break;
	    case 3:
		put_msg(NOT_A_DIR);
		break;
	    case 4:
		put_msg(PERMISSION_DENIED);
		break;
	    }
	    if (errflag) {
		put_proc_help();
		free(str);
		errflag = 1;
		break;
	    } 
	    dl->cwd = realloc(dl->cwd, strlen(str) + 1);
	    strcpy(dl->cwd, str);
	    free(str);
	    dl = dl_newdir(dl, NEWDIR_RESET);
	    DL_select.sdir[DL_select.now].wd_tbl = dl->cwd;
	    dl = (DIR_LIST *)dl_layout(dl);
	    put_path(dl);
	    put_dir(dl);
	    put_proc_help();
	    break;
      

	case QX_UPDATE_DIR:
	    dl = (DIR_LIST *)dl_updatedir(dl, DL_select.now);
	    if (!dl->need_update) {
		put_msg(DIR_NOT_MODIFYED);
		errflag = 1;
		break;
	    } else
		dl->need_update = 0;
	    dl = (DIR_LIST *)dl_layout(dl);
	    put_dir(dl);
	    break;

	case QX_NOHIDDEN:
	    dl->nohidden = !dl->nohidden;
	    dl = (DIR_LIST *)dl_newdir(dl, NEWDIR_KEEP);
	    dl = (DIR_LIST *)dl_layout(dl);
	    put_dir(dl);
	    break;

	case QX_HELP:
	    put_help_screen();
	    put_screen();
	    break;
      

	case QX_SHELL:
	    exec_shell();
	    put_screen();
	    break;
    

	case QX_SELECT: case QX_SELECT1:case QX_SELECT2: case QX_SELECT3: 
	case QX_SELECT4:case QX_SELECT5:case QX_SELECT6: case QX_SELECT7: 
	case QX_SELECT8:case QX_SELECT9:case QX_SELECT10:case QX_SELECT11:
	    if (DL_select.now == c - QX_SELECT)
		break;
	    gotoxy(0, 3);
	    dl = (DIR_LIST *)DL_SELECT_dir(c - QX_SELECT);
	    if (DL_select.now != c - QX_SELECT) {
		errflag = 1;
		break;
	    }
	    put_path(dl);
	    dl = (DIR_LIST *)dl_layout(dl); 
	    put_dir(dl);
	    dl->need_update = 0;
	    break;

      
	case QX_RENAME:
	    gotoxy(0, 1);
	    clrtoeol();
	    s = (char *)input("Neuer Name: ");
	    put_proc_help();
	    if (!s)
		break;
	    if (renaming(dl->fl->fl[dl->flpos].name, s) != 0) {
		errflag = 1;
		break;
	    }
	    dl = (DIR_LIST *)dl_newdir(dl, NEWDIR_KEEP);
	    dl = (DIR_LIST *)dl_layout(dl);
	    put_dir(dl);
	    break;

	case QX_FULLNAME:
	    errflag = 1;
	    s = (char *)convert_filename_to_printable(dl->fl->fl[dl->flpos].name, COLS);
	    put_msg(s);
	    free(s);
	    break;

	case QX_SHOWDIRTABLE:
	    dl_tmp = (DIR_LIST *)DL_SELECT_display_table();
	    if (dl_tmp != (DIR_LIST *)0) {
		dl = dl_tmp;
		dl = (DIR_LIST *)dl_layout(dl);
	    }
	    put_path(dl);
	    put_dir(dl);
	    break;
      
	}
    

    
	/* qx call self-defined procs */
    
	if (c >= 1000 || c == QX_START_PROC) {
	    if (c != QX_START_PROC) {
		if (Proc->page_max < 2) {
		    put_proc_pointer(c-1000,Proc->now);
		    Proc->now = c-1000;
		} else {
		    Proc->page_now = 0;
		    Proc->now = c-1000;
		    for (i = 0 ; i < Proc->now ; i++)
			if (i > Proc->page_mem[Proc->page_now+1]-2) 
			    Proc->page_now++;
		    put_proc_page();
		    put_proc_help();
		}
	    }
	    call_proc(dl);
	}

    
    
	gotoxy(dl->oldscr.x, dl->oldscr.y);
	fputs(DL_select.cursor_spaces, stdout);
	gotoxy(dl->screen.x, dl->screen.y);
	fputs(DL_select.cursor, stdout);
	fflush(stdout);
	dl->oldscr = dl->screen;
    }
}
