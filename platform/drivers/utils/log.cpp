#include "log.h"

#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <ctime>
#include <set>
#include <vector>
#include <unordered_map>

bool operationSettings[4] = {true, true, true, true};
static hal_verbosity_t default_verbosity = HAL_API_LEVEL_ERROR;

static bool mapInitialized = false;

hal_object_type_t LOG_USER1 = 0;

std::set<hal_object_type_t> HalObjectSet = {LOG_USER1};
std::unordered_map<hal_object_type_t, hal_verbosity_t> ObjectVerbosityMap;

void toggle_operation(const hal_operation_t operation,
                      const bool isEnabled) {
  operationSettings[operation] = isEnabled;
}

void toggle_all_operations(bool isEnabled) {
  memset(operationSettings, isEnabled, sizeof(operationSettings));
}

inline uint64_t hal_to_bf_sys_level(hal_verbosity_t verbosity) {
  switch (verbosity) {
    case HAL_API_LEVEL_ERROR:
      return BF_LOG_ERR;
    case HAL_API_LEVEL_WARN:
    case HAL_API_LEVEL_NOTICE:
      return BF_LOG_WARN;
    case HAL_API_LEVEL_INFO:
      return BF_LOG_INFO;
    case HAL_API_LEVEL_DEBUG:
      return BF_LOG_DBG;
    case HAL_API_LEVEL_DETAIL:
      return BF_LOG_INFO;
    default:
      return BF_LOG_ERR;
  }
}

void set_log_level(hal_verbosity_t new_verbosity) {
  default_verbosity = new_verbosity;
#ifndef TESTING
  bf_sys_trace_level_set(BF_MOD_SWITCHAPI,
                         hal_to_bf_sys_level(new_verbosity));
  bf_sys_log_level_set(BF_MOD_SWITCHAPI,
                       BF_LOG_DEST_STDOUT,
                       hal_to_bf_sys_level(new_verbosity));
#endif
}

hal_verbosity_t get_log_level() { return default_verbosity; }

void set_log_level_object(hal_object_type_t object_type,
                          hal_verbosity_t new_verbosity) {
  if (object_type == 0) return;
  if (ObjectVerbosityMap.find(object_type) != ObjectVerbosityMap.end()) {
    ObjectVerbosityMap[object_type] = new_verbosity;
  }
  return;
}

void set_log_level_all_objects(hal_verbosity_t verbosity) {
  for (auto it = HalObjectSet.begin(); it != HalObjectSet.end(); it++) {
    ObjectVerbosityMap[*it] = verbosity;
  }
  mapInitialized = true;
#ifndef TESTING
  bf_sys_trace_level_set(BF_MOD_SWITCHAPI, hal_to_bf_sys_level(verbosity));
  bf_sys_log_level_set(
      BF_MOD_SWITCHAPI, BF_LOG_DEST_FILE, hal_to_bf_sys_level(verbosity));
  //bf_sys_log_level_set(
  //    BF_MOD_SWITCHAPI, BF_LOG_DEST_STDOUT, hal_to_bf_sys_level(verbosity));
#endif
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec) {
  (void)vec;
  return os;
}

std::ostream &operator<<(std::ostream &os,
                         const hal_operation_t &operation) {
  switch (operation) {
    case HAL_GET_OPERATION:
      os << "get";
      break;
    case HAL_GET_ALL_OPERATION:
      os << "get_all";
      break;
    case HAL_DEL_OPERATION:
      os << "delete";
      break;
    case HAL_SET_OPERATION:
      os << "set";
      break;
    case HAL_ADD_OPERATION:
      os << "add";
      break;
    case HAL_MOD_OPERATION:
      os << "mod";
      break;
    case HAL_CLEAR_OPERATION:
      os << "clear";
      break;
  }
  return os;
}


hal_status_t logging_init(hal_verbosity_t verbosity) {
  set_log_level_all_objects(verbosity);
  default_verbosity = verbosity;
  return HAL_STATUS_SUCCESS;
}

static inline void hal_log_internal(hal_verbosity_t verbosity,
                                       const char *message) {
  (void)verbosity;
#ifdef TESTING
  std::cout << message << std::endl;
#else
  bf_sys_log_and_trace(
      BF_MOD_SWITCHAPI, hal_to_bf_sys_level(verbosity), "%s", message);
#endif
}

bool hal_log(hal_verbosity_t verbosity, const char *message) {
  if (verbosity <= default_verbosity) {
    hal_log_internal(verbosity, message);
    return true;
  }
  return false;
}

bool hal_log(hal_verbosity_t verbosity,
                hal_object_type_t object_type,
                const char *message) {
  if (object_type == 0) return hal_log(verbosity, message);

  if (ObjectVerbosityMap.find(object_type) != ObjectVerbosityMap.end()) {
    if (verbosity <= ObjectVerbosityMap[object_type]) {
      return hal_log(verbosity, message);
    }
  } else {
    return hal_log(verbosity, message);
  }
  return false;
}

bool hal_log(hal_verbosity_t verbosity,
                hal_object_type_t object_type,
                hal_operation_t operation,
                const char *message) {
  if (operationSettings[operation]) {
    return hal_log(verbosity, object_type, message);
  }
  return false;
}

const std::unordered_map<hal_object_type_t, hal_verbosity_t> &
get_log_level_all_objects() {
  return ObjectVerbosityMap;
}

