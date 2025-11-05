#pragma once

#define CUSTOM_CLUSTER_ID        0xFF00
#define CUSTOM_ATTR_ID_STR       0x0000
#define CUSTOM_COMMAND_REQ       0x0000
#define CUSTOM_COMMAND_RESP      0x0001
#define CUSTOM_SERVER_ENDPOINT   0x01
#define CUSTOM_CLIENT_ENDPOINT   0x10

typedef struct __attribute__((packed)) {
    uint8_t estado_incendio;  // 0 = apagado, 1 = encendido
    uint16_t temperatura;     // en décimas de grado (por ejemplo, 253 = 25.3 °C)
    uint16_t humo;            // ppm o valor relativo
    uint16_t co2;             // ppm
} sensor_data_t;