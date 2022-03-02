#include "dir.h"
#include <dirent.h> 
#include <stdlib.h>
#include <string.h>


/* Input file mode */

extern int IGETDIR_FILE_MODE;
#define I_FMREG 0
#define I_FMDIR 1


inline static int fl_name_cmp(FL *a, FL *b)
{
  return (strcmp(a->name , b->name));
}


DI *getdir(char *cwd, int nohidden)
{          
  DIR      *dirp;
  register int      i;
  int      cwd_len;
  struct   dirent   *dp;     
  struct   stat st;
  char     *tmp = (char *)0;
  DI       *d;
  
  dirp = opendir(cwd);  
  
  if (!dirp)                           /* it's readable ? */
    return NULL; 

  d = (DI *) malloc(sizeof(DI));
  d->no_of_files = 0;
  d->fl = (FL *)0;
  cwd_len = strlen(cwd);
  
  while(dp = readdir(dirp)) {
    if (nohidden == 1 && strcmp(dp->d_name, ".") && strcmp(dp->d_name, "..") && dp->d_name[0] == '.')
      continue;
    if (!(d->no_of_files % 200)) 
      d->fl = (FL *)(d->fl ? realloc(d->fl,sizeof(FL)*(d->no_of_files + 200)) :
		     malloc(sizeof(FL)*(d->no_of_files + 200)));

    d->fl[d->no_of_files].name = (char *)malloc((i=strlen(dp->d_name)) + 1);
    strcpy(d->fl[d->no_of_files].name,dp->d_name);
    
    tmp = (char *)(tmp ? realloc(tmp,cwd_len + i + 2) :
		   malloc(cwd_len + i + 2));
    strcpy(tmp, cwd);
    strcat(tmp, "/");
    strcat(tmp, dp->d_name);
    lstat(tmp,  &st);
    if ((st.st_mode & S_IFMT) != S_IFDIR && IGETDIR_FILE_MODE == I_FMDIR)
      free(d->fl[d->no_of_files].name);
    else
      d->fl[d->no_of_files++].stat = st;
  }
  closedir(dirp);
  free(tmp);
  d->fl = (FL *)realloc(d->fl,sizeof(FL) * d->no_of_files);
  
  qsort(d->fl,d->no_of_files,sizeof(FL), (void *)fl_name_cmp); 
  return d;
}


void freedir(DI *d)
{
  while(d->no_of_files--) 
    free(d->fl[d->no_of_files].name);
  free(d->fl);
  free(d);
}
