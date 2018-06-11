/*
    rmw_ndn
    Copyright (C) 2018 INRIA

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "rmw/rmw.h"

#include <rosidl_typesupport_cbor/message_introspection.h>
#include <rosidl_typesupport_cbor_cpp/identifier.hpp>
#include <rosidl_typesupport_cbor/identifier.h>

#include <stdlib.h>
#include <string.h>

#include <random>

#include "app.h"

#include "discovery.hpp"
#include "sync.hpp"
#include "topic.hpp"

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>

#include <iostream>
#include <string>

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...)

class Subscriber
{
public:
  typedef size_t (*deserialize_func_t)(void* ros_message, const char* buffer, size_t buffer_size);

private:
  ndn::Name _topic_name;
  deserialize_func_t _deserialize;
  std::vector<ndn::Data> _queue;
  DiscoveryHeartbeatEmiter _heartbeat;

public:
  explicit
  Subscriber(const std::string& topic_name, deserialize_func_t deserialize)
    : _topic_name(ndn::Name(topic_name))
    , _deserialize(deserialize)
    , _heartbeat("subscriber", topic_name)
  {
    DEBUG("Subscriber::topic_name = %s\n", topic_name.c_str());
    auto pubs = DiscoveryClient::instance().getDiscoveredPublishers();
    for(auto it = pubs[_topic_name.toUri()].begin() ; it != pubs[_topic_name.toUri()].end() ; it++) {
      reqSync(*it);
    }
    DiscoveryClient::instance()._pubs_cb[_topic_name.toUri()].push_back(std::bind(&Subscriber::reqSync, this, _1));
  }

private:
  void reqSync(uint64_t id) {
    requestSync(_topic_name.toUri(), id,
                std::bind(&Subscriber::onData, this, _1, _2, _3),
                std::bind(&Subscriber::onError, this, _1)
                );
  }

  void reqData(uint64_t id, uint64_t seq_num) {
    requestData(_topic_name.toUri(), id, seq_num,
                std::bind(&Subscriber::onData, this, _1, _2, _3),
                std::bind(&Subscriber::onError, this, _1)
                );
  }

private:
  void onError(uint64_t id) {
    DEBUG("Subscriber::onError %s %i\n", _topic_name.toUri().c_str(), (int)id);
    reqSync(id);
  }

  void onData(uint64_t id, uint64_t seq_num, const ndn::Data& data) {
    ndn::Name name = data.getName();
    DEBUG("Subscriber::onData %s\n", name.toUri().c_str());
    _queue.push_back(data);
    reqData(id, seq_num+1);
  }

public:
  bool can_take(void) {
    return !_queue.empty();
  }

  bool take(void* msg) {
    if(_queue.empty()) {
      return false;
    }

    ndn::Block data = _queue.front().getContent();
    _queue.erase(_queue.begin());
    return data.value_size() == _deserialize(msg, (const char*)data.value(), data.value_size());
  }
};

bool can_take(Subscriber* sub) {
  return sub->can_take();
}

rmw_subscription_t *
rmw_create_subscription(
    const rmw_node_t * node,
    const rosidl_message_type_support_t * type_support,
    const char * topic_name,
    const rmw_qos_profile_t * qos_policies,
    bool ignore_local_publications)
{
  (void) node;
  (void) type_support;
  (void) qos_policies;
  (void) ignore_local_publications;
  DEBUG("rmw_create_subscription" "\n");

  rmw_subscription_t * ret = (rmw_subscription_t *)malloc(sizeof(rmw_subscription_t));
  ret->implementation_identifier = rmw_get_implementation_identifier();
  ret->topic_name = topic_name;

  const rosidl_message_type_support_t * ts = get_message_typesupport_handle(type_support, rosidl_typesupport_cbor__identifier);
  if (!ts) {
    ts = get_message_typesupport_handle(type_support, rosidl_typesupport_cbor_cpp::typesupport_identifier);
    if (!ts) {
      DEBUG("type support not from this implementation\n");
      return NULL;
    }
  }
  rosidl_typesupport_cbor__MessageMembers* tsdata = (rosidl_typesupport_cbor__MessageMembers*)ts->data;

  Subscriber* sub = new Subscriber(topic_name, tsdata->deserialize_);
  ret->data = (void*)sub;

  return ret;
}

rmw_ret_t
rmw_destroy_subscription(rmw_node_t * node, rmw_subscription_t * subscription)
{
  (void) node;
  DEBUG("rmw_destroy_subscription" "\n");
  delete (Subscriber*)subscription->data;
  free(subscription);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_take(const rmw_subscription_t * subscription, void * ros_message, bool * taken)
{
  (void) subscription;
  (void) ros_message;
  (void) taken;
  DEBUG("rmw_take" "\n");
  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_with_info(
    const rmw_subscription_t * subscription,
    void * ros_message,
    bool * taken,
    rmw_message_info_t * message_info)
{
  (void) message_info;
  DEBUG("rmw_take_with_info" "\n");

  Subscriber* sub = (Subscriber*)subscription->data;
  *taken = sub->take(ros_message);

  return RMW_RET_OK;
}
