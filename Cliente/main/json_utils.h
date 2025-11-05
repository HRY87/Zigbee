#pragma once
#include <stdio.h>

typedef enum {
    ESTADO_APAGADO = 0,
    ESTADO_ENCENDIDO
} incendio_estado_t;

/**
 * Genera un JSON con el estado actual del sensor de incendio.
 * @param estado Estado actual del sensor.
 * @return Cadena JSON (debe liberarse con free()).
 */
char *generar_json(incendio_estado_t estado);

/**
 * Envía el JSON al coordinador (por ahora simulado por log).
 */
void enviar_json_a_coordinador(char*  json_str);
