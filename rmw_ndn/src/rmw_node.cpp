#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/get_topic_names_and_types.h"

#include <rcutils/logging_macros.h>
#include <rcutils/strdup.h>

#include <string>

#include <stdlib.h>

#include "app.h"

#include "discovery.hpp"

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...)

class Node
{
private:
  DiscoveryHeartbeatEmiter _heartbeat;

public:
  Node(const char* name)
    : _heartbeat("node", name)
  {
  }
};

const char *
rmw_get_implementation_identifier(void)
{
  //DEBUG("rmw_get_implementation_identifier" "\n");
  static const char * fake_impl_id = "rmw_ndn";
  return fake_impl_id;
}

rmw_ret_t
rmw_init(void)
{
  DEBUG("rmw_init" "\n");
  return RMW_RET_OK;
}

rmw_node_t *
rmw_create_node(
    const char * name,
    const char * namespace_,
    size_t domain_id,
    const rmw_node_security_options_t * security_options)
{
  (void) domain_id;
  (void) security_options;
  DEBUG("rmw_create_node" "\n");
  rmw_node_t *node = (rmw_node_t *)malloc(sizeof(rmw_node_t));
  node->implementation_identifier = rmw_get_implementation_identifier();
  node->data = (void*)new Node(name);

  const size_t namelen = strlen(name)+1;
  node->name = (const char*)malloc(namelen);
  memcpy((char*)node->name, name, namelen);

  const size_t nslen = strlen(namespace_)+1;
  node->namespace_ = (const char*)malloc(nslen);
  memcpy((char*)node->namespace_, namespace_, nslen);

  DiscoveryClient::instance();

  return node;
}

rmw_ret_t
rmw_destroy_node(rmw_node_t * node)
{
  DEBUG("rmw_destroy_node" "\n");
  free(node);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_get_node_names(
    const rmw_node_t * node,
    rcutils_string_array_t * node_names) {
  DEBUG("rmw_get_node_names" "\n");
  auto cpp_node_names = DiscoveryClient::instance().getDiscoveredNames();

  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  rcutils_ret_t rcutils_ret =
      rcutils_string_array_init(node_names, cpp_node_names.size(), &allocator);
  if (rcutils_ret != RCUTILS_RET_OK) {
    RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
        return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
  }
  for (size_t i = 0 ; i < cpp_node_names.size() ; ++i) {
    node_names->data[i] = rcutils_strdup(cpp_node_names[i].c_str(), allocator);
    if (!node_names->data[i]) {
      RMW_SET_ERROR_MSG("failed to allocate memory for node name")
          rcutils_ret = rcutils_string_array_fini(node_names);
      if (rcutils_ret != RCUTILS_RET_OK) {
        RCUTILS_LOG_ERROR_NAMED(
              "rmw_ndn",
              "failed to cleanup during error handling: %s", rcutils_get_error_string_safe())
      }
      return RMW_RET_BAD_ALLOC;
    }
  }
  return RMW_RET_OK;
}

rmw_ret_t
rmw_get_topic_names_and_types(
    const rmw_node_t * node,
    rcutils_allocator_t * allocator,
    bool no_demangle,
    rmw_names_and_types_t * topic_names_and_types) {
  DEBUG("rmw_get_topic_names_and_types" "\n");
  std::map<std::string, std::set<std::string>> topics = DiscoveryClient::instance().getDiscoveredTopics();

  // Copy data to results handle
  if (topics.size() > 0) {
    // Setup string array to store names
    rmw_ret_t rmw_ret = rmw_names_and_types_init(topic_names_and_types, topics.size(), allocator);
    if (rmw_ret != RMW_RET_OK) {
      return rmw_ret;
    }
    // Setup cleanup function, in case of failure below
    auto fail_cleanup = [&topic_names_and_types]() {
      rmw_ret_t rmw_ret = rmw_names_and_types_fini(topic_names_and_types);
      if (rmw_ret != RMW_RET_OK) {
        RCUTILS_LOG_ERROR_NAMED(
              "rmw_ndn",
              "error during report of error: %s", rmw_get_error_string_safe())
      }
    };
    // For each topic, store the name, initialize the string array for types, and store all types
    size_t index = 0;
    for (const auto & topic_n_types : topics) {
      // Duplicate and store the topic_name
      char * topic_name = rcutils_strdup(topic_n_types.first.c_str(), *allocator);
      if (!topic_name) {
        RMW_SET_ERROR_MSG_ALLOC("failed to allocate memory for topic name", *allocator);
        fail_cleanup();
        return RMW_RET_BAD_ALLOC;
      }
      topic_names_and_types->names.data[index] = topic_name;
      // Setup storage for types
      {
        rcutils_ret_t rcutils_ret = rcutils_string_array_init(
                                      &topic_names_and_types->types[index],
                                      topic_n_types.second.size(),
                                      allocator);
        if (rcutils_ret != RCUTILS_RET_OK) {
          RMW_SET_ERROR_MSG(rcutils_get_error_string_safe())
              fail_cleanup();
          return rmw_convert_rcutils_ret_to_rmw_ret(rcutils_ret);
        }
      }
      // Duplicate and store each type for the topic
      size_t type_index = 0;
      for (const auto & type : topic_n_types.second) {
        char * type_name = rcutils_strdup(type.c_str(), *allocator);
        if (!type_name) {
          RMW_SET_ERROR_MSG_ALLOC("failed to allocate memory for type name", *allocator)
              fail_cleanup();
          return RMW_RET_BAD_ALLOC;
        }
        topic_names_and_types->types[index].data[type_index] = type_name;
        ++type_index;
      }  // for each type
      ++index;
    }  // for each topic
  }
  return RMW_RET_OK;
}
