#include "defs.h"

int UID, GID;

int main(int argc, char **argv)
{
    char *rc_name;
    DIR_LIST *dl;

    UID = getuid();
    GID = getgid();

    initscr();
    setty();

    DL_SELECT_table_init_null();
    rc_name = (char *)findrc();
    if (!rc_name)
	qx_exit(-1);
    readrc(rc_name);
    DL_SELECT_table_init_set();
    dl = DL_SELECT_dir(0);
    dl = dl_layout(dl);
    put_screen();
    moves(dl);
    qx_exit(0);
}
