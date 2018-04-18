#include "rmw/rmw.h"

#include <stdlib.h>

#include "app.h"

#include <ndn-cxx/face.hpp>

#include <boost/chrono.hpp>
using namespace boost::chrono;

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...)

class Subscriber;
bool can_take(Subscriber* sub);

rmw_ret_t
rmw_wait(
    rmw_subscriptions_t * subscriptions,
    rmw_guard_conditions_t * guard_conditions,
    rmw_services_t * services,
    rmw_clients_t * clients,
    rmw_wait_set_t * waitset,
    const rmw_time_t * wait_timeout)
{
  (void) subscriptions;
  (void) guard_conditions;
  (void) services;
  (void) clients;
  (void) waitset;
  (void) wait_timeout;
  DEBUG("rmw_wait" "\n");

  const auto begin = system_clock::now();
  
  ndn::time::milliseconds timeout = ndn::time::milliseconds(0);
  if(wait_timeout) {
    timeout = ndn::time::milliseconds(wait_timeout->nsec/1000000 + wait_timeout->sec*1000);
    DEBUG("wait %i ms\n", timeout);
  }
  
  const auto end = begin + timeout;

  do {
    if(timeout == ndn::time::milliseconds(0)) {
      face.processEvents(ndn::time::milliseconds(-1));
    }
    else {
      face.processEvents(timeout);
    }

    bool stop = false;

    for(size_t i = 0 ; i < subscriptions->subscriber_count ; i++) {
      Subscriber* sub = (Subscriber*)subscriptions->subscribers[i];
      if(can_take(sub)) {
        DEBUG("Can take !\n");
        stop = true;
      }
    }

    if(stop) {
      for(size_t i = 0 ; i < subscriptions->subscriber_count ; i++) {
        Subscriber* sub = (Subscriber*)subscriptions->subscribers[i];
        if(!can_take(sub)) {
          subscriptions->subscribers[i] = NULL;
        }
        return RMW_RET_OK;
      }
      return RMW_RET_OK;
    }
  } while(system_clock::now() < end);

  return RMW_RET_TIMEOUT;
}
