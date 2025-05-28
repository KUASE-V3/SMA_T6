#include "persistence/OvmAddressRepository.hpp"

using namespace persistence;

OvmAddressRepository::OvmAddressRepository()
    : id_map_{
        {"T1",  "127.0.0.1:9000"},
        {"T2",  "127.0.0.1:9001"},
        {"T3",  "127.0.0.1:9002"},
        {"T4",  "127.0.0.1:9003"},
        {"T5",  "127.0.0.1:9004"},
        {"T6",  "127.0.0.1:9005"},
        {"T7",  "127.0.0.1:9006"},
        {"T8",  "127.0.0.1:9007"},
        {"T9",  "127.0.0.1:9008"},
        {"T10", "127.0.0.1:9009"}
    }
{}

std::string OvmAddressRepository::getEndpoint(const std::string& id) const {
    auto it = id_map_.find(id);
    return (it != id_map_.end() ? it->second : "");
}

std::vector<std::string> OvmAddressRepository::getAllEndpoints() const {
    std::vector<std::string> eps;
    eps.reserve(id_map_.size());
    for (const auto& kv : id_map_) {
        eps.push_back(kv.second);
    }
    return eps;
}
