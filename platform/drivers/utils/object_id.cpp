
#include "object_id.hpp"
#include "log.h"


namespace shc{

OjectID::OjectID(const uint32_t initial_size, bool zero_based, uint32_t offset) : 
  size_(initial_size), offset_(offset) {
  id_generator_ = bf_id_allocator_new(size_, zero_based); 
}

OjectID::~OjectID() {
  if (id_generator_) {
    bf_id_allocator_destroy(id_generator_);
  }
}

int32_t OjectID::Alloc(uint32_t *oid) {
  int id = bf_id_allocator_allocate(id_generator_);
  if (id == -1 || id > static_cast<int>(size_)) {
    HAL_LOG_WARN(LOG_USER1, "{}, used/total : {}/{}", hal_err_str(HAL_NO_SPACE), count_, size_);
    return HAL_NO_SPACE;
  }
  
  count_++;
  *oid = id + offset_;
  return 0;
}

int32_t OjectID::Free(uint32_t oid) {
  bf_id_allocator_release(id_generator_, oid - offset_);
  count_--;
  return 0;
}

uint32_t OjectID::ClearOffset(uint32_t oid) {
  return oid - offset_;
}


} // end of namespace shc
