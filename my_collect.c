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
/*---------------------------------------------------------------------------*/
void bc_recv(struct broadcast_conn *conn, const linkaddr_t *sender);
void uc_recv(struct unicast_conn *c, const linkaddr_t *from);
void beacon_timer_cb(void* ptr);
void report_timer_cb(void* ptr);
void topology_allocate();
int topology_set(linkaddr_t node, linkaddr_t parent);
linkaddr_t topology_get(linkaddr_t node);
void topology_print();
bool packetbuf_hdrcopy_linkaddr(linkaddr_t addr);
bool packetbuf_hdrcontains(linkaddr_t addr);
void packetbuf_hdrprint();
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
struct topology_report* topology = NULL;
int topology_size = 0;
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
    conn->metric = 0;
    topology_allocate();
    ctimer_set(&conn->beacon_timer, CLOCK_SECOND, beacon_timer_cb, conn);
  }
#if TOPOLOGY_REPORT_ENABLED == 1
  else{
    ctimer_set(&conn->report_timer, TOPOLOGY_REPORT_DELAY, report_timer_cb, conn);
  }
#endif
}

/* Topology report timer callback */
void report_timer_cb(void* ptr){
  struct my_collect_conn* conn = (struct my_collect_conn*)ptr; 
  struct topology_report report = {.source=linkaddr_node_addr, .parent=conn->parent};
  packetbuf_clear();
  packetbuf_copyfrom(&report, sizeof(report));
  unicast_send(&conn->uc, &conn->parent);
  printf("[REPORT]: send report [node: %02x:%02x, parent: %02x:%02x]\n", report.source.u8[0], report.source.u8[1], report.parent.u8[0], report.parent.u8[1]);
  ctimer_set(&conn->report_timer, TOPOLOGY_REPORT_PERIOD, report_timer_cb, conn);
}

/* Send beacon using the current seqn and metric */
void send_beacon(struct my_collect_conn* conn){
  /* Prepare the beacon message */
  struct beacon_msg beacon = {
    .seqn = conn->beacon_seqn, .metric = conn->metric};

  /* Send the beacon message in broadcast */
  packetbuf_clear();
  packetbuf_copyfrom(&beacon, sizeof(beacon));
  printf("[BEACON]: send seqn %d metric %d\n",
    conn->beacon_seqn, conn->metric);
  broadcast_send(&conn->bc);
}
/*---------------------------------------------------------------------------*/
/* Beacon timer callback */
void beacon_timer_cb(void* ptr){
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
void bc_recv(struct broadcast_conn *bc_conn, const linkaddr_t *sender){
  struct beacon_msg beacon;
  int16_t rssi;

  /* Get the pointer to the overall structure my_collect_conn from its field bc */
  struct my_collect_conn* conn = (struct my_collect_conn*)(((uint8_t*)bc_conn) - 
    offsetof(struct my_collect_conn, bc));

  /* Check if the received broadcast packet looks legitimate */
  if (packetbuf_datalen() != sizeof(struct beacon_msg)) {
    printf("[BEACON]: broadcast of wrong size\n");
    return;
  }
  memcpy(&beacon, packetbuf_dataptr(), sizeof(struct beacon_msg));

  rssi = packetbuf_attr(PACKETBUF_ATTR_RSSI);

  printf("[BEACON]: recv from %02x:%02x seqn %u metric %u rssi %d\n", 
      sender->u8[0], sender->u8[1], 
      beacon.seqn, beacon.metric, rssi);

  // The beacon is either too weak or too old, ignore it
  if (rssi < RSSI_THRESHOLD || beacon.seqn < conn->beacon_seqn) return; 
  if (beacon.seqn == conn->beacon_seqn){
    // The beacon is not new and the metric is higher than the previous, ignore it
    if(beacon.metric+1 >= conn->metric) return; 
  }
  /* Otherwise, memorize the new parent, the metric, and the seqn */
  linkaddr_copy(&conn->parent, sender);
  conn->metric = beacon.metric + 1;
  conn->beacon_seqn = beacon.seqn;
  printf("[BEACON]: new parent %02x:%02x, my metric %d\n", 
      sender->u8[0], sender->u8[1], conn->metric);

  ctimer_set(&conn->beacon_timer, BEACON_FORWARD_DELAY, beacon_timer_cb, conn);
}


/* Data Collection: send function */
int my_collect_send(struct my_collect_conn *conn){
  if (linkaddr_cmp(&conn->parent, &linkaddr_null)) return -1; // Inform the app that my_collect is currently unable to forward/deliver the packet
  if (!packetbuf_hdralloc(sizeof(struct collect_header))) return -2;
  struct topology_report report = {.source=linkaddr_node_addr, .parent=conn->parent};
  struct collect_header hdr = {.report = report, .hops = 0}; // Prepare the collection header
  memcpy(packetbuf_hdrptr(), &hdr, sizeof(hdr)); /* Copy the collection header in front of 
                                                  * the application payload (at the beginning of
                                                  * the packet buffer) */
  return unicast_send(&conn->uc, &conn->parent);
}
/*---------------------------------------------------------------------------*/
/* Data receive callback */
void uc_recv(struct unicast_conn *uc_conn, const linkaddr_t *from){
  /* Get the pointer to the overall structure my_collect_conn from its field uc */
  struct my_collect_conn* conn = (struct my_collect_conn*)(((uint8_t*)uc_conn) - 
    offsetof(struct my_collect_conn, uc));

  // ---------------- DOWNWARD DATA-----------------  
  // if the data comes from a parent, it must be downward data 
  if (linkaddr_cmp(from, &conn->parent)) {

    // retrieve the route length (on the top of the header), then remove it
    linkaddr_t hops;
    memcpy(&hops, packetbuf_dataptr(), sizeof(hops));
    printf("hops: %d\n", hops.u8[0]);
    if (!packetbuf_hdrreduce(sizeof(hops))) return;

    // retrieve the next node to hop to, but we don't remove it. It will be overwritten by the route length later
    linkaddr_t next;
    memcpy(&next, packetbuf_dataptr(), sizeof(next));

    printf("sr forward to %02x:%02x\n", next.u8[0], next.u8[1]);

    // check if we have reached the destination
    if (linkaddr_cmp(&next, &linkaddr_null)){
      if (!packetbuf_hdrreduce(sizeof(next))) return;
      conn->callbacks->sr_recv(conn, hops.u8[0]);
      return;
    }

    // overwrite the current node with the route length
    memcpy(packetbuf_dataptr(), &hops, sizeof(linkaddr_t));

    unicast_send(uc_conn, &next);
  }
  // ---------------- UPWARD DATA-----------------  
  else{
    if(packetbuf_datalen() == sizeof(struct topology_report)){
      struct topology_report report;
      memcpy(&report, packetbuf_dataptr(), sizeof(report));
      // if the current node is a sink, call the recv callback
      // FIXME we need to move the topology set here instead of sending a fake hops counter
      if (conn->is_sink) {
        topology_set(report.source, report.parent);
      }
      else{
        if (linkaddr_cmp(&conn->parent, &linkaddr_null)) {
          printf("[DATA]: ERROR, unable to forward data packet");
          return;
        }
        // else forward the packet to the parent
        unicast_send(&conn->uc, &conn->parent);
      }
    } else{
      struct collect_header hdr;

      memcpy(&hdr, packetbuf_dataptr(), sizeof(hdr));

      hdr.hops = hdr.hops + 1;

      if (conn->is_sink) {
        if (packetbuf_hdrreduce(sizeof(hdr))){
          topology_set(hdr.report.source, hdr.report.parent);
          conn->callbacks->recv(&hdr.report.source, &hdr.report.parent, hdr.hops); // Call the sink recv callback function
        }
        else
          printf("[DATA]: ERROR, the header could not be reduced!");
      }
      else {/* Non-sink node acting as a forwarder. Send the received packet to the node's parent in unicast */
        if (linkaddr_cmp(&conn->parent, &linkaddr_null)) {
          printf("[DATA]: ERROR, unable to forward data packet -- source: %02x:%02x", hdr.report.source.u8[0], hdr.report.source.u8[1]);
          return;
        }
        memcpy(packetbuf_dataptr(), &hdr, sizeof(hdr)); // Update the my-collect header in the packet buffer
        unicast_send(&conn->uc, &conn->parent); // Send the updated message to the node's parent in unicast
      }
    }
  }
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

// allocates the space for an addr, then copy it inside the packetbuf header
bool packetbuf_hdrcopy_linkaddr(linkaddr_t addr){
  if (!packetbuf_hdralloc(sizeof(addr))) return false;
  memcpy(packetbuf_hdrptr(), &addr, sizeof(addr));
  return true;
}

bool packetbuf_hdrcontains(linkaddr_t addr){
  linkaddr_t* route = (linkaddr_t*) packetbuf_hdrptr();
  int i = 0;
  printf("Loop check\n");
  while(!linkaddr_cmp(&route[i], &linkaddr_null)){
    if(linkaddr_cmp(&route[i], &addr)) return true;
    i++;
  }
  return false;
}

void packetbuf_hdrprint(){
  linkaddr_t* route = (linkaddr_t*) packetbuf_hdrptr();
  int i = 0;
  linkaddr_t hops = route[i];
  i++;
  printf("Route length: %d, nodes: [", hops.u8[0]);
  while(!linkaddr_cmp(&route[i], &linkaddr_null)){
    printf("%02x:%02x -> ", route[i].u8[0], route[i].u8[1]);
    i++;
  }
  printf("]\n");
}

/*TOPOLOGY---------------------------------------------------------------------------*/
int topology_set(linkaddr_t node, linkaddr_t parent){
  if(topology == NULL) return -1;
  int i;
  // search for node in topology
  for(i=0;i<topology_size;i++){
    // if found, update the parent
    if(linkaddr_cmp(&topology[i].source, &node)){
      linkaddr_copy(&topology[i].parent, &parent);
      topology_print();
      return 0;
    }
  }
  // otherwise, if the space is available, add a new entry and increment the topology size
  if(topology_size == MAX_NODES) return -2;
  linkaddr_copy(&topology[topology_size].source, &node);
  linkaddr_copy(&topology[topology_size].parent, &parent);
  topology_size++;
  return 0;
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
  if(topology != NULL) return;
  topology = (struct topology_report*) malloc(MAX_NODES * sizeof(struct topology_report));
  topology_size = 0;
}

void topology_print(){
  if(topology == NULL) return;
  printf("topology: \n");
  int i;
  for(i=0;i<topology_size;i++){
    printf("[node: %02x:%02x, parent: %02x:%02x]\n", topology[i].source.u8[0], topology[i].source.u8[1], topology[i].parent.u8[0], topology[i].parent.u8[1]);
  }
}
