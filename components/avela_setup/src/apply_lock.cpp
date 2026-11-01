#include "avela_setup/apply_lock.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace avela {
namespace {

SemaphoreHandle_t apply_lock()
{
    static StaticSemaphore_t storage{};
    static const SemaphoreHandle_t lock = xSemaphoreCreateMutexStatic(&storage);
    return lock;
}

}  // namespace

//--------------------------------------------------------------------------------------------------

ApplyLock::ApplyLock()
{
    xSemaphoreTake(apply_lock(), portMAX_DELAY);
}

//--------------------------------------------------------------------------------------------------

ApplyLock::~ApplyLock()
{
    xSemaphoreGive(apply_lock());
}

}  // namespace avela
