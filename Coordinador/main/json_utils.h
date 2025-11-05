#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include "esp_err.h"
#include "cJSON.h"

/* Estados simulados del incendio */
typedef enum {
    ESTADO_APAGADO = 0,
    ESTADO_ENCENDIDO
} incendio_estado_t;

/* Genera un JSON con el estado del incendio */
char *json_generar_estado(incendio_estado_t estado);

/* Procesa un JSON recibido y actúa según el contenido */
esp_err_t json_procesar_recibido(const char *json_str);

/* Ejecuta acción local según el estado (por ejemplo LED o log) */
void json_accion_estado(const char *estado);

#endif // JSON_UTILS_H
