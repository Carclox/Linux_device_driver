//
// Created by el_carclox on 8/06/2025.
//

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unisted.h>  // para funciones del sistema posix


#define DEVICE_NODE "/dev/rpi_led"


int main(){
    int fd;  // descriptor del archivo del dispositivo
    char buffer[2];
    ssize_t bytes_read;    //variable para almacenar le numero de bytes

    printf("___Iniciando prueba del driver rpi_led___\n";

    // 1. abrir el dispositivo
    fd = open(DEVICE_NODE, O_RDWR);
    if (fd < 0){
        perror("Error al abrir el dispositivo %s", DEVICE_NODE);
        printf("Asegurate de que el modulo del driver este cargado y el nodo del dispositivo este abierto");
        return EXIT_FAILURE;
    }
    printf("Dispositivo %s abierto con exito (fd: %d). %n", DEVICE_NODE,fd);
    // 2 encender led
    if (write(fd, "1", 1) < 0){
    perror("Error al escribir '1' (encender LED)");
    close(fd);
    return EXIT_FAILURE;
    }

    printf("Comando '1' enviado: el LED deberia estar encendido");
    sleep(1);

    //leer el estado del led
    bytes_read = read(fd, buffer, sizeof(buffer)-1);  // se resta un para eliminar el terminador nulo
	if (bytes_read < 0){
	perror("Error al leer el estado del LED despues de encenderlo");
	close(fd);
	return EXIT_FAILURE;
	}
	buffer[bytes_read] = '\0' //asegurar que el buffer este termiando en nulo
	printf("Estado del LED leido (despues de enecender): %s\n", buffer);

	//apagar el led
	if (write(fd,"0", 1)<0){
	perror("Error al escribir '0' (apagar LED)");
	close(fd);
	return EXIT_FAILURE;
	}
	printf("Comando '0' enviado, el LED deberia estar apagado\n");
	sleep(1);

	//leer elestado del led
	bytes_read = read(fd, buffer,sizeof(buffer)-1);
	if (bytes_read <0){
	perror("Error al leer el estado del led despues de apagralo");
	close(fd);
	return 	EXIT_FAILURE;
	}
	buffer[bytes_read] ='\0';
	printf("Estado del LED: %s\n", buffer);

	// cerrar el dispositivo
	if (close(fd)<0){
	perror("Error al cerrar el dispositivo");
	return EXIT_FAILURE;
	}
	printf("Dispositivo %s cerrado con exito.\n", DEVICE_NAME);
	printf("___prueba del driver rpi_led finalizada con exito___\n");

	return EXIT_SUSCESS; //salida con codigo de exito

}


