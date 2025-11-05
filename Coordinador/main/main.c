#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "driver/gpio.h"

#include "esp_zigbee_core.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "config.h"
#include "zboss_api.h"
#include "json_utils.h"
#include "rgb_led.h"
#include "custom_cluster_defs.h"

#if !defined ZB_COORDINATOR_ROLE
#error "Define ZB_COORDINATOR_ROLE en menuconfig (menuconfig → Zigbee → Device Type)"
#endif

static esp_err_t zb_custom_cmd_handler(const esp_zb_zcl_custom_cluster_command_message_t *message);

static const char *TAG = "Coordinador";

/*-------------------------------------------------------------
 * Funciones auxiliares
 *------------------------------------------------------------*/
static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    if (esp_zb_bdb_start_top_level_commissioning(mode_mask) != ESP_OK) {
        ESP_LOGW(TAG, "Error al iniciar commissioning, reintentando...");
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
                ESP_LOGI(TAG, "Dispositivo nuevo: formando red...");
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_FORMATION);
            } else {
                ESP_LOGI(TAG, "Coordinador reiniciado: abriendo red para uniones...");
                if (esp_zb_bdb_open_network(180) == ESP_OK) {
                    ESP_LOGI(TAG, "✅ Red abierta para uniones (180 s)");
                }
            }
        } else {
            ESP_LOGE(TAG, "Error iniciando stack Zigbee (%s)", esp_err_to_name(status));
        }
        break;

    case ESP_ZB_BDB_SIGNAL_FORMATION:
        if (status == ESP_OK) {
            esp_zb_ieee_addr_t pan_id;
            esp_zb_get_extended_pan_id(pan_id);
            ESP_LOGI(TAG,
                     "✅ Red formada (PAN_ID: 0x%04hx, Canal: %d, Dirección corta: 0x%04hx)",
                     esp_zb_get_pan_id(),
                     esp_zb_get_current_channel(),
                     esp_zb_get_short_address());

            if (esp_zb_bdb_open_network(180) == ESP_OK) {
                ESP_LOGI(TAG, "✅ Permit join activado (180 s)...");
            }
        } else {
            ESP_LOGW(TAG, "Falló formación, reintentando...");
            esp_zb_scheduler_alarm(
                (esp_zb_callback_t)bdb_start_top_level_commissioning_cb,
                ESP_ZB_BDB_MODE_NETWORK_FORMATION, 1000);
        }
        break;

    case ESP_ZB_ZDO_SIGNAL_DEVICE_ANNCE: {
        esp_zb_zdo_signal_device_annce_params_t *dev_params =
            (esp_zb_zdo_signal_device_annce_params_t *)esp_zb_app_signal_get_params(sg_p);
        ESP_LOGI(TAG, "📡 Nuevo dispositivo unido: short_addr=0x%04hx",
                 dev_params->device_short_addr);
        break;
    }

    default:
        ESP_LOGI(TAG, "Señal Zigbee: %s (0x%x) status:%s",
                 esp_zb_zdo_signal_to_string(sig_type),
                 sig_type, esp_err_to_name(status));
        break;
    }
}

// Primero definís el handler del comando custom
static esp_err_t zb_custom_cmd_handler(const esp_zb_zcl_custom_cluster_command_message_t *message)
{
    if (!message) return ESP_FAIL;

    ESP_LOGI(TAG, "📩 Comando custom recibido del cliente");
    // aquí procesás el payload
    return ESP_OK;
}

/*-------------------------------------------------------------
 * 🎯 Action Handler principal de Zigbee
 *------------------------------------------------------------*/
static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    if (callback_id == ESP_ZB_CORE_CMD_CUSTOM_CLUSTER_REQ_CB_ID) {
        return zb_custom_cmd_handler((const esp_zb_zcl_custom_cluster_command_message_t *)message);
    }

    ESP_LOGW(TAG, "⚠️ Callback Zigbee no manejado (0x%x)", callback_id);
    return ESP_OK;
}


/*-------------------------------------------------------------
 * Tarea Zigbee principal
 *------------------------------------------------------------*/
static void esp_zb_task(void *pvParameters)
{
    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZC_CONFIG();
    esp_zb_init(&zb_nwk_cfg);

    // Crear endpoint con cluster custom
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();
    esp_zb_endpoint_config_t ep_cfg = {
        .endpoint = CUSTOM_SERVER_ENDPOINT,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_CUSTOM_ATTR_DEVICE_ID,
        .app_device_version = 0,
    };

    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();

    // Cluster custom con rol SERVER
    esp_zb_attribute_list_t *custom_cluster = esp_zb_zcl_attr_list_create(CUSTOM_CLUSTER_ID);
    esp_zb_cluster_list_add_custom_cluster(cluster_list, custom_cluster, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    // Clusters básicos
    esp_zb_cluster_list_add_basic_cluster(cluster_list, esp_zb_basic_cluster_create(NULL), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_identify_cluster(cluster_list, esp_zb_identify_cluster_create(NULL), ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);

    // Registrar el endpoint
    esp_zb_ep_list_add_ep(ep_list, cluster_list, ep_cfg);
    esp_zb_device_register(ep_list);

    // Registrar action handler
    esp_zb_core_action_handler_register(zb_action_handler);

    // Canal Zigbee y arranque
    esp_zb_set_primary_network_channel_set(ESP_ZB_PRIMARY_CHANNEL_MASK);
    ESP_ERROR_CHECK(esp_zb_start(false));

    ESP_LOGI(TAG, "🟢 Coordinador Zigbee iniciado correctamente");
    esp_zb_stack_main_loop();
}


/*-------------------------------------------------------------
 * 🚀 app_main
 *------------------------------------------------------------*/
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    rgb_led_init(); // inicializa el LED

    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config  = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    xTaskCreate(esp_zb_task, "zb_task", 8192, NULL, 5, NULL);
}