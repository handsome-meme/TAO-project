#ifndef __LPM_TRIE_H__
#define __LPM_TRIE_H__

#include <map>
#include <string>
#include <vector>

#include "ipaddress.h"
#include "ipprefix.h"

namespace shc {

typedef std::map<shc::IpPrefix, std::string> Fib_Map_t;

class Fib_Hash {
  public:
    Fib_Hash() {}
    bool lpm_trie_is_exist(const shc::IpPrefix &prefix);
    bool lpm_trie_insert(const shc::IpPrefix &prefix, const std::string &route_info);
    bool lpm_trie_delete(const shc::IpPrefix &prefix);
    bool lpm_trie_lookup(const shc::IpAddress &dst_ip, std::string &route_info);
    bool lpm_trie_modify(const shc::IpPrefix &prefix, const std::string &route_info);
    bool lpm_trie_get(const shc::IpPrefix &prefix, std::string &route_info);

  private:
    std::vector<Fib_Map_t> &get_fib_table(const shc::IpPrefix &prefix) {
        if (prefix.isV4()) {
            return _fib_table;
        }
        return _fib6_table;
    }

    std::vector<Fib_Map_t> &get_fib_table(const shc::IpAddress &ip) {
        if (ip.isV4()) {
            return _fib_table;
        }
        return _fib6_table;
    }

  private:
    std::vector<Fib_Map_t> _fib_table = std::vector<Fib_Map_t>(33);
    std::vector<Fib_Map_t> _fib6_table = std::vector<Fib_Map_t>(129);
};

}  // namespace overlay

#endif
