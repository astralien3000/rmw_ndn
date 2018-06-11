/*
    rmw_ndn
    Copyright (C) 2018 INRIA

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "rmw/rmw.h"

#include "rmw/get_service_names_and_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <thread.h>
//#include <random.h>

#define ENABLE_DEBUG 0

#if ENABLE_DEBUG
#define DPUTS(str) puts(str)
#define DPRINTF(...) printf(__VA_ARGS__)
#define DPRINT(...) printf(__VA_ARGS__)
#else
#define DPUTS(str)
#define DPRINTF(...)
#define DPRINT(...)
#endif

rmw_client_t *
rmw_create_client(
  const rmw_node_t * node,
  const rosidl_service_type_support_t * type_support,
  const char * service_name,
  const rmw_qos_profile_t * qos_policies) {
  DPUTS("rmw_create_client");
  static rmw_client_t ret;
  return &ret;
}

rmw_ret_t
rmw_destroy_client(rmw_node_t * node, rmw_client_t * client)
{
  (void) node;
  (void) client;
  DPUTS("rmw_destroy_client");
  return RMW_RET_OK;
}

rmw_ret_t
rmw_send_request(
    const rmw_client_t * client,
    const void * ros_request,
    int64_t * sequence_id)
{
  (void) client;
  (void) ros_request;
  (void) sequence_id;
  DPUTS("rmw_send_request");
  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_response(
    const rmw_client_t * client,
    rmw_request_id_t * request_header,
    void * ros_response,
    bool * taken)
{
  (void) client;
  (void) request_header;
  (void) ros_response;
  (void) taken;
  DPUTS("rmw_take_response");
  return RMW_RET_OK;
}

rmw_service_t *
rmw_create_service(
    const rmw_node_t * node,
    const rosidl_service_type_support_t * type_support,
    const char * service_name,
    const rmw_qos_profile_t * qos_policies)
{
  (void) node;
  (void) type_support;
  (void) service_name;
  (void) qos_policies;
  DPUTS("rmw_create_service");
  return NULL;
}

rmw_ret_t
rmw_destroy_service(rmw_node_t * node, rmw_service_t * service)
{
  (void) node;
  (void) service;
  DPUTS("rmw_destroy_service");
  return RMW_RET_OK;
}

rmw_ret_t
rmw_take_request(
    const rmw_service_t * service,
    rmw_request_id_t * request_header,
    void * ros_request,
    bool * taken)
{
  (void) service;
  (void) request_header;
  (void) ros_request;
  (void) taken;
  DPUTS("rmw_take_request");
  return RMW_RET_OK;
}

rmw_ret_t
rmw_send_response(
    const rmw_service_t * service,
    rmw_request_id_t * request_header,
    void * ros_response)
{
  (void) service;
  (void) request_header;
  (void) ros_response;
  DPUTS("rmw_send_response");
  return RMW_RET_OK;
}

rmw_wait_set_t *
rmw_create_wait_set(size_t max_conditions)
{
  (void) max_conditions;
  //DPUTS("rmw_create_waitset");
  rmw_wait_set_t * ret = (rmw_wait_set_t *)malloc(sizeof(rmw_wait_set_t));
  ret->data = NULL;
  ret->guard_conditions = NULL;
  ret->implementation_identifier = rmw_get_implementation_identifier();
  return ret;
}

rmw_ret_t
rmw_destroy_wait_set(rmw_wait_set_t * waitset)
{
  (void) waitset;
  //DPUTS("rmw_destroy_waitset");
  free(waitset);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_count_publishers(
    const rmw_node_t * node,
    const char * topic_name,
    size_t * count)
{
  (void) node;
  (void) topic_name;
  (void) count;
  DPUTS("rmw_count_publishers");
  return RMW_RET_OK;
}

rmw_ret_t
rmw_count_subscribers(
    const rmw_node_t * node,
    const char * topic_name,
    size_t * count)
{
  (void) node;
  (void) topic_name;
  (void) count;
  DPUTS("rmw_count_subscribers");
  return RMW_RET_OK;
}

rmw_ret_t
rmw_get_gid_for_publisher(const rmw_publisher_t * publisher, rmw_gid_t * gid)
{
  (void) publisher;
  (void) gid;
  DPUTS("rmw_get_gid_for_publisher");
  return RMW_RET_OK;
}

rmw_ret_t
rmw_compare_gids_equal(const rmw_gid_t * gid1, const rmw_gid_t * gid2, bool * result)
{
  (void) gid1;
  (void) gid2;
  (void) result;
  DPUTS("rmw_compare_gids_equal");
  return RMW_RET_OK;
}

rmw_ret_t
rmw_service_server_is_available(
    const rmw_node_t * node,
    const rmw_client_t * client,
    bool * is_available)
{
  (void) node;
  (void) client;
  (void) is_available;
  DPUTS("rmw_service_server_is_available");
  return RMW_RET_OK;
}

rmw_ret_t
rmw_get_service_names_and_types(
  const rmw_node_t * node,
  rcutils_allocator_t * allocator,
  rmw_names_and_types_t * service_names_and_types) {
  DPUTS("rmw_get_service_names_and_types");
  return RMW_RET_OK;
}
