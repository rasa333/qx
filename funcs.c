#include <stdlib.h>
#include <ctype.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "defs.h"

int key_cmp();
int is_pathend();

extern int LINES, COLS;
extern int PREFIX_MAX, KEY_MAX;
extern KEYS *keytab;
extern char **Prefix;

char *ascii2str(char *s)
{
  char *t = (char *)(*s ? malloc(strlen(s) * 2 + 1) : 0);
  char *r = t;

  if (!*s)
    return (char *)0;
  while(*s) {
    if (*s < 0x20 || *s > 0x7e) {
      *r++ = '^';
      *r++ = (*s<0x20 ? *s + 0x40 : *s - 0x40);
      s++;
    } else
      *r++ = *s++;
  }
  *r = '\0';
  t = (char *) realloc(t, r - t + 1);

  return t;
}


char *str2ascii(char *s)
{
  char *t = (char *)(*s ? malloc(strlen(s) + 1) : 0);
  char *r = t;
  
  if (!*s)
    return (char *)0;
  while(*s) {
    if (*s == '^') {
      ++s;
      *r++ = toupper(*s) - 0x40;
      s++;
    } else
      *r++ = *s++;
  }
  *r++ = '\0';
  t = (char *) realloc(t, r - t + 1);

  return t;
}


char *get_prefix(int verbose)
{
  static char sequence[30], *tmp;
  int i = 0, j;

  while(!kbhit())
    ;

  while(kbhit() && i < 25)
    sequence[i++] = _ikey();
  sequence[i] = 0;
  for (j = 0 ; j < PREFIX_MAX ; j++)
    if (!strcmp(Prefix[j],sequence)) {
      if (verbose) {
	tmp = (char *)ascii2str(Prefix[j]);
	gotoxy(0,LINES-1);
	clrtoeol();
	fputs(tmp,stdout);
	free(tmp);
	fputs(" ",stdout);
	fflush(stdout);
	while(!kbhit())
	  ;
      }
      return sequence;
    }
  return sequence;
}



char *qx_getseq(int verbose)
{
  static char sequence[60], *prefix;
  int i = 0;
  
  prefix = get_prefix(verbose);
  i = strlen(prefix);
  strcpy(sequence,prefix);
  while (kbhit() && i < 55) {
    sequence[i++] = _ikey();
  } 
  sequence[i] = 0;

  return sequence;
}




int igetc()
{
  KEYS k,*f;

  k.scan = qx_getseq(1);
  f = (KEYS *)bsearch(&k,(KEYS *)keytab,KEY_MAX,sizeof(KEYS),key_cmp);

  return f ? f->id : k.scan[0];
}



int cigetc()
{
  KEYS k,*f;

  k.scan = qx_getseq(0);
  f = (KEYS *)bsearch(&k,(KEYS *)keytab,KEY_MAX,sizeof(KEYS),key_cmp);

  if (f)
    switch(f->id) {
    case _C_UP:
    case _C_DN:
    case C_UP:
    case C_DN:
      return f->id;
      break;
    default:
      return k.scan[0];
      break;
    }
  return k.scan[0];
}



int qx_exit(int err)
{
  resetty();
  gotoxy(0, LINES-1);
  clrtoeol();
  exit(err);
}


char *strenv(char *s)
{
  int len;
  char *envstr, *str;
  char *tmp;
  int j;

  if (!*s)
    return NULL;
  len = strlen(s);
  envstr = (char *)malloc(len + 1);
  str = (char *)malloc(len + 1);
  j = 0;
  while (*s) {
    if (*s == '$' || *s == '~') {
      s++;
      str[j] = 0;
      j = 0;
      if (*(s-1) == '$') {
	while (*s && !isspace(*s) && !strchr("/:.(){}-+",*s))
	  envstr[j++] = *s++;
	envstr[j] = '\0';
      } else {
	strcpy(envstr, "HOME");
      }
      tmp = (char *) getenv(envstr);

      if (tmp == NULL) {
	if (!strcmp(envstr,"EDITOR"))
	  tmp = DEFAULT_EDITOR;
	if (!strcmp(envstr,"PAGER")) 
	  tmp = DEFAULT_PAGER;
      }
      if (tmp == NULL) {
	str = (char *)realloc(str, strlen(str) + strlen(envstr) + 2 + len);
	strcat(str, "$");
	strcat(str, envstr);
      } else {
	str = (char *)realloc(str, strlen(str) + strlen(tmp) + 1 + len);
	strcat(str, tmp);
      }
      j = strlen(str);
    } else {
      str[j++] = *s++;
    }
  }
  free(envstr);
  str[j] = 0;

  return str;
}


/*************************************************************************/


#define	CMASK	0377
#define	QUOTE	0200
#define	QMASK	(CMASK&~QUOTE)

static	char	*cclass();

int match(char *s, char *p)
{
  int sc, pc;
  
  if (s == NULL || p == NULL)
    return(0);
  while ((pc = *p++ & CMASK) != '\0') {
    sc = *s++ & QMASK;
    switch (pc) {
    case '[':
      if ((p = cclass(p, sc)) == NULL)
	return(0);
      break;
      
    case '?':
      if (sc == 0)
	return(0);
      break;
      
    case '*':
      s--;
      do {
	if (*p == '\0' || match(s, p))
	  return(1);
      } while (*s++ != '\0');
      return(0);
      
    default:
      if (sc != (pc&~QUOTE))
	return(0);
    }
  }
  return(*s == 0);
}


/**************************************************************************/


static char *cclass(p, sub)
     register char *p;
     register int sub;
{
  register int c, d, not, found;
  
  if ((not = *p == '!') != 0)
    p++;
  found = not;
  do {
    if (*p == '\0')
      return(NULL);
    c = *p & CMASK;
    if (p[1] == '-' && p[2] != ']') {
      d = p[2] & CMASK;
      p++;
    } else
      d = c;
    if (c == sub || c <= sub && sub <= d)
      found = !not;
  } while (*++p != ']');
  return(found? p+1: NULL);
}




/************************************************************************/



char *itoa(int n)
{
  static int next;
  static char qbuf[8];
  register int r, k;
  int flag = 0;
  
  next = 0;
  if (n < 0) {
    qbuf[next++] = '-';
    n = -n;
  }
  if (n == 0) 
    qbuf[next++] = '0';
  else {
    k = 10000;
    while (k > 0) {
      r = n/k;
      if (flag || r > 0) {
	qbuf[next++] = '0' + r;
	flag = 1;
      }
      n -= r * k;
      k = k/10;
    }
  }
  qbuf[next] = 0;
  return(qbuf);
}


/**************************************************************************/




char *flgets(char *str, int n, FILE *file, int *lcnt)
{
  int ch;
  char *ptr;
  
  ptr = str;
  while ( --n > 0 && (ch = getc(file)) != EOF && ch != '\n' && ch != '#' )
    *ptr++ = ch;
  if (ch == '#')
      while(getc(file)!='\n')
	;
  if (ch == EOF)
    return(NULL);
    *ptr = '\0';
  *lcnt += 1;

  return(str);
}


/**************************************************************************/



char *findrc()
{
  char *str, *n;
  char **path = NULL;
  int cnt = 0;
  int i;

  str = strenv(PROFILE_PATH);
  path = wordlist(str, path, &cnt, is_pathend);

  puts(path[0]);
  for (i = 0 ; i < cnt ; i++) {
    if (access(path[i], R_OK) == 0) {
      puts("b");
      n = malloc(strlen(path[i]) + 1);
      strcpy(n, path[i]);
      free_char_array(path);
      free(str);
      return n;
    }
  }

  fprintf(stderr, CANNOT_FIND_RC);
  free_char_array(path);
  free(str);
  
  return NULL;
}

/**************************************************************************/

char *shortpath(char *s)
{
  char *str;
  char *envhome = getenv("HOME");
  int lenhome = strlen(envhome);
  int len;

  if (!s)
    return NULL;
  len = strlen(s);
  str = strdup(s);
  if (!strncmp(s, envhome, lenhome)) {
    strcpy(str, "~");
    strncat(str, &s[lenhome], len - lenhome);
  }
  if (str[strlen(str)-1] != '/') {
    str = realloc(str, strlen(str) + 2);
    strcat(str, "/");
  }
  
  return str;
}



int have_access(struct stat *st)
{
  int flag = 0;

  if (UID == 0)
    return 1;

  if (UID == st->st_uid && st->st_mode & S_IREAD && st->st_mode & S_IEXEC) 
    flag = 1;
  else
    if (GID == st->st_gid && st->st_mode & S_IRGRP && st->st_mode & S_IXGRP)
      flag = 1;
    else
      if (st->st_mode & S_IROTH && st->st_mode & S_IXOTH)
	flag = 1;
  
  return flag;
}

int pos(char *s, int c)
{
  int i = 0;	
  
  while( *s && *s++ != c) i++;

  return *s ? i : -1;
}
	


int rpos(char *s, int c)
{
  int i = 0;
	
  while( *++s ) i++;
  while( *--s && *s != c) i--;

  return *s ? i : -1;
}



char *convert_filename_to_printable(unsigned char *name, int max_len)
{
  char *str;
  char  octstr[20];
  int   i, j;

  if (!name)
    return NULL;
  if (max_len == -1)
    max_len = 1024;
  str = malloc(max_len + 1);
  i = 0;
  while(*name) {
    if (!isprint(*name) && strchr("äöüßÄÖÜ", *name) == NULL) {
      sprintf(octstr, "\\%3.3o", *name);
      j = 0;
      while(str[i++] = octstr[j++]) {
	if (i == max_len) {
	  str[i-1] = '\\';
	  str[i] = 0;
	  return str;
	}
      }
      i--;
      name++;
      continue;
    } 
    if (i == max_len) {
      str[i-1] = '\\';
      break;
    }
    str[i++] = *name++;
  }
  str[i] = 0;

  return str;
}      

int detect_invalid_filename(char *n)
{
  do 
    if (!isprint(*n))
      return 1;
  while(*++n);

  return 0;
}


int copyfile(char *from, char *to)
{
  char cpbuf[COPY_BUFFER];
  int m,n;
  int f1,f2;
  struct stat st;
  
  stat(from,&st);
  f1 = open(from, 0);
  f2 = creat(to, st.st_mode);
  if (f1 < 0 || f2 < 0)
    return 1;
  do {
    n = read(f1, cpbuf, COPY_BUFFER);
    m = write(f2, cpbuf, n);
    if (m != n) {
      close(f1);
      close(f2);
      return 1;
    }
  } while (n == COPY_BUFFER);
  close(f1);
  close(f2);
  
  return 0;
}



int renaming(char *from, char *to)
{	
  char *dest = (char *)malloc(strlen(from)+strlen(to)+2); 
  struct stat st;
  int err = 0;

  strcpy(dest,to);
  if (!stat(to,&st)) {
    if (st.st_mode & S_IFMT & S_IFDIR) {
      if (dest[strlen(dest)-1] != '/') 
	 strcat(dest,"/");
      strcat(dest,from);
    } else {
      put_msg("Datei existiert bereits.");
      free(dest);
      return 1;
    }
  }
  if (link(from, dest) < 0) 
    err = copyfile(from,dest);
  if (!err)
    unlink(from);
  free(dest);
  
  return err; 
}




int valid_pathname(char *s)
{
  if (s[0] != '/')
    return 0;
  if (strstr(s, "/../") || strstr(s, "/./") || strstr(s, "//") ||
      !strcmp(strrchr(s,'/'), "/..") || !strcmp(strrchr(s, '/'), "/.")) 
    return 0;
  return 1;
}


int check_dir(char *s)
{
  struct stat st;

  if (!valid_pathname(s))
    return 1;
  if (stat(s, &st))
    return 2;
  if ((st.st_mode & S_IFMT) != S_IFDIR)
    return 3;
  if (!have_access(&st)) 
    return 4;
  return 0;
}


char *dstrcat(char *buf, char *add)
{
  if (buf == NULL) {
    buf = malloc(strlen(add) + 1);
    *buf = 0;
  } else
    buf = realloc(buf, strlen(buf) + strlen(add) + 1);

  strcat(buf, add);

  return buf;
}


char **add_to_list(char **list, char *str)
{
  int i = 0;

  if (list == NULL) {
    list = malloc(sizeof(char *) * 2);
  } else {
    for (i = 0 ; list[i] != NULL ; i++)
      ;
    list = realloc(list, sizeof(char *) * (i + 2));
  }
  list[i] = strdup(str);
  list[i+1] = NULL;

  return list;
}

char *volume_str(double bytes)
{
  static char str[50], *p;
  
  if (bytes > 1024. * 1024 * 1024 * 1024 * 1024)
    snprintf(str, sizeof(str), "%4.1fP", bytes / 1024 / 1024 / 1024 / 1024 / 1024 );
  else if (bytes > 1024. * 1024 * 1024 * 1024)
    snprintf(str, sizeof(str), "%4.1fT", bytes / 1024 / 1024 / 1024 / 1024);
  else if (bytes > 1024. * 1024 * 1024)
    snprintf(str, sizeof(str), "%4.1fG", bytes / 1024 / 1024 / 1024);
  else if (bytes > 1024. * 1024)
    snprintf(str, sizeof(str), "%4.1fM", bytes / 1024 / 1024);
  else if ( bytes > 1024.)
    snprintf(str, sizeof(str), "%4.1fk", bytes / 1024);
  else
    snprintf(str, sizeof(str), "%4.0f ", bytes);
  
  p = strchr(str, '.');
  if (p != NULL && *(p + 1) == '0' ) {
    *p = *(p + 2);
    *(p + 1) = 0;
  }

  return str;
}

inline int isdigitstr(char *s)
{
  if (s == NULL || *s == 0)
    return 0;
  
  while(*s) {
    if (!isdigit(*s))
      return 0;
    s++;
  }

  return 1;
}

char *trim(char *s)
{
  char *p;
 
  while(isspace(*s))
    s++;
 
  if (!*s)
    return s;
 
  p = s;
  while(*p)
    p++;
  p--;
  while(isspace(*p))
    p--;
  *(p+1) = 0;
 
  return s;
}
