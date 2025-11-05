#include "rgb_led.h"
#include "esp_log.h"
#include "led_strip.h"

static const char *TAG = "RGB_LED";

/* Configuración de tu LED integrado */
#define LED_PIN        8      // GPIO del LED RGB (verifica tu placa, suele ser 8 en el ESP32-C6 DevKit)
#define LED_NUM_PIXELS 1      // cantidad de LEDs en la tira (normalmente 1)

static led_strip_handle_t led_strip;

/* Inicialización */
void rgb_led_init(void)
{
    ESP_LOGI(TAG, "Inicializando LED RGB con LED_STRIP driver...");

    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_PIN,
        .max_leds = LED_NUM_PIXELS,
        .led_model = LED_MODEL_WS2812,  // para LED RGB tipo NeoPixel
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_RGB,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "LED RGB inicializado en GPIO %d", LED_PIN);

    // Limpia al inicio
    led_strip_clear(led_strip);
}

/* Cambiar color */
void rgb_led_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, r, g, b));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));
}

/* Colores comunes */
void rgb_led_rojo(void)   { rgb_led_set_color(255, 0, 0); }
void rgb_led_verde(void)  { rgb_led_set_color(0, 255, 0); }
void rgb_led_azul(void)   { rgb_led_set_color(0, 0, 255); }
void rgb_led_apagar(void) { rgb_led_set_color(0, 0, 0); }

