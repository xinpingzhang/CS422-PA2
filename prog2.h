#ifndef __PROG_2
#define __PROG_2
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>

/* a "msg" is the data unit passed from layer 5 (teachers code) to layer  */
/* 4 (students' code).  It contains the data (characters) to be delivered */
/* to layer 5 via the students transport level protocol entities.         */
struct msg
{
    char data[20];
};

/* a packet is the data unit passed from layer 4 (students code) to layer */
/* 3 (teachers code).  Note the pre-defined packet structure, which all   */
/* students must follow. */
struct pkt
{
    int seqnum;
    int acknum;
    int checksum;
    char payload[20];
};

extern void stoptimer(int AorB);                   /* A or B is trying to stop timer */
extern void starttimer(int AorB, float increment); /* A or B is trying to stop timer */
extern void tolayer3(int AorB, struct pkt packet); /* A or B is trying to stop timer */
extern void tolayer5(int AorB, char datasent[20]);
//TRACE level
extern int TRACE;

//Current time in simulator
extern float time;
#define A 0
#define B 1

#endif