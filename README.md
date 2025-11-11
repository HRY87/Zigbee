# zigbee_comunicacion

Simulación de un sistema de alarma contra incendios utilizando comunicación Zigbee en ESP32-C6 DevKitC-1.

## Descripción

Este proyecto simula una red de alarma contra incendios donde los nodos de detección y alerta se comunican de forma inalámbrica mediante Zigbee, usando módulos ESP32-C6 DevKitC-1 con [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/). La transmisión y recepción de mensajes JSON permiten notificar eventos de emergencia y comandos de control en tiempo real.

Actualmente, el sistema comparte datos (como estados de sensores y comandos de activación de alarmas) entre nodos Zigbee. El proyecto se encuentra en desarrollo: ya existe comunicación Zigbee básica que intercambia mensajes en formato JSON.

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

## Instalación y configuración en Windows

### 1. Instalar ESP-IDF v5.3.4

Sigue la [Guía de Inicio Rápido de ESP-IDF (Windows)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#step-1-set-up-toolchain).

- Descarga el instalador de ESP-IDF para Windows.
- Ejecuta el instalador, selecciona la versión 5.3.4, y sigue las instrucciones.
- Reinicia el PC al finalizar.

### 2. Instalar esp-zigbee-sdk

La integración con esp-zigbee-sdk se realiza mediante el gestor de componentes de ESP-IDF. Consulta la [guía oficial Espressif Zigbee](https://docs.espressif.com/projects/esp-zigbee-sdk/en/latest/get-started/index.html) para ejemplos.

- En la raíz de tu proyecto, asegúrate de tener `idf_component.yml` que incluya:
  ```yaml
  dependencies:
    espressif/esp-zigbee-lib: "^1.0.0"
    espressif/esp-zboss-lib: "^1.0.0"
    # O según la versión recomendada
  ```
- Alternativamente, importa el ejemplo:
  ```bash
  idf.py create-project-from-example "esp-zigbee-sdk/examples/zigbee_something"
  ```

### 3. Clonar este repositorio

```bash
git clone https://github.com/HRY87/Zigbee.git
cd Zigbee
```

### 4. Configurar el proyecto

Abre el proyecto en Visual Studio Code con la extensión Espressif IDF ya instalada.

Asegúrate de haber seleccionado la versión correcta de la herramienta en la barra inferior (ESP-IDF v5.3.4).

Lanza el menú de configuración:
```bash
idf.py menuconfig
```
Selecciona tu placa (`ESP32-C6 DevKitC-1`) y revisa las opciones Zigbee si son relevantes.

### 5. Compilar

```bash
idf.py build
```

### 6. Flashear el firmware

Conecta la placa que vayas a programar. Asegúrate de elegir el puerto COM correspondiente.

```bash
idf.py -p COMx flash
```
> Reemplaza `COMx` con el número de puerto adecuado (por ejemplo, COM3).

### 7. Monitorizar la ejecución

```bash
idf.py -p COMx monitor
```
Para salir del monitor usa `Ctrl+]`.

---

## Referencias útiles

- [Documentación oficial ESP Zigbee SDK](https://docs.espressif.com/projects/esp-zigbee-sdk/en/latest/)
- [Guía de inicio rápido ESP-IDF en Windows](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#windows)

---

## Ejemplo de mensaje JSON

```json
{
  "type": "alert",
  "sensor": "humosala",
  "status": "incendio",
  "timestamp": "2025-11-11T18:30:00Z"
}
```

---

## Estado del proyecto

- [x] Comunicación Zigbee básica entre nodos ESP32-C6 DevKitC-1
- [x] Intercambio de mensajes JSON
- [ ] Lógica completa de alarmas y sensores
- [ ] Integración avanzada MQTT
- [ ] Documentación detallada y ejemplos avanzados


**Desarrollado por HRY87 · Proyecto académico y experimental**
