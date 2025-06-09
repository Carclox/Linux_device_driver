/*
 * Aplicación de usuario para interactuar con el controlador de LED GPIO del kernel.
 * Permite encender, apagar o leer el estado de un LED conectado al GPIO 23
 * a través del dispositivo de caracteres /dev/rpi_led.
 *
 * Uso:
 * ./app on           - Enciende el LED
 * ./app off          - Apaga el LED
 * ./app status       - Lee y muestra el estado actual del LED
 */



#include <stdio.h>   // Para printf, fprintf, perror
#include <stdlib.h>  // Para system, EXIT_FAILURE, EXIT_SUCCESS
#include <string.h>  // Para strcmp
#include <fcntl.h>   // Para open, O_RDWR
#include <unistd.h>  // Para close, read, write
#include <errno.h>   // Para errno

// --- Definiciones ---
#define DEVICE_PATH     "/dev/rpi_led"

#define DRIVER_PATH     "/home/PI/Linux_device_driver/obj/driver_led.ko"

// --- Prototipos de funciones ---
void cargar_driver();
void descargar_driver(); // Añadida para completar la limpieza
int app(int argc, char *argv[]); // Cambiado el nombre de los argumentos para mayor claridad

// --- Implementación de cargar_driver ---
void cargar_driver() {
    printf("Cargando driver al sistema...\n");
    char comando[256];
    snprintf(comando, sizeof(comando), "sudo insmod %s 2>&1", DRIVER_PATH);

    int resultado = system(comando);

    // system() devuelve el código de salida del comando.
    // 0 para éxito, o un valor diferente si hubo un error.
    // Si 'insmod' devuelve 1, system() suele devolver 256.
    if (resultado == 0) {
        printf("Driver cargado con exito.\n");
    } else if (resultado == 256) { // Ejemplo de un error común de insmod (module already loaded)
        printf("Advertencia: El driver 'driver_led.ko' ya estaba cargado o hubo un error menor. Código de salida: %d\n", resultado);
    } else {
        printf("Error al cargar el driver 'driver_led.ko'. Código de salida: %d\n", resultado);
        perror("Detalles del error"); // Mostrar detalles del error si system falló (-1)
    }
}

// --- Implementación de descargar_driver ---
// Función para limpiar el driver del kernel
void descargar_driver() {
    printf("Descargando driver del sistema...\n");
    char comando[256];
    snprintf(comando, sizeof(comando), "sudo rmmod driver_led 2>&1"); // Usar el nombre del módulo, no la ruta

    int resultado = system(comando);

    if (resultado == 0) {
        printf("Driver descargado con exito.\n");
    } else if (resultado == 256) { // Ejemplo de un error común de rmmod (module not found)
        printf("Advertencia: El driver 'driver_led' no estaba cargado o hubo un error menor al descargar. Código de salida: %d\n", resultado);
    }
    else {
        printf("Error al descargar el driver 'driver_led'. Código de salida: %d\n", resultado);
        perror("Detalles del error");
    }
}


// --- Implementación de la función principal de la aplicación ---
int app(int argc, char *argv[]) {
    int fd;
    char write_buf[2]; // buffer para escribir '0' o '1' y el terminador nulo si es necesario
    char read_buf[2];  // buffer para leer el estado y el terminador nulo
    ssize_t bytes_written, bytes_read; // Para almacenar el número de bytes leídos/escritos

    // Validar el número de argumentos
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <on | off | status>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // --- Intentar abrir el dispositivo ---
    // Si falla la primera vez, intentar cargar el driver y reintentar.
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Error: No se pudo abrir el dispositivo inicialmente");
        fprintf(stderr, "Asegúrate de que el driver rpi_led esté cargado o intenta cargarlo.\n");

        // Intentar cargar el driver y reintentar abrir el dispositivo
        //cargar_driver();
        fd = open(DEVICE_PATH, O_RDWR); // Reintentar abrir
        if (fd < 0) {
            perror("Error: No se pudo abrir el dispositivo después de intentar cargar el driver");
            fprintf(stderr, "Asegúrate de que el driver 'driver_led.ko' se haya cargado correctamente.\n");
            return EXIT_FAILURE;
        }
        printf("Dispositivo abierto exitosamente después de cargar el driver.\n");
    } else {
        printf("Dispositivo abierto exitosamente.\n");
    }

    // --- Procesar el comando del usuario ---
    if (strcmp(argv[1], "on") == 0) {
        write_buf[0] = '1';
        write_buf[1] = '\0'; // Asegurar terminación nula si el driver lee como cadena
        bytes_written = write(fd, write_buf, 1); // Escribir solo un byte ('1')
        if (bytes_written < 0) {
            perror("Error: Falla al escribir en el dispositivo para encender el LED");
            close(fd);
            return EXIT_FAILURE;
        }
        printf("Comando 'on' enviado. LED encendido.\n");
    } else if (strcmp(argv[1], "off") == 0) {
        write_buf[0] = '0';
        write_buf[1] = '\0'; // Asegurar terminación nula
        bytes_written = write(fd, write_buf, 1); // Escribir solo un byte ('0')
        if (bytes_written < 0) {
            perror("Error: Falla al escribir en el dispositivo para apagar el LED");
            close(fd);
            return EXIT_FAILURE;
        }
        printf("Comando 'off' enviado. LED apagado.\n");
    } else if (strcmp(argv[1], "status") == 0) {
        bytes_read = read(fd, read_buf, sizeof(read_buf) - 1); // Leer hasta 1 byte de datos + espacio para '\0'
        if (bytes_read < 0) {
            perror("Error: Falla al leer del dispositivo");
            close(fd);
            return EXIT_FAILURE;
        }
        read_buf[bytes_read] = '\0'; // Asegurar terminación nula para la cadena
        printf("Estado actual del LED: %s\n", read_buf);
    } else {
        // Comando desconocido
        fprintf(stderr, "Comando no reconocido. Uso: %s <on|off|status>\n", argv[0]);
        close(fd);
        return EXIT_FAILURE;
    }

    // Cerrar el descriptor de archivo cuando la operación haya terminado
    close(fd);
    return EXIT_SUCCESS; // Indicar que la aplicación finalizó exitosamente
}

// --- Función main ---
int main(int argc, char *argv[]) {
    // cargar driver, solo si la app es un superloop
    int status = app(argc, argv);
    // descargar_driver();
    return status; // Devuelve el estado de éxito o fallo de la función app
}


/*
instrucciones

compilar
con makefile

verificar que el driver este cargado
sudo insmod rpi_led_gpio_driver.ko

ejecutar para encender
sudo ./app on

ejecutar para apagar
sudo ./app off

para verificar el estado del pin
sudo ./app status
*/