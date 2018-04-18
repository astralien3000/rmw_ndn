#include "rmw/rmw.h"

#include <rosidl_typesupport_cbor/message_introspection.h>
#include <rosidl_typesupport_cbor_cpp/identifier.hpp>
#include <rosidl_typesupport_cbor/identifier.h>

#include <stdlib.h>
#include <string.h>

#include <random>

#include "app.h"

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
  uint64_t _seq_num;
  deserialize_func_t _deserialize;
  std::vector<ndn::Data> _queue;

public:
  explicit
  Subscriber(const std::string& topic_name, deserialize_func_t deserialize)
    : _topic_name(ndn::Name(topic_name))
    , _seq_num(0)
    , _deserialize(deserialize)
  {
    DEBUG("Subscriber::topic_name = %s\n", topic_name.c_str());
    requestSync();
  }

private:
  void requestSync()
  {
    ndn::Name name= ndn::Name(_topic_name).append("sync").appendVersion();
    DEBUG("Subscriber::requestSync %s\n", name.toUri().c_str());
    face.expressInterest(ndn::Interest(name).setMustBeFresh(true),
                         std::bind(&Subscriber::onSyncData, this, _2),
                         std::bind(&Subscriber::onNack, this, _1),
                         std::bind(&Subscriber::onTimeout, this, _1));
  }

  void requestData()
  {
    ndn::Name name= ndn::Name(_topic_name).appendNumber(_seq_num);
    DEBUG("Subscriber::requestData %s\n", name.toUri().c_str());
    face.expressInterest(ndn::Interest(name).setMustBeFresh(false),
                         std::bind(&Subscriber::onData, this, _2),
                         std::bind(&Subscriber::onNack, this, _1),
                         std::bind(&Subscriber::onTimeout, this, _1));
  }

  void onSyncData(const ndn::Data& data)
  {
    ndn::Name name = data.getName();
    DEBUG("Subscriber::onSyncData %s\n", name.toUri().c_str());
    ndn::name::Component seq_num_comp = *name.rbegin();
    if(seq_num_comp.isNumber()) {
      _seq_num = seq_num_comp.toNumber();
    }
    requestData();
  }

  void onData(const ndn::Data& data)
  {
    ndn::Name name = data.getName();
    DEBUG("Subscriber::onData %s\n", name.toUri().c_str());
    _queue.push_back(data);
    _seq_num++;
    requestData();
  }

  void onNack(const ndn::Interest& interest) {
    DEBUG("Subscriber::onNack %s\n", interest.getName().toUri().c_str());
    scheduler.scheduleEvent(ndn::time::seconds(1), [this] { requestSync(); });
  }

  void onTimeout(const ndn::Interest& interest) {
    DEBUG("Subscriber::onTimeout %s\n", interest.getName().toUri().c_str());
    requestSync();
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
