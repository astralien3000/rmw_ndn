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

#ifndef SYNC_HPP
#define SYNC_HPP

#include <stdlib.h>
#include <string.h>

#include <queue>
#include <iostream>

#include "app.h"

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...)

class SyncPublisher
{
private:
  ndn::Name _name;
  uint64_t _seq_num;
  ndn::Block _content;
  const ndn::RegisteredPrefixId* _reg_prefix_id;

public:
  SyncPublisher(void)
    : _seq_num(0), _reg_prefix_id(0) {
  }

public:
  void setName(std::string name, uint64_t id) {
    face.unsetInterestFilter(_reg_prefix_id);
    _name.clear();
    _name.append(name);
    _name.appendNumber(id);
    _name.append("sync");
    _reg_prefix_id = face.setInterestFilter(_name,
                                            std::bind(&SyncPublisher::onInterest, this, _2),
                                            std::bind([] {}),
                                            std::bind([] {})
                                            );
  }

public:
  void push(uint64_t seq_num, ndn::Block content) {
    _seq_num = seq_num;
    _content = content;
  }

private:
  void onInterest(const ndn::Interest& interest) {
    ndn::Name name = interest.getName();
    DEBUG("SyncPublisher::onInterest %s\n", name.toUri().c_str());

    if(_content.empty()) {
      DEBUG("SyncPublisher::onInterest SKIP : no data\n");
      return;
    }

    if(!_name.isPrefixOf(name)) {
      DEBUG("SyncPublisher::onInterest ERROR : unmatched prefix\n");
      return;
    }

    DEBUG("SYNC\n");
    ndn::Data data;

    name.appendNumber(_seq_num);

    data.setName(name);
    data.setContent(_content);
    data.setFreshnessPeriod(ndn::time::seconds(0));

    ndn::KeyChain key;
    key.sign(data);

    face.put(data);
  }
};

using SyncCallback = std::function<void(uint64_t, uint64_t, ndn::Data)>;
using SyncErrorCallback = std::function<void(uint64_t)>;

static inline void requestSync(const std::string topic_name, uint64_t id, SyncCallback sync_cb, SyncErrorCallback err_cb) {
  ndn::Name name;
  name.append(topic_name);
  name.appendNumber(id);
  name.append("sync");
  name.appendNumber(std::rand());

  DEBUG("requestSync %s\n", name.toUri().c_str());

  ndn::Interest interest(name);
  interest.setMustBeFresh(true);
  interest.setInterestLifetime(ndn::time::seconds(1));

  auto on_data = [sync_cb, id](const ndn::Data& data) {
    uint64_t seq_num = data.getName().rbegin()->toNumber();
    sync_cb(id, seq_num, data);
  };

  auto on_error = [err_cb, id]() {
    err_cb(id);
  };

  face.expressInterest(interest,
                       std::bind(on_data, _2),
                       std::bind([]{}),
                       std::bind(on_error));
}

#undef DEBUG

#endif // SYNC_HPP
