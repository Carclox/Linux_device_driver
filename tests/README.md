## test driver 

Este código es un paquete de pruebas automatizadas en C diseñada para verificar el correcto funcionamiento de un driver del kernel de Linux que controla un LED conectado a un GPIO, así como su aplicación de usuario (./app), que permite encender, apagar y consultar el estado del LED.

### Funcionamiento

 Descarga el driver del sistema (para iniciar en limpio).

 Carga el driver del sistema usando insmod.
 
 Ejecuta una serie de pruebas funcionales sobre la aplicación ./app:
 - Encender el LED (./app on) y verifica que la salida sea la esperada.
 - Verificar que el estado del LED sea "1" (./app status).
 - Apagar el LED (./app off) y verificar la salida.
 - Verificar que el estado sea "0".
 - Probar que un comando inválido (./app invalid_command) se maneje correctamente.

 Descarga nuevamente el driver para limpiar el sistema.
 
 Muestra un resumen de resultados al final.

### Comandos adicionales

Se ejecuta con el comando:
sudo ./test_rpi_led

Para verificacion adicional y ver las ultimas 20 lineas del log del kernel:
dmesg | tail -n 20 

Para descargar el modulo:
sudo rmmod rpi_led_gpio_driver


