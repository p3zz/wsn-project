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
linkaddr_t sink = {{0xF7, 0x9C}}; /* Firefly (testbed): node 1 will be our sink */
#define APP_NODES 10
linkaddr_t dest_list[] = {
  {{0xF3, 0x84}}, /* Firefly node 3 */
  {{0xD8, 0xB5}}, /* Firefly node 9 */
  {{0xF2, 0x33}}, /* Firefly node 12 */
  {{0xD9, 0x23}}, /* Firefly node 17 */
  {{0xF3, 0xC2}}, /* Firefly node 19 */
  {{0xDE, 0xE4}}, /* Firefly node 21 */
  {{0xF2, 0x64}}, /* Firefly node 27 */
  {{0xF7, 0xE1}}, /* Firefly node 30 */
  {{0xF2, 0xD7}}, /* Firefly node 33 */
  {{0xF3, 0xA3}}  /* Firefly node 34 */
};
#else
linkaddr_t sink = {{0x01, 0x00}}; /* TMote Sky (Cooja): node 1 will be our sink */
#define APP_NODES 9
linkaddr_t dest_list[] = {
  {{0x02, 0x00}},
  {{0x03, 0x00}},
  {{0x04, 0x00}},
  {{0x05, 0x00}},
  {{0x06, 0x00}}, 
  {{0x07, 0x00}}, 
  {{0x08, 0x00}}, 
  {{0x09, 0x00}}, 
  {{0xA, 0x00}}
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
  static int ret;

  PROCESS_BEGIN();

  /* Start energest to estimate node duty cycle */
  simple_energest_start();

  // ------------------SINK--------------------
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
      ret = sr_send(&my_collect, &dest);

      /* Check that the packet could be sent */
      if(ret == 0) {
        printf("App: sink could not send seqn %d to %02x:%02x\n",
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
  // ------------------NORMAL NODE--------------------
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
      my_collect_send(&my_collect);
      msg.seqn ++;
    }
#endif
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void recv_cb(const linkaddr_t *originator, const linkaddr_t *parent, uint8_t hops) {
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
static void sr_recv_cb(struct my_collect_conn *ptr, uint8_t hops){
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
static void report_recv_cb(struct my_collect_conn *ptr){
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
int sr_send(struct my_collect_conn* conn, linkaddr_t* dest){
  linkaddr_t next = *dest;
  linkaddr_t parent = topology_get(next);
  uint8_t hops = 0;

  // add checkpoint (null address) that represents the end of the route of the header
  if (!packetbuf_hdrcopy_linkaddr(linkaddr_null)) return -2;

  while(hops <= MAX_PATH_LENGTH){

    // if the node doesn't have a parent, exit
    if(linkaddr_cmp(&parent, &linkaddr_null)) return -3;

    // ROUTING LOOP CHECK
    // if the parent is found inside the route, there's a loop
    if(packetbuf_hdrcontains(parent)) return -4;

    hops++;
    // if the parent is a sink, add the route length to the header, then unicast the packet
    if(linkaddr_cmp(&parent, &sink)){
      // embed the hops counter inside a linkaddr_t so in the unicast receive callback i can overwrite the databuf without problems
      linkaddr_t h = {{hops, 0x00}};
      if (!packetbuf_hdrcopy_linkaddr(h)) return -2;
      // packetbuf_hdrprint();
      return unicast_send(&conn->uc, &next);
    }
    // otherwise, add the node to the route and compute the next parent
    if (!packetbuf_hdrcopy_linkaddr(next)) return -2;
    next = parent;
    parent = topology_get(next);
  }
  return -1;
}