### Funcionamiento del driver:

Permite que el usuario controle el led al modificar el valor de **/dev/rpi_led**, para encender el LED se escribe **1** en el archivo, para apagarlo se ecribe **0**
El comando **cat /dev/rpi_led** lee el estado actual del pin llamando a la función **rpi_led_read**

El valor del pin se ha ajustado para el **pin 24** de la Raspberry Pi Zero 2W.

**rpi_led_init** (Inicialización del Módulo)
Al ejecutar el módulo con el comando insmod:
-Se reserva un número de dispositivo mayor/menor.
-Se crea el dispositivo **/dev/rpi_led.**
-Se solicita acceso al **GPIO 536**, lo configura como salida y lo pone en nivel bajo (LED apagado).

**Pin GPIO del LED (LED_GPIO_PIN):** Define el número de pin GPIO (usando la numeración BCM) al que está conectado el LED. Es crucial que este valor sea correcto para el hardware.  Define el número de pin GPIO (usando la numeración BCM) y se conecta el LED al pin 24 de la Raspberry Pi Zero 2W.

las funciones **rpi_led_open()** y **rpi_led_release()** se llaman para abrir y cerrar el driver, estas solo imprimen mensajes de log para diagnostico.

La función **rpi_led_read()** lee el estado actual del pin GPIO (0 o 1) y devuelve este valor. La función **rpi_led_write()** interpreta el valor del pin GPIO, interpreta "1" como "encender el LED", "0" como "apagar el LED" y en caso de que haya otro caracter devuelve -EINVAL. 
