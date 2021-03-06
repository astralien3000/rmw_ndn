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

#ifndef TOPIC_HPP
#define TOPIC_HPP

#include <stdlib.h>
#include <string.h>

#include <queue>
#include <iostream>

#include "app.h"

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...)

class TopicPublisher
{
private:
  static constexpr size_t WINDOW = 10;

private:
  ndn::Name _name;
  uint64_t _seq_num;
  std::set<uint64_t> _req_seq_num;
  std::vector<ndn::Data> _queue;
  const ndn::RegisteredPrefixId* _reg_prefix_id;

public:
  TopicPublisher(void)
    : _seq_num(0), _reg_prefix_id(0) {
  }

public:
  void setName(std::string name, uint64_t id) {
    face.unsetInterestFilter(_reg_prefix_id);
    _name.clear();
    _name.append(name);
    _name.appendNumber(id);
    _name.append("data");
    _reg_prefix_id = face.setInterestFilter(_name,
                           std::bind(&TopicPublisher::onInterest, this, _2),
                           std::bind([] {}),
                           std::bind([] {}));
  }

public:
  void push(ndn::Data data) {
    DEBUG("TopicPublisher::push %s\n", data.getName().toUri().c_str());
    DEBUG("TopicPublisher::push size %i\n", (int)_req_seq_num.size());
    DEBUG("TopicPublisher::push seq %i\n", (int)_seq_num);
    for(auto it = _req_seq_num.begin() ; it != _req_seq_num.end() ; it++) {
      if(*it == _seq_num) {
        face.put(data);
        _req_seq_num.erase(it);
        break;
      }
    }

    _queue.push_back(data);
    _seq_num++;

    if(_queue.size() > WINDOW) {
      _queue.erase(_queue.begin());
    }
  }

private:
  void onInterest(const ndn::Interest& interest) {
    ndn::Name name = interest.getName();
    DEBUG("TopicPublisher::onInterest %s\n", name.toUri().c_str());

    if(_queue.empty()) {
      DEBUG("TopicPublisher::onInterest SKIP : no data\n");
      return;
    }

    if(!_name.isPrefixOf(name)) {
      DEBUG("TopicPublisher::onInterest ERROR : unmatched prefix\n");
      return;
    }

    DEBUG("DATA\n");
    uint64_t req_seq_num = name.rbegin()->toNumber();

    if(req_seq_num >= _seq_num) {
      DEBUG("TopicPublisher::onInterest SKIP : data %i not produced (%i)\n", (int)req_seq_num, (int)_seq_num);
      _req_seq_num.insert(req_seq_num);
      return;
    }

    const uint64_t diff_seq_num = _seq_num - req_seq_num;
    if(diff_seq_num > _queue.size()) {
      DEBUG("TopicPublisher::onInterest SKIP : data %i outdated (%i)\n", (int)req_seq_num, (int)_seq_num);
      return;
    }

    DEBUG("TopicPublisher::onInterest PUBLISH : data %i %s\n", (int)req_seq_num, _queue[_queue.size()-diff_seq_num].getName().toUri().c_str());
    face.put(_queue[_queue.size()-diff_seq_num]);
  }
};

using DataCallback = std::function<void(uint64_t, uint64_t, ndn::Data)>;
using DataErrorCallback = std::function<void(uint64_t)>;

static inline void requestData(const std::string& topic_name, uint64_t id, uint64_t seq_num, DataCallback data_cb, DataErrorCallback err_cb) {
  ndn::Name name;
  name.append(topic_name);
  name.appendNumber(id);
  name.append("data");
  name.appendNumber(seq_num);

  DEBUG("requestData %s\n", name.toUri().c_str());

  ndn::Interest interest(name);
  interest.setMustBeFresh(false);
  interest.setInterestLifetime(ndn::time::seconds(1));

  auto on_data = [data_cb, id](const ndn::Data& data) {
    uint64_t seq_num = data.getName().rbegin()->toNumber();
    data_cb(id, seq_num, data);
  };

  auto on_error = [err_cb, id]() {
    err_cb(id);
  };

  face.expressInterest(interest,
                       std::bind(on_data, _2),
                       std::bind(on_error),
                       std::bind(on_error));
}

#undef DEBUG

#endif // TOPIC_HPP
