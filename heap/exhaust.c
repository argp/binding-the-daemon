/*
 * exhaust.c -- Patroklos Argyroudis, argp at domain census-labs.com
 *
 * Fill the slabs of the '256' anonymous zone with items filled with
 * '0x41' bytes.
 *
 * $Id: exhaust.c,v 8167bfa7e34f 2010/02/17 13:36:17 argp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>

#define BUF_SIZE    256
#define OP_ALLOC    1
#define OP_FREE     2
#define SLOTS       100

struct arg
{
    char *buf;
    u_int len;
    int op;
    u_int slot;
};

int
main(int argc, char *argv[])
{
    int sn, slot, nrobj;
    struct module_stat mstat;
    struct arg varg;

    if(argc != 2)
    {
        printf("usage: %s <items' #>\n", argv[0]);
        return 0;
    }

    nrobj = atoi(argv[1]);
    
    varg.len = BUF_SIZE;
    varg.buf = calloc(varg.len + 1, sizeof(char));

    if(varg.buf == NULL)
    {
        perror("calloc");
        exit(1);
    }

    memset(varg.buf, 0x41, varg.len);
    
    mstat.version = sizeof(mstat);
    modstat(modfind("bug"), &mstat);  
    sn = mstat.data.intval;

    for(slot = 0; slot < nrobj; slot++)
    {
        varg.op = OP_ALLOC;
        varg.slot = slot;
        syscall(sn, varg);
    }
    
    free(varg.buf);
    return 0;
}

/* EOF */
