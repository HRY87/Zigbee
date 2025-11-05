#include "json_utils.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rgb_led.h"
#include <string.h>

#define GPIO_LED_ALERTA 2  // mismo pin que el coordinador

static const char *TAG = "JSON_UTILS";

/*-------------------------------------------------------------
 * Generación de JSON
 *------------------------------------------------------------*/
char *json_generar_estado(incendio_estado_t estado)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *datos = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "datos", datos);
    cJSON_AddStringToObject(datos, "estado", (estado == ESTADO_ENCENDIDO) ? "encendido" : "apagado");

    cJSON *mensajes = cJSON_CreateArray();
    cJSON_AddItemToArray(mensajes,
                         cJSON_CreateString("Simulación de estado de incendio actualizada"));
    cJSON_AddItemToObject(root, "mensajes", mensajes);

    cJSON *validaciones = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "validaciones", validaciones);
    cJSON_AddNumberToObject(validaciones, "codigo", 200);
    cJSON_AddStringToObject(validaciones, "estado", "OK");

    cJSON_AddItemToObject(root, "errores", cJSON_CreateArray());

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_str; // el caller debe hacer free()
}

/*-------------------------------------------------------------
 * Función JSON → acción en el LED
 *------------------------------------------------------------*/
void procesar_json_recibido(const char *json_str) {
    cJSON *root = cJSON_Parse(json_str);
    if (!root) {
        ESP_LOGE(TAG, "Error parseando JSON");
        return;
    }

    cJSON *datos = cJSON_GetObjectItem(root, "datos");
    if (datos) {
        cJSON *estado = cJSON_GetObjectItem(datos, "estado");
        if (estado && estado->valuestring) {
            ESP_LOGI(TAG, "🔥 Estado incendio recibido: %s", estado->valuestring);
            json_accion_estado(estado->valuestring); // 👈 Llama función separada
        }
    }

    cJSON_Delete(root);
}


/*-------------------------------------------------------------
 * Acción local en función del estado recibido
 *------------------------------------------------------------*/
void json_accion_estado(const char *estado)
{
    if (strcmp(estado, "encendido") == 0) {
        ESP_LOGW(TAG, "🚨 ¡INCENDIO DETECTADO! LED rojo");
        rgb_led_rojo();
    } else if (strcmp(estado, "apagado") == 0) {
        ESP_LOGI(TAG, "✅ Estado normal. LED verde");
        rgb_led_verde();
    } else {
        ESP_LOGW(TAG, "⚠ Estado desconocido: %s", estado);
        rgb_led_azul();
    }
}

