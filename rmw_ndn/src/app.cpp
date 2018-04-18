#include "app.h"

#include <stdio.h>
#include <string.h>

#include <boost/asio/io_service.hpp>

boost::asio::io_service ioService;
ndn::Face face(ioService);
ndn::Scheduler scheduler(ioService);
