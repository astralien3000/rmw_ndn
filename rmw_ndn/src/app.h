#ifndef APP_HPP
#define APP_HPP

#include <ndn-cxx/face.hpp>
#include <ndn-cxx/interest.hpp>
#include <ndn-cxx/data.hpp>

#include <ndn-cxx/util/scheduler.hpp>

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

extern ndn::Face face;
extern ndn::Scheduler scheduler;

#ifdef __cplusplus
}
#endif

#endif//APP_HPP
