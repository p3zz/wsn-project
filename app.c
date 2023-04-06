#include "contiki.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include "leds.h"
#include "net/netstack.h"
#include <stdio.h>
#include "core/net/linkaddr.h"
#include "my_collect.h"
#include "simple-energest.h"
#include <stdlib.h>
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
struct collect_header* topology = NULL;
int topology_size = 0;
void topology_allocate();
int topology_set(linkaddr_t node, linkaddr_t parent);
linkaddr_t topology_get(linkaddr_t node);
void topology_print();
bool contains(linkaddr_t* route, linkaddr_t addr);
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
static void recv_cb(const linkaddr_t *originator, const linkaddr_t *parent);
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
    topology_size = (int) (sizeof(dest_list) / sizeof(linkaddr_t));
    topology_allocate();

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
static void recv_cb(const linkaddr_t *originator, const linkaddr_t *parent) {
  test_msg_t msg;
  if (packetbuf_datalen() != sizeof(msg)) {
    printf("App: wrong length: %d\n", packetbuf_datalen());
    return;
  }
  memcpy(&msg, packetbuf_dataptr(), sizeof(msg));
  printf("App: Recv from %02x:%02x seqn %u parent %02x:%02x\n",
    originator->u8[0], originator->u8[1], msg.seqn, parent->u8[0], parent->u8[1]);
  int res = topology_set(*originator, *parent);
  switch(res){
    case 0:
      // printf("topology ok\n");
      break;
    case -1:
      // printf("topology does not exist\n");
      break;
    case -2:
      // printf("topology source not found\n");
      break;
  }
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

// TODO add routing loops control
int sr_send(struct my_collect_conn* conn, linkaddr_t* dest){
  linkaddr_t next = *dest;
  linkaddr_t parent = topology_get(next);
  uint8_t hops = 0;

  // add checkpoint (null address) that represents the end of the route of the header
  if (!packetbuf_hdralloc(sizeof(linkaddr_t))) return -2;
  memcpy(packetbuf_hdrptr(), &linkaddr_null, sizeof(linkaddr_t));

  while(hops != topology_size){

    if(linkaddr_cmp(&parent, &linkaddr_null)) return -3;

    // routing loop check
    // retrieve the route saved inside the header
    linkaddr_t* route = (linkaddr_t*) packetbuf_hdrptr();
    // if the parent is found inside the route, there's a loop
    if(contains(route, parent)){
      return -4;
    };

    hops++;
    // if the parent is a sink, add the route length to the header, then unicast the packet
    if(linkaddr_cmp(&parent, &sink)){
      // embed the hops counter inside a linkaddr_t so in the unicast receive callback i can overwrite the databuf without problems
      linkaddr_t h = {{hops, 0x00}};
      if(!packetbuf_hdralloc(sizeof(h))) return -2;
      memcpy(packetbuf_hdrptr(), &h, sizeof(h));
      printf("Route computed: %d hops\n", h.u8[0]);
      return unicast_send(&conn->uc, &next);
    }
    // otherwise, add the node to the route and compute the next parent
    if(!packetbuf_hdralloc(sizeof(next))) return -2;
    memcpy(packetbuf_hdrptr(), &next, sizeof(next));
    next = parent;
    parent = topology_get(next);
  }
  return -1;
}

bool contains(linkaddr_t* route, linkaddr_t addr){
  int i = 0;
  printf("Loop check\n");
  while(!linkaddr_cmp(&route[i], &linkaddr_null)){
    if(linkaddr_cmp(&route[i], &addr)) return true;
    printf("checking %02x:%02x against %02x:%02x\n", route[i].u8[0], route[i].u8[1], addr.u8[0], addr.u8[1]);
    i++;
  }
  return false;
}

/*TOPOLOGY---------------------------------------------------------------------------*/
int topology_set(linkaddr_t node, linkaddr_t parent){
  if(topology == NULL) return -1;
  int i;
  for(i=0;i<topology_size;i++){
    if(linkaddr_cmp(&topology[i].source, &node)){
      linkaddr_copy(&topology[i].parent, &parent);
      topology_print();
      return 0;
    }
  }
  return -2;
}

linkaddr_t topology_get(linkaddr_t node){
  if(topology == NULL) return linkaddr_null;
  int i;
  for(i=0;i<topology_size;i++){
    if(linkaddr_cmp(&topology[i].source, &node)){
      return topology[i].parent;
    }
  }
  return linkaddr_null;
}

void topology_allocate(){
  topology = (struct collect_header*) malloc(topology_size * sizeof(struct collect_header));
  int i;
  for(i=0;i<topology_size;i++){
    linkaddr_copy(&topology[i].source, &dest_list[i]);
    linkaddr_copy(&topology[i].parent, &linkaddr_null);
    printf("allocate: [node: %02x:%02x]\n", topology[i].source.u8[0], topology[i].source.u8[1]);
  }
}

void topology_print(){
  if(topology == NULL) return;
  printf("topology: \n");
  int i;
  for(i=0;i<topology_size;i++){
    printf("[node: %02x:%02x, parent: %02x:%02x]\n", topology[i].source.u8[0], topology[i].source.u8[1], topology[i].parent.u8[0], topology[i].parent.u8[1]);
  }
}