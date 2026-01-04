#pragma once
#include <stdint.h>
#include <bf_rt/bf_rt.hpp>
// #include "utils/log.h"

namespace shc {

// __attribute__((unused))
// static bool mac_array_to_uint64(uint64_t& dst, const uint8_t from[6]) {
//     dst = 0;
//     uint64_t tmp;
//     for (uint32_t i = 6; i > 0; i--) {
//         tmp = from[i - 1];
//         dst += tmp << (6 - i) * 8;
//     }
//     return true;
// }

// __attribute__((unused))
// static bool uint64_to_mac_array(uint8_t* dst, uint64_t from) {
//     uint8_t byte = 0;
//     byte = (int8_t)(from >> 40);
//     dst[0] = byte;
//     byte = (int8_t)(from >> 32);
//     dst[1] = byte;
//     byte = (int8_t)(from >> 24);
//     dst[2] = byte;
//     byte = (int8_t)(from >> 16);
//     dst[3] = byte;
//     byte = (int8_t)(from >> 8);
//     dst[4] = byte;
//     byte = (int8_t)from;
//     dst[5] = byte;
//     return true;
// }

__attribute__((unused))
static bf_status_t session_complete(std::shared_ptr<bfrt::BfRtSession> session) {
  bf_status_t bf_status = BF_SUCCESS;

  // if (!session) return HAL_INVALID_ARG;

  bf_status = session->sessionCompleteOperations();
  if (bf_status != BF_SUCCESS) {
  //   hal_log(HAL_API_LEVEL_ERROR,
  //              HAL_OT_NONE,
  //              "{}.{}:{}: status: {} failed to session complete",
  //              __NS__,
  //              __func__,
  //              __LINE__,
  //              bf_err_str(bf_status));
  //   return bf_rt_status_xlate(bf_status);
    return bf_status;
  }

  return bf_status;
}

__attribute__((unused))
static bf_status_t transaction_start(std::shared_ptr<bfrt::BfRtSession> session) {
  bf_status_t bf_status = BF_SUCCESS;

  // if (!session) return HAL_INVALID_ARG;

  bf_status = session->beginTransaction(true);
  if (bf_status != BF_SUCCESS) {
    // hal_log(HAL_API_LEVEL_ERROR,
    //            HAL_OT_NONE,
    //            "{}.{}:{}: status: {} failed to start transaction",
    //            __NS__,
    //            __func__,
    //            __LINE__,
    //            bf_err_str(bf_status));
    // return bf_rt_status_xlate(bf_status);
    return bf_status;
  }

  return bf_status;
}

__attribute__((unused))
static bf_status_t transaction_commit(std::shared_ptr<bfrt::BfRtSession> session) {
  bf_status_t bf_status = BF_SUCCESS;

  // if (!session) return HAL_INVALID_ARG;

  bf_status = session->commitTransaction(true);
  if (bf_status != BF_SUCCESS) {
    // hal_log(HAL_API_LEVEL_ERROR,
    //            HAL_OT_NONE,
    //            "{}.{}:{}: status: {} failed to commit transaction",
    //            __NS__,
    //            __func__,
    //            __LINE__,
    //            bf_err_str(bf_status));
    // return bf_rt_status_xlate(bf_status);
    return bf_status;
  }

  return session_complete(session);
}

__attribute__((unused))
static bf_status_t transaction_abort(std::shared_ptr<bfrt::BfRtSession> session) {
  bf_status_t bf_status = BF_SUCCESS;

  // if (!session) return HAL_INVALID_ARG;

  bf_status = session->abortTransaction();
  if (bf_status != BF_SUCCESS) {
  //   hal_log(HAL_API_LEVEL_ERROR,
  //              HAL_OT_NONE,
  //              "{}.{}:{}: status: {} failed to abort transaction",
  //              __NS__,
  //              __func__,
  //              __LINE__,
  //              bf_err_str(bf_status));
  //   return bf_rt_status_xlate(bf_status);
    return bf_status;
  }

  return session_complete(session);
}

}