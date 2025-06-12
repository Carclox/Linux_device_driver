//
// Created by el_carclox on 5/06/2025.
//

#include <stdio.h>    // Para printf, fprintf
#include <stdlib.h>   // Para system, EXIT_SUCCESS, EXIT_FAILURE
#include <string.h>   // Para strcmp
#include <unistd.h>   // Para sleep (si quieres pausas)

// Define la ruta a tu ejecutable de la aplicación y el driver
#define APP_PATH        "./app" // Asume que el ejecutable 'app' está en el mismo directorio
#define DRIVER_MODULE_NAME "rpi_led_driver" // Nombre del módulo, no la ruta del .ko
#define DRIVER_KO_PATH  "/home/PI/Linux_device_driver/obj/rpi_led_driver.ko" // Ruta completa al archivo .ko

// --- Funciones auxiliares para cargar/descargar driver ---
void load_driver() {
    printf("--- Intentando cargar el driver '%s' ---\n", DRIVER_KO_PATH);
    char command[256];
    snprintf(command, sizeof(command), "sudo insmod %s 2>&1", DRIVER_KO_PATH);
    int result = system(command);
    if (result == 0) {
        printf("Driver cargado con éxito.\n");
    } else if (result == 256) { // Common exit code for 'module already loaded'
        printf("Advertencia: El driver ya estaba cargado o hubo un error menor (código %d).\n", result);
    } else {
        fprintf(stderr, "Error: Fallo al cargar el driver (código %d).\n", result);
        perror("system"); // Muestra detalles adicionales del error del sistema
    }
    sleep(1); // Pequeña pausa para que el driver se inicialice completamente
}

void unload_driver() {
    printf("--- Intentando descargar el driver '%s' ---\n", DRIVER_MODULE_NAME);
    char command[256];
    snprintf(command, sizeof(command), "sudo rmmod %s 2>&1", DRIVER_MODULE_NAME);
    int result = system(command);
    if (result == 0) {
        printf("Driver descargado con éxito.\n");
    } else if (result == 256) { // Common exit code for 'module not found'
        printf("Advertencia: El driver no estaba cargado o hubo un error menor al descargar (código %d).\n", result);
    } else {
        fprintf(stderr, "Error: Fallo al descargar el driver (código %d).\n", result);
        perror("system");
    }
    sleep(1); // Pequeña pausa para asegurar la limpieza
}

// --- Función para ejecutar la aplicación con un comando y capturar la salida ---
// Retorna 0 si la ejecución fue exitosa y la salida esperada se encontró, 1 en caso contrario.
int run_app_command(const char *command_arg, const char *expected_output_fragment) {
    char full_command[256];
    char buffer[128];
    FILE *fp;

    snprintf(full_command, sizeof(full_command), "%s %s 2>&1", APP_PATH, command_arg);
    printf("Ejecutando: %s\n", full_command);

    // Ejecutar el comando y capturar su salida
    fp = popen(full_command, "r");
    if (fp == NULL) {
        perror("Fallo al ejecutar el comando popen");
        return 1;
    }

    char *output_start = NULL;
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("  Salida de app: %s", buffer); // Imprime la salida de la aplicación
        if (output_start == NULL) { // Busca la línea que contiene el estado
            if (strstr(buffer, "Estado actual del LED:") != NULL) {
                output_start = buffer;
            } else if (strcmp(command_arg, "on") == 0 && strstr(buffer, "LED encendido.") != NULL) {
                output_start = buffer;
            } else if (strcmp(command_arg, "off") == 0 && strstr(buffer, "LED apagado.") != NULL) {
                output_start = buffer;
            }
        }
    }

    int pclose_status = pclose(fp);
    if (pclose_status != 0) {
        fprintf(stderr, "Error: El comando '%s' terminó con un código de salida no cero: %d\n", full_command, WEXITSTATUS(pclose_status));
        return 1;
    }

    // Verificar si la salida esperada está presente
    if (expected_output_fragment != NULL && output_start != NULL) {
        if (strstr(output_start, expected_output_fragment) != NULL) {
            printf("  -> VERIFICADO: Salida esperada encontrada.\n\n");
            return 0; // Éxito
        } else {
            fprintf(stderr, "  -> FALLO: Salida esperada '%s' NO encontrada en '%s'.\n\n", expected_output_fragment, output_start);
            return 1; // Fallo
        }
    } else if (expected_output_fragment == NULL && pclose_status == 0) {
        printf("  -> VERIFICADO: Comando ejecutado sin error y sin salida esperada específica.\n\n");
        return 0; // Éxito para comandos sin salida específica a verificar
    } else {
        fprintf(stderr, "  -> FALLO: No se pudo verificar la salida.\n\n");
        return 1;
    }
}


// --- Función principal de prueba ---
int main() {
    int tests_passed = 0;
    int tests_failed = 0;

    printf("**********************************************\n");
    printf("* INICIANDO PRUEBAS AUTOMATIZADAS DEL DRIVER *\n");
    printf("**********************************************\n\n");

    // Asegurarse de que el driver esté descargado al inicio para una prueba limpia
    unload_driver();
    sleep(1); // Esperar un poco para asegurar la descarga

    // 1. Cargar el driver
    load_driver();
    sleep(1); // Dar tiempo para que el dispositivo /dev/rpi_led se cree

    // 2. Probar 'on'
    printf("--- Caso de Prueba 1: Encender el LED ---\n");
    if (run_app_command("on", "LED encendido.")) {
        tests_failed++;
    } else {
        tests_passed++;
    }

    // 3. Probar 'status' (debería ser '1')
    printf("--- Caso de Prueba 2: Verificar estado ON ---\n");
    if (run_app_command("status", "Estado actual del LED: 1")) {
        tests_failed++;
    } else {
        tests_passed++;
    }

    // 4. Probar 'off'
    printf("--- Caso de Prueba 3: Apagar el LED ---\n");
    if (run_app_command("off", "LED apagado.")) {
        tests_failed++;
    } else {
        tests_passed++;
    }

    // 5. Probar 'status' (debería ser '0')
    printf("--- Caso de Prueba 4: Verificar estado OFF ---\n");
    if (run_app_command("status", "Estado actual del LED: 0")) {
        tests_failed++;
    } else {
        tests_passed++;
    }

    // 6. Probar comando inválido
    printf("--- Caso de Prueba 5: Comando inválido ---\n");
    // Esperamos un código de salida distinto de 0 y un mensaje de error
    // run_app_command no tiene una forma directa de verificar stderr o exit code no cero
    // Para este caso, solo verificaremos que no haya "Éxito" y que se imprima la salida esperada de error.
    char invalid_command[256];
    snprintf(invalid_command, sizeof(invalid_command), "%s invalid_command 2>&1", APP_PATH);
    printf("Ejecutando: %s\n", invalid_command);
    FILE *fp_invalid = popen(invalid_command, "r");
    if (fp_invalid == NULL) {
        perror("Fallo al ejecutar popen para comando inválido");
        tests_failed++;
    } else {
        char buffer_invalid[128];
        int found_error_msg = 0;
        while (fgets(buffer_invalid, sizeof(buffer_invalid), fp_invalid) != NULL) {
            printf("  Salida de app (inválido): %s", buffer_invalid);
            if (strstr(buffer_invalid, "Comando no reconocido. Uso:") != NULL) {
                found_error_msg = 1;
            }
        }
        int pclose_status_invalid = pclose(fp_invalid);
        if (WEXITSTATUS(pclose_status_invalid) == EXIT_FAILURE && found_error_msg) {
            printf("  -> VERIFICADO: Comando inválido manejado correctamente.\n\n");
            tests_passed++;
        } else {
            fprintf(stderr, "  -> FALLO: Manejo de comando inválido incorrecto.\n\n");
            tests_failed++;
        }
    }


    // 7. Descargar el driver al finalizar
    unload_driver();

    printf("**********************************************\n");
    printf("* RESUMEN DE PRUEBAS *\n");
    printf("**********************************************\n");
    printf("Pruebas pasadas: %d\n", tests_passed);
    printf("Pruebas fallidas: %d\n", tests_failed);

    if (tests_failed > 0) {
        return EXIT_FAILURE;
    } else {
        printf("\n¡Todas las pruebas pasaron con éxito!\n");
        return EXIT_SUCCESS;
    }
}




