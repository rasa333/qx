#include <pwd.h>       
#include <sys/types.h>
#include <grp.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include "defs.h"

extern int LINES, COLS;
extern int KEY_MAX;
extern KEYS *keytab;

char *mo[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/**************************************************************************/

void put_lines(DIR_LIST *dl, int begin_line, int no_of_lines)
{
    int i , j = 0;           /* i and j for the two main loops */
    int fmode, c, len;
    long t,cur_time;
    char *line = (char *)malloc(COLS + 1);
    char *lo,*perm_buf;
    char *ntmp;
    char *buffer = (char *)malloc(COLS + 1);
    char *sfmt = (char *)malloc(COLS + 1);
    char    tmp[20], file[2048], buf[1024];
    struct  passwd *pw;
    struct  group *gr;
    struct  tm *tm;
  
    for (i = begin_line ; i < begin_line + no_of_lines && j < dl->fl->no_of_files ; i++) {
	line[0] = 0;
	for (j = MLE * i ; j < MLE * i + MLE && j < dl->fl->no_of_files ; j++) {
	    lo = DL_select.layout_tbl[dl->layout_now];
	    while(*lo) {
		if (*lo == '%') {
		    switch(*++lo) {
		    case 'n':
		    case 'N':
			if ((dl->fl->fl[j].stat.st_mode & S_IFMT) == S_IFLNK) {
			    snprintf(file, sizeof(file), "%s/%s", dl->cwd, dl->fl->fl[j].name);
			    len = readlink(file, buf, sizeof(buf));
			    if (len < 0) {
				snprintf(file, sizeof(file), "%s", dl->fl->fl[j].name);
			    } else {
				buf[len] = 0;
				snprintf(file, sizeof(file), "%s -> %s", dl->fl->fl[j].name, buf);
			    }
			} else {
			    snprintf(file, sizeof(file), "%s", dl->fl->fl[j].name);
			}
			ntmp = (char *)convert_filename_to_printable(file, dl->max_filename_len_cut);
			if (isupper(*lo)) 
			    sprintf(sfmt, "%%-%d.%ds", dl->max_filename_len_cut, dl->max_filename_len_cut);
			else
			    sprintf(sfmt, "%%%d.%ds", dl->max_filename_len_cut, dl->max_filename_len_cut);
			sprintf(buffer, sfmt, ntmp);
			strcat(line, buffer);
			free(ntmp);
			break;
	    
		    case 's':
		    case 'S':
			switch(dl->fl->fl[j].stat.st_mode & S_IFMT) {
			case S_IFBLK: 
			case S_IFCHR: 
			    sprintf(tmp, MAJOR_MINOR_FMT, major(dl->fl->fl[j].stat.st_rdev), 
				    minor(dl->fl->fl[j].stat.st_rdev));
			    break;
			default:
			    if (isupper(*lo))
				sprintf(tmp, "%s", volume_str((double)dl->fl->fl[j].stat.st_size));
			    else
				sprintf(tmp, "%ld", dl->fl->fl[j].stat.st_size);
			    break;
			}
			sprintf(sfmt, "%%%d.%ds",dl->max_sizestr_len, dl->max_sizestr_len);
			sprintf(buffer, sfmt, tmp);
			strcat(line, buffer);
			break;
	    
		    case 'P':
		    case 'p':
			fmode = dl->fl->fl[j].stat.st_mode;
			switch(fmode & S_IFMT) {
			case S_IFLNK:
			    c = 'l';
			    break;
			case S_IFDIR: 
			    c = 'd';
			    break;
			case S_IFBLK: 
			    c = 'b';
			    break;
			case S_IFCHR: 
			    c = 'c';
			    break;
			case S_IFIFO: 
			    c = 'p';
			    break;
			case S_IFREG:
			    c = '-';
			    break;
			default: 
			    c = '?';
			    break;
			}
			tmp[0] = c;
			if (*lo == 'p') {
			    if (UID == dl->fl->fl[j].stat.st_uid) {
				tmp[1] = (fmode & S_IREAD) ? 'r' : '-';
				tmp[2] = (fmode & S_IWRITE)? 'w' : '-';
				tmp[3] = (fmode & S_IEXEC) ? 'x' : '-';
			    } else
				if (GID == dl->fl->fl[j].stat.st_gid) {
				    tmp[1] = (fmode & S_IRGRP) ? 'r' : '-';
				    tmp[2] = (fmode & S_IWGRP) ? 'w' : '-';
				    tmp[3] = (fmode & S_IXGRP) ? 'x' : '-';
				} else { 
				    tmp[1] = (fmode & S_IROTH) ? 'r' : '-';
				    tmp[2] = (fmode & S_IWOTH) ? 'w' : '-';
				    tmp[3] = (fmode & S_IXOTH) ? 'x' : '-';
				}
			    tmp[4] = 0;
			} else {
			    perm_buf = tmp;
			    perm_buf++;
			    for ( c = 0 ; c < 3 ; ++c ) {
				*perm_buf++ = ((fmode & (S_IREAD  >> ( c * 3 ))) ? 'r' : '-');
				*perm_buf++ = ((fmode & (S_IWRITE >> ( c * 3 ))) ? 'w' : '-');
				*perm_buf   = ((fmode & (S_IEXEC  >> ( c * 3 ))) ? 'x' : '-');
				if (!c && (fmode & S_ISUID))
				    *perm_buf = (*perm_buf == 'x' ? 's' : 'S');
				else 
				    if (c == 1 && (fmode & S_ISGID))
					*perm_buf = (*perm_buf == 'x' ? 's' : 'S');
				    else 
					if (c == 2 && (fmode & S_ISVTX))
					    *perm_buf = (*perm_buf == 'x' ? 't' : 'T');
				++perm_buf;
			    }
			    *perm_buf = 0;
			}
			strcat(line, tmp);
			break;
	    
		    case 'o':
		    case 'O':
			if (pw = getpwuid(dl->fl->fl[j].stat.st_uid))
			    strcpy(tmp, pw->pw_name);
			else 
			    strcpy(tmp, itoa(dl->fl->fl[j].stat.st_uid));
			if (isupper(*lo))
			    sprintf(buffer, "%-8.8s", tmp);
			else
			    sprintf(buffer,"%8.8s", tmp);
			strcat(line, buffer);
			break;
	    
		    case 'g':
		    case 'G':
			if (gr = getgrgid(dl->fl->fl[j].stat.st_gid))
			    strcpy(tmp, gr->gr_name);
			else 
			    strcpy(tmp, itoa(dl->fl->fl[j].stat.st_gid));
			if (isupper(*lo))
			    sprintf(buffer,"%-8.8s", tmp);
			else
			    sprintf(buffer,"%8.8d", tmp);
			strcat(line, buffer);
			break;
	    
		    case 'a': 
		    case 'c': 
		    case 'm': 
			if (*lo == 'a') 
			    t = dl->fl->fl[j].stat.st_atime;
			if (*lo == 'c') 
			    t = dl->fl->fl[j].stat.st_ctime;
			if (*lo == 'm') 
			    t = dl->fl->fl[j].stat.st_mtime;
			time(&cur_time);
			tm = localtime(&t);
			if (cur_time - t > 6L * 30L * 24L * 60L * 60L ||
			    cur_time - t < 0L) 
			    sprintf(tmp, "%s %0.2d  %d", mo[tm->tm_mon],tm->tm_mday,1900+tm->tm_year);
			else
			    sprintf(tmp, "%s %0.2d %0.2d:%0.2d", mo[tm->tm_mon],tm->tm_mday,tm->tm_hour,tm->tm_min);
			strcat(line,tmp);
			break;
	    
		    case 'l':
		    case 'L':
			if (isupper(*lo))
			    sprintf(buffer, "%-3.ld", dl->fl->fl[j].stat.st_nlink);
			else
			    sprintf(buffer, "%3.ld", dl->fl->fl[j].stat.st_nlink);
			strcat(line,buffer);
			break;
	    
		    case 'i':
		    case 'I':
			if (isupper(*lo))
			    sprintf(buffer, "%-6.ld", dl->fl->fl[j].stat.st_ino);
			else
			    sprintf(buffer, "%6.ld", dl->fl->fl[j].stat.st_ino);
			strcat(line, buffer);
			break; 
	    
		    case 't':
			fmode = dl->fl->fl[j].stat.st_mode;
			switch( fmode & S_IFMT ) {
			case S_IFDIR: 
			    tmp[0] = '/';
			    break;
			case S_IFBLK: 
			    tmp[0] = '#';
			    break;
			case S_IFCHR: 
			    tmp[0] = '%';
			    break;
			case S_IFIFO: 
			    tmp[0] = '|';
			    break;
			default:
			    tmp[0] = fmode & S_IEXEC ? '*' : ' ';
			    break;
			}
			tmp[1] = '\0';
			strcat(line, tmp);
			break;
	    
		    case '^':
			strcat(line, dl->tagged[j] ? "+" : " ");
			break;

		    case '=':
			strcat(line, DL_select.cursor_spaces);
			break;
		    }
		} else 
		    strncat(line, lo, 1);
		lo++;
	    }
	    if ((j+1) % MLE != 0)
		strcat(line, DL_select.separator);
	}
	fputs(line, stdout);
	clrtoeol();
	fputc('\n', stdout);
    }
    free(line);
    free(buffer);
    free(sfmt);
}


/**************************************************************************/



void put_proc_help()
{
    char *tmp;

    gotoxy(0, 1);
    fputs(Proc->pl[Proc->now].help, stdout);
    fputs(" [",stdout);
    tmp = ascii2str(Proc->pl[Proc->now].key);
    fputs(tmp,stdout);
    free(tmp);
    fputs("]",stdout);
    clrtoeol();
}


/**************************************************************************/


void put_path(DIR_LIST *dl)
{
    char *tmp;
    int i;

    tmp = (char *)shortpath(dl->cwd);
    if (tmp[(i = strlen(tmp)-1)] == '/' && i)
	tmp[i] = 0;
    gotoxy(0,2);
    fputs("Directory [",stdout);
    fputs(itoa(DL_select.now),stdout);
    fputs(",L",stdout);
    fputs(itoa(dl->layout_now),stdout);
    fputs("]: ",stdout);
    fputs(tmp,stdout);
    clrtoeol();
    free(tmp);
}


/**************************************************************************/


int put_msg(char *str)
{
    register int i = 0, div_value = 1;
    register char c;

    gotoxy( 0 , LINES - 1 );
    clrtoeol();
    while( *str ) {
	if (isprint(*str) || strchr("äöüßÄÖÜ", *str) || *str == '\t')
	    putchar( *str );
	if ( (i / (COLS - strlen(MORE_KEY_MSG)-2)) == div_value) {
	    div_value++;
	    while( !isspace(*str) ) {
		str--;
		i--;
		fputs("\b \b",stdout);
	    }
	    while( isspace(*str) ) {
		str--;
		i--;
		fputs("\b \b",stdout);
	    }
	    fputs(MORE_KEY_MSG,stdout);
	    c = _ikey();
	    if (c == 'q') 
		return 0;
	    putchar('\r');
	    clrtoeol();
	    while( isspace(*str) ) {
		str++;
		if ( *str == '\t' ) 
		    i+= 8-i%8;
		else
		    i++;
	    }
	}
	str++;
	if ( *str == '\t' ) 
	    i+= 8-i%8;
	else
	    i++;
    }
    fflush(stdout);
  
    return 1;
}


/**************************************************************************/


void put_proc_page()
{
    register int i;
  
    gotoxy(0,0);
    if (Proc->page_now)
	fputs("<< ",stdout);
    for (i = Proc->page_mem[Proc->page_now] ; i < Proc->page_mem[Proc->page_now+1] ; i++) {
	if (i == Proc->now)
	    if (DL_select.noinverse == FALSE)
		standout();
	fputs(Proc->pl[i].label,stdout);
	if (i == Proc->now)
	    if (DL_select.noinverse == FALSE)
		standend();
	if (i == Proc->now)
	    fputs("<  ",stdout);
	else
	    fputs("   ",stdout);
    }
    if (Proc->page_now < Proc->page_max-1)
	fputs(">> ",stdout);
    clrtoeol();
}


/**********************************************************************/



void put_proc_pointer(int label_to_set, int label_to_unset)
{
    register int len, i, cnt = 2, unset_set;
  
    while(cnt--) {
	unset_set = cnt ? label_to_unset : label_to_set;
	len = i = 0;
	while (i < unset_set - Proc->page_mem[Proc->page_now])
	    len += strlen(Proc->pl[Proc->page_mem[Proc->page_now]+i++].label)+3;
	len += Proc->page_now ? 3 : 0;
	gotoxy(len,0);
	if (!cnt)
	    if (DL_select.noinverse == FALSE)
		standout();
	write(1, Proc->pl[unset_set].label,strlen(Proc->pl[unset_set].label));
	if (!cnt) {
	    if (DL_select.noinverse == FALSE)
		standend();
	    putchar('<');
	} else
	    putchar(' ');
    }
    put_proc_help();
}


/**************************************************************************/


void put_proc_screen()
{
    put_proc_page();
    put_proc_help();
}

/**************************************************************************/


void put_dl_label(DIR_LIST *dl)
{
    int i;

    if (DL_select.noinverse == FALSE)
	standout();
    else {
	for (i = 0 ; i < strlen(dl->label) ; i++)
	    if (dl->label[i] == ' ')
		dl->label[i] = '_';
    }
    for (i = 0 ; i < (dl->fl->no_of_files < MLE ? dl->fl->no_of_files : MLE) ; i++) {
	fputs(dl->label, stdout);
	if (MLE > 1 && i != MLE-1)
	    fputs(DL_select.separator,stdout);
    }
    if (DL_select.noinverse == FALSE)
	standend();
    clrtoeol();
    putchar('\n');
}



/**************************************************************************/

void put_center(char *t)
{
    jump_xpos(COLS / 2 - strlen(t) / 2);
    puts(t);
}


/**************************************************************************/


void put_help_screen()
{
    int i,j = 0;
    char buf[512];
    char *tmp;

    clear();
    put_center("-*- (Q)uick uni(X) 2.0alpha -*-");
    put_center("Copyright by Heiko Jappe");
    puts("\n");
    put_center(HELP_BANNER);
    putchar('\n');
    puts(HELP_INTERNAL);
    putchar('\n');
    for (i = 1 ; i < KEY_MAX ; i++) {
	if (keytab[i].id < 1000)
	    if (strcmp(keytab[i].scan,"")) {
		j++;
		tmp = NULL;
		sprintf(buf,"%-7s %-20.20s: %s\n",
			keytab[i].scan[0] == 32 ? "\" \" " : 
			(tmp=(char *)ascii2str(keytab[i].scan)),
			keytab[i].kbd_name,keytab[i].help);
		if (tmp != NULL)
		    free(tmp);
		fputs(buf,stdout);
		if (j == 20 || j == 60) {
		    puts(HELP_SCREEN_PROMPT);
		    if (_ikey() == 'q')
			return;
		    clear();
		}
	    } else 
		i++;
    }
    putchar('\n');
    puts(HELP_SCREEN_PROMPT);
    if (_ikey() == 'q')
	return;

    /* User defined procs */

    j = 0;
    clear();
    puts(HELP_EXTERNAL);
    putchar('\n');
    for (i = 1 ; i < KEY_MAX ; i++) {
	if (keytab[i].id > 999)
	    if (strcmp(keytab[i].scan,"")) {
		j++;
		tmp = NULL;
		sprintf(buf,"%-7s %-7.7s: %-30s",
			keytab[i].scan[0] == 32 ? "\" \" " : 
			(tmp=(char *)ascii2str(keytab[i].scan)),
			keytab[i].kbd_name,keytab[i].help);
		if (tmp != NULL)
		    free(tmp);
		if (j%2) {
		    buf[38] = '|';
		    buf[39] = ' ';
		    buf[40] = '\0';
		} else 
		    buf[39] = '\0';
		fputs(buf,stdout);
		if (!(j%2))
		    putchar('\n');
		if (j == 20 || j == 60) {
		    puts(HELP_SCREEN_PROMPT);
		    if (_ikey() == 'q')
			return;
		    clear();
		}
	    } else 
		i++;
    }
    putchar('\n');
    puts(HELP_SCREEN_PROMPT);
    _ikey();
}


/**************************************************************************/

void put_dir(DIR_LIST *dl)
{
    gotoxy(0,3);
    put_dl_label(dl);
    put_lines(dl, Y_TOP_LINE, Y_SIZE_SCREEN);
    clrtobot();
}

/**************************************************************************/



void put_screen()
{
    clear();      
    put_proc_screen();
    put_path(DL_select.sdir[DL_select.now].dl_tbl);
    put_dir(DL_select.sdir[DL_select.now].dl_tbl);
}
