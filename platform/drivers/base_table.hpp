#pragma once
#include <mutex>

namespace shc {
class BaseTable {
  public:
    void tableMutexLock(void) {table_mutex_.lock();}
    void tableMutexUnlock(void) {table_mutex_.unlock();}
    // hal_status_t setScaleRatio(float scale_ratio){
    //   scale_ratio_ = scale_ratio; 
    //   return HAL_STATUS_SUCCESS;
    // }
    // float getScaleRatio(){
    //   return scale_ratio_;
    // }
  private:
    std::mutex table_mutex_;
    float scale_ratio_;
};


}