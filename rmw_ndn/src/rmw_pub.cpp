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
  Publisher(const char* topic_name)
    : topic_name(topic_name)
    , seq_num(0)
  {
    face.setInterestFilter(this->topic_name,
                           std::bind(&Publisher::onInterest, this, _2),
                           std::bind([this] {
      DEBUG("Publisher::Publisher Prefix '%s' registered\n", this->topic_name.toUri().c_str());
    }),
                           [] (const ndn::Name& prefix, const std::string& reason) {
      DEBUG("Publisher::Publisher Failed to register prefix '%s' (%s)\n", prefix.toUri().c_str(), reason.c_str());
    });
  }

public:
  std::vector<const void*> queue;

  void push(const void* msg) {
    queue.push_back(msg);
    seq_num++;

    if(queue.size() > WINDOW) {
      queue.erase(queue.begin());
    }
  }

private:
  void onInterest(const ndn::Interest& interest) {
    ndn::Name name = interest.getName();
    std::cout << "Publisher::onInterest " << interest << std::endl;

    if(!topic_name.isPrefixOf(name)) {
      DEBUG("Publisher::onInterest ERROR : unmatched prefix\n");
      return;
    }

    if(name[topic_name.size()] == ndn::name::Component("sync")) {
      std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>();

      static const std::string content = "looltest";

      data->setName(name);
      data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());
      data->setFreshnessPeriod(ndn::time::seconds(10));

      static ndn::KeyChain key;
      key.sign(*data);

      std::cout << *data << std::endl;
      face.put(*data);
    }
    else {
      DEBUG("DATA\n");
    }
  }

private:
  ndn::Name topic_name;
  uint64_t seq_num;
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
  rosidl_typesupport_cbor__MessageMembers* tsdata = (rosidl_typesupport_cbor__MessageMembers*)type_support->data;

  rmw_publisher_t * ret = (rmw_publisher_t *)malloc(sizeof(rmw_publisher_t));
  ret->implementation_identifier = rmw_get_implementation_identifier();
  ret->topic_name = topic_name;

  Publisher* pub = new Publisher(topic_name);
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
