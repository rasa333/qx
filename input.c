#include <stdio.h> 
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#include "defs.h"

extern int INPUT_RETURN_MODE;


static char *c_rt(void);
static char *c_lf(void);
static char *c_up(void);
static char *c_dn(void);
static char *c_next_word(void);
static char *c_prev_word(void);
static char *c_end(void);
static char *c_home(void);
static char *c_backspace(void);
static char *c_tab(void);
static char *c_delete(void);
static char *c_clreol(void);
static char *c_enter(void);
static char *c_abort(void);
static char *c_overwrite(void);
static char *c_refresh(void);

iKEYS std_ikeytab[] = {
  "\01",       c_home,
  "\02",       c_lf,
  "\05",       c_end,
  "\06",       c_rt,
  "\010",      c_backspace,
  "\011",      c_tab,
  "\012",      c_enter,
  "\013",      c_clreol,
  "\014",      c_refresh,
  "\016",      c_dn,
  "\020",      c_up,
  "\026",      c_overwrite,
  "\030",      c_next_word,
  "\031",      c_prev_word,
  "\033",      c_abort,
  "\033[A",    c_up,
  "\033[B",    c_dn,
  "\033[C",    c_rt,
  "\033[D",    c_lf,
  "\033[F",    c_end,
  "\033[H",    c_home,
  "\033[L",    c_overwrite,
  "\177",      c_delete,
  NULL,   0
};

/* default settings for input */

struct set_input set_input = {
  is_printable_char,	/* pointer to func to check c is allowed */
  0,            /* maximal length of string; 0=no limit */
  50,           /* 0=no history;else number of lines to store */
  0,            /* length of edit buffer yet */
  0,	        /* position of cursor in buffer yet */
  (char *)0,    /* pointer to edit buffer */	
  (char *)0,	/* pointer to prompt string */	
  0,            /* 0=insert-mode; 1=overwrite-mode */            
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

struct set_input *ip_get(struct set_input *ip)
{
  *ip = set_input;
  return ip;
}

int ip_set(struct set_input ip)
{
  set_input = ip;
}



static void h_add(char *s)
{
  if (ih_head == 0) {
    ih_tail = (iHist *) malloc(sizeof(iHist));
    ih_head = ih_tail;
    ih_head->next = (iHist *)0;
    ih_head->prev = (iHist *)0;
    ih_head->buf = strdup(s);
  } else {
    ih_tail->next = (iHist *) malloc(sizeof(iHist));
    ih_tail->next->buf = (char *) strdup(s);
    ih_tail->next->prev = ih_tail;
    ih_tail = ih_tail->next;
    ih_tail->next = (iHist *)0;
  }
}

static void _h_del_node(iHist *x)
{
  if (x == ih_head) {
    ih_head = x->next;
    ih_head->prev = (iHist *)0;
  } else {
    if (x == ih_tail) {
      ih_tail = x->prev;
      ih_tail->next = (iHist *)0;
    } else {
      x->next->prev = x->prev;
      x->prev->next = x->next;
    }
  }
}

static void h_del(iHist *x)
{
  _h_del_node(x);
  free(x);
}

static void history(s)
char *s;
{
  if (!ilen)
    return;
  if (ih_cnt < set_input.history_flag) {
    h_add(s);
    ih_cnt++;
  } else {
    h_del(ih_head);
    h_add(s);
  }
  ih_now = 0;
}



char _ikey()
{
  char c; 

  read(0, &c, 1);

  return c;
}



char *getseq()
{
  char *sequence = (char *) malloc(strlen(METAstr) + 10);
  int i = 0;
  
  do 
    sequence[i++] = _ikey();
  while (kbhit() && sequence[i-1] == METAstr[i-1]);
  sequence[i] = 0;

  return sequence;
}

int is_printable_char(c)
int c;
{
  return isprint(c);
}


void jump_xpos(int x)
{
  int tabs, bs;

  tabs = x / 8 + (x % 8 != 0);
  bs = tabs * 8 - x;
  putchar(13);
  while(tabs--)
    putchar(9);
  while(bs--)
    putchar(8);
}


void xpos_prompt(xpos)
int xpos;
{
  jump_xpos(xpos);
  fputs(ipst,stdout);
}

static void update_line(int refresh_flag)
{
  int i;

  if (refresh_flag) {
    xpos_prompt(ixbegin);
    ipos = ipos > ilen ? ilen : ipos;
    ixscl = ipos == ilen ? ilen < imdc - 1 ? 0 : ilen - imdc + 1 : ipos < imdc
      ? 0 : ipos;
    for (i = 0 ; 
	 i < (ilen < imdc - 1 ? ilen : (ipos == ilen ? imdc - 1 : imdc)) ;
	 i++)
      putchar(istr[ixscl + i]);
    if (old_ilen > ilen && ilen < ixscl + imdc) {
      for (i = 0 ; i < (old_ilen < imdc ? old_ilen : imdc) - ilen ; i++) 
	putchar(' ');
      while(i--)
	putchar(8);
    } 
    if (ilen != ipos)
      for (i = (ilen < imdc ? ilen : ixscl + imdc) ; i > ipos ; i--)
	putchar(8);
    else {
      putchar(' ');
      putchar(8);
    }
  } else
    if (ipos < ixscl || ipos > ixscl + imdc - 1) {
      ixscl = ipos > ixscl + imdc - 1 ? ipos - (imdc - 1) : ipos;
      xpos_prompt(ixbegin);
      for (i = 0 ; i < (ipos != ilen ? imdc : imdc - 1) && 
	   istr[ixscl + i] != '\0' ; i++)
	putchar(istr[ixscl + i]);
      if (ipos == ixscl)
	xpos_prompt(ixbegin);
      else {
	if (ilen == ipos)
	  putchar(' ');
	putchar(8);
      }
    } else {
      if (ipos != old_ipos) {
	if (ipos > old_ipos)
	  for (i = old_ipos ; i < ipos ; i++) 
	    putchar(istr[i]);
	if (ipos < old_ipos)
	  for (i = old_ipos ; i > ipos ; i--)
	    putchar(8);
      }
      if (ilen != old_ilen) {
	for (i = ipos ; i < (ilen < imdc + ixscl ? ilen : ixscl + imdc) ; i++)
	  putchar(istr[i]);

	if (old_ilen > ilen) {
	  for (i = 0 ; i < (old_ilen < ixscl + imdc ? old_ilen - ilen :
			    ipos - ixscl + imdc); i++) 
	    putchar(' ');
	  while(i--)
	    putchar(8);
	} 
	if (ipos != ilen)
	  for (i = (ilen < imdc || ixscl + imdc > ilen 
		    ? ilen 
		    : ixscl + imdc) ; i > ipos ; i--)
	    putchar(8);
      }
    }
  old_ilen = ilen;
  old_ipos = ipos;
  fflush(stdout);
}



static char *c_rt()
{
  ipos++;
  if (ipos > ilen)
    ipos = ilen;
  return NULL;
}

static char *c_lf()
{
  ipos--;
  if (ipos < 0)
    ipos = 0;
  return NULL;
}

static char *c_next_word()
{
  while(!isspace(istr[ipos]) && ipos < ilen)
    ipos++;
  while(isspace(istr[ipos]) && ipos < ilen)
    ipos++;
  return NULL;
}

static char *c_prev_word()
{
  while(!isspace(istr[ipos]) && ipos > 0)
    ipos--;
  while(isspace(istr[ipos]) && ipos > 0)
    ipos--;
  while(!isspace(istr[ipos]) && ipos > 0)
    ipos--;
  while(isspace(istr[ipos]) && ipos < ilen)
    ipos++;
  return NULL;
}

static char *c_home()
{
  ipos = 0;
  return NULL;
}

static char *c_end()
{
  ipos = ilen;
  return NULL;
}

static char *c_clreol()
{
  ilen = ipos;
  return NULL;
}

static char *c_refresh()
{
  DIR_LIST *dl = DL_select.sdir[DL_select.now].dl_tbl;

  clear();      
  put_proc_page();
  put_path(dl);
  put_dir(dl);
  gotoxy(0,1);
  update_line(1);

  return NULL;
}

static char *c_backspace()
{
  int i;

  if (ipos) {
    ipos--;
    ilen--;
    for( i = ipos ; i < ilen ; i++ )
      istr[i] = istr[i+1];
  }
  return NULL;
}

static char *c_tab()
{
  DI *d;
  char *name, *exp, *path, *xpath, *basen, *dirn;
  int i, diff;
  int found = 0;
  int *idx, cnt_equ, flag = 1;
  int name_len;

  for (i = ipos ; i != 0 && !isspace(istr[i]) ; i--)
    ;
  i += (i != 0); 
  path = malloc(ipos - i + 1);
  strncpy(path,&istr[i],ipos - i);
  path[ipos - i] = 0;

  xpath = strenv(path);
  dirn = dirname(xpath);
  basen = basename(xpath);
  d = getdir(dirn, TRUE);
  name = (char *)malloc(strlen(basen) + 1);
  strcpy(name,basen);
  name_len = strlen(name);
  free(path);
  free(xpath);
  free(dirn);
  if (d == (DI *)-1) {
    free(name);
    return NULL;
  }

  if (d->no_of_files == 2) {
    freedir(d);
    free(name);
    return NULL;
  }
  idx = malloc(sizeof(int) * (d->no_of_files - 2));

  if (d->no_of_files == 3 && !name_len) {
    exp = (char *) malloc(strlen(d->fl[2].name) + 2);
    strcpy(exp,d->fl[2].name);
    found = 1; 
    idx[0] = 2;
  } else {
    for (i = 2 ; i < d->no_of_files ; i++) {
      if (!strncmp(name,d->fl[i].name,name_len)) {
	if (!found) {
	  diff = strlen(d->fl[i].name) - name_len;
	  exp = (char *)malloc(diff + 2);
	  strncpy(exp,&d->fl[i].name[name_len],diff);
	  exp[diff] = 0;
	}
	idx[found++] = i;
      }
    }
  }
  if (found > 1) {
    cnt_equ = name_len;
    while(flag) {
      for (i = 1 ; i < found && flag ; i++) 
	flag = d->fl[idx[i]].name[cnt_equ] == d->fl[idx[0]].name[cnt_equ] &&
	  d->fl[idx[i]].name[cnt_equ] && d->fl[idx[0]].name[cnt_equ];
      cnt_equ += flag; 
    }
    diff = cnt_equ - name_len;
    if (diff) {
      exp = (char *) realloc(exp, diff + 1);
      strncpy(exp,&d->fl[idx[0]].name[name_len], diff);
      exp[diff] = 0;
    } else {
      free(exp);
      found = 0;
    }
  }
  if (found == 1 && (d->fl[idx[0]].stat.st_mode & S_IFMT) == S_IFDIR) 
    strcat(exp,"/");
  
  free(name);
  free(idx);
  freedir(d);
  
  if (!found)
    return NULL;

  return exp;
}

static char *c_delete()
{
  int i;

  if ( ipos != ilen ) {
    ilen--;
    for( i = ipos ; i < ilen ; i++ )
      istr[i] = istr[i+1];
  }
  return NULL;
}

static char *c_up()
{
  if (!ih_head || ih_now == ih_head)
    return NULL;
  ih_now = ih_now ? ih_now->prev : ih_tail;
  ipos = ilen = strlen(ih_now->buf);
  ioffset = ilen + 64;
  istr = (char *) realloc(istr,ioffset);
  strcpy(istr,ih_now->buf);
  irefresh = 1;
  return NULL;
}


static char *c_dn()
{
  if (!ih_head || ih_now == 0)
    return NULL;
  ih_now = ih_now->next;
  if (ih_now == 0) {
    ioffset = 64;
    istr = (char *) realloc(istr,ioffset);
    *istr = ilen = ipos = 0;
  } else {
    ipos = ilen = strlen(ih_now->buf);
    ioffset = ilen + 64;
    istr = (char *) realloc(istr,ioffset);
    strcpy(istr,ih_now->buf);
  }
  irefresh = 1;
  return NULL;
}

static char *c_overwrite()
{
  set_input.overwrite_flag =! set_input.overwrite_flag;
  return NULL;
}

static char *c_enter()
{
  istr[ilen] = 0;
  if (set_input.history_flag)
    history(istr);
  putchar('\n');
  return NULL;
}

static char *c_abort()
{
  return NULL;
}

int ikey_cmp(iKEYS *a, iKEYS *b)
{
  return strcmp(a->scan, b->scan);
}

void init_ikeys(iKEYS *tab)
{
  int i;

  if (ikeytab != 0) {
    while(ikeycnt--) {
      free(ikeytab[ikeycnt].scan);
      ikeytab[ikeycnt].input_func = 0;
    }
    free(ikeytab);
  }
  ikeycnt = 0;
  while(tab[ikeycnt].scan)
    ikeycnt++;
  ikeytab = (iKEYS *) malloc(ikeycnt * sizeof(iKEYS));
  for (i = 0 ; i < ikeycnt ; i++) {
    ikeytab[i].scan = (char *) malloc(strlen(tab[i].scan) + 1);
    strcpy(ikeytab[i].scan,tab[i].scan);
    ikeytab[i].input_func = tab[i].input_func;
  }
}


int add_ikey(scan,input_func)
char *scan;
char *(*input_func)();
{
  int i;

  if (!ikeytab)
    init_ikeys(std_ikeytab);
  for (i = 0 ; i < ikeycnt && strcmp(ikeytab[i].scan,scan) ; i++)
    ;
  if (i != ikeycnt)
    return -1;
  ikeytab = (iKEYS *)realloc(ikeytab,(ikeycnt+2) * sizeof(iKEYS));
  ikeytab[ikeycnt].scan = (char *)malloc(strlen(scan) + 1);
  strcpy(ikeytab[ikeycnt].scan,scan);
  ikeytab[ikeycnt].input_func = input_func;
  ikeycnt++;

  return 0;
}

int del_ikey(scan)
char *scan;
{
  int i;

  if (!ikeytab)
    init_ikeys(std_ikeytab);
  for (i = 0 ; i < ikeycnt && strcmp(ikeytab[i].scan,scan) ; i++)
    ;
  if (i == ikeycnt)
    return -1;
  for ( ; i < ikeycnt ; i++ ) 
    ikeytab[i] = ikeytab[i+1];
  ikeycnt--;
  free(ikeytab[ikeycnt].scan);
  ikeytab = (iKEYS *)realloc(ikeytab,(ikeycnt+1) * sizeof(iKEYS));

  return 0;
}
 
int rep_ikey(scan1,scan2)
char *scan1;
char *scan2;
{
  int i;

  if (!ikeytab)
    init_ikeys(std_ikeytab);
  for (i = 0 ; i < ikeycnt && strcmp(ikeytab[i].scan,scan1) ; i++)
    ;
  if (i == ikeycnt)
    return -1;
  ikeytab[i].scan = (char *) realloc(ikeytab[i].scan,strlen(scan2) + 1);
  strcpy(ikeytab[i].scan,scan2);

  return 0;
}

void org_ikey()
{
  if (!ikeytab)
    init_ikeys(std_ikeytab);

  qsort(ikeytab, ikeycnt, sizeof(iKEYS), (void *)ikey_cmp);
}

char *input(char *prompt)
{
  int i;
  iKEYS k,*f;
  char *tmp, *insert;
  int inputs_mode = 0;

  if (!ikeytab)
    init_ikeys(std_ikeytab);
  ipst = prompt;
  i = strlen(ipst);
  if (imdc + i > iCOLS - 1) 
    imdc = iCOLS - i - 1;
  if (imdc <= 1)
    return NULL;
  if (istr == NULL) {
    ioffset = 64;
    istr = (char *)malloc(ioffset);
    ilen = ipos = 0;
  } else {
    ilen = strlen(istr);
    ioffset = ilen + 64;
    istr = (char *)realloc(istr,ioffset);
    inputs_mode = 1;
  }
  irefresh = 1;
  for(;;) {
    update_line(irefresh);
    irefresh = 0;
    k.scan = getseq();
    f = (iKEYS *)bsearch(&k, (iKEYS *)ikeytab, ikeycnt, sizeof(iKEYS), (void *)ikey_cmp);
    insert = f ? (*f->input_func)() : k.scan;
    if (insert) {
      if (inputs_mode) {
	ilen = 0;
	ioffset = ilen + 64;
	istr = (char *)realloc(istr,ioffset);
	*istr = '\0';
	inputs_mode = 0;
	update_line(1);
      }
      i = strlen(insert);
      if ((*set_input.compar_char)(insert[0])) {
	if (set_input.overwrite_flag && 
	    (!set_input.edit_buf_maximal_length ||
	     set_input.edit_buf_maximal_length >= ipos + i)) {
	  int j = i;
	  ilen = ipos + i > ilen ? ipos + i : ilen;
	  if (ilen + 1 > ioffset) {
	    ioffset = ilen + 64;
	    istr = (char *) realloc(istr, ioffset);
	  }
	  while(j--)
	    istr[ipos + j] = insert[j];
	  ipos += i;
	}
	if (!set_input.overwrite_flag && 
	    (!set_input.edit_buf_maximal_length ||
	     set_input.edit_buf_maximal_length >= ilen + i)) {
	  tmp = (char *) malloc(ilen - ipos + 10);
	  *tmp = '\0';
	  strncpy(tmp,&istr[ipos],ilen - ipos);
	  if (ilen + i + 1 > ioffset) {
	    ioffset = ilen + i + 64;
	    istr = (char *) realloc(istr, ioffset);
	  }
	  istr[ipos] = 0;
	  strcat(istr,insert);
	  strcat(istr,tmp);
	  free(tmp);
	  ilen += i;
	  ipos += i;
	}
      }
      free(insert);
    }
    istr[ilen] = 0;
    inputs_mode = 0;
    if (f) {
      free(k.scan);
      if (f->input_func == c_enter) {
	tmp = (char *)(ilen ? strdup(istr) : NULL);
	free(istr);
	istr = NULL;
	INPUT_RETURN_MODE = I_ENTER;
	return tmp;
      }
      if (f->input_func == c_abort) {
	free(istr);
	istr = NULL;
	INPUT_RETURN_MODE = I_ABORT;
	return NULL;
      }
    }
  }
}

char *inputs(char *p, char *s)
{
  istr = strdup(s);
  ipos = 0;
  return input(p);
}
