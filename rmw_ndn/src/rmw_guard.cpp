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

#include <rmw/rmw.h>

#include <stdlib.h>

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...)

const rmw_guard_condition_t *
rmw_node_get_graph_guard_condition(const rmw_node_t * node)
{
  DEBUG("rmw_node_get_graph_guard_condition" "\n");
  rmw_guard_condition_t * ret = (rmw_guard_condition_t *)malloc(sizeof(rmw_guard_condition_t));
  ret->data = NULL;
  ret->implementation_identifier = rmw_get_implementation_identifier();
  return ret;
}

rmw_guard_condition_t *
rmw_create_guard_condition(void)
{
  DEBUG("rmw_create_guard_condition" "\n");
  rmw_guard_condition_t * ret = (rmw_guard_condition_t *)malloc(sizeof(rmw_guard_condition_t));
  ret->data = NULL;
  ret->implementation_identifier = rmw_get_implementation_identifier();
  return ret;
}

rmw_ret_t
rmw_destroy_guard_condition(rmw_guard_condition_t * guard_condition)
{
  DEBUG("rmw_destroy_guard_condition" "\n");
  free(guard_condition);
  return RMW_RET_OK;
}

rmw_ret_t
rmw_trigger_guard_condition(const rmw_guard_condition_t * guard_condition)
{
  (void) guard_condition;
  DEBUG("rmw_trigger_guard_condition" "\n");
  return RMW_RET_OK;
}
