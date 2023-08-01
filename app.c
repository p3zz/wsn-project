#include "contiki.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include "leds.h"
#include "net/netstack.h"
#include <stdio.h>
#include "core/net/linkaddr.h"
#include "my_collect.h"
#include "simple-energest.h"
/*---------------------------------------------------------------------------*/
#define APP_UPWARD_TRAFFIC 1
#define APP_DOWNWARD_TRAFFIC 1
/*---------------------------------------------------------------------------*/
#define MSG_PERIOD (30 * CLOCK_SECOND)
#define SR_MSG_PERIOD (10 * CLOCK_SECOND)
#define MSG_DELAY (30 * CLOCK_SECOND)
#define SR_MSG_DELAY (75 * CLOCK_SECOND)
#define COLLECT_CHANNEL 0xAA
/*---------------------------------------------------------------------------*/
#ifndef CONTIKI_TARGET_SKY
linkaddr_t sink = {{0x15, 0x13}}; /* Firefly (testbed): node 55 will be our sink */
#define APP_NODES 21
linkaddr_t dest_list[] = {
  /* {{0xF7, 0x9C}}, */  /*  1 */
  /* {{0xD9, 0x76}}, */  /*  2 */
  /* {{0xF3, 0x84}}, */  /*  3 */
  /* {{0xF3, 0xEE}}, */  /*  4 */
  /* {{0xF7, 0x92}}, */  /*  5 */
  /* {{0xF3, 0x9A}}, */  /*  6 */
  /* {{0xDE, 0x21}}, */  /*  7 */
  /* {{0xF2, 0xA1}}, */  /*  8 */
  /* {{0xD8, 0xB5}}, */  /*  9 */
  /* {{0xF2, 0x1E}}, */  /* 10 */
  /* {{0xD9, 0x5F}}, */  /* 11 */
  /* {{0xF2, 0x33}}, */  /* 12 */
  /* {{0xDE, 0x0C}}, */  /* 13 */
  /* {{0xF2, 0x0E}}, */  /* 14 */
  /* {{0xD9, 0x49}}, */  /* 15 */
  /* {{0xF3, 0xDC}}, */  /* 16 */
  /* {{0xD9, 0x23}}, */  /* 17 */
  /* {{0xF3, 0x8B}}, */  /* 18 */
  /* {{0xF3, 0xC2}}, */  /* 19 */
  /* {{0xF3, 0xB7}}, */  /* 20 */
  /* {{0xDE, 0xE4}}, */  /* 21 */
  /* {{0xF3, 0x88}}, */  /* 22 */
  /* {{0xF7, 0x9A}}, */  /* 23 */
  /* {{0xF7, 0xE7}}, */  /* 24 */
  /* {{0xF2, 0x85}}, */  /* 25 */
  /* {{0xF2, 0x27}}, */  /* 26 */
  /* {{0xF2, 0x64}}, */  /* 27 */
  /* {{0xF3, 0xD3}}, */  /* 28 */
  /* {{0xF3, 0x8D}}, */  /* 29 */
  /* {{0xF7, 0xE1}}, */  /* 30 */
  /* {{0xDE, 0xAF}}, */  /* 31 */
  /* {{0xF2, 0x91}}, */  /* 32 */
  /* {{0xF2, 0xD7}}, */  /* 33 */
  /* {{0xF3, 0xA3}}, */  /* 34 */
  /* {{0xF2, 0xD9}}, */  /* 35 */
  /* {{0xD9, 0x9F}}, */  /* 36 */
  {{0xF3, 0x90}}, /* 50 HALL-A */
  {{0xF2, 0x3D}}, /* 51 HALL-A */
  {{0xF7, 0xAB}}, /* 52 HALL-A */
  {{0xF7, 0xC9}}, /* 53 HALL-A */
  {{0xF2, 0x6C}}, /* 54 HALL-A */
  /* {{0x15, 0x13}}, */  /* 55 HALL-A */
  {{0xF2, 0xFC}}, /* 56 HALL-A */
  {{0xF1, 0xF6}}, /* 57 HALL-A */
  {{0x15, 0x3F}}, /* 58 HALL-A */
  {{0x15, 0x5D}}, /* 61 HALL-A */
  {{0xF3, 0xCF}}, /* 62 HALL-A */
  {{0xF3, 0xC3}}, /* 63 HALL-A */
  {{0xF7, 0xD6}}, /* 64 HALL-A */
  {{0xF7, 0xB6}}, /* 65 HALL-A */
  {{0xF7, 0xB7}}, /* 70 HALL-A */
  {{0xF3, 0xF3}}, /* 71 HALL-A */
  {{0xF1, 0xF3}}, /* 72 HALL-A */
  {{0xF2, 0x48}}, /* 73 HALL-A */
  {{0xF3, 0xDB}}, /* 74 HALL-A */
  {{0xF3, 0xFA}}, /* 75 HALL-A */
  {{0xF3, 0x83}}, /* 76 HALL-A */
  {{0xF2, 0xB4}}, /* 77 HALL-A */
  /* {{0x15, 0xDB}}, */  /* 100 */
  /* {{0x15, 0x3D}}, */  /* 101 */
  /* {{0x16, 0x5B}}, */  /* 102 */
  /* {{0x14, 0xC3}}, */  /* 103 */
  /* {{0x15, 0x8C}}, */  /* 104 */
  /* {{0x15, 0xF3}}, */  /* 105 */
  /* {{0x16, 0x1B}}, */  /* 106 */
  /* {{0x14, 0x97}}, */  /* 107 */
  /* {{0x15, 0xB4}}, */  /* 108 */
  /* {{0x14, 0xDE}}, */  /* 109 */
  /* {{0x16, 0x36}}, */  /* 110 */
  /* {{0x14, 0xF2}}, */  /* 111 */
  /* {{0x15, 0x5A}}, */  /* 113 */
  /* {{0x16, 0x16}}, */  /* 114 */
  /* {{0x15, 0xD4}}, */  /* 115 */
  /* {{0x15, 0xDA}}, */  /* 116 */
  /* {{0x14, 0xDA}}, */  /* 117 */
  /* {{0x14, 0xEA}}, */  /* 118 */
  /* {{0x14, 0x9B}}, */  /* 119 */
  /* {{0x14, 0xE6}}, */  /* 121 */
  /* {{0x16, 0x31}}, */  /* 122 */
  /* {{0x14, 0xC9}}, */  /* 123 */
  /* {{0x14, 0x99}}, */  /* 124 */
  /* {{0x15, 0xBC}}, */  /* 125 */
  /* {{0x15, 0x7B}}, */  /* 126 */
  /* {{0x16, 0xFE}}, */  /* 127 */
  /* {{0x15, 0xF2}}, */  /* 128 */
  /* {{0x14, 0xE8}}, */  /* 129 */
  /* {{0x14, 0xA8}}, */  /* 130 */
  /* {{0x15, 0x87}}, */  /* 131 */
  /* {{0x15, 0xB0}}, */  /* 132 */
  /* {{0x15, 0x20}}, */  /* 133 */
  /* {{0x15, 0x92}}, */  /* 134 */
  /* {{0x14, 0xCE}}, */  /* 135 */
  /* {{0x15, 0x3E}}, */  /* 136 */
  /* {{0x15, 0x4C}}, */  /* 137 */
  /* {{0x16, 0x71}}, */  /* 138 */
  /* {{0xF2, 0xEB}}, */  /* 139 */
  /* {{0xF2, 0xE1}}, */  /* 140 */
  /* {{0xF7, 0xC3}}, */  /* 141 */
  /* {{0xF3, 0xAF}}, */  /* 142 */
  /* {{0xF7, 0xAF}}, */  /* 143 */
  /* {{0xF3, 0xF0}}, */  /* 144 */
  /* {{0x16, 0x5F}}, */  /* 145 */
  /* {{0x15, 0xEA}}, */  /* 146 */
  /* {{0x16, 0x33}}, */  /* 147 */
  /* {{0x16, 0x2D}}, */  /* 148 */
  /* {{0x15, 0xC4}}, */  /* 149 */
  /* {{0x15, 0x4F}}, */  /* 150 */
  /* {{0x16, 0x28}}, */  /* 151 */
  /* {{0x16, 0x99}}, */  /* 152 */
  /* {{0x15, 0x95}}, */  /* 153 */
  /* {{0x16, 0x5C}}, */  /* 154 */
};
#else
linkaddr_t sink = {{0x01, 0x00}}; /* TMote Sky (Cooja): node 1 will be our sink */
#define APP_NODES 14
linkaddr_t dest_list[] = {
  {{0x02, 0x00}},
  {{0x03, 0x00}},
  {{0x04, 0x00}},
  {{0x05, 0x00}},
  {{0x06, 0x00}},
  {{0x07, 0x00}}, 
  {{0x08, 0x00}}, 
  {{0x09, 0x00}}, 
  {{0xA, 0x00}},
  {{0xB, 0x00}},
  {{0xC, 0x00}},
  {{0xD, 0x00}},
  {{0xE, 0x00}},
  {{0xF, 0x00}},
};
#endif
/*---------------------------------------------------------------------------*/
PROCESS(app_process, "App process");
AUTOSTART_PROCESSES(&app_process);
/*---------------------------------------------------------------------------*/
/* Application packet */
typedef struct {
  uint16_t seqn;
}
__attribute__((packed))
test_msg_t;
/*---------------------------------------------------------------------------*/
static struct my_collect_conn my_collect;
static void recv_cb(const linkaddr_t *originator, const linkaddr_t *parent, uint8_t hops);
/*
 * Source Routing Callback
 * This function is called upon receiving a message from the sink in a node.
 * Params:
 *  ptr: a pointer to the connection of the collection protocol
 *  hops: number of hops of the route followed by the packet to reach the destination
 */
static void sr_recv_cb(struct my_collect_conn *ptr, uint8_t hops);
static void report_recv_cb(struct my_collect_conn *ptr);
/*---------------------------------------------------------------------------*/
static struct my_collect_callbacks sink_cb = {
  .recv = recv_cb,
  .sr_recv = NULL,
  .report_recv = report_recv_cb
};
/*---------------------------------------------------------------------------*/
static struct my_collect_callbacks node_cb = {
  .recv = NULL,
  .sr_recv = sr_recv_cb,
  .report_recv = NULL
};
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(app_process, ev, data)
{
  static struct etimer periodic;
  static struct etimer rnd;
  static test_msg_t msg = {.seqn=0};
  static uint8_t dest_idx = 0;
  static linkaddr_t dest = {{0x00, 0x00}};

  PROCESS_BEGIN();

  /* Start energest to estimate node duty cycle */
  simple_energest_start();

  /* ------------------SINK-------------------- */ 
  if (linkaddr_cmp(&sink, &linkaddr_node_addr)) {
    printf("App: I am sink %02x:%02x\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
    my_collect_open(&my_collect, COLLECT_CHANNEL, true, &sink_cb);

#if APP_DOWNWARD_TRAFFIC == 1

    etimer_set(&periodic, SR_MSG_DELAY);
    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic));
      /* Fixed interval */
      etimer_set(&periodic, SR_MSG_PERIOD);
      etimer_set(&rnd, random_int(SR_MSG_PERIOD * 0.5));
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&rnd));

      /* Set application data packet */
      packetbuf_clear();
      memcpy(packetbuf_dataptr(), &msg, sizeof(msg));
      packetbuf_set_datalen(sizeof(msg));

      /* Change the destination link address to a different node */
      linkaddr_copy(&dest, &dest_list[dest_idx]);

      /* Send the packet downwards */
      printf("App: sink sending seqn %d to %02x:%02x\n",
        msg.seqn, dest.u8[0], dest.u8[1]);
      int ret = sr_send(&my_collect, &dest);

      switch(ret){
        case -3:
          printf("App: error sending sending seqn %d to %02x:%02x, loop found\n",
        msg.seqn, dest.u8[0], dest.u8[1]);
          break;
        case -2:
          printf("App: error sending sending seqn %d to %02x:%02x, cannot alloc header\n",
        msg.seqn, dest.u8[0], dest.u8[1]);
          break;
        case -1:
          printf("App: error sending sending seqn %d to %02x:%02x, null parent\n",
        msg.seqn, dest.u8[0], dest.u8[1]);
          break;
        case 0:
          printf("App: error sending sending seqn %d to %02x:%02x, generic error\n",
        msg.seqn, dest.u8[0], dest.u8[1]);
          break;
        case 1:
          printf("App: seqn %d to %02x:%02x successfully sent\n",
        msg.seqn, dest.u8[0], dest.u8[1]);
      }

      /* Update sequence number and next destination address */
      msg.seqn++;
      dest_idx++;
      if (dest_idx >= APP_NODES) {
        dest_idx = 0;
      }
    }
#endif
  }
  /* ------------------NORMAL NODE-------------------- */
  else {
    printf("App: I am normal node %02x:%02x\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
    my_collect_open(&my_collect, COLLECT_CHANNEL, false, &node_cb);
    
#if APP_UPWARD_TRAFFIC == 1
    etimer_set(&periodic, MSG_DELAY);
    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic));
      /* Fixed interval */
      etimer_set(&periodic, MSG_PERIOD);
      etimer_set(&rnd, random_int(MSG_PERIOD * 0.5));
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&rnd));

      packetbuf_clear();
      memcpy(packetbuf_dataptr(), &msg, sizeof(msg));
      packetbuf_set_datalen(sizeof(msg));
      printf("App: Send seqn %d\n", msg.seqn);
      int ret = my_collect_send(&my_collect);
      switch(ret){
        case -2:
          printf("App: error sending seqn %d, cannot alloc header\n", msg.seqn);
          break;
        case -1:
          printf("App: error sending seqn %d, null parent\n", msg.seqn);
          break;
        case 0:
          printf("App: error sending seqn %d, generic error\n", msg.seqn);
          break;
        case 1:
          printf("App: seqn %d successfully sent\n", msg.seqn);
          break;
      }
      msg.seqn ++;
    }
#endif
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void 
recv_cb(const linkaddr_t *originator, const linkaddr_t *parent, uint8_t hops)
{
  test_msg_t msg;
  if (packetbuf_datalen() != sizeof(msg)) {
    printf("App: wrong length: %d\n", packetbuf_datalen());
    return;
  }
  memcpy(&msg, packetbuf_dataptr(), sizeof(msg));
  printf("App: Recv from %02x:%02x seqn %d hops %d parent %02x:%02x\n",
    originator->u8[0], originator->u8[1], msg.seqn, hops, parent->u8[0], parent->u8[1]);
}
/*---------------------------------------------------------------------------*/
static void
sr_recv_cb(struct my_collect_conn *ptr, uint8_t hops)
{
  test_msg_t sr_msg;
  if (packetbuf_datalen() != sizeof(test_msg_t)) {
    printf("App: sr_recv wrong length: %d\n", packetbuf_datalen());
    return;
  }
  memcpy(&sr_msg, packetbuf_dataptr(), sizeof(test_msg_t));
  printf("App: sr_recv from sink seqn %u hops %u node metric %u\n",
    sr_msg.seqn, hops, ptr->metric);
}
/*---------------------------------------------------------------------------*/
static void
report_recv_cb(struct my_collect_conn *ptr)
{
  struct topology_report msg;
  if (packetbuf_datalen() != sizeof(msg)) {
    printf("App: report_recv wrong length: %d\n", packetbuf_datalen());
    return;
  }
  memcpy(&msg, packetbuf_dataptr(), sizeof(msg));
  printf("App: report_recv from sink node %02x:%02x parent %02x:%02x\n",
    msg.source.u8[0], msg.source.u8[1], msg.parent.u8[0], msg.parent.u8[1]);
}
/*---------------------------------------------------------------------------*/
int
sr_send(struct my_collect_conn* conn, linkaddr_t* dest){
  linkaddr_t next = *dest;
  linkaddr_t parent = topology_get(next);
  uint8_t hops = 0;

  /* add checkpoint (null address) that represents the end of the route of the header */ 
  if (!packetbuf_hdrcopy_linkaddr(linkaddr_null)) {
    return -2;
  }

  while(hops <= MAX_PATH_LENGTH){

    /* if the node doesn't have a parent, exit */ 
    if(linkaddr_cmp(&parent, &linkaddr_null)) {
      return -1;
    }

    /* ROUTING LOOP CHECK */ 
    /* if the parent is found inside the route, there's a loop */ 
    if(packetbuf_hdrcontains(parent)) {
      return -3;
    }

    hops++;
    /* if the parent is a sink, add the route length to the header, then unicast the packet */ 
    if(linkaddr_cmp(&parent, &sink)){
      /* embed the hops counter inside a linkaddr_t so in the unicast receive callback i can overwrite the databuf without problems */ 
      linkaddr_t h = {{hops, 0x00}};
      if (!packetbuf_hdrcopy_linkaddr(h)) {
        return -2;
      }
      return unicast_send(&conn->uc, &next);
    }
    /* otherwise, add the node to the route and compute the next parent */ 
    if (!packetbuf_hdrcopy_linkaddr(next)) {
      return -2;
    }
    next = parent;
    parent = topology_get(next);
  }
  return -1;
}