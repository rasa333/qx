#include <signal.h>
#include <unistd.h>
#include <time.h>

#include "defs.h"


extern int LINES, COLS;
int IGETDIR_FILE_MODE;
PROC *Proc;

DL_SELECT DL_select;

char *Def_Layout[] = {"%P %l %O %G %S %m %=%^%t%N",
		      "%P %l %i %O %G %S %m %=%^%t%N",
		      "%=%^%t%N %S",
		      "%=%^%N",
		      "%S %c %a %m %=%^%n",
		      "%=%m %^%t%n",
                      "%S%=%O %G %^%N",
                      "%o %s %=%t%^%N",
		      "%s %=%n%^%p",
                      "%=%N%^%s %P"};


void DL_SELECT_table_init_null()
{
  int i;

  for (i = 0 ; i < 10 ; i++)
    DL_select.layout_tbl[i] = NULL;
  for (i = 0 ; i < 12 ; i++) {
    DL_select.sdir[i].wd_tbl = NULL;
    DL_select.sdir[i].time_tbl = 0;
    DL_select.sdir[i].set_flpos_to_newest = 0;
  }
  DL_select.cursor = DL_select.separator = NULL;
  DL_select.now = 0;
  DL_select.update = FALSE;
  DL_select.noinverse = FALSE;
  IGETDIR_FILE_MODE = I_FMREG;
}


void DL_SELECT_table_init_set()
{
  int i, len;

  for (i = 0 ; i < 10 ; i++) {
    if (DL_select.layout_tbl[i] == NULL)
      DL_select.layout_tbl[i] = Def_Layout[i];
  }
  
  if (DL_select.cursor == NULL)
    DL_select.cursor = DEFAULT_CURSOR;
  if (DL_select.separator == NULL)
    DL_select.separator = DEFAULT_SEPARATOR;

  if (DL_select.sdir[0].wd_tbl == NULL) {
    DL_select.sdir[0].wd_tbl = getcwd(NULL, 1024);
  }

  len = strlen(DL_select.cursor);
  DL_select.cursor_spaces = malloc(len + 1);
  *DL_select.cursor_spaces = 0;
  for (i = 0 ; i < len ; i++)
    strcat(DL_select.cursor_spaces, " ");

  for (i = 0 ; i < 12 ; i++) 
    if (DL_select.sdir[i].wd_tbl != NULL) {
      DL_select.sdir[i].dl_tbl = dl_init(DL_select.sdir[i].wd_tbl, DL_select.sdir[i].nohidden);
    }
}


DIR_LIST *DL_SELECT_dir(int n)
{
  char *s;
  int rc, i;

  if (DL_select.sdir[n].dl_tbl != NULL) {
    DL_select.now = n;
    return DL_select.sdir[n].dl_tbl;
  }
  put_msg("Eintrag unbelegt. Bitte ein Verzeichnis fuer diesen Eintrag eingeben.");
  gotoxy(0, 1);
  clrtoeol();
  IGETDIR_FILE_MODE = I_FMDIR;
  s = input(DIR_PROMPT);
  IGETDIR_FILE_MODE = I_FMREG;
  if (!s) {  
    gotoxy(0,LINES-1);
    clrtoeol();
    put_proc_help();
    return DL_select.sdir[DL_select.now].dl_tbl;
  }
  if (s[(i = strlen(s)-1)] == '/' && i)
    s[i] = 0;
  DL_select.sdir[n].wd_tbl = strenv(s);
  free(s);
  put_msg("");
  switch(rc = check_dir(DL_select.sdir[n].wd_tbl)) {
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
  if (rc) {
    free(DL_select.sdir[n].wd_tbl);
    DL_select.sdir[n].wd_tbl = (char *)0;
    put_proc_help();
    return DL_select.sdir[DL_select.now].dl_tbl;
  }
  DL_select.sdir[n].dl_tbl = dl_init(DL_select.sdir[n].wd_tbl, DL_select.sdir[n].nohidden);
  DL_select.now = n;
  gotoxy(0, 1);
  clrtoeol();
  s = (char *) input("Update Time (sec): ");
  if (s) {
    DL_select.sdir[n].time_tbl = atol(s);
    free(s);
  }
  put_proc_help();

  return DL_select.sdir[n].dl_tbl;
}


long Lt = 0L;

void DL_SELECT_update()
{
  long t;
  DIR_LIST *dl;
  int i;
  
  time(&t);
  if (Lt == t)
    return;
  if (Lt == 0L) {
    Lt = t;
    return;
  }
  Lt = t;

  for (i = 0 ; i < 12 ; i++) {
    dl = DL_select.sdir[i].dl_tbl;
    if (dl != (DIR_LIST *)0 && DL_select.sdir[i].time_tbl != 0 &&
	!(t % DL_select.sdir[i].time_tbl)) {
      dl = dl_updatedir(dl, i);
      if (dl->need_update)
	DL_select.update = TRUE;
    }
  }
}


int is_num_char(c)
int c;
{
  return ((c >= '0' && c <= '9') || c == ' ');
}


struct set_input select_num_input = {
  is_num_char,	/* pointer to func to check c is allowed */
  6,            /* maximal length of string; 0=no limit */
  0,            /* 0=no history;else number of lines to store */
  0,            /* length of edit buffer yet */
  0,	        /* position of cursor in buffer yet */
  (char *)0,    /* pointer to edit buffer */	
  (char *)0,	/* pointer to prompt string */	
  1,            /* 0=insert-mode; 1=overwrite-mode */            
  80,           /* default value for column */
  /* update-line */
  0,            /* scroll column position (ixscl) */
  0,            /* last ilen value (old_ilen) */
  0,            /* last ipos value (old_ipos) */
  0,            /* 0=refresh part of line;1=whole line */
  80,           /* maximal chars to display (imdc) */
  0,            /* column begin position (ixbegin) */
  /* ikeytab */
  "\033[",
  (iKEYS *)0,
  0,
  /* ioffset */
  0
  };


DIR_LIST *DL_SELECT_display_table()
{
  int i, c = 0, y = 0, oy, abort, empty, errflag = 0;
  char *tmp, *fpath, *spath, *s;
  char lbuf[1024];
  struct set_input oip;

  ip_get(&oip);
  gotoxy(0,2);
  clrtobot();
  if (DL_select.noinverse == FALSE)
    standout();
  for (i = 0 ; i < COLS ; i++)
    putchar('=');
  gotoxy(0,LINES-2);
  for (i = 0 ; i < COLS ; i++)
    putchar('=');
  if (DL_select.noinverse == FALSE)
  standend();
  gotoxy(0,4);
  puts("Directory Buffer Table");
  putchar('\n');
  if (DL_select.noinverse == FALSE)
    standout();
  fputs("   Update  Directory",stdout);
  if (DL_select.noinverse == FALSE)
    standend();
  clrtoeol();
  putchar('\n');
  for (i = 0 ; i < 12 ; i++) {
    tmp = (char *)shortpath(DL_select.sdir[i].wd_tbl);
    if (DL_select.sdir[i].time_tbl)
      sprintf(lbuf,"   %6d%c%c%s",
	      (DL_select.sdir[i].time_tbl),
	      DL_select.sdir[i].set_flpos_to_newest ? '*' : ' ',
	      DL_select.now == i ? '>' : ' ',
	      (DL_select.sdir[i].wd_tbl != NULL ? tmp : "(empty)"));
    else
      sprintf(lbuf,"   (none)%c%c%s",
	      DL_select.sdir[i].set_flpos_to_newest ? '*' : ' ',
	      DL_select.now == i ? '>' : ' ',
	      (DL_select.sdir[i].wd_tbl != NULL ? tmp : "(empty)"));
    if (strlen(lbuf) > COLS-2) {
      lbuf[COLS-2] = '\\';
      lbuf[COLS-1] = '\0';
    }
    puts(lbuf);
    if (tmp) 
      free(tmp);
  }
  if (DL_select.noinverse == FALSE)
    standout();
  puts("----------------------");
  if (DL_select.noinverse == FALSE)
    standend();
  puts(SELECT_MENU);
  
  oy = y;
  while(c != 27 && c != 'q') {
    if (y > 11)
      y = 0;
    if (y < 0)
      y = 11;
    gotoxy(0,7 + oy);
    fputs("  ",stdout);
    gotoxy(0,7 + y);
    fputs("->",stdout);
    fflush(stdout);
    oy = y;
    c = cigetc();
    if (errflag) {
      errflag = 0;
      put_msg("");
    }
    switch(c) {

    case C_UP: 
    case _C_UP:
      y--;
      break;

    case C_DN:
    case _C_DN:
      y++;
      break;

    case 10:
      if (DL_select.sdir[y].time_tbl)
	strcpy(lbuf,itoa(DL_select.sdir[y].time_tbl));
      else
	strcpy(lbuf,"");
      gotoxy(0,y + 7);
      fputs("         ",stdout);
      ip_set(select_num_input);
      s = (char *)inputs("-> ",lbuf);
      if (s) {
	DL_select.sdir[y].time_tbl = atol(s);
	free(s);
      } else {
	DL_select.sdir[y].time_tbl = 0L;
      }
      ip_set(oip);
      gotoxy(3,7 + y);
      if (DL_select.sdir[y].time_tbl)
	sprintf(lbuf,"-> %6d%c%c",DL_select.sdir[y].time_tbl,
		DL_select.sdir[y].set_flpos_to_newest ? '*' : ' ',
		DL_select.now == y ? '>' : ' ');
      else
	sprintf(lbuf,"-> (none)%c%c",
		DL_select.sdir[y].set_flpos_to_newest ? '*' : ' ',
		DL_select.now == y ? '>' : ' ');

      abort = empty = 0;
      fpath = spath = NULL;
      if (!DL_select.sdir[y].wd_tbl) {
	gotoxy(11,y + 7);
	clrtoeol();
	spath = (char *)malloc(1);
	strcpy(spath,"");
	empty = 1;
      } else
	spath = (char *) shortpath(DL_select.sdir[y].wd_tbl);

      IGETDIR_FILE_MODE = I_FMDIR;
      s = (char *)inputs(lbuf,spath);
      IGETDIR_FILE_MODE = I_FMREG;
      if (!s && !DL_select.sdir[y].wd_tbl) 
	abort = 1;
      if (!s && DL_select.sdir[y].wd_tbl) 
	s = (char *)strdup(DL_select.sdir[y].wd_tbl);
      if (s[(i = strlen(s)-1)] == '/' && i)
	s[i] = 0;
      fpath = (char *)strenv(s);
      if (!fpath)
	abort = 1;
      if (s)
	free(s);
      if (!abort) {
	switch(abort = errflag = check_dir(fpath)) {
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
      }
      if (abort) {
	gotoxy(0,7 + y);
	if (!empty) {
	  fputs(lbuf,stdout);
	  fputs(spath,stdout);
	  clrtoeol();
	} else {
	  printf("-> (none)  (empty)");
	  clrtoeol();
	  DL_select.sdir[y].time_tbl = 0L;
	}
	if (fpath)
	  free(fpath);
	if (spath)
	  free(spath);
	break;
      }
      if (spath)
	free(spath);
      if (!empty) 
	dl_free(DL_select.sdir[y].dl_tbl);
      DL_select.sdir[y].wd_tbl = (char *)strdup(fpath);
      DL_select.sdir[y].dl_tbl = (DIR_LIST *)dl_init(DL_select.sdir[y].wd_tbl, DL_select.sdir[y].nohidden);
      spath = (char *)shortpath(fpath);
      gotoxy(0,7 + y);
      fputs(lbuf,stdout);
      fputs(spath,stdout);
      clrtoeol();
      free(fpath);
      free(spath);
      y++;
      break;

    case 9:
      if (DL_select.sdir[y].wd_tbl) {
	DL_select.now = y;
	return DL_select.sdir[y].dl_tbl;
	break;
      } 
      put_msg(SELECT_ENTRY_IS_EMPTY);
      errflag = 1;
      break;

    case '*':
      if (DL_select.sdir[y].wd_tbl) {
	DL_select.sdir[y].set_flpos_to_newest =! 
	  DL_select.sdir[y].set_flpos_to_newest;
	gotoxy(9,7 + y);
	if (DL_select.sdir[y].set_flpos_to_newest)
	  putchar('*');
	else
	  putchar(' ');
	break;
      } 
      put_msg(SELECT_ENTRY_IS_EMPTY);
      errflag = 1;
      break;
    }
  }
  return (DIR_LIST *)0;
}



