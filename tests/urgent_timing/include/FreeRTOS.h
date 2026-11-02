#pragma once
#include <cstdint>
using TickType_t = uint32_t;
using BaseType_t = int;
constexpr BaseType_t pdTRUE = 1;
constexpr BaseType_t pdFALSE = 0;
constexpr BaseType_t pdPASS = 1;
constexpr TickType_t portMAX_DELAY = 0xffffffffu;
constexpr TickType_t configTICK_RATE_HZ = 1024u;
#define pdMS_TO_TICKS(ms) static_cast<TickType_t>((static_cast<uint64_t>(ms) * configTICK_RATE_HZ) / 1000u)
