#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sysmacros.h>

#include "input.h"
#include "dir.h"
#include "config.h"


#define SPACE               32
#define ESC	            27
#define DEL	           127
#define BELL                 7    /* abort key - emacs like */
#define BS	             8
#define TAB		     9    /* input() - path expand key */
#define CR	            13


/* key-sequence identification for keytab table */

#define C_RT		   300
#define _C_RT              301
#define C_LF	           302
#define _C_LF              303
#define C_UP		   304
#define _C_UP              305
#define C_DN		   306
#define _C_DN              307
#define C_END		   308
#define _C_END             309
#define C_HOME	           310
#define _C_HOME            311
#define C_PGUP             312
#define _C_PGUP            313
#define C_PGDN	           314
#define _C_PGDN            315
#define QX_QUIT            316
#define QX_TAG             317
#define QX_HELP	           318
#define QX_MATCH           319
#define QX_REFRESH         320
#define QX_TAGALL	   321
#define QX_CHDIR	   322
#define QX_INC             323
#define QX_DEC             324
#define QX_LO              325
#define QX_LO1             326
#define QX_LO2             327
#define QX_LO3             328
#define QX_LO4             329
#define QX_LO5             330
#define QX_LO6             331
#define QX_LO7             332
#define QX_LO8             333
#define QX_LO9             334
#define QX_UNTAGALL        335
#define QX_GOTO_DIR        336
#define QX_UPDATE_DIR      337
#define QX_SHELL           338
#define QX_GOTO_HOME       339
#define QX_START_PROC      340
#define QX_SELECT          341
#define QX_SELECT1         342
#define QX_SELECT2         343
#define QX_SELECT3         344
#define QX_SELECT4         345
#define QX_SELECT5         346
#define QX_SELECT6         347
#define QX_SELECT7         348
#define QX_SELECT8         349
#define QX_SELECT9         350
#define QX_SELECT10        351
#define QX_SELECT11        352
#define QX_RENAME          353   
#define QX_FULLNAME        354
#define QX_SHOWDIRTABLE    355
#define QX_TAG_INVERSE     356
#define QX_NOHIDDEN        357

typedef struct {
    char *scan;
    int   id;
    char *kbd_name;
    char *help;
} KEYS;

extern int UID, GID;           /* getuid() and getgid() */



/* Messages and prompts */

#ifdef GERMAN
/* Sometimes qx will ask for "yes" or "no". This is for the "sure" feature */
#define KEY_FOR_YES          'j'
#define KEY_FOR_NO           'n'
#define PRESS_KEY_MSG       "Taste fuer weiter ..."
#define NOT_A_DIR           "Kein Verzeichnis"
#define DIR_PROMPT          "Verzeichnis: "
#define PERMISSION_DENIED   "Keine Berechtigung"
#define NOT_UNIQUE          "Nicht eindeutig"
#define CANNOT_FIND_RC      "\".qxrc\" kann nicht geoeffnet werden\n"
#define CONTINUE_KEY_MSG    " -space-"
#define MORE_KEY_MSG        " -more-"
#define ARE_YOU_SURE_PROMPT "Sicher? j/n "
#define NO_MATCH            "Keine Uebereinstimmung"
#define DIR_NOT_MODIFYED    "Verzeichnis nicht modifiziert"
#define DIRECTORY_IS_EMPTY  "Verzeichnis ist leer"
#define HELP_SCREEN_PROMPT  "\nTaste fuer weiter ('q' fuer Abbruch)"
#define CANNOT_FIND_DOCFILE "Textdatei kann nicht geoeffnet werden"
#define GET_PATTERN_PROMPT  "Pattern: "
#define P_NEEDS_PARENTHESES "Fehler in \".qxrc\": Die Sequenz \"%p\"\
 benoetigt geschweifte Klammern,\
 um den Anfang bzw. das Ende der auszudruckenden Zeichenkette bestimmen zu\
 koennen"
#define UNKNOWN_LAYOUT_CHAR "Unbekanntes 'layout'-Steuerzeichen\n" 
#define INPUT_LINE_TO_LONG  "Eingabezeile ist zu lang"
#define CURSOR_OR_PTR_MISSING "Zeiger- und/oder Markierungsequenz in \"layout\(%s)\" nicht gesetzt\n"
#define ONE_CURSOR_OR_PTR_ONLY "Zeiger- und/oder Markierungsequenz in \"layout\(%s)\" zu viel gesetzt\n"
#define NEED_TAGGED_FILES   "Dieser Befehl benoetigt mindestens eine markierte Datei"
#define DIR_NOT_EXIST         "Angewaehltes Verzeichnis existiert nicht"
#define PATH_NOT_ABSOLUT      "Nur komplette Pfadnamen"
#define INVALID_FILENAME_DETECTED "Dateiname mit nicht darstellbaren Zeichen entdeckt. Die interne 'Rename' Funktion kann zum umbenehnen verwendet werden."
#define UPDATE_DIR_MSG  "Aktuallisiert:"
#define SELECT_ENTRY_IS_EMPTY  "Eintrag ist unbenutzt"
#define SELECT_MENU           "(Down) Naechster Eintrag  (Tab) Verzeichnis waehlen    (Enter) Eintrag aendern\n(Up)   Vorheriger Eintrag (*)   Zeiger update ein/aus  (Esc/Q) Zurueck"

/* readrc */
#define RC_ERROR_STRING            "Fehler:"
#define RC_PROC_UNKNOWN_CMD        "Unbekanntes Prozedur-Kommando."
#define RC_PROC_KEY_BUSY           "Taste ist bereits belegt."
#define RC_PROC_UNKNOWN_MODE       "Modusangabe unbekannt."
#define RC_UNKNOWN_COMMAND         "Unbekanntes Kommando."
#define RC_BINDKEY_FUNC_NON_EXIST  "Funktionsbindung nicht moeglich. Interne Funktion existiert nicht."
#define RC_LAYOUT_ARG_OUT_OF_RANGE "Kommando 'layout' benoetigt zwei Argumente."
#define RC_LAYOUT_ILL_ARG_RANGE    "Ungueltiges Argument. Erlaubt sind hier Werte von 0 bis 9."
#define RC_LAYOUT_ENTRY_BUSY       "Layout Eintrag ist bereits deffiniert."
#define RC_SELECT_ARG_OUT_OF_RANGE "Kommando 'selectdir' benoetigt zwei bis vier Argumente."
#define RC_SELECT_ILL_ARG_RANGE    "Ungueltiges Argument. Erlaubt sind hier Werte von 0 bis 11."
#define RC_SELECT_PERM_DENIED      "Keine Berechtigung fuer dieses Verzeichnis."
#define RC_SELECT_DIR_NON_EXIST   "Verzeichnis existiert nicht."
#define RC_SELECT_PATH_NOT_ABSOLUT       "Pfadname ist nicht komplett."
#define RC_SELECT_NOT_A_DIR              "Angegebener Pfad ist kein Verzeichnis."
#define RC_SELECT_SWITCH_REQUESTED   "Als vierter Parameter wird ein Stern (*) oder nichts erwartet."
#define RC_ARG_MISSING            "Falsche Anzahl Parameter."
#define RC_PREFIX_EXIST           "Prefix existiert bereits."
#define RC_PREFIX_NON_EXIST       "Prefix existiert nicht."
#define RC_PREFIX_ONLY_ADDORDEL   "Prefix Option nur 'add' oder 'del'"

/* help screen */
#define HELP_BANNER     "Tastaturbelegungen"
#define HELP_INTERNAL   "Integrierte Funktionen:"
#define HELP_EXTERNAL     "Externe Prozeduren:"
#endif   /* GERMAN */
/**/



#ifndef bool
#define bool char
#endif


#ifndef index
#define index strchr
#define rindex strrchr
#endif

#ifndef TRUE
#define TRUE   1
#define FALSE  0
#endif

typedef struct {
    char   *str;
    char  **list;
    int     mode;
} EXEC;


typedef struct {
    char      *label;
    char      *help;
    char      *key;
    EXEC      *exec;
    int        exec_cnt;
    char      *kbd_name;
} PROC_LIST;


typedef struct {
    PROC_LIST *pl;
    int        max;
    int        now;
    int        page_max;
    int        page_now;
    int       *page_mem;
} PROC;

extern PROC *Proc;

/* Flags for the proc-mode            Meaning: */

#define P__SURE            001     /* Ask for "Sure?" before execution */
#define P__UPDATE          002     /* Update the directory after execution */
#define P__CLS             004     /* Clears the screen before execution */
#define P__WAIT            010     /* Wait after execution */


typedef struct {
    DI     *fl;
    bool   *tagged;
    int     flpos;
    bool    need_update;
    int     no_of_files_tagged;
    int     layout_entry_len;
    int     layout_now;
    int     max_filename_len;
    int     max_filename_len_cut;
    int     max_sizestr_len;
    int     max_line_entries;
    bool    nohidden;
    char   *cwd;
    char   *label;
    struct {
	int p_cursor;
	int p_selector;
    } position;
    struct {
	int x;
	int y;
    } screen, oldscr;
} DIR_LIST;





typedef struct {
    char     *layout_tbl[10];
    int       now;
    bool      update;
    char     *separator;
    char     *cursor;
    char     *cursor_spaces;
    bool      noinverse;
    struct {
	char     *wd_tbl;
	DIR_LIST *dl_tbl;
	int       time_tbl;
	bool      set_flpos_to_newest;
	bool      nohidden;
    } sdir[12];
} DL_SELECT;

extern DL_SELECT DL_select;




#define MAX_LINE_ENTRIES   dl->max_line_entries
#define ENTRY_LENGTH       dl->layout_entry_len
#define MLE                MAX_LINE_ENTRIES
#define Y_SIZE_SCREEN      (LINES - 5)
#define Y_CUR_LINE         (dl->flpos / MAX_LINE_ENTRIES)
#define Y_MAX              (((dl->fl->no_of_files)/MAX_LINE_ENTRIES) + (((dl->fl->no_of_files)%MLE==0) ? 0 : 1 ))
#define Y_TOP_LINE         (Y_CUR_LINE - dl->screen.y + 4) 
#define Y_BOT_LINE         (Y_TOP_LINE + Y_SIZE_SCREEN) 
#define PAGE_ENTRIES       (MAX_LINE_ENTRIES * Y_SIZE_SCREEN)

#define NEWDIR_RESET       0
#define NEWDIR_KEEP        1

#define TRUNCATE           0
#define NOTRUNCATE         1

#define COPY_BUFFER        10240
#define MAJOR_MINOR_FMT    "%3d,%3d"
#define TRUE               1
#define FALSE              0


extern char *convert_filename_to_printable(unsigned char *name, int max_len);
extern DIR_LIST *dl_init(char *wd, bool nohidden);
extern int isdigitstr(char *s);
extern char *dirname(char *);
extern char *formtxt(char *,int,int);
extern char *itoa(int);
extern int qx_exit(int);
extern void catchit(int);
extern int key_cmp();
extern char *dstrcat(char *buf, char *add);
extern char *input(char *prompt);
extern char *inputs(char *p, char *s);
extern char *strenv(char *s);
extern char *ascii2str(char *s);
extern char *str2ascii(char *s);
extern char *trim(char *s);
extern char *shortpath(char *s);
extern char *findrc();
extern char *volume_str(double bytes);
extern char **add_to_list(char **list, char *str);
extern int igetc();
extern int have_access(struct stat *st);
extern int rpos(char *s, int c);
extern int renaming(char *from, char *to);
extern char *flgets(char *str, int n, FILE *file, int *lcnt);
extern int cigetc();

extern void resetty();
extern int setty();

extern DIR_LIST *dl_layout(DIR_LIST *dl);
extern DIR_LIST *dl_newdir(DIR_LIST *dl, int flpos_mode);
extern DIR_LIST *dl_updatedir(DIR_LIST *dl, int dlidx);
extern char **wordlist(char *str, char **arg, int *cnt, int (*trunc)());
extern char **free_char_array(char **a);

extern void initscr();
extern void clear();
extern void clrtobot();
extern void clrtoeol();
extern void insertln();
extern void insert_n_lines(int y, int n);
extern void delete_n_lines(int y, int n);
extern void deleteln();
extern void gotoxy(int x, int y);
extern void standout();
extern void standend();

extern void put_screen();
extern void put_lines(DIR_LIST *dl, int begin_line, int no_of_lines);
extern int put_msg(char *str);
extern void put_dir(DIR_LIST *dl);
extern void put_help_screen();
extern void put_proc_page();
extern void put_proc_help();
extern void put_proc_pointer(int label_to_set, int label_to_unset);
extern void put_path(DIR_LIST *dl);
extern int match(char *s, char *p);
extern int check_dir(char *s);


extern void moves(DIR_LIST *dl);

extern void DL_SELECT_table_init_null();
extern void DL_SELECT_table_init_set();
extern DIR_LIST *DL_SELECT_dir(int n);
extern void DL_SELECT_update();
extern DIR_LIST *DL_SELECT_display_table();

extern int readrc(char *rc_name);

extern int kbhit();

extern void exec_shell();

extern void call_proc(DIR_LIST *dl);

extern struct set_input *ip_get(struct set_input *ip);
extern int ip_set(struct set_input ip);
extern char _ikey();
extern void jump_xpos(int x);

extern char **wordlist(char *str, char **arg, int *cnt, int (*trunc)());
extern int is_space(char c);
extern int parser(char *cmd, char **cmd_list, char ***arglist, int *argcount);

extern void dl_free(DIR_LIST *dl);

extern char *dirname (char *path);
extern char *basename (char *name);
