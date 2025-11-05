Configuración del Coordinador Zigbee — esp_zb_switch (ESP32-C6)
Descripción

Este proyecto implementa un Zigbee Coordinator (coordinador de red) usando el ESP32-C6.
El coordinador es el nodo principal en una red Zigbee — se encarga de:

Crear la red.
Administrar los dispositivos que se unen (routers y end devices).
Enviar y recibir comandos de control.

⚙️ Requisitos previos

ESP-IDF v5.3.4 o superior
Visual Studio Code con la extensión ESP-IDF
Python 3.8+
ESP32-C6 (compatible con Zigbee)
Cable USB o JTAG

Puerto COM disponible (por ejemplo, COM8 en Windows)

Estructura del proyecto
Coordinador/
├── main/
│   ├── CMakeLists.txt
│   └── main.c
├── CMakeLists.txt
├── sdkconfig
├── partition.csv
└── README.txt

Configuración inicial en VS Code

Abre la carpeta Coordinador/ en Visual Studio Code.
Asegúrate de que la extensión de ESP-IDF esté activa.

En la barra inferior, configurá lo siguiente:

Acción	Descripción	Comando equivalente
Versión de ESP-IDF	Seleccioná v5.3.4 o superior	idf.py --version
Método de Flash	JTAG o UART según tu conexión	(Configuración de extensión)
Puerto COM	Seleccioná tu puerto (ej. COM8)	idf.py -p COM8 flash
Target (Dispositivo)	Seleccioná esp32c6	idf.py set-target esp32c6

Configuración del proyecto (menuconfig)
Abre la terminal ESP-IDF (ícono 💻 o Ctrl + Shift + P → ESP-IDF: Open ESP-IDF Terminal).

Ejecutá:
idf.py menuconfig


Dentro del menú, asegurate de configurar lo siguiente: Zigbee

Ruta: Component config → Zigbee

Opciones:
Enable Zigbee stack
Zigbee role: Coordinator
(Opcional) Configurá canal, PAN ID o TX Power si querés personalizar la red.

Partition Table
Ruta: Partition Table → (Custom partition table CSV)

Seleccioná tu archivo:
partition.csv
Logging (opcional)

Ruta: Component config → Log output
Podés subir el nivel de log a INFO o DEBUG para ver más detalles en el monitor.
Guardá y salí (S → Enter → Q).
Reconfigurá el proyecto: idf.py reconfigure

🧱 Archivo de particiones (partition.csv)
ADVERTENCIA: Para estar esta opcion hay que seleccionar al sdkconfig, en la configuracion "partition table"
Si tu proyecto no lo tiene, podés crear uno básico como este:
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x6000,
phy_init, data, phy,     0xf000,  0x1000,
factory,  app,  factory, 0x10000, 1M,
storage,  data, spiffs,  ,        512K,

Esto asigna espacio para NVS, PHY, la app principal y almacenamiento SPIFFS opcional.

Compilar y flashear (COM8 es un ejemplo)
1)  Compilar el proyecto: idf.py build
2)  Flashear el firmw: idf.py -p COM8 flash
2.1) (Opcional) Borrar y volver a flashear: idf.py -p COM8 erase_flash build flash
3)  Monitorear la salida: idf.py -p COM8 monitor

Salida esperada en el monitor
Si todo está correctamente configurado, deberías ver algo como:
I (1234) ZIGBEE: Zigbee Coordinator started
I (1240) ZIGBEE: PAN ID: 0x1234, Channel: 15
I (1250) APP_MAIN: Waiting for Zigbee devices to join...

Cuando un End Device (por ejemplo, esp_zb_light) se una a la red, aparecerá algo como:
I (3210) ZIGBEE: New device joined, short address: 0x5678

Notas finales
Asegurate de que el cliente (end device) esté configurado en el mismo canal Zigbee.
Si usás dos ESP32-C6, cada uno debe estar conectado a puertos COM distintos.
En caso de fallas de compilación, podés limpiar el proyecto:
idf.py fullclean
idf.py build
