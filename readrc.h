#define _END       0

/* Main command defines */

#define _PROC         1
#define _BINDKEY      2
#define _PREFIX       3
#define _SEPARATOR    4
#define _HISTORY      5 
#define _CURSOR       6 
#define _LAYOUT       7
#define _UNBINDKEY    8
#define _SELECTDIR    9
#define _NOINVERSE   10

/* Proc command defines */

#define _HELP          1
#define _CASE          2
#define _EXEC          3
#define _KEY           4
#define _RESERVED_1    5
#define _RESERVED_2    6

char *rc_main_cmds[] = {
  "end" ,
  "proc" ,
  "bindkey" ,
  "prefix" ,
  "separator" ,
  "history" ,
  "cursor" ,
  "layout" ,  
  "unbindkey",
  "dir",
  "noinverse",
  "\0"
  },
  *rc_proc_cmds[] = {
    "end" ,
    "help" ,
    "case" ,
    "exec" ,
    "key" ,
    "_reserved_1" ,
    "_reserved_2" ,
    "\0"
    },
  *rc_key_cmds[] = {
    "end"   ,
    "right" ,   "_right",
    "left"  ,	"_left",
    "up"    ,	"_up",	
    "down"  ,	"_down",
    "end"   ,	"_end",	
    "home"  ,	"_home",
    "pgup"  ,	"_pgup",
    "pgdn"  ,	"_pgdn",
    "quit"  ,	
    "tag"   ,	
    "help"  ,
    "match" ,  
    "chdir" ,	
    "refresh-dir",
    "tag-all"  ,
    "inc"   ,
    "dec"   ,
    "lo0"    ,
    "lo1"   ,
    "lo2"   ,
    "lo3"   ,
    "lo4"   ,
    "lo5"   ,
    "lo6"   ,
    "lo7"   ,
    "lo8"   ,
    "lo9"   ,
    "untag-all",
    "goto-dir",
    "refresh-dir",
    "shell",
    "goto-home",
    "start-proc",
    "sl0",
    "sl1",
    "sl2",
    "sl3",
    "sl4",
    "sl5",
    "sl6",
    "sl7",
    "sl8",
    "sl9",
    "sl10",
    "sl11",
    "rename",
    "fullname",
    "showdirtable",
    "\0" },*rc_procmode_cmds[] = {
      "",
      "sure"  ,
      "update",
      "cls"   ,
      "wait",
      "\0"};


