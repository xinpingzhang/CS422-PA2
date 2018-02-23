#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include "prog2.h"


int tracef(int level, const char *format, ...)
{
    int ret = 0;
    if(TRACE >= level)
    {
        va_list args;
        va_start(args, format);
        ret = vprintf(format, args);
        va_end (args);
    }
    return ret;
}

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{

}

/* called when A's timer goes off */
void A_timerinterrupt()
{

}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{

}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{

}

/* called when B's timer goes off */
void B_timerinterrupt()
{

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{

}