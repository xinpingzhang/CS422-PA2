#include <stdio.h>
#include <string.h>
#include <unistd.h>

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
extern int TRACE;

#define A 0
#define B 1

static int a_base;
static int a_ack;
static struct pkt a_last_pkt;

/********* STUDENTS WRITE THE NEXT SEVEN ROUTINES *********/
#define MAGIC_COOKIE 0x59696E67

int checksum(struct pkt *pkt)
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

int is_corrupt(struct pkt *pkt)
{
    return checksum(pkt) != pkt->checksum;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    if(a_last_pkt.acknum != -1)
    {
        if(TRACE >= 2)
        {
            printf("A: Packet %d in flight! Dropping this one!\n", a_last_pkt.acknum);
            return;
        }
    }
    a_last_pkt.seqnum = a_base++;
    a_ack = !a_ack;
    a_last_pkt.acknum = a_ack;

    memcpy(a_last_pkt.payload, message.data, sizeof(message));

    a_last_pkt.checksum = checksum(&a_last_pkt);
    if(TRACE >= 2)
    {
        printf("A: Sending packet %d\n", a_ack);
    }
    //give 7 unit of time for grace period.
    // starttimer(A, 7);
    tolayer3(A, a_last_pkt);
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    if(is_corrupt(&packet))
    {
        if(TRACE >= 2)
        {
            printf("A: Packet Corrupted! Resend last packet.\n");
        }
        tolayer3(A, a_last_pkt);
        return;
    }
    if(packet.acknum != a_ack)
    {
        if(TRACE >= 2)
        {
            printf("A: NAK received, resend last packet.\n");
        }
        tolayer3(A, a_last_pkt);
        return;
    }
    if(TRACE >= 2)
    {
        printf("A: %d ACKed!\n", packet.acknum);
    }
    // stoptimer(A);
    a_last_pkt.acknum = -1;
    tolayer5(A, packet.payload);
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    if(TRACE >= 2)
    {
        printf("A: timeout, resend last packet.\n");
    }
    tolayer3(A, a_last_pkt);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    a_base = 0;
    a_ack = 1;
    a_last_pkt.acknum = -1;
}

static int b_base;
static int b_ack;
static struct pkt b_last_pkt;

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    if(is_corrupt(&packet))
    {
        if(TRACE >= 2)
        {
            printf("B: Packet Corrupted! Send NAK\n");
            write(STDOUT_FILENO, packet.payload, sizeof(packet.payload));
        }
        //ack the last packet
        tolayer3(B, b_last_pkt);
        return;
    }
    if(packet.acknum != b_ack)
    {
        if(TRACE >= 2)
        {
            printf("B: Duplicate packet!\n Re ACK last one!");
            write(STDOUT_FILENO, packet.payload, sizeof(packet.payload));
        }
        tolayer3(B, b_last_pkt);
        return;
    }
    if(TRACE >= 2)
    {
        printf("B: Packet OK! ACKing %d\n", b_ack);
    }
    //send ack
    b_last_pkt.seqnum = b_base++;
    b_last_pkt.acknum = b_ack;
    b_ack = !b_ack;
    b_last_pkt.checksum = checksum(&b_last_pkt);
    tolayer3(B, b_last_pkt);

    //deliver to application layer
    tolayer5(B, packet.payload);
}

/* called when B's timer goes off */
void B_timerinterrupt()
{
}

/* the following rouytine will be called once (only) before any other */
/* entity B routines are called. You can use it to do any initialization */
void B_init()
{
    b_base = 0;
    b_ack = 0;
    b_last_pkt.seqnum = -1;
    b_last_pkt.acknum = 1;
    memset(b_last_pkt.payload, 0, sizeof(b_last_pkt.payload));
    b_last_pkt.checksum = checksum(&b_last_pkt);
}