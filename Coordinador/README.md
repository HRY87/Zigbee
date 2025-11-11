# Coordinador Zigbee (ESP32-C6)

Descripción
----------
Este proyecto implementa un coordinador Zigbee en un ESP32-C6. Su función principal es iniciar el stack Zigbee para que se conecten los nodos EndDevice y comunicar eventos/consultas con un servidor MQTT usando mensajes JSON.

Requisitos
----------
- ESP-IDF 5.3.4
- Target: ESP32-C6
- Componentes necesarios:
  - esp-zboss-lib
  - esp-zigbee-lib
- Hardware: placa ESP32-C6 y antena/ alimentación adecuada
- Servidor MQTT (p. ej. Mosquitto) accesible desde la red del microcontrolador

Estructura del proyecto
-----------------------
- main.c (punto de entrada)
- *.c / *.h (módulos Zigbee, MQTT, utilidades)
- components/esp-zboss-lib
- components/esp-zigbee-lib
- CMakeLists.txt, sdkconfig

Instalación de ESP-IDF (resumen)
--------------------------------
1. Instalar ESP-IDF v5.3.4 siguiendo la guía oficial: https://docs.espressif.com
2. En Linux/macOS:
   - git clone --branch v5.3.4 https://github.com/espressif/esp-idf.git $HOME/esp/esp-idf
   - source $HOME/esp/esp-idf/export.sh
3. En Windows (PowerShell): usar la herramienta de instalación oficial y activar el entorno.

Agregar dependencias
--------------------
- Colocar esp-zboss-lib y esp-zigbee-lib en la carpeta components/ o agregarlos como submódulos.
  Ejemplo (desde la raíz del repo):
  - git submodule add <url_esp-zboss-lib> components/esp-zboss-lib
  - git submodule add <url_esp-zigbee-lib> components/esp-zigbee-lib

Configuración (idf.py menuconfig)
---------------------------------
- idf.py set-target esp32c6
- idf.py menuconfig
  - Configurar parámetros de red Wi‑Fi
  - Configurar broker MQTT (URI, usuario/clave si aplica)
  - Configurar opciones del stack Zigbee (rol: Coordinator)

Compilación y flasheo
---------------------
1. Establecer target:
   idf.py set-target esp32c6
2. Build:
   idf.py build
3. Flashear (ejemplo puerto COM en Windows o /dev/ttyUSB0 en Linux):
   idf.py -p COM3 flash monitor
   (Remplazar COM3 por el puerto correcto)

MQTT y mensajes JSON
--------------------
- Topics (ejemplo):
  - Publicación de telemetría: zigbee/telemetry
  - Comandos/consultas: zigbee/command
  - Respuestas: zigbee/response

- Ejemplo: Telemetría publicada por el coordinador
  Topic: zigbee/telemetry
  Payload:
  {
    "device": "sensor_01",
    "type": "temperature",
    "value": 23.5,
    "ts": 1680000000
  }

- Ejemplo: Consulta de usuario
  Topic: zigbee/command
  Payload:
  {
    "cmd": "read",
    "device": "sensor_01",
    "attribute": "battery"
  }
  Respuesta por zigbee/response:
  {
    "device": "sensor_01",
    "attribute": "battery",
    "value": 95
  }

Buenas prácticas y notas
------------------------
- Asegurar que el broker MQTT sea accesible desde la red del ESP.
- Revisar logs del monitor serial para debugging del stack Zigbee (formas de activar logs en menuconfig).
- Mantener las bibliotecas esp-zboss-lib y esp-zigbee-lib actualizadas y compatibles con ESP-IDF v5.3.4.
- Para pruebas locales usar herramientas como mosquitto_sub / mosquitto_pub o MQTT Explorer.

Solución de problemas
---------------------
- Si build falla, verificar versión de ESP-IDF y rutas de componentes.
- Si el dispositivo no se une, revisar antena, alimentación y configuración Zigbee (canal/pan id).
- Consultar la documentación de esp-zigbee-lib para llamadas y callbacks específicos del stack.

Descripción de archivos (.c / .h)
--------------------------------
A continuación se describen los archivos y módulos más relevantes del proyecto. Estas explicaciones ayudan a entender responsabilidades y puntos de extensión.

- main.c
  - Responsabilidad: Punto de entrada de la aplicación.
  - Hace: Inicializa ESP-IDF, red Wi‑Fi, cliente MQTT y stack Zigbee; configura callbacks/globales; crea tareas principales.
  - Funciones clave: app_main(), init_system(), zb_event_dispatcher(), mqtt_command_dispatch().
  - Nota: Mantener lógica de orquestación aquí; delegar detalles a módulos específicos.

- zigbee.c / zigbee.h
  - Responsabilidad: Encapsular interacción con esp-zigbee-lib / esp-zboss-lib.
  - Hace: Inicializar stack Zigbee, configurar rol Coordinator, manejar eventos Zigbee (uniones, reportes, solicitudes), gestionar tabla de dispositivos Zigbee.
  - Funciones clave (p. ej.):
    - int zigbee_init(void);
    - void zigbee_start_coordinator(void);
    - void zigbee_handle_event(const zb_buf_t *buf);
  - zigbee.h: tipos públicos, callbacks registration, constantes (canal, PAN ID).

- mqtt.c / mqtt.h
  - Responsabilidad: Gestionar conexión MQTT, publicaciones y suscripciones.
  - Hace: Crear y mantener cliente MQTT, reconexiones, serializar/deserializar JSON, exponer helpers para publish/subscribe.
  - Funciones clave (p. ej.):
    - esp_err_t mqtt_init(void);
    - esp_err_t mqtt_publish_json(const char *topic, const char *payload);
    - void mqtt_subscribe_command(const char *topic);
  - mqtt.h: topics por defecto (zigbee/telemetry, zigbee/command, zigbee/response).

- devices.c / devices.h
  - Responsabilidad: Registro y gestión de dispositivos Zigbee conocidos.
  - Hace: Añadir/quitar dispositivos, mapear EUI64 <-> ID interno, almacenar atributos (tipo, última lectura, estado de enlace).
  - Funciones clave: device_register(), device_find_by_eui(), device_update_attribute().

- json_utils.c / json_utils.h
  - Responsabilidad: Construcción y parsing de mensajes JSON (usa cJSON u otra librería).
  - Hace: Helpers para formar payloads de telemetría, respuestas a comandos y parsing de peticiones entrantes.
  - Funciones clave: json_build_telemetry(...), json_parse_command(...).

- config.h
  - Responsabilidad: Parámetros de compilación y configuración por defecto.
  - Contiene defines como MQTT_BROKER_URI, ZIGBEE_CHANNEL, PAN_ID, DEVICE_PREFIX y timeouts.
  - Nota: Valores sensibles o configurables en tiempo de ejecución deben mantenerse en menuconfig o NVS.

- logger.c / logger.h
  - Responsabilidad: Wrappers de logging para estandarizar formato y niveles.
  - Hace: macros/log helpers que usan esp_log para incluir módulo y nivel.

- platform.c / platform.h
  - Responsabilidad: Abstracción de plataforma (GPIO, LEDs, botones, NVS/almacenamiento).
  - Hace: Inicializar hardware específico (p. ej. leds de estado), leer botones de reset, persistir configuración mínima (NVS).

- helpers/ota.c (si aplica)
  - Responsabilidad: Actualización OTA del firmware.
  - Hace: Descargar nuevo binario desde servidor, validar y aplicar actualización.

Dónde añadir nuevos módulos
---------------------------
- Añadir nuevo .c/.h en la carpeta main/ o en components/ y actualizar CMakeLists.txt.
- Mantener separación de responsabilidades: lógica de dominio en módulos específicos; orquestación en main.c.

Puntos de integración importantes
---------------------------------
- Callbacks Zigbee -> mqtt: Cuando llega un reporte Zigbee, zigbee.c construye JSON y llama a mqtt_publish_json().
- Comandos MQTT -> Zigbee: mqtt.c parsea comandos JSON y llama a zigbee API (p. ej. read attribute, write attribute).
- Persistencia: devices.c puede sincronizarse a NVS para mantener lista de dispositivos después de reinicio.
