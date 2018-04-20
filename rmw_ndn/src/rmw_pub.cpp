#include "rmw/rmw.h"

#include <rosidl_typesupport_cbor/message_introspection.h>
#include <rosidl_typesupport_cbor_cpp/identifier.hpp>
#include <rosidl_typesupport_cbor/identifier.h>

#include <stdlib.h>
#include <string.h>

#include <queue>
#include <iostream>

#include "app.h"

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...)

#define WINDOW (10)

class Publisher
{
public:
  typedef size_t (*serialize_func_t)(const void* ros_message, char* buffer, size_t buffer_size);

private:
  ndn::Name _topic_name;
  uint64_t _seq_num;
  uint64_t _req_seq_num;
  serialize_func_t _serialize;
  std::vector<ndn::Data> _queue;

public:
  Publisher(const char* topic_name, serialize_func_t serialize)
    : _topic_name(topic_name)
    , _seq_num(0)
    , _req_seq_num(0)
    , _serialize(serialize)
  {
    face.setInterestFilter(_topic_name,
                           std::bind(&Publisher::onInterest, this, _2),
                           std::bind([this] {
      DEBUG("Publisher::Publisher Prefix '%s' registered\n", _topic_name.toUri().c_str());
    }),
                           [] (const ndn::Name& prefix, const std::string& reason) {
      DEBUG("Publisher::Publisher Failed to register prefix '%s' (%s)\n", prefix.toUri().c_str(), reason.c_str());
    });

    heartbeat();
  }

private:
  void heartbeat(void) {
    scheduler.scheduleEvent(ndn::time::seconds(1), std::bind(&Publisher::heartbeat, this));
    ndn::Interest interest;
    ndn::Name interest_name = discovery_prefix;
    interest_name.append("topic").append(_topic_name);
    interest.setName(interest_name);
    interest.setMustBeFresh(true);
    interest.setInterestLifetime(ndn::time::seconds(1));
    face.expressInterest(interest,
                         std::bind([](const ndn::Data&) {}, _2),
                         std::bind([](const ndn::Interest&) {}, _1),
                         std::bind([](const ndn::Interest&) {}, _1));
  }

public:
  void push(const void* msg) {
    char* data = (char*)malloc(512);
    size_t size = 0;

    size = _serialize(msg, data, 512);
    data = (char*)realloc(data, size);

    ndn::Data data_msg;
    ndn::Name name = _topic_name;
    std::ostringstream os;
    os << _seq_num;
    name.append((const uint8_t*)os.str().c_str(), os.str().size());
    data_msg.setName(name);
    data_msg.setContent((const uint8_t *)data, size);

    ndn::KeyChain key;
    key.sign(data_msg);

    _queue.push_back(data_msg);

    if(_req_seq_num >= _seq_num) {
      face.put(data_msg);
    }

    _seq_num++;

    if(_queue.size() > WINDOW) {
      _queue.erase(_queue.begin());
    }
  }

private:
  void onInterest(const ndn::Interest& interest) {
    ndn::Name name = interest.getName();
    DEBUG("Publisher::onInterest %s\n", name.toUri().c_str());

    if(_queue.empty()) {
      DEBUG("Publisher::onInterest SKIP : no data\n");
      return;
    }

    if(!_topic_name.isPrefixOf(name)) {
      DEBUG("Publisher::onInterest ERROR : unmatched prefix\n");
      return;
    }

    if(name[_topic_name.size()] == ndn::name::Component("sync")) {
      DEBUG("SYNC\n");

      ndn::Data data;
      std::ostringstream os;
      os << _seq_num;
      name.append((const uint8_t*)os.str().c_str(), os.str().size());

      data.setName(name);
      data.setContent(_queue.back().getContent());
      data.setFreshnessPeriod(ndn::time::seconds(0));

      ndn::KeyChain key;
      key.sign(data);

      face.put(data);
    }
    else {
      DEBUG("DATA\n");

      ndn::name::Component req_seq_num_comp = name[_topic_name.size()];
      std::string req_seq_num_str = req_seq_num_comp.toUri();
      uint64_t req_seq_num = std::stoull(req_seq_num_str);

      if(req_seq_num >= _seq_num) {
        DEBUG("Publisher::onInterest SKIP : data %i not produced (%i)\n", (int)req_seq_num, (int)_seq_num);
        _req_seq_num = req_seq_num;
        return;
      }

      const uint64_t diff_seq_num = _seq_num - req_seq_num;
      if(diff_seq_num > _queue.size()) {
        DEBUG("Publisher::onInterest SKIP : data %i outdated (%i)\n", (int)req_seq_num, (int)_seq_num);
        return;
      }

      face.put(_queue[_queue.size()-diff_seq_num]);
    }
  }
};

rmw_publisher_t *
rmw_create_publisher(
    const rmw_node_t * node,
    const rosidl_message_type_support_t * type_support,
    const char * topic_name,
    const rmw_qos_profile_t * qos_policies)
{
  (void) node;
  (void) type_support;
  (void) topic_name;
  (void) qos_policies;
  DEBUG("rmw_create_publisher" "\n");

  rmw_publisher_t * ret = (rmw_publisher_t *)malloc(sizeof(rmw_publisher_t));
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

  Publisher* pub = new Publisher(topic_name, tsdata->serialize_);
  ret->data = (void*)pub;

  return ret;
}

rmw_ret_t
rmw_destroy_publisher(rmw_node_t * node, rmw_publisher_t * publisher)
{
  (void) node;
  DEBUG("rmw_destroy_publisher" "\n");
  delete (Publisher*)publisher->data;
  free(publisher);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_publish(const rmw_publisher_t * publisher, const void * ros_message)
{
  (void) publisher;
  (void) ros_message;
  DEBUG("rmw_publish" "\n");

  Publisher* pub = (Publisher*)publisher->data;
  pub->push(ros_message);

  return RMW_RET_OK;
}
