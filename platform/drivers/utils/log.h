
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../hal_types.h"
#include <bfsys/bf_sal/bf_sys_log.h>
#include <fmt/format.h>
#include <fmt/ostream.h>


// can't call this SWITCH_OBJECT_TYPE_NONE because it is a generated type
// Do not use this outside of S3

extern hal_object_type_t LOG_USER1;

#define HAL_OT_NONE 0

typedef enum _hal_operation_t {
  HAL_GET_OPERATION,
  HAL_GET_ALL_OPERATION,
  HAL_SET_OPERATION,
  HAL_ADD_OPERATION,
  HAL_MOD_OPERATION,
  HAL_DEL_OPERATION,
  HAL_CLEAR_OPERATION,
} hal_operation_t;

// operations for user to enable logging
void toggle_operation(hal_operation_t operation, bool isEnabled);
void toggle_all_operations(bool isEnabled);
void set_log_level(hal_verbosity_t new_verbosity);
hal_verbosity_t get_log_level();
void set_log_level_object(hal_object_type_t object_type,
                          hal_verbosity_t verbosity);
void set_log_level_all_objects(hal_verbosity_t verbosity);

static inline const char *hal_verbosity_to_string(
    hal_verbosity_t verbosity) {
  switch (verbosity) {
    case HAL_API_LEVEL_ERROR:
      return "error";
    case HAL_API_LEVEL_WARN:
      return "warn";
    case HAL_API_LEVEL_INFO:
      return "info";
    case HAL_API_LEVEL_DEBUG:
      return "debug";
    case HAL_API_LEVEL_DETAIL:
      return "detail";
    default:
      return "error";
  }
}

#ifdef __cplusplus
#include <string>
#include <vector>
#include <unordered_map>

template <typename T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &vec);
std::ostream &operator<<(std::ostream &os, const hal_operation_t &operation);

template <typename T>
std::string formatNumber(T number) {
  return fmt::format("{:n}", number);
}

template <typename Arg1, typename... Args>
std::string createMessage(const char *format,
                          const Arg1 &arg1,
                          const Args &... args) {
  fmt::memory_buffer local_buf;
  fmt::format_to(local_buf, format, arg1, args...);
  return fmt::to_string(local_buf);
}

// operations for actual logging
bool hal_log(hal_verbosity_t verbosity, const char *message);
bool hal_log(hal_verbosity_t verbosity,
                hal_object_type_t object_type,
                const char *message);
bool hal_log(hal_verbosity_t verbosity,
                hal_object_type_t object_type,
                hal_operation_t operation,
                const char *message);

template <typename Arg1, typename... Args>

bool hal_log(hal_verbosity_t verbosity,
                hal_object_type_t object_type,
                const char *format,
                const Arg1 &arg1,
                const Args &... args) {
  if (verbosity > get_log_level()) return true;
  std::string message = createMessage(format, arg1, args...);
  return hal_log(verbosity, object_type, message.c_str());
}

template <typename Arg1, typename... Args>
bool hal_log(hal_verbosity_t verbosity,
                hal_object_type_t object_type,
                hal_operation_t operation,
                const char *format,
                const Arg1 &arg1,
                const Args &... args) {
  if (verbosity > get_log_level()) return true;
  std::string message = createMessage(format, arg1, args...);
  return hal_log(verbosity, object_type, operation, message.c_str());
}

const std::unordered_map<hal_object_type_t, hal_verbosity_t> &
get_log_level_all_objects();


#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define __FILE_LINE_FUNC__  __FILENAME__, __LINE__, __FUNCTION__

#define HAL_LOG_ERROR(object_type, format, ...)      \
                      hal_log(HAL_API_LEVEL_ERROR,   \
									            object_type,           \
                              "{}:{} {} | " format,  \
                              __FILE_LINE_FUNC__,    \
                              ##__VA_ARGS__)

#define HAL_LOG_WARN(object_type, format, ...)       \
                      hal_log(HAL_API_LEVEL_WARN,    \
									            object_type,           \
                              "{}:{} {} | " format,  \
                              __FILE_LINE_FUNC__,    \
                              ##__VA_ARGS__)

#define HAL_LOG_INFO(object_type, format, ...)       \
                      hal_log(HAL_API_LEVEL_INFO,    \
									            object_type,           \
                              "{}:{} {} | " format,  \
                              __FILE_LINE_FUNC__,    \
                              ##__VA_ARGS__)

#define HAL_LOG_DEBUG(object_type, format, ...)      \
                      hal_log(HAL_API_LEVEL_DEBUG,   \
									            object_type,           \
                              "{}:{} {} | " format,   \
                              __FILE_LINE_FUNC__,    \
                              ##__VA_ARGS__)

#define HAL_LOG_DETAIL(object_type, format, ...)     \
                      hal_log(HAL_API_LEVEL_DETAIL,  \
									            object_type,           \
                              "{}:{} {} | " format,   \
                              __FILE_LINE_FUNC__,    \
                              ##__VA_ARGS__)

#define SWITCH_DEBUG_LOG(__fn)                     \
  if (get_log_level() >= HAL_API_LEVEL_DEBUG) { \
    __fn;                                          \
  }

#define SWITCH_DETAIL_LOG(__fn)                     \
  if (get_log_level() >= HAL_API_LEVEL_DETAIL) { \
    __fn;                                           \
  }

hal_status_t logging_init(hal_verbosity_t verbosity);

#define CHECK_RET_AND_LOG_ERR(ret)                              \
  do {                                                 \
    if (unlikely(ret)) {                                 \
      hal_log(HAL_API_LEVEL_ERROR,               \
                 static_cast<hal_object_type_t>(0), \
                 "{}:{} ERROR {} reason {}",           \
                 __func__,                             \
                 __LINE__,                             \
                 hal_err_str(ret),          \
                 #ret);                                  \
      return ret;                                      \
    }                                                  \
  } while (0)

#define CHECK_RET_AND_LOG_WARN(ret)                              \
  do {                                                 \
    if (unlikely(ret)) {                                 \
      hal_log(HAL_API_LEVEL_WARN,               \
                 static_cast<hal_object_type_t>(0), \
                 "{}:{} WARN {} reason {}",           \
                 __func__,                             \
                 __LINE__,                             \
                 hal_err_str(ret),          \
                 #ret);                                  \
      return ret;                                      \
    }                                                  \
  } while (0)

#define CHECK_RET_AND_LOG_INFO(ret)                              \
  do {                                                 \
    if (unlikely(ret)) {                                 \
      hal_log(HAL_API_LEVEL_INFO,               \
                 static_cast<hal_object_type_t>(0), \
                 "{}:{} INFO {} reason {}",           \
                 __func__,                             \
                 __LINE__,                             \
                 hal_err_str(ret),          \
                 #ret);                                  \
      return ret;                                      \
    }                                                  \
  } while (0)

#define ENTER()                                      \
  do {                                               \
    hal_log(HAL_API_LEVEL_DEBUG,               \
               static_cast<hal_object_type_t>(0), \
               "{}: Enter",                          \
               __func__);                            \
  } while (0)
#define EXIT()                                       \
  do {                                               \
    hal_log(HAL_API_LEVEL_DEBUG,               \
               static_cast<hal_object_type_t>(0), \
               "{}: Exit",                           \
               __func__);                            \
  } while (0)

#endif /* __cplusplus */

#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

