#ifndef DISCOVERY_HPP
#define DISCOVERY_HPP

#include <string>

#include "app.h"

//#define DEBUG(...) printf(__VA_ARGS__)
#define DEBUG(...)

class DiscoveryHeartbeatEmiter
{
private:
  ndn::Name _name;
  ndn::EventId _evt;

public:
  ~DiscoveryHeartbeatEmiter(void) {
    scheduler.cancelEvent(_evt);
  }

public:
  DiscoveryHeartbeatEmiter(std::string type, std::string name) {
    _name.append(type);
    _name.append(name);
    heartbeat();
  }

  DiscoveryHeartbeatEmiter(std::string type, std::string name, uint64_t id) {
    _name.append(type);
    _name.append(name);
    _name.appendNumber(id);
    heartbeat();
  }

public:
  void setName(std::string type, std::string name) {
    _name.clear();
    _name.append(type);
    _name.append(name);
  }

  void setName(std::string type, std::string name, uint64_t id) {
    _name.clear();
    _name.append(type);
    _name.append(name);
    _name.appendNumber(id);
  }

private:
  void heartbeat(void) {
    _evt = scheduler.scheduleEvent(ndn::time::seconds(1), std::bind(&DiscoveryHeartbeatEmiter::heartbeat, this));
    ndn::Interest interest;
    ndn::Name interest_name = discovery_prefix;
    interest_name.append(_name);
    DEBUG("HEARTBEAT %s\n", interest_name.toUri().c_str());
    interest.setName(interest_name);
    interest.setMustBeFresh(true);
    interest.setInterestLifetime(ndn::time::seconds(0));
    face.expressInterest(interest,
                         std::bind([](const ndn::Data&) {}, _2),
                         std::bind([](const ndn::Interest&) {}, _1),
                         std::bind([](const ndn::Interest&) {}, _1));
  }
};

class DiscoveryClient
{
private:
  std::map<std::string, int> _discovered_nodes;
  std::map<std::string, std::map<uint64_t, int>> _discovered_publishers;
  std::map<std::string, int> _discovered_subscribers;

private:
  DiscoveryClient(void) {
    face.setInterestFilter(discovery_prefix,
                           std::bind(&DiscoveryClient::onInterest, this, _2),
                           std::bind([] {}),
                           std::bind([] {}));
  }

public:
  static DiscoveryClient& instance(void) {
    static DiscoveryClient ret;
    return ret;
  }

public:
  std::vector<std::string> getDiscoveredNames(void) {
    std::vector<std::string> ret;
    for(auto it = _discovered_nodes.begin() ; it != _discovered_nodes.end() ; it++) {
      ret.push_back(it->first);
    }
    return ret;
  }

  std::map<std::string, std::set<std::string>> getDiscoveredTopics(void) {
    std::map<std::string, std::set<std::string>> ret;
    for(auto it = _discovered_subscribers.begin() ; it != _discovered_subscribers.end() ; it++) {
      ret[it->first] = std::set<std::string>();
    }
    for(auto it = _discovered_publishers.begin() ; it != _discovered_publishers.end() ; it++) {
      ret[it->first] = std::set<std::string>();
    }
    return ret;
  }

  std::map<std::string, std::set<uint64_t>> getDiscoveredPublishers(void) {
    std::map<std::string, std::set<uint64_t>> ret;
    for(auto it = _discovered_publishers.begin() ; it != _discovered_publishers.end() ; it++) {
      ret[it->first] = std::set<uint64_t>();
      for(auto it2 = it->second.begin() ; it2 != it->second.end() ; it2++) {
        ret[it->first].insert(it2->first);
      }
    }
    return ret;
  }

private:
  void onInterest(const ndn::Interest& interest) {
    ndn::Name name = interest.getName();
    DEBUG("Discovery::onInterest %s\n", name.toUri().c_str());
    if(name[2] == ndn::name::Component("node")) {
      DEBUG("NODE\n");
      _discovered_nodes[name.getSubName(3).toUri()] = 1;
    }
    if(name[2] == ndn::name::Component("publisher")) {
      ndn::Name pname = name.getSubName(3);
      ndn::Name topic_name = pname.getPrefix(pname.size()-1);
      uint64_t id = pname.rbegin()->toNumber();
      DEBUG("PUBLISHER %s %i\n", topic_name.toUri().c_str(), (int)id);
      _discovered_publishers[topic_name.toUri()][id] = 1;
    }
    if(name[2] == ndn::name::Component("subscriber")) {
      DEBUG("SUBSCRIBER %s\n", name.getSubName(3).toUri().c_str());
      _discovered_subscribers[name.getSubName(3).toUri()] = 1;
    }
  }
};

#undef DEBUG

#endif//DISCOVERY_HPP
