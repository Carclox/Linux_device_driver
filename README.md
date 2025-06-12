# Linux_device_driver
Tarea 2 - Programación de Sistemas Embebidos Linux

## Integrantes:
* Carlos S. Rangel | 1005065786
* Valeria Garcia Rodas | 1192801328

### Propósito general

El objetivo principal de este proyecto es desarrollar la aplicación **rpi_led_driver.c** en lenguaje C que controla un LED conectado a GPIO. El controlador implementa operaciones **init, exit, open, release, read y write**, y debe tener una interfaz de archivos /dev para la interación de usuario.

### Funcionamiento de la app: 

Al momento de cargar el módulo se crea el archivo **/dev/rpi_led** en el directorio. Este archivo controla un pin GPIO (536) en donde se conecta el led.

**rpi_led_init** (Inicialización del Módulo)

Al ejecutar el módulo con el comando insmod:
-Se reserva un número de dispositivo mayor/menor.
-Se crea el dispositivo /dev/rpi_led.
-Se solicita acceso al GPIO 536, lo configura como salida y lo pone en nivel bajo (LED apagado).

las funciones rpi_led_open() y rpi_led_release() se llaman para abrir y cerrar el driver, estas solo imprimen mensajes de log para diagnostico.

### Funcionamiento del driver:

Permite que el usuario controle el led al modificar el valor de **/dev/rpi_led**, para encender el LED se escribe **1** en el archivo, para apagarlo se ecribe **0**
El comando **cat /dev/rpi_led** lee el estado actual del pin llamando a la función **rpi_led_read**

