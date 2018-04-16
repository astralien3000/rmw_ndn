#include "rmw/rmw.h"

#include "rosidl_typesupport_cbor/message_introspection.h"

#include <stdlib.h>
#include <string.h>

#include "app.h"

#define DEBUG(...) printf(__VA_ARGS__)

class Publisher
{
public:
  Publisher(const char* topic_name)
    : m_baseName(topic_name)
    , m_counter(0)
  {
    face.setInterestFilter(m_baseName,
                             std::bind(&Publisher::onInterest, this, _2),
                             std::bind([] {
                                 std::cout << "Prefix registered" << std::endl;
                               }),
                             [] (const ndn::Name& prefix, const std::string& reason) {
                               std::cout << "Failed to register prefix: " << reason << std::endl;
                             });
  }

private:
  void
  onInterest(const ndn::Interest& interest)
  {
    std::cout << "<< interest for " << interest << std::endl;

    // create data packet with the same name as interest
    std::shared_ptr<ndn::Data> data = std::make_shared<ndn::Data>(interest.getName());

    // prepare and assign content of the data packet
    std::ostringstream os;
    os << "C++ LOOL LINE #" << (m_counter++) << std::endl;
    std::string content = os.str();
    data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());

    // set metainfo parameters
    data->setFreshnessPeriod(ndn::time::seconds(10));

    // sign data packet
    //m_keyChain.sign(*data);

    // make data packet available for fetching
    face.put(*data);
  }

private:
  ndn::Name m_baseName;
  uint64_t m_counter;
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

  printf("TOPIC_NAME : %s\n", ret->topic_name);

  Publisher* pub = new Publisher(topic_name);
  ret->data = (void*)pub;

  return ret;
}

rmw_ret_t
rmw_destroy_publisher(rmw_node_t * node, rmw_publisher_t * publisher)
{
  (void) node;
  DEBUG("rmw_destroy_publisher" "\n");
  //_pub_destroy((pub_t*)publisher->data);
  free(publisher);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_publish(const rmw_publisher_t * publisher, const void * ros_message)
{
  (void) publisher;
  (void) ros_message;
  DEBUG("rmw_publish" "\n");

  pub_t* pub = (pub_t*)publisher->data;
  //_pub_push_data(pub, ros_message);

  return RMW_RET_OK;
}
