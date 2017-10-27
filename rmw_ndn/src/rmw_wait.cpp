#include "rmw/rmw.h"

#include <stdlib.h>

//#include <xtimer.h>

#include "app.hpp"
#include "sub.hpp"

/*
#define ENABLE_DEBUG 0
#include <debug.h>
*/
#include <stdio.h>
#define DEBUG(...) printf(__VA_ARGS__)

using Sub = rmw::ndn::Subscription;

rmw_ret_t
rmw_wait(
    rmw_subscriptions_t * subscriptions,
    rmw_guard_conditions_t * guard_conditions,
    rmw_services_t * services,
    rmw_clients_t * clients,
    rmw_waitset_t * waitset,
    const rmw_time_t * wait_timeout)
{
  (void) subscriptions;
  (void) guard_conditions;
  (void) services;
  (void) clients;
  (void) waitset;
  (void) wait_timeout;
  DEBUG("rmw_wait" "\n");
  /*
  const uint32_t begin = xtimer_now_usec();
  
  uint32_t timeout = 0;
  if(wait_timeout) {
    timeout = wait_timeout->nsec/1000 + wait_timeout->sec*1000000;
  }
  
  const uint32_t end = begin + timeout;

  do {
    thread_yield();
    rmw::ndn::Application::update();

    bool stop = false;

    for(size_t i = 0 ; i < subscriptions->subscriber_count ; i++) {
      Sub* sub = (Sub*)subscriptions->subscribers[i];
      if(sub->can_take()) {
        DEBUG("[%i] => %p can take !\n", (int)i, subscriptions->subscribers[i]);
        stop = true;
      }
    }

    if(stop) {
      for(size_t i = 0 ; i < subscriptions->subscriber_count ; i++) {
        Sub* sub = (Sub*)subscriptions->subscribers[i];
        if(!(sub->can_take())) {
          subscriptions->subscribers[i] = NULL;
        }
      }
      return RMW_RET_OK;
    }

  } while(xtimer_now_usec() < end);
  */
  return RMW_RET_TIMEOUT;
}
