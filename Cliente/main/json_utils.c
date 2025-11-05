#include "json_utils.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include "custom_cluster_defs.h"
#include "esp_zigbee_core.h"
#include "zcl/esp_zigbee_zcl_command.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "custom_cluster_defs.h"

static const char *TAG = "JSON_UTILS";

/*-------------------------------------------------------------
 * Generar JSON según el estado del incendio
 *------------------------------------------------------------*/
char *generar_json(incendio_estado_t estado)
{
    const char *estado_str = (estado == ESTADO_ENCENDIDO) ? "encendido" : "apagado";

    cJSON *root = cJSON_CreateObject();
    cJSON *datos = cJSON_CreateObject();
    cJSON_AddStringToObject(datos, "estado", estado_str);
    cJSON_AddItemToObject(root, "datos", datos);

    cJSON *mensajes = cJSON_CreateArray();
    cJSON_AddItemToArray(mensajes, cJSON_CreateString("Lectura de sensor procesada correctamente"));
    cJSON_AddItemToObject(root, "mensajes", mensajes);

    cJSON *validaciones = cJSON_CreateObject();
    cJSON_AddNumberToObject(validaciones, "codigo", 200);
    cJSON_AddStringToObject(validaciones, "estado", "OK");
    cJSON_AddItemToObject(root, "validaciones", validaciones);

    cJSON_AddItemToObject(root, "errores", cJSON_CreateArray());

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json_str; // el usuario debe liberar
}

/*-------------------------------------------------------------
 * Enviar JSON al coordinador (simulado por ahora)
 *------------------------------------------------------------*/
void enviar_datos_a_coordinador(sensor_data_t *data)
{
    esp_zb_zcl_custom_cluster_cmd_req_t req = {0};

    req.zcl_basic_cmd.dst_addr_u.addr_short = 0x0000;
    req.zcl_basic_cmd.dst_endpoint = CUSTOM_SERVER_ENDPOINT;
    req.zcl_basic_cmd.src_endpoint = CUSTOM_CLIENT_ENDPOINT;
    req.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    req.cluster_id = CUSTOM_CLUSTER_ID;
    req.profile_id = ESP_ZB_AF_HA_PROFILE_ID;
    req.direction = ESP_ZB_ZCL_CMD_DIRECTION_TO_SRV;
    req.custom_cmd_id = CUSTOM_COMMAND_REQ;
    req.data.type = ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING;
    req.data.size = sizeof(sensor_data_t);
    req.data.value = (uint8_t *)data;

    esp_zb_lock_acquire(portMAX_DELAY);
    esp_zb_zcl_custom_cluster_cmd_req(&req);
    esp_zb_lock_release();

    ESP_LOGI("ZB_NODE", "📡 Datos enviados: estado=%u temp=%u humo=%u co2=%u",
             data->estado_incendio, data->temperatura, data->humo, data->co2);
}