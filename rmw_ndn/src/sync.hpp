#ifndef SYNC_HPP
#define SYNC_HPP

#include <stdlib.h>
#include <string.h>

#include <queue>
#include <iostream>

#include "app.h"

#define DEBUG(...) printf(__VA_ARGS__)
//#define DEBUG(...)

class SyncPublisher
{
private:
  ndn::Name _name;
  uint64_t _seq_num;
  ndn::Block _content;
  const ndn::RegisteredPrefixId* _reg_prefix_id;

public:
  SyncPublisher(std::string name, uint64_t id)
    : _seq_num(0)
  {
    _name.append(name);
    _name.appendNumber(id);
    _name.append("sync");
    _reg_prefix_id = face.setInterestFilter(_name,
                           std::bind(&SyncPublisher::onInterest, this, _2),
                           std::bind([this] {}),
                           std::bind([this] {}));
  }

public:
  void setName(std::string name, uint64_t id) {
    face.unsetInterestFilter(_reg_prefix_id);
    _name.clear();
    _name.append(name);
    _name.appendNumber(id);
    _name.append("sync");
    DEBUG("new id : %i\n", (int)id);
    _reg_prefix_id = face.setInterestFilter(_name,
                           std::bind(&SyncPublisher::onInterest, this, _2),
                           std::bind([] {}),
                           std::bind([] {}));
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

class SyncSubscriber
{
public:
  typedef void (*Callback)(void);

private:
  ndn::Name _name;
  uint64_t _seq_num;
  Callback _cb;

public:
  explicit
  SyncSubscriber(const std::string topic_name, uint64_t id, Callback cb)
    : _seq_num(0)
    , _cb(cb)
  {
    _name.append(topic_name);
    _name.appendNumber(id);
    _name.append("sync");
    DEBUG("SyncSubscriber::_name = %s\n", _name.toUri().c_str());
    requestSync();
  }

private:
  void requestSync()
  {
    ndn::Name name= ndn::Name(_name);
    name.appendNumber(std::rand());
    DEBUG("SyncSubscriber::requestSync %s\n", name.toUri().c_str());
    ndn::Interest interest(name);
    interest.setMustBeFresh(true);
    interest.setInterestLifetime(ndn::time::seconds(10));
    face.expressInterest(interest,
                         std::bind(&SyncSubscriber::onSyncData, this, _2),
                         std::bind(&SyncSubscriber::onNack, this, _1),
                         std::bind(&SyncSubscriber::onTimeout, this, _1));
  }

  void onSyncData(const ndn::Data& data)
  {
    /*
    ndn::Name name = data.getName();
    DEBUG("SyncSubscriber::onSyncData %s\n", name.toUri().c_str());
    ndn::name::Component seq_num_comp = *name.rbegin();
    std::string req_seq_num_str = seq_num_comp.toUri();
    uint64_t req_seq_num_ll = std::stoull(req_seq_num_str);
    if(_seq_num <= req_seq_num_ll) {
      _queue.push_back(data);
      _seq_num = req_seq_num_ll+1;
    }
    requestData();
    */
  }

  void onNack(const ndn::Interest& interest) {
    DEBUG("SyncSubscriber::onNack %s\n", interest.getName().toUri().c_str());
    scheduler.scheduleEvent(ndn::time::seconds(1), [this] { requestSync(); });
  }

  void onTimeout(const ndn::Interest& interest) {
    DEBUG("SyncSubscriber::onTimeout %s\n", interest.getName().toUri().c_str());
    requestSync();
  }
};

#undef DEBUG

#endif // SYNC_HPP
