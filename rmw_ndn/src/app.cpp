#include "app.h"

#include <stdio.h>
#include <string.h>

#include <boost/asio/io_service.hpp>

boost::asio::io_service ioService;
ndn::Face face(ioService);
ndn::Scheduler scheduler(ioService);

typedef struct app {
} app_t;

void app_add_sub(sub_t* sub) {
}

void app_rm_sub(sub_t* sub) {
}

void app_add_pub(pub_t* pub) {
}

void app_rm_pub(pub_t* pub) {
}

void app_create(void) {
}

void app_destroy(void) {
}

void app_update(void) {
}
