#pragma once

#include <stdint.h>

extern "C" {
#include "hal_table_type.h"
#include <bfutils/id/id.h>
}

namespace shc {

class OjectID {
 public:
  OjectID(const uint32_t initial_size, bool zero_based, uint32_t offset);
  ~OjectID();
  hal_status_t Alloc(uint32_t *oid);
  hal_status_t Free(uint32_t oid);
  uint32_t ClearOffset(uint32_t oid);
  uint32_t GetUsage() {return count_;}
  uint32_t GetSize() {return size_;}
  uint32_t GetLeft() {return size_ - count_;}

 private:
  bf_id_allocator *id_generator_ = 0;
  uint32_t size_ = 0;
  uint32_t offset_ = 0;
  uint32_t count_ = 0;
};

}// namespace shc