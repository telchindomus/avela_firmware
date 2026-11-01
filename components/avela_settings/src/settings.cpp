#include "avela_settings/settings.hpp"

#include "avela_strings/fixed_string.hpp"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "nvs.h"
#include "nvs_flash.h"

namespace avela {
namespace {

constexpr const char* name_space = "avela";
constexpr const char* log_tag = "settings";
constexpr std::size_t key_capacity = NVS_KEY_NAME_MAX_SIZE;  // 15 characters and the terminator

using Key = FixedString<key_capacity>;

//--------------------------------------------------------------------------------------------------

Key key_text(std::string_view key)
{
    Key text;
    text.append(key);
    return text;
}

//--------------------------------------------------------------------------------------------------

// the saves' lock, made on first use (static storage, no heap)
SemaphoreHandle_t save_lock()
{
    static StaticSemaphore_t storage{};
    static const SemaphoreHandle_t lock = xSemaphoreCreateMutexStatic(&storage);
    return lock;
}

}  // namespace

//--------------------------------------------------------------------------------------------------

Settings::Lock::Lock()
{
    xSemaphoreTake(save_lock(), portMAX_DELAY);
}

//--------------------------------------------------------------------------------------------------

Settings::Lock::~Lock()
{
    xSemaphoreGive(save_lock());
}

//--------------------------------------------------------------------------------------------------

void Settings::init()
{
    esp_err_t result = nvs_flash_init();
    if (result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(log_tag, "NVS partition erased (%s)", esp_err_to_name(result));
        ESP_ERROR_CHECK(nvs_flash_erase());
        result = nvs_flash_init();
    }
    ESP_ERROR_CHECK(result);
}

//--------------------------------------------------------------------------------------------------

bool Settings::read_blob(std::string_view key, std::span<std::byte> bytes)
{
    nvs_handle_t handle = 0;
    if (nvs_open(name_space, NVS_READONLY, &handle) != ESP_OK) {
        return false;  // nothing has been saved yet
    }
    const Key name = key_text(key);
    std::size_t size = 0;
    bool found = nvs_get_blob(handle, name.c_str(), nullptr, &size) == ESP_OK && size == bytes.size();
    if (found) {
        found = nvs_get_blob(handle, name.c_str(), bytes.data(), &size) == ESP_OK;
    }
    nvs_close(handle);
    return found;
}

//--------------------------------------------------------------------------------------------------

bool Settings::write_blob(std::string_view key, std::span<const std::byte> bytes)
{
    nvs_handle_t handle = 0;
    esp_err_t result = nvs_open(name_space, NVS_READWRITE, &handle);
    if (result == ESP_OK) {
        result = nvs_set_blob(handle, key_text(key).c_str(), bytes.data(), bytes.size());
        if (result == ESP_OK) {
            result = nvs_commit(handle);
        }
        nvs_close(handle);
    }
    if (result != ESP_OK) {
        ESP_LOGE(log_tag, "saving %.*s failed: %s", static_cast<int>(key.size()), key.data(), esp_err_to_name(result));
        return false;
    }
    return true;
}

}  // namespace avela
