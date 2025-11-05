#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "nvs_flash.h"

#include "ha/esp_zigbee_ha_standard.h"
#include "config.h"
#include "json_utils.h"
#include "custom_cluster_defs.h"

#if !defined ZB_ED_ROLE
#error Define ZB_ED_ROLE en menuconfig para compilar el End Device Zigbee.
#endif

static const char *TAG = "CLIENTE";

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message);

/*-------------------------------------------------------------
 * Reintento de commissioning si falla
 *------------------------------------------------------------*/
static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    if (esp_zb_bdb_start_top_level_commissioning(mode_mask) != ESP_OK) {
        ESP_LOGW(TAG, "Falla en commissioning, reintentando...");
    }
}

/*-------------------------------------------------------------
 * Manejador de señales Zigbee
 *------------------------------------------------------------*/
void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *sg_p = signal_struct->p_app_signal;
    esp_zb_app_signal_type_t sig_type = *sg_p;
    esp_err_t status = signal_struct->esp_err_status;

    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Inicializando stack Zigbee...");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;

    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (status == ESP_OK) {
            if (esp_zb_bdb_is_factory_new()) {
                ESP_LOGI(TAG, "Dispositivo nuevo, intentando unirse a red...");
            } else {
                ESP_LOGI(TAG, "End device reiniciado, intentando re-unión...");
            }
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
        } else {
            ESP_LOGE(TAG, "Error iniciando Zigbee (%s)", esp_err_to_name(status));
        }
        break;

    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (status == ESP_OK) {
            esp_zb_ieee_addr_t pan_id;
            esp_zb_get_extended_pan_id(pan_id);
            ESP_LOGI(TAG,
                     "✅ Unido exitosamente a red (PAN_ID: 0x%04hx, Canal:%d, Dirección corta:0x%04hx)",
                     esp_zb_get_pan_id(),
                     esp_zb_get_current_channel(),
                     esp_zb_get_short_address());
        } else {
            ESP_LOGW(TAG, "⚠ Falló unión (%s), reintentando en 2s...", esp_err_to_name(status));
            esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb,
                                   ESP_ZB_BDB_MODE_NETWORK_STEERING, 2000);
        }
        break;

    default:
        ESP_LOGI(TAG, "Señal Zigbee: %s (0x%x) status:%s",
                 esp_zb_zdo_signal_to_string(sig_type),
                 sig_type, esp_err_to_name(status));
        break;
    }
}

/*-------------------------------------------------------------
 * Simulación de sensor de incendio
 *------------------------------------------------------------*/
static void sensor_incendio_task(void *pv)
{
    incendio_estado_t estado = ESTADO_APAGADO;

    while (1) {
        // Alterna el estado entre encendido y apagado (simulación)
        estado = (estado == ESTADO_APAGADO) ? ESTADO_ENCENDIDO : ESTADO_APAGADO;

        // Genera JSON
        char *json_str = generar_json(estado);
        ESP_LOGI(TAG, "🔥 JSON generado: %s", json_str);

        // Enviar JSON al coordinador (simulación por ahora)
        enviar_json_a_coordinador(json_str);

        free(json_str);
        vTaskDelay(pdMS_TO_TICKS(10000)); // cada 10 segundos
    }
}

/*-------------------------------------------------------------
 * Tarea principal Zigbee
 *------------------------------------------------------------*/
/* --- Tarea principal Zigbee --- */
static void esp_zb_task(void *pvParameters)
{
    /* 1️⃣ Inicialización del stack Zigbee */
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    esp_zb_init(&zb_nwk_cfg);

    /* 2️⃣ Aquí va la CREACIÓN DEL ENDPOINT CLIENTE */
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();
    esp_zb_endpoint_config_t ep_cfg = {
        .endpoint = CUSTOM_CLIENT_ENDPOINT,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_CUSTOM_ATTR_DEVICE_ID,
        .app_device_version = 0,
    };

    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();
    esp_zb_attribute_list_t *custom_cluster = esp_zb_zcl_attr_list_create(CUSTOM_CLUSTER_ID);

    /* Custom cluster con rol CLIENTE */
    esp_zb_cluster_list_add_custom_cluster(cluster_list, custom_cluster, ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);

    /* Clusters obligatorios (básico e identify) */
    esp_zb_cluster_list_add_basic_cluster(cluster_list, esp_zb_basic_cluster_create(NULL), ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);
    esp_zb_cluster_list_add_identify_cluster(cluster_list, esp_zb_identify_cluster_create(NULL), ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE);

    /* Registrar endpoint */
    esp_zb_ep_list_add_ep(ep_list, cluster_list, ep_cfg);
    esp_zb_device_register(ep_list);

    /* 3️⃣ Registrar el manejador de acciones */
    esp_zb_core_action_handler_register(zb_action_handler);

    /* 4️⃣ Configurar canal y arrancar Zigbee */
    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));

    /* 5️⃣ Loop principal del stack */
    esp_zb_stack_main_loop();
}

static esp_err_t zb_custom_cmd_handler(const esp_zb_zcl_custom_cluster_command_message_t *message)
{
    if (!message) return ESP_FAIL;
    ESP_LOGI(TAG, "📩 Respuesta desde coordinador cmd_id=%d", message->info.command.id);
    ESP_LOG_BUFFER_CHAR(TAG, (uint8_t *)message->data.value + 1, message->data.size - 1);
    return ESP_OK;
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    if (callback_id == ESP_ZB_CORE_CMD_CUSTOM_CLUSTER_REQ_CB_ID)
        return zb_custom_cmd_handler((const esp_zb_zcl_custom_cluster_command_message_t *)message);
    else
        ESP_LOGW(TAG, "Zigbee callback desconocido (0x%x)", callback_id);
    return ESP_OK;
}

/*-------------------------------------------------------------
 * Función principal
 *------------------------------------------------------------*/
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config  = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    // Tareas Zigbee y simulación de sensor
    xTaskCreate(esp_zb_task, "zb_task", 8192, NULL, 5, NULL);
    xTaskCreate(sensor_incendio_task, "sensor_incendio", 4096, NULL, 4, NULL);
}
