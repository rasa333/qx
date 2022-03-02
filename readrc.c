#include "defs.h"
#include "keydefs.h"
#include "readrc.h"

extern int LINES, COLS;
extern struct set_input set_input;

KEYS *keytab;
int KEY_MAX;

char **Prefix;
int PREFIX_MAX;



int key_cmp(KEYS *a, KEYS *b)
{
    return strcmp(a->scan, b->scan);
}

/**************************************************************************/

int readrc(char *rc_name)
{
    int i = 0, n, j, rc;
    char cmd[80];
    char *errorptr = (char *) 0;
    char **arg = (char **) 0;
    char *tmp;
    int line_cnt = 0, page_len = 0, cnt = 0;
    int nopatterns, found;
    FILE *fp = fopen(rc_name, "r");

    KEY_MAX = sizeof(std_keytab) / sizeof(KEYS);
    keytab = (KEYS *) malloc(sizeof(KEYS) * KEY_MAX);
    for (i = 0; i < KEY_MAX; i++)
        keytab[i] = std_keytab[i];

    PREFIX_MAX = sizeof(std_prefix) / sizeof(char **);
    Prefix = (char **) malloc(sizeof(char **) * PREFIX_MAX);
    for (i = 0; i < PREFIX_MAX; i++)
        Prefix[i] = strdup(std_prefix[i]);

    Proc = NULL;
    while (errorptr == NULL && flgets(cmd, 80, fp, &line_cnt)) {
        puts(cmd);
        n = parser(cmd, rc_main_cmds, &arg, &cnt);

        switch (n) {
	case _PROC:
	  if (Proc == NULL) {
                    Proc = malloc(sizeof(PROC));
                    Proc->pl = malloc(sizeof(PROC_LIST));
                    Proc->max = 0;
                    Proc->now = 0;
                    Proc->page_max = 1;
                    Proc->page_now = 0;
                    Proc->page_mem = malloc(sizeof(int));
                    Proc->page_mem[0] = 0;
                } else {
                    Proc->max++;
                    Proc->pl = realloc(Proc->pl, sizeof(PROC_LIST) * (Proc->max + 1));
                }
                Proc->pl[Proc->max].exec = NULL;
                Proc->pl[Proc->max].exec_cnt = 0;
                Proc->pl[Proc->max].label = strdup(arg[1]);
                page_len += strlen(Proc->pl[Proc->max].label) + 3;

                if (page_len > COLS) {
                    Proc->page_mem = realloc(Proc->page_mem, sizeof(int) * (Proc->page_max + 1));
                    Proc->page_mem[Proc->page_max++] = Proc->max;
                    page_len = strlen(Proc->pl[Proc->max].label) + 6;
                }

                while (errorptr == (char *) 0 && flgets(cmd, 80, fp, &line_cnt) && n != _END) {

                    n = parser(cmd, rc_proc_cmds, &arg, &cnt);
                    switch (n) {

                        case _HELP:
                            Proc->pl[Proc->max].help = (char *) malloc(strlen(arg[1]) + 1);
                            strcpy(Proc->pl[Proc->max].help, arg[1]);
                            break;

                        case _EXEC:
                            nopatterns = FALSE;
                            Proc->pl[Proc->max].exec = (Proc->pl[Proc->max].exec == NULL ? malloc(sizeof(EXEC)) :
                                                        realloc(Proc->pl[Proc->max].exec,
                                                                sizeof(EXEC) * (Proc->pl[Proc->max].exec_cnt + 1)));
                            Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].str = NULL;
                            Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].list = NULL;
                            for (i = 1; i < cnt; i++) {
                                if (!strcmp(arg[i], "|") || !strcmp(arg[i], "||")) {
                                    if (!strcmp(arg[i], "||"))
                                        nopatterns = TRUE;
                                    i++;
                                    break;
                                }
                                Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].str =
                                        dstrcat(Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].str, arg[i]);
                                Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].str =
                                        dstrcat(Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].str, " ");
                            }
                            trim(Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].str);

                            if (nopatterns == FALSE) {
                                for (; i < cnt; i++) {
                                    if (!strcmp(arg[i], "|")) {
                                        i++;
                                        break;
                                    }
                                    Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].list =
                                            add_to_list(Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].list,
                                                        arg[i]);
                                }
                            }
                            found = i;
                            Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].mode = 0;
                            for (; i < cnt; i++) {
                                for (j = 1; rc_procmode_cmds[j][0]; j++) {
                                    if (!strcmp(arg[i], rc_procmode_cmds[j])) {
                                        found++;
                                        switch (j) {
                                            case 1:
                                                Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].mode |= P__SURE;
                                                break;
                                            case 2:
                                                Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].mode |= P__UPDATE;
                                                break;
                                            case 3:
                                                Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].mode |= P__CLS;
                                                break;
                                            case 4:
                                                Proc->pl[Proc->max].exec[Proc->pl[Proc->max].exec_cnt].mode |= P__WAIT;
                                                break;
                                        }
                                    }
                                }
                            }
                            if (found != cnt) {
                                errorptr = RC_PROC_UNKNOWN_MODE;
                                break;
                            }
                            Proc->pl[Proc->max].exec_cnt++;
                            break;

                        case _KEY:
                            tmp = (char *) str2ascii(arg[1]);
                            for (i = 0; i < Proc->max; i++) {
                                if (!strcmp(tmp, Proc->pl[i].key)) {
                                    errorptr = RC_PROC_KEY_BUSY;
                                    break;
                                }
                            }
                            Proc->pl[Proc->max].key = tmp;
                            if (cnt == 3) {
                                Proc->pl[Proc->max].kbd_name = malloc(strlen(arg[2]) + 1);
                                strcpy(Proc->pl[Proc->max].kbd_name, arg[2]);
                            } else {
                                Proc->pl[Proc->max].kbd_name = malloc(1);
                                strcpy(Proc->pl[Proc->max].kbd_name, "");
                            }
                            break;

                        case -1:
                            errorptr = RC_PROC_UNKNOWN_CMD;
                            break;
                    }
                }
                break;

            case _BINDKEY:
                if (cnt < 3) {
                    errorptr = RC_ARG_MISSING;
                    break;
                }
                tmp = (char *) str2ascii(arg[2]);
                for (i = 0; rc_key_cmds[i][0] != 0; i++) {
                    if (!strcmp(arg[1], rc_key_cmds[i])) {
                        keytab[i - 1].scan = (char *) tmp;
                        if (cnt == 4) {
                            keytab[i - 1].kbd_name = (char *) malloc(strlen(arg[3]) + 1);
                            strcpy(keytab[i - 1].kbd_name, arg[3]);
                        } else {
                            keytab[i - 1].kbd_name = (char *) malloc(1);
                            strcpy(keytab[i - 1].kbd_name, "");
                        }
                        break;
                    }
                }
                break;

            case _UNBINDKEY:
                if (cnt != 2) {
                    errorptr = RC_ARG_MISSING;
                    break;
                }
                for (i = 0; rc_key_cmds[i][0] != 0; i++) {
                    if (!strcmp(arg[1], rc_key_cmds[i])) {
                        keytab[i - 1].scan = (char *) 0;
                        break;
                    }
                }
                if (i == KEY_MAX)
                    errorptr = RC_BINDKEY_FUNC_NON_EXIST;
                break;

            case _LAYOUT:
                if (cnt != 3) {
                    errorptr = RC_LAYOUT_ARG_OUT_OF_RANGE;
                    break;
                }
                i = atoi(arg[1]);
                if (i < 0 || i > 9) {
                    errorptr = RC_LAYOUT_ILL_ARG_RANGE;
                    break;
                }
                if (DL_select.layout_tbl[i]) {
                    errorptr = RC_LAYOUT_ENTRY_BUSY;
                    break;
                }
                DL_select.layout_tbl[i] = malloc(strlen(arg[2]) + 1);
                strcpy(DL_select.layout_tbl[i], arg[2]);
                break;

            case _SEPARATOR:
                if (cnt != 2) {
                    errorptr = RC_ARG_MISSING;
                    break;
                }
                DL_select.separator = strdup(arg[1]);
                break;

            case _HISTORY:
                if (cnt != 2) {
                    errorptr = RC_ARG_MISSING;
                    break;
                }
                set_input.history_flag = atoi(arg[1]);
                if (set_input.history_flag < 0 || set_input.history_flag > 1000)
                    set_input.history_flag = 20;
                break;

            case _CURSOR:
                if (cnt != 2) {
                    errorptr = RC_ARG_MISSING;
                    break;
                }
                DL_select.cursor = strdup(arg[1]);
                break;

            case _SELECTDIR:
                if (cnt < 3 || cnt > 6) {
                    errorptr = RC_SELECT_ARG_OUT_OF_RANGE;
                    break;
                }
                i = atoi(arg[1]);
                if (i < 0 || i > 11) {
                    errorptr = RC_SELECT_ILL_ARG_RANGE;
                    break;
                }
                if (DL_select.sdir[i].wd_tbl == NULL)
                    DL_select.sdir[i].wd_tbl = strenv(arg[2]);
                switch (rc = check_dir(DL_select.sdir[i].wd_tbl)) {
                    case 1:
                        errorptr = RC_SELECT_PATH_NOT_ABSOLUT;
                        break;
                    case 2:
                        errorptr = RC_SELECT_DIR_NON_EXIST;
                        break;
                    case 3:
                        errorptr = RC_SELECT_NOT_A_DIR;
                        break;
                    case 4:
                        errorptr = RC_SELECT_PERM_DENIED;
                        break;
                }
                if (rc)
                    break;
                if (cnt > 3) {
                    for (j = 3; j < cnt; j++) {
                        if (isdigitstr(arg[j]))
                            DL_select.sdir[i].time_tbl = atoi(arg[j]);
                        else if (!strcmp(arg[j], "*"))
                            DL_select.sdir[i].set_flpos_to_newest = TRUE;
                        else if (!strcasecmp(arg[j], "nohidden"))
                            DL_select.sdir[i].nohidden = TRUE;
                    }
                }
                break;

            case _PREFIX:
                if (cnt != 3) {
                    errorptr = RC_ARG_MISSING;
                    break;
                }
                if (!strcmp(arg[1], "add")) {
                    tmp = str2ascii(arg[2]);
                    for (i = 0; i < PREFIX_MAX; i++)
                        if (!strcmp(tmp, Prefix[i])) {
                            errorptr = RC_PREFIX_EXIST;
                            break;
                        }
                    PREFIX_MAX++;
                    Prefix = (char **) realloc(Prefix, sizeof(char **) * PREFIX_MAX);
                    Prefix[PREFIX_MAX - 1] = (char *) strdup(tmp);
                    free(tmp);
                } else if (!strcmp(arg[1], "del")) {
                    tmp = (char *) str2ascii(arg[2]);
                    for (i = 0; strcmp(tmp, Prefix[i]) && i < PREFIX_MAX; i++);
                    free(tmp);
                    if (i == PREFIX_MAX) {
                        errorptr = RC_PREFIX_NON_EXIST;
                        break;
                    }
                    for (; i < PREFIX_MAX; i++)
                        Prefix[i] = Prefix[i + 1];
                    PREFIX_MAX--;
                    Prefix = (char **) realloc(Prefix, (PREFIX_MAX + 1) * sizeof(char *));
                } else
                    errorptr = RC_PREFIX_ONLY_ADDORDEL;
                break;

            case _NOINVERSE:
                DL_select.noinverse = TRUE;
                break;

            case -1:
                errorptr = RC_UNKNOWN_COMMAND;
                break;
        }
    }
    fclose(fp);

    if (errorptr) {
        fprintf(stderr, "%s\n%s:%d:%s\n", RC_ERROR_STRING, rc_name, line_cnt, errorptr);
        qx_exit(-1);
    }


    if (Proc->max) {
        Proc->page_mem = (int *) realloc(Proc->page_mem, sizeof(int) * (Proc->page_max + 1));
        Proc->page_mem[Proc->page_max] = Proc->max + 1;
    }
    KEY_MAX += Proc->max;
    keytab = (KEYS *) realloc(keytab, sizeof(KEYS) * (KEY_MAX + 1));

    for (n = KEY_MAX - Proc->max; n < KEY_MAX + 1; n++) {
        keytab[n].scan = Proc->pl[n - KEY_MAX + Proc->max].key;
        keytab[n].id = 1000 + (n - KEY_MAX + Proc->max);
        keytab[n].kbd_name = Proc->pl[n - KEY_MAX + Proc->max].kbd_name;
        keytab[n].help = Proc->pl[n - KEY_MAX + Proc->max].help;
    }
    free_char_array(arg);
    KEY_MAX++;

    qsort(keytab, KEY_MAX, sizeof(KEYS), (void *) key_cmp);

    set_input.update_line_edit_buf_max_disp = set_input.default_cols = COLS;
}
