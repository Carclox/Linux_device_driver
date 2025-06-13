### Funcionamiento de la app: 

Al momento de cargar el módulo se crea el archivo **/dev/rpi_led** que está conectado a un módulo del kernel. Este archivo controla un pin GPIO (536) en donde se conecta el led.

El programa abre el archivo **/dev/rpi_led** usando **open()**, luego envía comandos a través de **write()** o consulta su estado con **read()**.

-Escribe **'1'** para encender el LED.

-Escribe **'0'** para apagarlo.

-Lee el estado del pin GPIO como una cadena ("1" o "0").

Si el usuario no proporciona un argumento o escribe uno incorrecto, la app muestra el uso correcto y termina.