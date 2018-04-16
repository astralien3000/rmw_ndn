#include "rmw/rmw.h"

#include <stdlib.h>

#include "app.h"

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...)

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
  (void) domain_id;
  (void) security_options;
  DEBUG("rmw_create_node" "\n");
  rmw_node_t *node = (rmw_node_t *)malloc(sizeof(rmw_node_t));
  node->implementation_identifier = rmw_get_implementation_identifier();
  node->data = NULL;

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
  app_destroy();
  return RMW_RET_OK;
}
