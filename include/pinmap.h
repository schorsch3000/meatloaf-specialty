/* FujiNet Hardware Pin Mapping */
#ifndef PINMAP_H
#define PINMAP_H

// ESP32 WROVER
#include "pinmap/fujinet-v1.6.h"
#include "pinmap/fujiapple-rev0.h"
#include "pinmap/fujiloaf-rev0.h"
#include "pinmap/lolin-d32-pro.h"

// ESP32 WROOM
#include "pinmap/ttgo-t1.h"
#include "pinmap/esp-wroom-32.h"
#include "pinmap/esp-wroom-32-pi1541.h"

// ESP32 S2/S3
#include "pinmap/lolin-s2-mini.h"
#include "pinmap/esp32s3.h"

#ifndef LED_STRIP
/* LED Strip NEW */
#define NUM_LEDS            5
#define LED_DATA_PIN        4
#define LED_BRIGHTNESS      15 // max mA the LED can use determines brightness
#define LED_TYPE            WS2812B
#define RGB_ORDER           GRB
// LED order on the strip starting with 0
#define LED_WIFI_NUM        0
#define LED_BUS_NUM         4
#define LED_BT_NUM          2
#endif

#endif // PINMAP_H