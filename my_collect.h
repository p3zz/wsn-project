#ifndef __MY_COLLECT_H__
#define __MY_COLLECT_H__
/*---------------------------------------------------------------------------*/
#include <stdbool.h>
#include "contiki.h"
#include "net/rime/rime.h"
#include "net/netstack.h"
#include "core/net/linkaddr.h"
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
};
/*---------------------------------------------------------------------------*/
/* Callback structure */
struct my_collect_callbacks {
  void (*recv)(const linkaddr_t *originator, const linkaddr_t *parent);
  void (*sr_recv)(struct my_collect_conn *ptr, uint8_t hops);
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
/* Send packet to the sink */
int my_collect_send(struct my_collect_conn *c);
/*---------------------------------------------------------------------------*/
int sr_send(struct my_collect_conn* conn, linkaddr_t* dest);
/*---------------------------------------------------------------------------*/
#endif /* __MY_COLLECT_H__ */
