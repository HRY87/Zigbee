README — ESP32-C6 Zigbee Ejemplo con Clustes personalizado

Descripción general
Este repositorio contiene dos proyectos independientes desarrollados con ESP-IDF v5.3.4 o superior para el microcontrolador ESP32-C6, enfocados en la comunicación Zigbee.
Cada proyecto se compila por separado y cumple una función diferente dentro de una red Zigbee:

Proyecto     |   Rol Zigbee	 |                    Descripción breve
Coordinador  | Coordinator   |  Crea la red Zigbee y permite que los dispositivos se unan. Puede enviar comandos de control por medio del boton boot
Cliente	     | End Device 	 |   Se une a una red existente y responde a los comandos del coordinador (simula una luz Zigbee).

Estructura general del repositorio
ZigbeeEjemplo/
Coordinador/            → Ejemplo de Coordinador Zigbee
	main/               → Código fuente principal (main.c)
	components/         → Componentes personalizados
	CMakeLists.txt      → Configuración de build del proyecto
	sdkconfig           → Configuración generada por ESP-IDF
	partition.csv       → Tabla de particiones (si aplica)
	README.txt          → Instrucciones específicas del coordinador
Cliente/                → Ejemplo de Cliente (End Device)
	main/               → Código fuente principal (main.c)
	components/         → Componentes personalizados
	CMakeLists.txt      → Configuración de build del proyecto
	sdkconfig           → Configuración generada por ESP-IDF
	partition.csv       → Tabla de particiones (si aplica)
	README.txt          → Instrucciones específicas del cliente
CMakeLists.txt          → Archivo raíz (opcional si se usa como contenedor)
README.txt              → Este archivo (documentación general)

Requisitos generales
-ESP-IDF v5.3.4 o superior
-Visual Studio Code con la extensión ESP-IDF
-Python 3.8+
-ESP32-C6 (chip con soporte Zigbee nativo)
-Cable USB o JTAG
-Puertos COM identificados (por ejemplo COM8 y COM9 si usás dos dispositivos)

Cómo usar los proyectos
Cada carpeta (Coordinador y Cliente) es un proyecto ESP-IDF independiente.
Por lo tanto, se abren, configuran, compilan y flashean por separado.

Para abrir un proyecto:
1) Abre Visual Studio Code.
2) Selecciona Archivo → Abrir carpeta.
3) Elige una de las siguientes carpetas:
   esp_zb_switch/ → Coordinador
   esp_zb_light/ → Cliente

Cada una contiene su propio sdkconfig y partition.csv.

Flujo de trabajo recomendado

Coordinador:
1) Abrir y compilar el Coordinador: Abre la carpeta Coordinador/.
2) Selecciona el target: idf.py set-target esp32c6
3) Configurá el Zigbee como Coordinator: idf.py menuconfig
4) Compilá y flasheá: idf.py -p COM8 build flash monitor (COM8 es un ejemplo)

Cliente:
1) Abrir y compilar el Cliente: Abre la carpeta Cliente/.
2) Selecciona el mismo target: idf.py set-target esp32c6
3) Configurá el Zigbee como End Device: idf.py menuconfig
4) Compilá y flasheá: idf.py -p COM9 build flash monitor (COM9 es un ejemplo)

Verificar la comunicación: 
1) En el monitor del Coordinador deberías ver: Device joined: 0x1234
2) En el monitor del Cliente: Joined network successfully

Ejemplos incluidos


Buenas prácticas de repositorio
Subir a GitHub:
/main/
/components/
CMakeLists.txt
partition.csv
sdkconfig
README.txt

No subir:
/build/
/managed_components/
/sdkconfig.old/

Consejos importantes:
Si modificás configuraciones en menuconfig, no te olvides de ejecutar "idf.py reconfigure" para que note los cambios.
Usar diferentes puertos COMx para flashear coordinador y cliente, para evitar errores
