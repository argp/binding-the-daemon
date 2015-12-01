/*
 * bug.c -- Patroklos Argyroudis, argp at domain census-labs.com
 *
 * Sample kernel heap (UMA) vulnerability based on the signedness.org
 * challenge #3 by karl.
 *
 * $Id: bug.c,v 8167bfa7e34f 2010/02/17 13:36:17 argp $
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/module.h>
#include <sys/sysproto.h>
#include <sys/sysent.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/malloc.h>

#define SLOTS 100

static char *slots[SLOTS];

#define OP_ALLOC    1
#define OP_FREE     2

struct argz
{
    char *buf;
    u_int len;
    int op;
    u_int slot;
};

static int
bug(struct thread *td, void *arg)
{
    struct argz *uap = arg;
    
    if(uap->slot >= SLOTS)
    {
        return 1;
    }
    
    switch(uap->op)
    {
        case OP_ALLOC:
            if(slots[uap->slot] != NULL)
            {
                return 2;
            }
            
            slots[uap->slot] = malloc(uap->len & ~0xff, M_TEMP, M_WAITOK);
            
            if(slots[uap->slot] == NULL)
            {
                return 3;
            }
            
            uprintf("[*] bug: %d: item at %p\n", uap->slot,
                    slots[uap->slot]);
            
            copyin(uap->buf, slots[uap->slot] , uap->len);
            break;
        
        case OP_FREE:
            if(slots[uap->slot] == NULL)
            {
                return 4;
            }
            
            free(slots[uap->slot], M_TEMP);
            slots[uap->slot] = NULL;
            break;
        
        default:
            return 5;
    }
    
    return 0;
}


static struct sysent bug_sysent =
{
    4,          /* sy_narg */
    bug         /* sy_call */
};

static int offset = NO_SYSCALL;

static int
event_handler(struct module *module, int cmd, void *arg)
{
    int i, retval = 0;
    
    switch(cmd)
    {
        case MOD_LOAD:
            for(i = 0; i < SLOTS; i++)
            {
                slots[i] = NULL;
            }
            
            uprintf("bug loaded at %d\n", offset);
            break;
        
        case MOD_UNLOAD:
            for(i = 0; i < SLOTS; i++)
            {
                if(slots[i])
                {
                    free(slots[i], M_TEMP);
                }
            }
            
            uprintf("bug unloaded from %d\n", offset);
            break;
        
        default:
            retval = EOPNOTSUPP;
            break;
    }
    
    return retval;
}

SYSCALL_MODULE(bug, &offset, &bug_sysent, event_handler, NULL);

/* EOF */
