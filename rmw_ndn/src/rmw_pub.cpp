#include "rmw/rmw.h"

//#include "rosidl_typesupport_test/message_introspection.h"

#include <stdlib.h>
#include <string.h>

#include "pub.hpp"

/*
#define ENABLE_DEBUG 0
#include <debug.h>
*/
#include <stdio.h>
#define DEBUG(...) printf(__VA_ARGS__)

using Pub = rmw::ndn::Publisher;

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
  //rosidl_typesupport_test__MessageMembers* tsdata = (rosidl_typesupport_test__MessageMembers*)type_support->data;

  rmw_publisher_t * ret = (rmw_publisher_t *)malloc(sizeof(rmw_publisher_t));
  ret->implementation_identifier = rmw_get_implementation_identifier();
  ret->topic_name = topic_name;

  //Pub* pub = new Pub(topic_name, tsdata->serialize_);
  //ret->data = (void*)pub;

  return ret;
}

rmw_ret_t
rmw_destroy_publisher(rmw_node_t * node, rmw_publisher_t * publisher)
{
  (void) node;
  DEBUG("rmw_destroy_publisher" "\n");
  /*
  delete (Pub*)publisher->data;
  free(publisher);
  */
  return RMW_RET_OK;
}

rmw_ret_t
rmw_publish(const rmw_publisher_t * publisher, const void * ros_message)
{
  (void) publisher;
  (void) ros_message;
  DEBUG("rmw_publish" "\n");
  /*
  std_msgs__msg__String* msg = (std_msgs__msg__String*)ros_message;
  DEBUG("msg: %s\n", msg->data.data);

  Pub* pub = (Pub*)publisher->data;
  pub->push_data(ros_message);
  */
  return RMW_RET_OK;
}
