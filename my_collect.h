#ifndef __MY_COLLECT_H__
#define __MY_COLLECT_H__
/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include "contiki.h"
#include "net/rime/rime.h"
#include "net/netstack.h"
#include "core/net/linkaddr.h"
#include <stdlib.h>
/*---------------------------------------------------------------------------*/
#define MAX_PATH_LENGTH 40
#define MAX_NODES 40
#define TOPOLOGY_REPORT_PERIOD (60 * CLOCK_SECOND)
#define TOPOLOGY_REPORT_DELAY (10 * CLOCK_SECOND)
#define TOPOLOGY_REPORT_ENABLED 1
// New Beacon New Report
#define NBNR_ENABLED 1
// New Parent New Report
#define NPNR_ENABLED 0
#define BEACON_INTERVAL (60 * CLOCK_SECOND) 
#define BEACON_FORWARD_DELAY (random_rand() % CLOCK_SECOND)
#define RSSI_THRESHOLD -95
/*---------------------------------------------------------------------------*/
int topology_set(linkaddr_t node, linkaddr_t parent);
linkaddr_t topology_get(linkaddr_t node);
void topology_print();
bool packetbuf_hdrcopy_linkaddr(linkaddr_t addr);
bool packetbuf_hdrcontains(linkaddr_t addr);
void packetbuf_hdrprint();
int random_int(int max);
/*---------------------------------------------------------------------------*/
/* Connection object */
struct my_collect_conn {
  struct broadcast_conn bc;
  struct unicast_conn uc;
  const struct my_collect_callbacks* callbacks;
  linkaddr_t parent;
  struct ctimer beacon_timer;
  uint16_t metric;
  uint16_t beacon_seqn;
  bool is_sink;
  struct ctimer report_timer;
};
/*---------------------------------------------------------------------------*/
/* Callback structure */
struct my_collect_callbacks {
  void (*recv)(const linkaddr_t *originator, const linkaddr_t *parent, uint8_t hops);
  void (*sr_recv)(struct my_collect_conn *ptr, uint8_t hops);
  void (*report_recv)(struct my_collect_conn *ptr);
};
/*---------------------------------------------------------------------------*/
/* Initialize a collect connection 
 *  - conn      -- a pointer to the connection object 
 *  - channels  -- starting channel C (my_collect uses two: C and C+1)
 *  - is_sink   -- initialise in either sink or forwarder mode
 *  - callbacks -- a pointer to the callback structure
 */
void my_collect_open(
    struct my_collect_conn* conn, 
    uint16_t channels, 
    bool is_sink,
    const struct my_collect_callbacks *callbacks);
/*---------------------------------------------------------------------------*/
/* Topology report*/
struct topology_report {
  linkaddr_t source;
  linkaddr_t parent;
};
/* __attribute__((packed)); */ 
/*---------------------------------------------------------------------------*/
/* Header structure for data packets */
struct collect_header {
  struct topology_report report; 
  uint8_t hops;
};
/* __attribute__((packed)); */ 
/*---------------------------------------------------------------------------*/
/* Beacon message structure */
struct beacon_msg {
  uint16_t seqn;
  uint16_t metric;
} __attribute__((packed));
/*---------------------------------------------------------------------------*/
/* Send packet to the sink */
int my_collect_send(struct my_collect_conn *c);
/*---------------------------------------------------------------------------*/
int sr_send(struct my_collect_conn* conn, linkaddr_t* dest);
/*---------------------------------------------------------------------------*/
#endif /* __MY_COLLECT_H__ */
