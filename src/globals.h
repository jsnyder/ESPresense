#pragma once
#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#if defined(BOARD_HAS_PSRAM)
#include <esp_heap_caps.h>
#endif
#include "Logger.h"
#include "BleFingerprintCollection.h"

// Allocator that places the JSON pool in PSRAM where available, keeping the
// (shared) telemetry/discovery document out of scarce internal SRAM.
#if defined(BOARD_HAS_PSRAM)
// Try PSRAM first, fall back to internal DRAM if PSRAM isn't available yet
// (e.g. during early static-init before bootloader has fully registered the
// PSRAM heap region). MALLOC_CAP_8BIT forces byte-addressable DRAM —
// MALLOC_CAP_INTERNAL can return IRAM which faults on data access.
struct SpiramAllocator {
    void* allocate(size_t size) {
        void* p = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
        if (!p) p = heap_caps_malloc(size, MALLOC_CAP_8BIT);
        return p;
    }
    void deallocate(void* ptr) { heap_caps_free(ptr); }
    void* reallocate(void* ptr, size_t new_size) {
        void* p = heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
        if (!p) p = heap_caps_realloc(ptr, new_size, MALLOC_CAP_8BIT);
        return p;
    }
};
using SharedJsonDocument = BasicJsonDocument<SpiramAllocator>;
// PSRAM is cheap; HA discovery payloads are 500-700 bytes today but creep up
// as entities are added. 8 KB gives comfortable headroom and costs nothing
// worth measuring on WROVER / S3-N8R8.
#define SHARED_JSON_DOC_CAPACITY (8 * 1024)
#else
using SharedJsonDocument = DynamicJsonDocument;
// Internal-SRAM boards can't afford a huge pool; 2 KB is roughly 3x the prior
// 768-byte ceiling and covers HA discovery without silent field-drop.
#define SHARED_JSON_DOC_CAPACITY 2048
#endif

/*----------------------------------------------------------------------------
globals.h

Note: #define VAR_DECLS 1 before including this file to DECLARE and INITIALIZE
global variables.  Include this file without defining VAR_DECLS to extern
these variables.
----------------------------------------------------------------------------*/

/*----------------------------------------------
Setup variable declaration macros.
----------------------------------------------*/
#ifndef VAR_DECLS
# define _DECL extern
# define _INIT(x)
# define _INIT_N(x)
#else
# define _DECL
# define _INIT(x)  = x
# define UNPACK( ... ) __VA_ARGS__
# define _INIT_N(x) UNPACK x
#endif

_DECL String room, id, statusTopic, teleTopic, roomsTopic, setTopic, configTopic;
_DECL AsyncMqttClient mqttClient;
_DECL String homeAssistantDiscoveryPrefix;
_DECL SharedJsonDocument doc _INIT_N(((SHARED_JSON_DOC_CAPACITY)));
_DECL String localIp;
_DECL AsyncWebSocket ws _INIT_N((("/ws")));
_DECL bool enrolling;
_DECL String enrolledId;
_DECL unsigned long enrollingEndMillis;

// I2C
_DECL bool I2C_Bus_1_Started;
_DECL bool I2C_Bus_2_Started;
