#include <term.h>

#include "defs.h"


char cap_buffer[1024],tcstrings[256];

char 
  *SO,		/* Stand-Out Modus	*/
  *SE,		/* Ende SO Modus	*/
  *CL,		/* Clear Screen		*/
  *CE,          /* Clear to EOLine      */
  *AL,          /* Add New Line         */
  *DL,          /* Del Line             */
  *CD,          /* Clear end of Display */
  *CM; 		/* Cursor motion        */

int COLS, LINES;
int NO_ALDL;           /* Terminal can't add or delete line */


void initscr()
{
  char *tname, *tcs;

  NO_ALDL = 0;
  
  if ((tname = getenv("TERM")) == NULL) { 
    fprintf(stderr, "TERM not defined\n"); 
    exit(1); 
  }

  if (tgetent(cap_buffer, tname) == -1) { 
    fprintf(stderr, "unknown Terminal: %s\n", tname ); 
    exit(1); 
  }

  tcs = tcstrings;   
  *tcs++ = 0; 			
  SO = tgetstr("so", &tcs);
  SE = tgetstr("se", &tcs);
  CL = tgetstr("cl", &tcs);
  AL = tgetstr("al", &tcs);
  DL = tgetstr("dl", &tcs);
  CE = tgetstr("ce", &tcs);
  CM = tgetstr("cm", &tcs);
  CD = tgetstr("cd", &tcs);
  LINES = tgetnum("li");
  COLS  = tgetnum("co");

  if (!CL || !CE || !CM || !CD) {
    fprintf(stderr,"Dieses Programm ist fuer ihr Terminal nicht geeignet.\n");
    exit(0);
  }
  if (!DL || !AL)
    NO_ALDL = 1;
}


/**************************************************************************/


int outchar(c)
     int c;
{
  putchar(c);
}


/**************************************************************************/


void clear()
{
  tputs(CL,1,outchar);
  fflush(stdout);
}


/**************************************************************************/


void gotoxy(int x, int y)
{
  tputs(tgoto(CM,x,y), 1, outchar);
  fflush(stdout);
}  


/**************************************************************************/


void clrtobot()
{
  tputs(CD,1,outchar);
  fflush(stdout);
}


/**************************************************************************/



void clrtoeol()
{
  if (CE) {
    tputs(CE,1,outchar);
    fflush(stdout);
  }
}


/**************************************************************************/


void insertln()
{
  tputs(AL,1,outchar);
  fflush(stdout);
}


/**************************************************************************/


void insert_n_lines(int y, int n)
{
  int i;

  gotoxy(0, LINES - 1 - n);
  clrtobot();
  gotoxy(0, y);
  for ( i = 0 ; i < n ; i++ ) 
    insertln();
}


/**************************************************************************/


void deleteln()
{
  tputs(DL,1,outchar);
  fflush(stdout);
}


/**************************************************************************/


void delete_n_lines(int y, int n)
{
  int i;

  gotoxy(0, y);
  for ( i = 0 ; i < n ; i++ )
    deleteln();
}


/**************************************************************************/


void standout()
{
  tputs(SO,1,outchar);
  fflush(stdout);
}


/**************************************************************************/


void standend()
{
  tputs(SE,1,outchar);
  fflush(stdout);
}
