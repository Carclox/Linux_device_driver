/*
 * Aplicación de usuario para interactuar con el controlador de LED GPIO del kernel.
 * Permite encender, apagar o leer el estado de un LED conectado al GPIO 23
 * a través del dispositivo de caracteres /dev/rpi_led.
 *
 * Uso:
 * ./rpi_led_user_app on       - Enciende el LED
 * ./rpi_led_user_app off      - Apaga el LED
 * ./rpi_led_user_app status   - Lee y muestra el estado actual del LED
 */



 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <fcntl.h> // control de archivos (open)
 #include <unistd.h> // funciones del sistema POSIX (close, read, write)


 // ruta al nodo del dispositivo creado por el kernel
#define DEVICE_PATH "/dev/rpi_led"

int main(int argc, char *argv[]){
    int fd;
    char write_buf[2]; // buffer para escribir 0 o 1
    char read_buf[2]; //  buffer para leer el estado
    ssize_t bytes_written, bytes_read;

    // verificar el numero de argumentos de la linea de comandos
    if (argc != 2){
        fprintf(stderr, "Uso: %s <on|off|status>\n", argv[0]);
        return EXIT_FAILURE;
    }
    //ABRIR EL DISPOSITIVO
    // O_RDWR  abrir para lectura y escritura
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0){
        perror("Error: No se pudo abrir el dispositivo");
        fprintf(stderr, "Asegurate de que el driver rpi_led_driver esté cargado y el dispositivo %s exista\n", DEVICE_PATH);
        return EXIT_FAILURE;
    }

    // procesar el comando del usuario
    if (strcmp(argv[1], "on")==0){
        //comando para encender el led
        write_buf[0] = '1';
        write_buf[1] = '\0'; // terminador nulo
        bytes_written = write(fd, write_buf,1); //escribir solo un byte
        if (bytes_written < 0){
            perror("Error: falla al escribir en el dispositivo para encender el LED");
            close(fd);
            return EXIT_FAILURE;
        }
        printf("Comando 'on' enviado. LED encendido\n");
    } else if (strcmp(argv[1], "off")==0){
        // apagar el led
        write_buf[0] = '0';
        write_buf[1] = '\0';
        bytes_written = write(fd, write_buf,1); // escribir un solo byte
        if (bytes_written < 0){
            perror("Error: falla al escribir en el dispositivo para apagar el LED");
            close(fd);
            return EXIT_FAILURE;
        }
        printf("Comando 'off'enviado. LED apagado.\n");      
    } else if (strcmp(argv[1], "status")==0){
        // leer el esatado del LED
        bytes_read = read(fd, read_buf, sizeof(read_buf)-1); // leer hasta 1 byte de datos
        if (bytes_read < 0){
            perror("Error: Falla al leer del dispositivo");
            close(fd);
            return EXIT_FAILURE;
        }
        read_buf[bytes_read] = '\0' // asegurar terminacion nula para la cadena
        printf("estado actual del LED: %s\n", read_buf);     
    } else {
        // comando desconocido
        fprint(stderr, "Comando no reconocido. Uso: %s <on|off|status>\n",argv[0]);
        close(fd);
        return EXIT_FAILURE;
    }
    //Cerrar el dispositivo
    close(fd);
    printf("dispositivo cerrado.\n");
    return EXIT_SUCCESS;
}


/*
instrucciones
Mejorar 

compilar
gcc rpi_led_user_app.c -o rpi_led_user_app

verificar que el driver este cargado
sudo insmod rpi_led_gpio_driver.ko

ejecutar para encender
sudo ./rpi_led_user_app on

ejecutar para apagar
sudo ./rpi_led_user_app off

para verificar el estado del pin
sudo ./rpi_led_user_app status
*/