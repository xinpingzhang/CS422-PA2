#include "prog2.h"

static int a_base;
static int a_ack;
static struct pkt a_last_pkt;
static float start_time = 0;

static int b_base;
static int b_ack;
static struct pkt b_last_pkt;

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

int is_corrupt(struct pkt *pkt)
{
    return checksum(pkt) != pkt->checksum;
}

/* called from layer 5, passed the data to be sent to other side */
void A_output(struct msg message)
{
    if(a_last_pkt.acknum != -1)
    {
        tracef(2, "A: Packet %d in flight! Dropping this one!\n", a_last_pkt.acknum);
        return;
    }
    start_time = time;
    a_last_pkt.seqnum = a_base++;
    a_ack = !a_ack;
    a_last_pkt.acknum = a_ack;
    
    assert(a_ack == b_ack);

    memcpy(a_last_pkt.payload, message.data, sizeof(message));

    a_last_pkt.checksum = checksum(&a_last_pkt);
    tracef(2, "A: Sending packet %d\n", a_ack);
    //give 7 unit of time for grace period.
    starttimer(A, TIMEOUT_PERIOD);
    tolayer3(A, a_last_pkt);
}

/* called from layer 3, when a packet arrives for layer 4 */
void A_input(struct pkt packet)
{
    if(a_last_pkt.acknum == -1)
    {
        tracef(2, "A: No packet in flight, ignore.\n");
        return;
    }
    if(is_corrupt(&packet))
    {
        tracef(2, "A: Packet Corrupted! Resend packet %d.\n", a_last_pkt.acknum);
        tolayer3(A, a_last_pkt);
        return;
    }
    if(packet.acknum != a_ack)
    {
        tracef(2, "A: NAK received, resend packet. %d\n", a_last_pkt.acknum);
        tolayer3(A, a_last_pkt);
        return;
    }
    tracef(2, "A: %d ACKed!\n", packet.acknum);
    stoptimer(A);
    tracef(2, "A: transmission time: %f\n", time - start_time);
    a_last_pkt.acknum = -1;
}

/* called when A's timer goes off */
void A_timerinterrupt()
{
    if(a_last_pkt.acknum == -1)
    {
        tracef(2, "A: Packet %d ACKed already, ignoring timeout\n", a_ack);
        return;
    }
    tracef(2, "A: timeout, resend last packet.\n");
    tolayer3(A, a_last_pkt);
    starttimer(A, TIMEOUT_PERIOD);
}

/* the following routine will be called once (only) before any other */
/* entity A routines are called. You can use it to do any initialization */
void A_init()
{
    a_base = 0;
    a_ack = 1;
    a_last_pkt.acknum = -1;
}

/* called from layer 3, when a packet arrives for layer 4 at B*/
void B_input(struct pkt packet)
{
    if(is_corrupt(&packet))
    {
        tracef(2, "B: Packet Corrupted! Send NAK(ReACK last one)\n");
        // write(STDOUT_FILENO, packet.payload, sizeof(packet.payload));
        //ack the last packet
        tolayer3(B, b_last_pkt);
        return;
    }
    if(packet.acknum != b_ack)
    {
        tracef(2, "B: Duplicate packet!\n Re ACK last one!");
        // write(STDOUT_FILENO, packet.payload, sizeof(packet.payload));
        tolayer3(B, b_last_pkt);
        return;
    }
    tracef(2, "B: Packet OK! ACKing %d\n", b_ack);
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