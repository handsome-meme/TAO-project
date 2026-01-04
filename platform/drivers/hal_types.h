#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <bf_types/bf_types.h>

typedef int hal_status_t;
typedef uint16_t hal_object_type_t;

#define HAL_STATUS_VALUES                                                    \
      HAL_STATUS_(HAL_STATUS_FIRST_TEST, "HAL STATUS FIRST TEST"),           \
      HAL_STATUS_(HAL_STATUS_FAILURE, "GENERAL FAILURE(hal)"),               \
      HAL_STATUS_(HAL_ALREADY_EXISTS, "Already exists(hal)"),                \
      HAL_STATUS_(HAL_NO_SPACE,"Not enough space(hal)"),                     \
      HAL_STATUS_(HAL_INVALID_ARG, "Invalid arguments(hal)"),                \
      HAL_STATUS_(HAL_NOT_SUPPORTED, "Not currently supported(hal)"),        \
      HAL_STATUS_(HAL_OBJECT_NOT_FOUND,"Object not found(hal)")

// Extend the bf_status
typedef enum {
#define HAL_STATUS_(x, y) x
  HAL_STATUS_SUCCESS = 0,
  HAL_DUMMY = BF_STS_MAX - 1,
  HAL_STATUS_VALUES,
  HAL_STS_MAX
#undef HAL_STATUS_
} hal_status_enum;

static const char *hal_err_strings[HAL_STS_MAX + 1] = {
#define BF_STATUS_(x, y) y
#define HAL_STATUS_(x, y) y
    BF_STATUS_VALUES, 
    HAL_STATUS_VALUES, "Unknown error"
#undef BF_STATUS_
#undef HAL_STATUS_
};
static inline const char *hal_err_str(hal_status_t sts) {
  if (HAL_STS_MAX <= sts || 0 > sts) {
    return hal_err_strings[HAL_STS_MAX];
  } else {
    return hal_err_strings[sts];
  }
}

/**
 * @brief Logging levels
 */
typedef enum _hal_verbosity_t {
  HAL_API_LEVEL_ERROR,
  HAL_API_LEVEL_WARN,
  HAL_API_LEVEL_NOTICE,
  HAL_API_LEVEL_INFO,
  HAL_API_LEVEL_DEBUG,
  HAL_API_LEVEL_DETAIL,
  HAL_API_LEVEL_MAX
} hal_verbosity_t;


#ifdef __cplusplus
}
#endif

