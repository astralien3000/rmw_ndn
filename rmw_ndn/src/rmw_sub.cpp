#include "rmw/rmw.h"

#include "rosidl_typesupport_cbor/message_introspection.h"

#include <stdlib.h>
#include <string.h>

#include <random>

#include "app.h"

#define DEBUG(...) printf(__VA_ARGS__)

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>

#include <iostream>
#include <string>

class Subscriber
{
public:
  explicit
  Subscriber(const std::string& topic_name)
    : topic_name(ndn::Name(topic_name))
    , seq_num(0)
  {
    DEBUG("Subscriber::topic_name = %s\n", topic_name.c_str());
    requestSync();
  }

private:
  void requestSync()
  {
    ndn::Name name= ndn::Name(topic_name).append("sync").appendVersion();
    DEBUG("Subscriber::requestSync %s\n", name.toUri().c_str());
    face.expressInterest(ndn::Interest(name).setMustBeFresh(true),
                         std::bind(&Subscriber::onSyncData, this, _2),
                         std::bind(&Subscriber::onNack, this, _1),
                         std::bind(&Subscriber::onTimeout, this, _1));
  }

  void requestData()
  {
    ndn::Name name= ndn::Name(topic_name).appendNumber(seq_num);
    DEBUG("Subscriber::requestData %s\n", name.toUri().c_str());
    face.expressInterest(ndn::Interest(name).setMustBeFresh(true),
                         std::bind(&Subscriber::onData, this, _2),
                         std::bind(&Subscriber::onNack, this, _1),
                         std::bind(&Subscriber::onTimeout, this, _1));
  }

  void onSyncData(const ndn::Data& data)
  {
    DEBUG("Subscriber::onSyncData %s\n", data.getName().toUri().c_str());
    std::cout << data << std::endl;
    requestData();
  }

  void onData(const ndn::Data& data)
  {
    DEBUG("Subscriber::onData %s\n", data.getName().toUri().c_str());
    std::cout << data << std::endl;
    requestData();
  }

  void
  onNack(const ndn::Interest& interest) {
    DEBUG("Subscriber::onNack %s\n", interest.getName().toUri().c_str());
    //requestSync();
  }

  void
  onTimeout(const ndn::Interest& interest) {
    DEBUG("Subscriber::onTimeout %s\n", interest.getName().toUri().c_str());
    requestSync();
  }

private:
  ndn::Name topic_name;
  uint64_t seq_num;
};

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
  rosidl_typesupport_cbor__MessageMembers* tsdata = (rosidl_typesupport_cbor__MessageMembers*)type_support->data;

  rmw_subscription_t * ret = (rmw_subscription_t *)malloc(sizeof(rmw_subscription_t));
  ret->implementation_identifier = rmw_get_implementation_identifier();
  ret->topic_name = topic_name;

  Subscriber* sub = new Subscriber(topic_name);
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

  sub_t* sub = (sub_t*)subscription->data;
  *taken = false;//_sub_take(sub, ros_message);

  return RMW_RET_OK;
}
