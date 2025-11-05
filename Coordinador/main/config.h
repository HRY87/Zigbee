#pragma once
#include "esp_zigbee_core.h"

/*-------------------------------------------------------------
 * Configuración Zigbee
 *------------------------------------------------------------*/
#define MAX_CHILDREN              10        /* máximo de dispositivos conectados */
#define INSTALLCODE_POLICY_ENABLE false     /* sin install code para pruebas */
#define ESP_ZB_PRIMARY_CHANNEL_MASK (1 << 15)   // Canal 15 fijo para ambos
#define ESP_ZB_PERMIT_JOIN_DURATION 180         // 180 segundos para permitir uniones
/* Coordinador Zigbee */
#define ESP_ZB_ZC_CONFIG()                                     \
    {                                                          \
        .esp_zb_role = ESP_ZB_DEVICE_TYPE_COORDINATOR,         \
        .install_code_policy = INSTALLCODE_POLICY_ENABLE,      \
        .nwk_cfg.zczr_cfg = { .max_children = MAX_CHILDREN },  \
    }

/* Radio nativa (ESP32-C6 tiene radio Zigbee integrado) */
#define ESP_ZB_DEFAULT_RADIO_CONFIG()                          \
    { .radio_mode = ZB_RADIO_MODE_NATIVE, }

/* Host sin conexión externa */
#define ESP_ZB_DEFAULT_HOST_CONFIG()                           \
    { .host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE, }
