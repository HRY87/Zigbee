# Pruebas con ESP-IDF y Zigbee

Simulación de un sistema de alarma contra incendios utilizando comunicación Zigbee en ESP32-C6 DevKitC-1.

## Tecnologías y componentes utilizados

- **Placa**: ESP32-C6 DevKitC-1 (tanto para dispositivo cliente como para coordinador)
- **Framework**: [ESP-IDF v5.3.4](https://github.com/espressif/esp-idf/releases/tag/v5.3.4)
- **SDK Zigbee**: [esp-zigbee-sdk](https://docs.espressif.com/projects/esp-zigbee-sdk/en/latest/)
- **IDE recomendado**: Visual Studio Code con extensión Espressif IDF
- **Librerías/componentes:**
  - `esp-zigbee-lib`
  - `esp-zboss-lib`
  - `mqtt`
  - `cjson`
  - `led_strip`
- **Lenguaje**: C

---

## Requisitos previos (Windows)

1. **Placas ESP32-C6 DevKitC-1** (al menos dos).
2. **Cable USB para cada placa**.
3. **Windows 10 o superior**.
4. **Visual Studio Code** instalado.
5. **Extensión "Espressif IDF"** en Visual Studio Code ([descargar aquí](https://marketplace.visualstudio.com/items?itemName=espressif.esp-idf-extension)).
6. **Python 3.8+** instalado y añadido al entorno de sistema.

---
