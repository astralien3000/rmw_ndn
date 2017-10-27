#include <rmw/rmw.h>
#include <rmw/allocators.h>
#include <rmw/error_handling.h>

#include <stdlib.h>

#include "app.hpp"

/*
#define ENABLE_DEBUG 0
#include <debug.h>
*/
#include <stdio.h>
#define DEBUG(...) printf(__VA_ARGS__)

const char *
rmw_get_implementation_identifier(void)
{
  DEBUG("rmw_get_implementation_identifier" "\n");
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
  (void) name;
  (void) domain_id;
  DEBUG("rmw_create_node" "\n");
  rmw_node_t *node = (rmw_node_t *)malloc(sizeof(rmw_node_t));
  node->implementation_identifier = rmw_get_implementation_identifier();
  node->data = NULL;

  node->name =
    static_cast<const char *>(rmw_allocate(sizeof(char) * strlen(name) + 1));
  if (!node->name) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
    node->namespace_ = nullptr;  // to avoid free on uninitialized memory
  }
  memcpy(const_cast<char *>(node->name), name, strlen(name) + 1);

  node->namespace_ =
    static_cast<const char *>(rmw_allocate(sizeof(char) * strlen(namespace_) + 1));
  if (!node->namespace_) {
    RMW_SET_ERROR_MSG("failed to allocate memory");
  }
  memcpy(const_cast<char *>(node->namespace_), namespace_, strlen(namespace_) + 1);

  //rmw::ndn::Application::create();

  return node;
}

rmw_ret_t
rmw_destroy_node(rmw_node_t * node)
{
  DEBUG("rmw_destroy_node" "\n");
  free(node);
  rmw::ndn::Application::destroy();
  return RMW_RET_OK;
}

rmw_ret_t
rmw_get_node_names(
  const rmw_node_t * node,
  rcutils_string_array_t * node_names)
{
  DEBUG("rmw_get_node_names" "\n");
  return RMW_RET_OK;
}
