#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include "prog2.h"


/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#define MAGIC_COOKIE 0x59696E67
#define TIMEOUT_PERIOD 16

static int tracef(int level, const char *format, ...)
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

//TCP style adding
static int checksum(struct pkt *pkt)
{
    int cookie = MAGIC_COOKIE;
    int *data = (int*)&pkt->payload[0];
    int size = sizeof(pkt->payload)/sizeof(int);
    for(int i = 0; i < size; i ++)
    {
        cookie ^= data[i];
    }
    cookie ^= pkt->seqnum;
    cookie ^= pkt->acknum;
    return cookie;
}

static int is_corrupt(struct pkt *pkt)
{
    return checksum(pkt) != pkt->checksum;
}

#define BUF_SIZE 64
// #define WND_SIZE 20
static int base;
static int unacked;
static int next_slot;
static struct pkt a_buf[BUF_SIZE];


/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    if(unacked >= BUF_SIZE)
    {
        tracef(2, "A: Buffer full! Dropping this one\n");
        return;
    }
    struct pkt *pkt = &a_buf[next_slot];
    
    //A's ack num is unused...
    pkt->acknum = ~next_slot;
    pkt->seqnum = next_slot;

    memcpy(pkt->payload, message.data, sizeof(pkt->payload));
    pkt->checksum = checksum(pkt);
    next_slot++;

    next_slot %= BUF_SIZE;

    if(unacked < BUF_SIZE)
    {
        tolayer3(A, *pkt);
        if(unacked == 0)
        {
            tracef(2, "A: Starting Timer\n");
            starttimer(A, TIMEOUT_PERIOD);
        }
        unacked++;
        tracef(2, "A: Sending packet %d\n", pkt->seqnum);
    }else
    {
        tracef(2, "A: Buffering packet %d\n", pkt->seqnum);
    }
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    if(is_corrupt(&packet))
    {
        tracef(2, "A: ACK %d Corrupted! Ignore ACK!\n", packet.acknum);
        return;
    }
    if(base == packet.acknum+1)
    {
        tracef(2, "A: Duplicate ACK %d!\n", packet.acknum);
        A_timerinterrupt();
        return;
    }
    base = packet.acknum+1;
    base %= BUF_SIZE;
    if(next_slot < base)
    {
        unacked = next_slot + BUF_SIZE - base;
    }else
    {
        unacked = next_slot - base;
    }
    tracef(2, "A: ACK OK! Base: %d unACKed: %d\n", base, unacked);
    stoptimer(A);
    if(unacked > 0)
    {
        tracef(2, "A: Restarting timer...\n");
        starttimer(A, TIMEOUT_PERIOD);
    }
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    tracef(2, "A: Resending: base %d, count: %d next_slot %d\n", base, unacked, next_slot);
    for(int i = base; i != next_slot; i = (i+1) % BUF_SIZE)
    {
        tolayer3(A, a_buf[i]);
        tracef(2, "i = %d, next_slot = %d\n", i, next_slot);
    }
    starttimer(A, TIMEOUT_PERIOD);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    base = 0;
    unacked = 0;
    next_slot = 0;
}


static int expected_seq = 0;
static struct pkt b_last_pkt;
static int b_base = 0;

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    if(is_corrupt(&packet))
    {
        tracef(2, "B: Packet %d corrupted! ReACK %d\n", packet.seqnum, b_last_pkt.acknum);
        tolayer3(B, b_last_pkt);
        return;
    }
    if(packet.seqnum != expected_seq)
    {
        tracef(2, "B: Packet %d Out of Order! Expecting %d ReACK %d\n", packet.seqnum, expected_seq, b_last_pkt.acknum);
        tolayer3(B, b_last_pkt);
        return;
    }
    tolayer5(B, packet.payload);
    tracef(2, "B: Packet OK! ACKing packet %d\n", packet.seqnum);
    b_last_pkt.acknum = packet.seqnum;
    b_last_pkt.seqnum = b_base++;
    b_last_pkt.checksum = checksum(&b_last_pkt);
    tolayer3(B, b_last_pkt);
    expected_seq++;
    expected_seq %= BUF_SIZE;
}

/* called when B's timer goes off */
void B_timerinterrupt()
{

}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    b_last_pkt.acknum = -1;
    b_last_pkt.seqnum = b_base++;
    memset(b_last_pkt.payload, 0, sizeof(b_last_pkt.payload));
    strcpy(b_last_pkt.payload, "SAGSH");
    b_last_pkt.checksum = checksum(&b_last_pkt);
}