#include <stdbool.h>
#include "contiki.h"
#include "lib/random.h"
#include "net/rime/rime.h"
#include "leds.h"
#include "net/netstack.h"
#include <stdio.h>
#include "core/net/linkaddr.h"
#include "my_collect.h"
/*---------------------------------------------------------------------------*/
#define BEACON_INTERVAL (CLOCK_SECOND * 60) /* [Lab 7] Try to change this period to  
                                             * analyse how it affects the radio-on time
                                             * (energy consumption) of you solution */ 
#define BEACON_FORWARD_DELAY (random_rand() % CLOCK_SECOND)
/*---------------------------------------------------------------------------*/
#define RSSI_THRESHOLD -95
/*---------------------------------------------------------------------------*/
/* Callback function declarations */
void bc_recv(struct broadcast_conn *conn, const linkaddr_t *sender);
void uc_recv(struct unicast_conn *c, const linkaddr_t *from);
void beacon_timer_cb(void* ptr);
/*---------------------------------------------------------------------------*/
/* Rime Callback structures */
struct broadcast_callbacks bc_cb = {
  .recv = bc_recv,
  .sent = NULL
};
struct unicast_callbacks uc_cb = {
  .recv = uc_recv,
  .sent = NULL
};
/*---------------------------------------------------------------------------*/
void my_collect_open(struct my_collect_conn* conn, uint16_t channels, 
                bool is_sink, const struct my_collect_callbacks *callbacks){

  linkaddr_copy(&conn->parent, &linkaddr_null);
  conn->metric = 65535;
  conn->beacon_seqn = 0;
  conn->is_sink = is_sink;
  conn->callbacks = callbacks;

  /* Open the underlying Rime primitives */
  broadcast_open(&conn->bc, channels,     &bc_cb);
  unicast_open  (&conn->uc, channels + 1, &uc_cb);

  if (conn->is_sink) {
    conn->metric = 0; /* The sink hop count is (by definition) *always* equal to 0.
                       * Remember to update this field *before sending the first*
                       * beacon message in broadcast! */
    /* Schedule the first beacon message flood */
    ctimer_set(&conn->beacon_timer, CLOCK_SECOND, beacon_timer_cb, conn);
  }
}
/* Send beacon using the current seqn and metric */
void
send_beacon(struct my_collect_conn* conn)
{
  /* Prepare the beacon message */
  struct beacon_msg beacon = {
    .seqn = conn->beacon_seqn, .metric = conn->metric};

  /* Send the beacon message in broadcast */
  packetbuf_clear();
  packetbuf_copyfrom(&beacon, sizeof(beacon));
  printf("my_collect: sending beacon: seqn %d metric %d\n",
    conn->beacon_seqn, conn->metric);
  broadcast_send(&conn->bc);
}
/*---------------------------------------------------------------------------*/
/* Beacon timer callback */
void
beacon_timer_cb(void* ptr)
{
  struct my_collect_conn* conn = (struct my_collect_conn*)ptr; /* [Side note] Even an implicit cast
                                                                * struct my_collect_conn* conn = ptr;
                                                                * works correctly */
  send_beacon(conn);

  /* --- Sink-only logic: Rebuild the tree from scratch after the beacon interval --- */
  if (conn->is_sink) {
    conn->beacon_seqn ++; /* Before beginning a new beacon flood, the sink 
                           * should ALWAYS increase the beacon sequence number! */
    /* Schedule the next beacon message flood */
    ctimer_set(&conn->beacon_timer, BEACON_INTERVAL, beacon_timer_cb, conn);
  }
}
/*---------------------------------------------------------------------------*/
/* Beacon receive callback */
void
bc_recv(struct broadcast_conn *bc_conn, const linkaddr_t *sender)
{
  struct beacon_msg beacon;
  int16_t rssi;

  /* Get the pointer to the overall structure my_collect_conn from its field bc */
  struct my_collect_conn* conn = (struct my_collect_conn*)(((uint8_t*)bc_conn) - 
    offsetof(struct my_collect_conn, bc));

  /* Check if the received broadcast packet looks legitimate */
  if (packetbuf_datalen() != sizeof(struct beacon_msg)) {
    printf("my_collect: broadcast of wrong size\n");
    return;
  }
  memcpy(&beacon, packetbuf_dataptr(), sizeof(struct beacon_msg));

  rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);

  printf("my_collect: recv beacon from %02x:%02x seqn %u metric %u rssi %d\n", 
      sender->u8[0], sender->u8[1], 
      beacon.seqn, beacon.metric, rssi);

  if (rssi < RSSI_THRESHOLD || beacon.seqn < conn->beacon_seqn)
    return; // The beacon is either too weak or too old, ignore it
  if (beacon.seqn == conn->beacon_seqn) { // The beacon is not new, check the metric
    if (beacon.metric+1 >= conn->metric)
      return; // Worse or equal than what we have, ignore it
  }
  /* Otherwise, memorize the new parent, the metric, and the seqn */
  linkaddr_copy(&conn->parent, sender);
  conn->metric = beacon.metric + 1;
  conn->beacon_seqn = beacon.seqn;
  printf("my_collect: new parent %02x:%02x, my metric %d\n", 
      sender->u8[0], sender->u8[1], conn->metric);

  ctimer_set(&conn->beacon_timer, BEACON_FORWARD_DELAY, beacon_timer_cb, conn);
}
/* Data Collection: send function */
int
my_collect_send(struct my_collect_conn *conn)
{
  if (linkaddr_cmp(&conn->parent, &linkaddr_null)) // The node is still disconnected 
    return -1; // Inform the app that my_collect is currently unable to forward/deliver the packet
  if (!packetbuf_hdralloc(sizeof(struct collect_header))) return -2; 
  struct collect_header hdr = {.source=linkaddr_node_addr, .parent=conn->parent}; // Prepare the collection header
  memcpy(packetbuf_hdrptr(), &hdr, sizeof(hdr)); /* Copy the collection header in front of 
                                                  * the application payload (at the beginning of
                                                  * the packet buffer) */
  /* 5.4 */
  return unicast_send(&conn->uc, &conn->parent);
}
/*---------------------------------------------------------------------------*/
/* Data receive callback */
void
uc_recv(struct unicast_conn *uc_conn, const linkaddr_t *from)
{
  /* Get the pointer to the overall structure my_collect_conn from its field uc */
  struct my_collect_conn* conn = (struct my_collect_conn*)(((uint8_t*)uc_conn) - 
    offsetof(struct my_collect_conn, uc));

  struct collect_header hdr;

  /* Check if the received unicast message looks legitimate */
  if (packetbuf_datalen() < sizeof(struct collect_header)) {
    printf("my_collect: too short unicast packet %d\n", packetbuf_datalen());
    return;
  }

  memcpy(&hdr, packetbuf_dataptr(), sizeof(hdr));

  if (conn->is_sink) {
    if (packetbuf_hdrreduce(sizeof(struct collect_header)))
      conn->callbacks->recv(&hdr.source, &hdr.parent); // Call the sink recv callback function 
    else
      printf("my_collect: ERROR, the header could not be reduced!");
  }
  else {/* Non-sink node acting as a forwarder. Send the received packet to the node's parent in unicast */
    if (linkaddr_cmp(&conn->parent, &linkaddr_null)) {  /* Just to be sure, and to detect potential bugs. 
                                                         * If the node is disconnected, my-collect will be 
                                                         * unable to forward the data packet upwards */
      printf("my_collect: ERROR, unable to forward data packet -- "
        "source: %02x:%02x", hdr.source.u8[0], hdr.source.u8[1]);
      return;
    }
    memcpy(packetbuf_dataptr(), &hdr, sizeof(hdr)); // Update the my-collect header in the packet buffer
    unicast_send(&conn->uc, &conn->parent); // Send the updated message to the node's parent in unicast
  }
}
/*---------------------------------------------------------------------------*/
int sr_send(struct my_collect_conn* conn, linkaddr_t* dest){
  return 1;  
}