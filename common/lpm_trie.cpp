#include "lpm_trie.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;
using namespace shc;

namespace shc {

bool Fib_Hash::lpm_trie_is_exist(const shc::IpPrefix &prefix) {
    auto &fib_table = get_fib_table(prefix);
    unsigned int prefixlen = prefix.getMaskLength();
    if (prefixlen < fib_table.size()) {
        const auto &tempmap = fib_table[prefixlen];
        auto it = tempmap.find(prefix.getSubnet());
        if (it != tempmap.end()) {
            return true;
        }
    }
    return false;
}

bool Fib_Hash::lpm_trie_modify(const shc::IpPrefix &prefix, const std::string &route_info) {
    auto &fib_table = get_fib_table(prefix);
    unsigned int prefixlen = prefix.getMaskLength();
    if (prefixlen < fib_table.size()) {
        auto &tempmap = fib_table[prefixlen];
        auto it = tempmap.find(prefix.getSubnet());
        if (it != tempmap.end()) {
            it->second = route_info;
            return true;
        }
    }
    return false;
}

bool Fib_Hash::lpm_trie_get(const shc::IpPrefix &prefix, std::string &route_info) {
    auto &fib_table = get_fib_table(prefix);
    unsigned int prefixlen = prefix.getMaskLength();
    if (prefixlen < fib_table.size()) {
        const auto &tempmap = fib_table[prefixlen];
        auto it = tempmap.find(prefix.getSubnet());
        if (it != tempmap.end()) {
            route_info = it->second;
            return true;
        }
    }
    return false;
}

bool Fib_Hash::lpm_trie_insert(const shc::IpPrefix &prefix, const std::string &route_info) {
    auto &fib_table = get_fib_table(prefix);
    unsigned int prefixlen = prefix.getMaskLength();
    if (prefixlen >= fib_table.size()) {
        return false;
    }
    fib_table[prefixlen][prefix.getSubnet()] = route_info;
    return true;
}

bool Fib_Hash::lpm_trie_lookup(const shc::IpAddress &dst_ip, std::string &route_info) {
    auto &fib_table = get_fib_table(dst_ip);
    int prefixlen = static_cast<int>(fib_table.size());
    string ipstr  = dst_ip.to_string() + '/';
    auto iter = fib_table.rbegin();
    while (iter != fib_table.rend()) {
        prefixlen--;
        shc::IpPrefix tempprefix(ipstr + std::to_string(prefixlen));
        auto iter1 = iter->find(tempprefix.getSubnet());
        if (iter1 != iter->end()) {
            route_info = iter1->second;
            return true;
        }
        iter++;
    }
    return false;
}

bool Fib_Hash::lpm_trie_delete(const shc::IpPrefix &prefix) {
    if (!lpm_trie_is_exist(prefix)) {
        return true;
    }
    auto &fib_table = get_fib_table(prefix);
    unsigned int prefixlen = prefix.getMaskLength();
    if (prefixlen >= fib_table.size()) {
        return false;
    }
    fib_table[prefixlen].erase(prefix.getSubnet());
    return true;
}

}  // namespace shc