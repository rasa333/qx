#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>
#include "defs.h" 

extern int LINES, COLS;

DIR_LIST *dl_layout(DIR_LIST *dl)
{
    int check, len, i, max_filename_len;
    char *str, tmp[50];
    char *buffer = (char *)malloc(COLS + 1);
    char *sfmt = (char *)malloc(COLS + 1);

    if (strstr(DL_select.layout_tbl[dl->layout_now], "%S")) {
	dl->max_sizestr_len = 0;
	for (i = 0 ; i < dl->fl->no_of_files ; i++) {
	    switch(dl->fl->fl[i].stat.st_mode & S_IFMT) {
	    case S_IFBLK:
	    case S_IFCHR:
		sprintf(tmp, MAJOR_MINOR_FMT, major(dl->fl->fl[i].stat.st_rdev), minor(dl->fl->fl[i].stat.st_rdev));
		break;
	    default:
		sprintf(tmp, "%s", volume_str(dl->fl->fl[i].stat.st_size));
		break;
	    }
	    if ((len = strlen(tmp)) > dl->max_sizestr_len)
		dl->max_sizestr_len = len;
	}
	dl->max_sizestr_len++;
    }
    
    max_filename_len = dl->max_filename_len;
    do {
	len = check = 0;
	*dl->label = 0;
	str = DL_select.layout_tbl[dl->layout_now];
	while(*str) {
	    if (*str == '%') {
		str++;
		switch(*str) {
		case 'n':
		case 'N':
		    len += max_filename_len;
		    if (len > COLS-1)
			break;
		    if (isupper(*str)) {
			sprintf(sfmt,"%%-%d.%ds", max_filename_len, dl->max_filename_len);
		    } else {
			sprintf(sfmt,"%%%d.%ds", max_filename_len, dl->max_filename_len);
		    }
		    sprintf(buffer, sfmt, "Name");
		    strcat(dl->label, buffer);
		    break;
		case 's':
		case 'S':
		    len += dl->max_sizestr_len;
		    if (len > COLS-1)
			break;
		    sprintf(sfmt, "%%%d.%ds", dl->max_sizestr_len, dl->max_sizestr_len);
		    sprintf(buffer, sfmt, "Size");
		    strcat(dl->label, buffer);
		    break;
		case 'o':
		    len += 8;
		    if (len > COLS-1)
			break;
		    strcat(dl->label,"   Owner");
		    break;
		case 'O': 
		    len += 8;
		    if (len > COLS-1)
			break;
		    strcat(dl->label,"Owner   ");
		    break;
		case 'g': 
		    len += 8;
		    if (len > COLS-1)
			break;
		    strcat(dl->label,"   Group");
		    break;
		case 'G': 
		    len += 8;
		    if (len > COLS-1)
			break;
		    strcat(dl->label,"Group   ");
		    break;
		case 'a':
		    len += 12;
		    if (len > COLS-1)
			break;
		    strcat(dl->label,"Access Date ");
		    break;
		case 'c':
		    len += 12;
		    if (len > COLS-1)
			break;
		    strcat(dl->label,"Creat Date  ");
		    break;
		case 'm':
		    len += 12;
		    if (len > COLS-1)
			break;
		    strcat(dl->label,"Modify Date ");
		    break;
		case 'P':
		    len += 10;
		    if (len > COLS-1)
			break;
		    strcat(dl->label,"Permission");
		    break;
		case 'p':
		    len += 4;
		    if (len > COLS-1)
			break;
		    strcat(dl->label,"Perm");
		    break;
		case 'i':
		case 'I':
		    len += 6;
		    if (len > COLS-1)
			break;
		    strcat(dl->label, "Inode");
		    break;
		case 'l':
		case 'L':
		    len += 3;
		    if (len > COLS-1)
			break;
		    strcat(dl->label,"Lnk");
		    break;
		case 't':
		    len++;
		    if (len > COLS-1)
			break;
		    strcat(dl->label," ");
		    break;
		case '=':
		    dl->position.p_cursor = len;
		    len += strlen(DL_select.cursor);
		    if (len > COLS-1)
			break;
		    strcat(dl->label, DL_select.cursor_spaces);
		    check++;
		    break;
		case '^':
		    dl->position.p_selector = len;
		    len++;
		    if (len > COLS-1)
			break;
		    strcat(dl->label," ");
		    check++;
		    break;
		default:
		    fprintf(stderr, UNKNOWN_LAYOUT_CHAR);
		    qx_exit(-1);
		    break;
		}
	    } else {
		len++;
		if (len < COLS)
		    strcat(dl->label, " ");
	    }
	    str++;
	}
	if (len > COLS - 1)
	    max_filename_len--;
    } while(len > COLS && max_filename_len > 2);
    if (check != 2) {
	if (check < 2)
	    fprintf(stderr, CURSOR_OR_PTR_MISSING, dl->layout_now ? itoa(dl->layout_now) : "DEFAULT layout");
	else
	    fprintf(stderr, ONE_CURSOR_OR_PTR_ONLY, dl->layout_now ? itoa(dl->layout_now) : "DEFAULT layout");
	clrtobot();
	qx_exit(1);
    }
    free(buffer);
    free(sfmt);
    dl->max_filename_len_cut = max_filename_len;
    dl->layout_entry_len = len + strlen(DL_select.separator); 
    if (dl->layout_entry_len > COLS)
	dl->max_line_entries = 1;
    else
	dl->max_line_entries = COLS / dl->layout_entry_len;
    dl->screen.x = ENTRY_LENGTH * (dl->flpos % MLE) + dl->position.p_cursor;
    if (dl->flpos == 1)
	dl->screen.y = dl->flpos / MLE + 4;
    else
	dl->screen.y = (Y_CUR_LINE - Y_TOP_LINE) + 4;
    if (Y_TOP_LINE < 0)
	dl->screen.y = Y_CUR_LINE + 4;
    dl->oldscr = dl->screen;

    return dl;
}


DIR_LIST *dl_setupdir(DIR_LIST *dl)
{
    int i, len, slen;
    char tmp[20], file[1024], buf[1024], *s;

    dl->tagged = (dl->tagged ? realloc(dl->tagged, dl->fl->no_of_files) :
		  malloc(dl->fl->no_of_files));
    dl->max_filename_len = dl->max_sizestr_len = 0;
    for (i = 0 ; i < dl->fl->no_of_files ; i++) {
	s = convert_filename_to_printable(dl->fl->fl[i].name, -1);
	len = strlen(s);
	free(s);
	if ((dl->fl->fl[i].stat.st_mode & S_IFMT) ==  S_IFLNK) {
	    snprintf(file, sizeof(file), "%s/%s", dl->cwd, dl->fl->fl[i].name);
	    slen = readlink(file, buf, sizeof(buf));
	    if (slen != -1) {
		buf[slen] = 0;
		s = convert_filename_to_printable(buf, -1);
		slen = strlen(s);
		free(s);
		if (len + slen + 4 > dl->max_filename_len)
		    dl->max_filename_len = len + slen + 4;
	    }
	} else {
	    if (len > dl->max_filename_len)
		dl->max_filename_len = len;
	}
	switch(dl->fl->fl[i].stat.st_mode & S_IFMT) {
	case S_IFBLK:
	case S_IFCHR:
	    sprintf(tmp, MAJOR_MINOR_FMT, major(dl->fl->fl[i].stat.st_rdev), minor(dl->fl->fl[i].stat.st_rdev));
	    break;
	default:
	    sprintf(tmp, "%ld", dl->fl->fl[i].stat.st_size);
	    break;
	}
	if ((len = strlen(tmp)) > dl->max_sizestr_len)
	    dl->max_sizestr_len = len;
	dl->tagged[i] = 0;
    }
    dl->max_sizestr_len++;

    return dl;
}


DIR_LIST *dl_newdir(DIR_LIST *dl, int flpos_mode)
{
    int i;
    char *fname = NULL;

    if (flpos_mode == NEWDIR_KEEP && dl->fl != NULL)
	fname = strdup(dl->fl->fl[dl->flpos].name);
    if (dl->fl != NULL)
	freedir(dl->fl);
    dl->fl = getdir(dl->cwd, dl->nohidden);        /* Never return -1, please! */
    dl = (DIR_LIST *)dl_setupdir(dl);

    switch(flpos_mode) {
    case NEWDIR_RESET:
	dl->flpos = 0;
	for (i = 0 ; i < dl->fl->no_of_files ; i++) {
	    if (!strcmp(dl->fl->fl[i].name, "..")) {
		dl->flpos = i;
		break;
	    }
	}
	break;

    case NEWDIR_KEEP:
	if (dl->flpos > dl->fl->no_of_files - 1) 
	    dl->flpos = dl->fl->no_of_files - 1;
	else {
	    if (fname != NULL) {
		for (i = 0 ; i < dl->fl->no_of_files ; i++) {
		    if (!strcmp(dl->fl->fl[i].name, fname)) {
			dl->flpos = i;
			break;
		    }
		}
	    } else {
		dl->flpos = 0;
		for (i = 0 ; i < dl->fl->no_of_files ; i++) {
		    if (!strcmp(dl->fl->fl[i].name, "..")) {
			dl->flpos = i;
			break;
		    }
		}
	    }
	}
	break;
    }
    if (fname != NULL)
	free(fname);
  
    return dl;
}


DIR_LIST *dl_updatedir(DIR_LIST *dl, int dlidx)
{
    DI *tmp_d;
    int i;
    int flp = 0, update = 0;
    long highest_mtime = 0L;

    tmp_d = getdir(dl->cwd, dl->nohidden);
    if (dl->fl->no_of_files != tmp_d->no_of_files) {
	update = 1;
	for (i = 2 ; i < tmp_d->no_of_files ; i++)
	    if (tmp_d->fl[i].stat.st_mtime > highest_mtime) {
		highest_mtime = tmp_d->fl[i].stat.st_mtime;
		flp = i;
	    }
    } else {
	for (i = 2 ; i < dl->fl->no_of_files ; i++) {
	    if (tmp_d->fl[i].stat.st_mtime != dl->fl->fl[i].stat.st_mtime) {
		update = 1;
		if (tmp_d->fl[i].stat.st_mtime > highest_mtime) {
		    highest_mtime = tmp_d->fl[i].stat.st_mtime;
		    flp = i;
		}
	    }
	}
    }
    if (update) {
	freedir(dl->fl);
	dl->fl = tmp_d;
	dl->need_update = 1;
	if (DL_select.sdir[dlidx].set_flpos_to_newest)
	    dl->flpos = flp;
	dl = (DIR_LIST *)dl_setupdir(dl);
    } else
	freedir(tmp_d);
  
    return dl;
}
  


DIR_LIST *dl_init(char *wd, bool nohidden)
{
    DIR_LIST *dl;

    dl              = malloc(sizeof(DIR_LIST));
    dl->fl          = NULL;
    dl->tagged      = NULL;
    dl->cwd         = wd;
    dl->no_of_files_tagged = 0;
    dl->label       = malloc(COLS + 1);
    dl->layout_now  = 0;
    dl->need_update = 0;
    dl->flpos       = 0;
    dl->nohidden    = nohidden;
    dl              = dl_newdir(dl, NEWDIR_RESET);

    return dl;
}

void dl_free(DIR_LIST *dl)
{
    if (dl->fl != (DI *)-1 && dl->fl != (DI *)0)
	freedir(dl->fl);
    if (dl->cwd) {
	free(dl->cwd);
	dl->cwd = NULL;
    }
    if (dl->tagged)
	free(dl->tagged);
    free(dl->label);
    free(dl);
    dl = NULL;
}
