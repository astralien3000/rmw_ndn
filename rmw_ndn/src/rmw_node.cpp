#include "rmw/rmw.h"
#include "rmw/error_handling.h"
#include "rmw/convert_rcutils_ret_to_rmw_ret.h"
#include "rmw/get_topic_names_and_types.h"

#include <rcutils/logging_macros.h>
#include <rcutils/strdup.h>

#include <string>

#include <stdlib.h>

#include "app.h"

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...)

class Discovery
{
private:
  ndn::Name _node_name;
  std::map<std::string, int> _discovered_nodes;
  std::map<std::string, int> _discovered_topics;

public:
  Discovery(const char* name)
    : _node_name(name)
  {
    face.setInterestFilter(discovery_prefix,
                           std::bind(&Discovery::onInterest, this, _2),
                           std::bind([this] {
      DEBUG("Discovery::Discovery Prefix '%s' registered\n", discovery_prefix.toUri().c_str());
    }),
                           [] (const ndn::Name& prefix, const std::string& reason) {
      DEBUG("Discovery::Discovery Failed to register prefix '%s' (%s)\n", prefix.toUri().c_str(), reason.c_str());
    });

    heartbeat();
  }

public:
  std::vector<std::string> getDiscoveredNames(void) {
    std::vector<std::string> ret;
    for(auto it = _discovered_nodes.begin() ; it != _discovered_nodes.end() ; it++) {
      ret.push_back(it->first);
    }
    return ret;
  }

  std::map<std::string, std::set<std::string>> getDiscoveredTopics(void) {
    std::map<std::string, std::set<std::string>> ret;
    for(auto it = _discovered_topics.begin() ; it != _discovered_topics.end() ; it++) {
      ret[it->first] = std::set<std::string>();
    }
    return ret;
  }

private:
  inline void send(void) {
    ndn::Interest interest;
    ndn::Name interest_name = discovery_prefix;
    interest_name.append("node").append(_node_name);
    interest.setName(interest_name);
    interest.setMustBeFresh(true);
    interest.setInterestLifetime(ndn::time::seconds(1));
    face.expressInterest(interest,
                         std::bind([](const ndn::Data&) {}, _2),
                         std::bind([](const ndn::Interest&) {}, _1),
                         std::bind([](const ndn::Interest&) {}, _1));
  }

  inline void clean(void) {
    std::vector<decltype(_discovered_nodes)::iterator> erase_list;
    for(auto it = _discovered_nodes.begin() ; it != _discovered_nodes.end() ; it++) {
      if(it->second) {
        it->second--;
      }
      else {
        erase_list.push_back(it);
      }
    }
    for(auto it = erase_list.begin() ; it != erase_list.end() ; it++) {
      _discovered_nodes.erase(*it);
    }
  }

  void heartbeat(void) {
    scheduler.scheduleEvent(ndn::time::seconds(1), std::bind(&Discovery::heartbeat, this));
    send();
    //clean();
  }

private:
  void onInterest(const ndn::Interest& interest) {
    ndn::Name name = interest.getName();
    DEBUG("Discovery::onInterest %s\n", name.toUri().c_str());
    if(name[2] == ndn::name::Component("node")) {
      DEBUG("NODE\n");
      _discovered_nodes[name.getSubName(3).toUri()] = 5;
    }
    if(name[2] == ndn::name::Component("topic")) {
      DEBUG("TOPIC\n");
      _discovered_topics[name.getSubName(3).toUri()] = 5;
    }
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
  node->data = (void*)new Discovery(name);

  const size_t namelen = strlen(name)+1;
  node->name = (const char*)malloc(namelen);
  memcpy((char*)node->name, name, namelen);

  const size_t nslen = strlen(namespace_)+1;
  node->namespace_ = (const char*)malloc(nslen);
  memcpy((char*)node->namespace_, namespace_, nslen);

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
  auto discov = (Discovery *)node->data;
  auto cpp_node_names = discov->getDiscoveredNames();

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
  auto discov = (Discovery *)node->data;
  std::map<std::string, std::set<std::string>> topics = discov->getDiscoveredTopics();

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
