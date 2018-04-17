#include "rmw/rmw.h"

#include "rosidl_typesupport_cbor/message_introspection.h"

#include <stdlib.h>
#include <string.h>

#include <queue>

#include "app.h"

#define DEBUG(...) printf(__VA_ARGS__)

#define WINDOW (10)

class Publisher
{
public:
  typedef size_t (*serialize_func_t)(const void* ros_message, char* buffer, size_t buffer_size);

private:
  ndn::Name _topic_name;
  uint64_t seq_num;
  serialize_func_t _serialize;
  std::vector<ndn::Data*> queue;

public:
  Publisher(const char* topic_name, serialize_func_t serialize)
    : _topic_name(topic_name)
    , seq_num(0)
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
  }

public:
  void push(const void* msg) {
    DEBUG("test\n");
    char* data = (char*)malloc(512);
    size_t size = 0;

    DEBUG("test %p\n");
    size = _serialize(msg, data, 512);
    DEBUG("test\n");
    data = (char*)realloc(data, size);
    DEBUG("test\n");

    ndn::Data* data_msg = new ndn::Data();
    ndn::Name name = _topic_name;
    name.appendNumber(seq_num);
    data_msg->setName(name);
    data_msg->setContent((const uint8_t *)data, size);
    DEBUG("test\n");

    std::cout << *data_msg << std::endl;

    queue.push_back(data_msg);
    seq_num++;

    if(queue.size() > WINDOW) {
      queue.erase(queue.begin());
    }
  }

private:
  void onInterest(const ndn::Interest& interest) {
    ndn::Name name = interest.getName();
    std::cout << "Publisher::onInterest " << interest << std::endl;

    if(queue.empty()) {
      DEBUG("Publisher::onInterest SKIP : no data\n");
      return;
    }

    if(!_topic_name.isPrefixOf(name)) {
      DEBUG("Publisher::onInterest ERROR : unmatched prefix\n");
      return;
    }

    if(name[_topic_name.size()] == ndn::name::Component("sync")) {
      DEBUG("SYNC\n");

      ndn::Data* data = new ndn::Data();
      static const std::string content = "test";
      name.appendNumber(seq_num);

      data->setName(name);
      data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());
      data->setFreshnessPeriod(ndn::time::seconds(0));

      static ndn::KeyChain key;
      key.sign(*data);

      std::cout << *data << std::endl;
      face.put(*data);
    }
    else {
      DEBUG("DATA\n");

      ndn::name::Component req_seq_num_comp = name[_topic_name.size()];

      if(!req_seq_num_comp.isNumber()) {
        DEBUG("Publisher::onInterest ERROR : not a sequence number\n");
        return;
      }

      uint64_t req_seq_num = req_seq_num_comp.toNumber();

      if(req_seq_num > seq_num) {
        DEBUG("Publisher::onInterest SKIP : data %i not produced (%i)\n", (int)req_seq_num, (int)seq_num);
        return;
      }

      ndn::Data* data = new ndn::Data();
      static const std::string content = "test";

      data->setName(name);
      data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());
      data->setFreshnessPeriod(ndn::time::seconds(1));

      static ndn::KeyChain key;
      key.sign(*data);

      std::cout << *data << std::endl;
      face.put(*data);
    }
  }
};

#include <rosidl_typesupport_cbor/identifier.h>
#include <rosidl_typesupport_c/identifier.h>
#include <rosidl_typesupport_introspection_c/identifier.h>

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
    DEBUG("type support not from this implementation\n");
    return NULL;
  }
  rosidl_typesupport_cbor__MessageMembers* tsdata = (rosidl_typesupport_cbor__MessageMembers*)ts->data;

  DEBUG("%s\n", type_support->typesupport_identifier);
  DEBUG("%s\n", ts->typesupport_identifier);
  DEBUG("%s\n", rosidl_typesupport_cbor__identifier);
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
