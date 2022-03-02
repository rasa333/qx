int is_printable_char(int);


typedef struct {
  char *scan;
  char *(*input_func)(void);
} iKEYS;


                    /* input-history linked-list */

struct _ihistory {
  char *buf;
  struct _ihistory *next;
  struct _ihistory *prev;
};
typedef struct _ihistory iHist;


                   /* input global settings and variables */

struct set_input {
  int (*compar_char)(int);     /* pointer to func to check c is allowed */
  int edit_buf_maximal_length; /* max length of input buffer; 0=unlimited */
  int history_flag;            /* 0=no history;else number of lines to store */
  int edit_buf_length;         /* length of edit buffer yet (ilen) */
  int edit_buf_position;       /* position of cursor in buffer yet (ipos) */
  char *edit_buf;              /* pointer to edit buffer (istr) */
  char *prompt_buf;            /* pointer to prompt string (ipst) */
  int overwrite_flag;          /* 0=insert-mode; 1=overwrite-mode */
  int default_cols;            /* default value for column on screen */
  int update_line_scroll_xpos;           /* scroll column position (ixscl) */
  int update_line_old_ilen;              /* last ilen value (old_ilen) */
  int update_line_old_ipos;              /* last ipos value (old_ipos) */
  int update_line_line_refresh_flag;     /* 0=refr. part of line;1=whole li (irefresh) */
  int update_line_edit_buf_max_disp;  /* maximal chars to display (imdc) */
  int update_line_begin_xpos;            /* column begin position (ixbegin) */
  char *keys_METAstr;        /* meta-string table */
  iKEYS *keys_ikeytab;       /* key-string table */
  int keys_ikeycnt;          /* number of elements in key-string table */
  iHist *history_ih_head;      /* history double-linked-list first element */
  iHist *history_ih_tail;      /* history double-linked-list last element */
  iHist *history_ih_now;       /* actually pointer element */
  int history_ih_cnt;         /* number of elements in history */
  int mem_ioffset;               /* memory offset for edit_buf */
};

#define ipos     set_input.edit_buf_position
#define ilen     set_input.edit_buf_length
#define istr     set_input.edit_buf
#define ipst     set_input.prompt_buf
#define iCOLS    set_input.default_cols
#define ixscl    set_input.update_line_scroll_xpos
#define old_ipos set_input.update_line_old_ipos
#define old_ilen set_input.update_line_old_ilen
#define irefresh set_input.update_line_line_refresh_flag
#define imdc     set_input.update_line_edit_buf_max_disp
#define ixbegin  set_input.update_line_begin_xpos
#define ih_head  set_input.history_ih_head
#define ih_tail  set_input.history_ih_tail
#define ih_now   set_input.history_ih_now
#define ih_cnt   set_input.history_ih_cnt
#define METAstr  set_input.keys_METAstr
#define ikeytab  set_input.keys_ikeytab
#define ikeycnt  set_input.keys_ikeycnt
#define ioffset  set_input.mem_ioffset

/* Input return mode */

extern int INPUT_RETURN_MODE;
#define I_ENTER 0
#define I_ABORT 1

/* Input file mode */

extern int IGETDIR_FILE_MODE;
#define I_FMREG 0
#define I_FMDIR 1



