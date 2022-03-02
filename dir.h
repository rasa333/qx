#include <sys/types.h>
#include <sys/stat.h>

typedef struct
{
	char *name;
	struct stat stat;
} FL;

typedef struct 
{
	FL *fl;
	int no_of_files;
} DI;

extern DI *getdir(char *cwd, int nohidden);
extern void freedir(DI *d);
