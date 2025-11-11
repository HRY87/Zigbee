# Cliente Zigbee — Red de alarmas contra incendios

Resumen
- Firmware para dispositivos finales Zigbee usados en detección de incendios.
- El dispositivo permanece en bajo consumo y sólo despierta para procesar:
  - un evento local (humo, temperatura, tamper, batería) o
  - una petición explícita del coordinador para reportar estado.

Entorno y dependencias
- ESP-IDF 5.3.4 (probado con esta versión).
- Librerías Zigbee: esp-zboos-lib y esp-zigbee-lib.
- Hardware: SoC ESP32 compatible y coordinador Zigbee/bridge.

Comportamiento principal
- Modo bajo consumo (deep sleep / light sleep). Fuentes de wake típicas: GPIO (interrupción de sensor), timer o petición desde el coordinador.
- Al wake: inicializa periféricos mínimos, lee sensores, crea el payload (JSON), transmite al coordinador/backend y vuelve a sleep según política de power_manager.
- Objetivo: minimizar tiempo activo y uso de radio para maximizar autonomía.

Estructura recomendada del repositorio
- main/ : código fuente del firmware (archivos .c y .h).
- components/ : componentes personalizados (si aplica).
- config/ : plantillas de configuración (p. ej. config.example.h o config.json).
- docs/ : documentación y diagramas.
- README.md : este documento.

Compilación y flash (ESP-IDF)
1. Instalar y configurar ESP-IDF 5.3.4 según la guía oficial.
2. En la raíz del proyecto:
   - idf.py set-target <target>
   - idf.py menuconfig   (ajustar particiones, serial, power management, etc.)
   - idf.py build
   - idf.py -p <PUERTO> flash
   - idf.py -p <PUERTO> monitor

Notas de depuración y consumo
- Revisar en el monitor:
  - inicialización del stack Zigbee (esp-zigbee-lib),
  - eventos de wake/sleep,
  - logs de transmisión y reintentos.
- Para evaluar consumo real usar medidor de corriente y habilitar trazas de power-management.
- Si no responde a peticiones del coordinador: comprobar fuentes de wake configuradas, permisos de puerto serie y compatibilidad de versiones de librerías.

Buenas prácticas
- Mantener ventanas de awake cortas y deterministas.
- Implementar reintentos con backoff y persistencia limitada para eventos críticos.
- Documentar formatos de payload (ej. JSON) y tópicos si se usa MQTT.
- Añadir tests unitarios para handlers y un simulador de sensores.

Descripción de archivos en main/
- A continuación se describen los archivos principales del firmware. Son ejemplos de responsabilidades y API recomendadas; varios elementos son específicos de la aplicación y no los proporciona esp-zigbee-lib ni esp-zboos-lib.

  - main.c
    - Punto de entrada (app_main). Inicializa logging, power_manager, drivers de sensores, stack Zigbee (vía zigbee_client) y utilidades JSON.
    - Registra callbacks y controla la secuencia wake → procesar evento → enviar reporte → sleep.
    - Funciones/claves: app_main(), inicialización de subsistemas, manejo de errores críticos.

  - config.h
    - Parámetros de build-time: pines, thresholds, timeouts, intervalos de reporte y macros de debug.
    - Mejor: mover opciones editables a Kconfig/menuconfig y dejar defines mínimos aquí.

  - json_utils.c / json_utils.h
    - Serialización/deserialización de payloads (p. ej. cJSON).
    - Construye reportes JSON (device id, tipo evento, valor sensor, timestamp, batería) y parsea comandos recibidos.
    - Funciones sugeridas: json_build_report(...), json_parse_command(...).

  - zigbee_client.c / zigbee_client.h
    - Responsabilidad:
      - Inicializa y configura el stack Zigbee (esp-zigbee-lib) y traduce entre el stack y la aplicación.
      - NOTA: la implementación de clusters y endpoints personalizados suele ser responsabilidad de la aplicación; las librerías sólo proveen las APIs de stack.
    - API recomendada:
      - esp_err_t zigbee_client_init(void);
      - esp_err_t zigbee_client_register_endpoint(uint8_t ep, const endpoint_descriptor_t *desc);
      - esp_err_t zigbee_client_send_report(uint8_t ep, uint16_t cluster_id, const void *payload, size_t len);
      - void zigbee_client_set_command_handler(command_handler_t handler);
    - Recomendaciones:
      - Registrar endpoints y clusters en zigbee_client_init(); si se necesita ahorro de energía, diferir partes pesadas hasta el primer wake.
      - No bloquear en handlers del stack: reenviar mensajes a una cola y procesar en una tarea propia.
      - Manejar confirmaciones/ACK según la topología (la librería puede exponer eventos, pero la política de reintentos es de la app).

  - custom_cluster_defs.h
    - Propósito:
      - Definir IDs de clusters y atributos personalizados usados por tu aplicación (p. ej. cluster para control de LED RGB, comandos específicos de alarma).
      - Estos definiciones no están en esp-zigbee-lib; son aplicación-específicas.
    - Ejemplo mínimo:
      - #ifndef CUSTOM_CLUSTER_DEFS_H
      - #define CUSTOM_CLUSTER_DEFS_H
      - /* Cluster IDs (usar rangos correctos según especificación Zigbee o manufacturer specific) */
      - #define CLUSTER_ID_ALARM_CUSTOM        0xFC00
      - #define CLUSTER_ID_RGB_CONTROL         0xFC01
      - /* Atributos y comandos */
      - #define ATTR_ID_RGB_COLOR              0x0000
      - #endif
    - Recomendaciones:
      - Documentar el formato de payloads (endianness, layout) y mantener compatibilidad con el coordinador.
      - Registrar estos clusters/atributos al crear endpoints en zigbee_client.

  - sensors.c / sensors.h
    - Responsabilidad:
      - Lectura y filtrado de sensores: humo, temperatura, tamper, ADC para batería.
      - Detección de umbrales, debouncing y generación de eventos que notifiquen a la aplicación.
    - API sugerida:
      - esp_err_t sensors_init(void);
      - esp_err_t sensors_read(sensor_id_t id, sensor_value_t *out);
      - esp_err_t sensors_register_callback(sensor_event_cb_t cb, void *ctx);
    - Consideraciones:
      - Para ahorro de energía, configurar interrupciones GPIO para wake en sensores digitales; para ADC, usar comparadores/hardware si está disponible.
      - Hacer sampling y debounce cortos y parametrizables.

  - power_manager.c / power_manager.h
    - Responsabilidad:
      - Abstracción de modos de bajo consumo, configuración de fuentes de wake (GPIO, timer) y política de awake (duración mínima/máxima).
    - API sugerida:
      - esp_err_t power_manager_init(void);
      - esp_err_t power_manager_request_awake(uint32_t ms);
      - esp_err_t power_manager_enter_sleep(void);
      - wake_reason_t power_manager_get_wake_reason(void);
    - Puntos clave:
      - Implementar timeout de awake (watchdog) para evitar quedarse despierto indefinidamente.
      - Coordinar con zigbee_client para asegurar que la radio está lista antes de transmitir.
      - Exponer métricas (contador de wakes, tiempo awake acumulado) para optimización.

  - comms.c / comms.h (opcional)
    - Responsabilidad:
      - Comunicación con backend fuera de la red Zigbee (MQTT/HTTP) si aplica.
      - Normalizar payloads (json_utils) y gestionar reconexiones y QoS.
    - API sugerida:
      - esp_err_t comms_init(void);
      - esp_err_t comms_send(const char *topic, const void *payload, size_t len);
      - esp_err_t comms_wait_ack(uint32_t timeout_ms);
    - Recomendaciones:
      - Diseñar para operar en ventanas awake cortas; evitar reconexiones largas en la ruta crítica.
      - Usar backoff exponencial para reintentos fuera de la ventana active si el backend no responde.

  - rgb_led.c / rgb_led.h
    - Propósito:
      - Driver de control para un LED RGB (PWM o control por driver externo).
      - Esta funcionalidad es de aplicación y no forma parte de esp-zigbee-lib.
    - API sugerida:
      - esp_err_t rgb_init(void);
      - esp_err_t rgb_set_color(uint8_t r, uint8_t g, uint8_t b);
      - esp_err_t rgb_set_brightness(uint8_t level);
      - void rgb_deinit(void);
    - Consideraciones:
      - Implementar modos de ahorro (apagar LED en sleep).
      - Si RGB se usa como indicador de alarma, sincronizar con la política de power_manager para evitar wakes innecesarios.
      - Si se expone vía Zigbee (cluster custom), json_utils/zigbee_client deben conocer el formato de comando.

Plantilla rápida para documentar archivos reales
- Reemplazar cada entrada con:
  - <archivo.c> / <archivo.h> : responsabilidad breve y funciones públicas principales.

Contacto / notas finales
- Este README es una guía funcional y de arquitectura. Para que sea exhaustivo, copia la sección "Descripción de archivos en main/" y completa las descripciones con los nombres exactos y funciones públicas de tus archivos en main/.
