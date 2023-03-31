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
static void recv_cb(const linkaddr_t *originator, uint8_t hops);
/*
 * Source Routing Callback
 * This function is called upon receiving a message from the sink in a node.
 * Params:
 *  ptr: a pointer to the connection of the collection protocol
 *  hops: number of hops of the route followed by the packet to reach the destination
 */
static void sr_recv_cb(struct my_collect_conn *ptr, uint8_t hops);
/*---------------------------------------------------------------------------*/
static struct my_collect_callbacks sink_cb = {
  .recv = recv_cb,
  .sr_recv = NULL,
};
/*---------------------------------------------------------------------------*/
static struct my_collect_callbacks node_cb = {
  .recv = NULL,
  .sr_recv = sr_recv_cb,
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

  if (linkaddr_cmp(&sink, &linkaddr_node_addr)) {
    printf("App: I am sink %02x:%02x\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
    my_collect_open(&my_collect, COLLECT_CHANNEL, true, &sink_cb);

#if APP_DOWNWARD_TRAFFIC == 1
    /* Wait a bit longer at the beginning to gather enough topology information */
    etimer_set(&periodic, 75 * CLOCK_SECOND);
    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic));
      /* Fixed interval */
      etimer_set(&periodic, SR_MSG_PERIOD);
      /* Random shift within the first half of the interval */
      etimer_set(&rnd, random_rand() % (SR_MSG_PERIOD / 2));
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
#endif /* APP_DOWNWARD_TRAFFIC == 1 */
  }
  else {
    printf("App: I am normal node %02x:%02x\n", linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);
    my_collect_open(&my_collect, COLLECT_CHANNEL, false, &node_cb);
    
#if APP_UPWARD_TRAFFIC == 1
    etimer_set(&periodic, 30 * CLOCK_SECOND);
    while(1) {
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic));
      /* Fixed interval */
      etimer_set(&periodic, MSG_PERIOD);
      /* Random shift within the interval */
      etimer_set(&rnd, random_rand() % (MSG_PERIOD/2));
      PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&rnd));

      packetbuf_clear();
      memcpy(packetbuf_dataptr(), &msg, sizeof(msg));
      packetbuf_set_datalen(sizeof(msg));
      printf("App: Send seqn %d\n", msg.seqn);
      my_collect_send(&my_collect);
      msg.seqn ++;
    }
#endif /* APP_UPWARD_TRAFFIC == 1 */
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static void recv_cb(const linkaddr_t *originator, uint8_t hops) {
  test_msg_t msg;
  if (packetbuf_datalen() != sizeof(msg)) {
    printf("App: wrong length: %d\n", packetbuf_datalen());
    return;
  }
  memcpy(&msg, packetbuf_dataptr(), sizeof(msg));
  printf("App: Recv from %02x:%02x seqn %u hops %u\n",
    originator->u8[0], originator->u8[1], msg.seqn, hops);
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
