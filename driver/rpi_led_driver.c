/*
Control de dispositivo de carácter Linux
para control de LED por GPIO en raspberry pi zero 2w

************* DOCUMENTACION DOXIGEN ************
//#include <soc/bcm2835/raspberrypi-firmware.h>

// ---------------- Instrucciones importantes

*** Guardar el archivo fuente como rpi_led_driver.c

compilar en bash con comando make

cargar el modulo
sudo insmod rpi_led_driver.ko

verificar los mensajes del kernel
dmesg | tail [20]

verificar nodo del dispositivo
ls -l /dev/rpi_led

encender led
echo -n "1" | sudo tee /dev/rpi_led
# O si te da un error de permisos con tee, usa:
# sudo bash -c 'echo -n "1" > /dev/rpi_led'

apagar led
echo -n "0" | sudo tee /dev/rpi_led
# O:
# sudo bash -c 'echo -n "0" > /dev/rpi_led'

leer estado
cat /dev/rpi_led

descargar el modulo
sudo rmmod rpi_led_driver

*** verificar con dmesg | tail que el led se apaga y el driver se desacrga limpiamente
*/

#include <linux/module.h>
#include <linux/kernel.h>      // Para KERN_INFO, printk
#include <linux/init.h>
#include <linux/fs.h>          // Para struct file_operations, y funciones de fs
#include <linux/cdev.h>        // Para cdev_init, cdev_add, cdev_del
#include <linux/uaccess.h>     // Para copy_to_user, copy_from_user
#include <linux/device.h>
#include <linux/gpio.h>
// #include <linux/gpio/consumer.h> // Podría ser necesario si realmente usaras gpiod_*

#define         DEVICE_NAME         "rpi_led"

// Metadatos del modulo
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos S rangel, Valeria Garcia Rodas");
MODULE_DESCRIPTION("GPIO LED driver");
MODULE_ALIAS("platform:rpi-LED-ctrl");

//--------------------------------//
// Definiciones globales //

// Numero de pin GPIO BCM para el LED
// hay un offset raro
#define         LED_GPIO_PIN        536   // con offset para el pin 24

static  dev_t   rpi_led_dev_num;     // numero mayor / menor del dispositivo
static struct cdev rpi_led_cdev;     // estructura cdev para el dispositivo
static struct class *rpi_led_class; // clase de dispositivo para sys/class

//---------------------------------------//
// Funciones de operaciones de archivo (File operations)

/**
* @brief Se llama cuando el usuario abre el nodo del dispositivo /dev/rpi_led.
* @param inode Puntero al inode del archivo
* @param file Puntero a la estructura file del archivo.
* @return int 0 en exito, un codigo de error negativo en fallo
*/
static int rpi_led_open(struct inode *inode, struct file *file)
{
    /* esta funcion registra la apertura del dispositivo */
    printk(KERN_INFO "rpi_led: Dispositivo abierto. \n");
    return 0;
}

/**
 * @brief Se llama cuando el usuario cierra el nodo del dispositivo /dev/rpi_led
 * @param inode puntero al inode del archivo
 * @param file puntero a la estrcutura file del dispositivo
 * @return int 0 en exito, un codigo de error negativo en fallo.
 */
static int rpi_led_release(struct inode *inode, struct file *file)
{
    // se registra el cierrre del dispositivo
    printk(KERN_INFO "rpi_led: Dispositivo cerrado.\n");
    return 0;
}

/**
 * @brief Se llama cuando el usuario lee el nodo del dispositivo
 * Devuelve el estado actual del LED (0 para OFF, 1 para ON).
 * @param file puntero a la estructura file del archivo.
 * @param user_buf Buffer de usuario al que se le copiaran los datos
 * @param count Numero de bytes a leer
 * @param ppos Puntero a la posicion de lectura/escritura (no usado aqui)
 * @return ssize_t Numero de bytes leidos (0 para EOF, negativo en error)
 */
static ssize_t rpi_led_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    char state_str[2]; // Para "0\0" o "1\0" (1 byte para el dígito + 1 para el terminador nulo)
    int current_state;
    ssize_t len;

    // leer el estado actual del pin GPIO
    current_state = gpio_get_value(LED_GPIO_PIN);

    // formatear el estado como una cadena. snprintf asegura terminación nula si hay espacio.
    len = snprintf(state_str, sizeof(state_str), "%d", current_state);

    // Asegurarse de no copiar más bytes de los que el usuario solicitó
    if (len > count) {
        len = count; // Limitar la copia a la cantidad solicitada
    }

    // copiar el estado al buffer de usuario
    if (copy_to_user(user_buf, state_str, len)) {
        printk(KERN_ERR "rpi_led: Fallo al copiar al espacio de usuario en read. \n");
        return -EFAULT; // bad address
    }
    printk(KERN_INFO "rpi_led: Leído estado del LED: %d \n", current_state);
    return len;
}

/**
 * @brief Se llama cuando el usuario escribe en el nodo del dispositivo /dev/rpi_led
 * Controla el LED (0 para OFF, 1 para ON)
 * @param file puntero a la estructura file del archivo
 * @param user_buf Buffer de usuario desde el que se leeran los datos
 * @param count Numero de Bytes a escribir
 * @param ppos Puntero a la posicion de lectura/escritura (no usado)
 * @return ssize_t numero de bytes escritos (negativo en error)
 */
static ssize_t rpi_led_write(struct file *file, const char __user *user_buf, size_t count, loff_t *ppos)
{
    char kbuf; // buffer en el kernel para recibir el dato del usuario
    if (count == 0) {
        return 0; // nada que escribir
    }
    // copiar un byte desde el buffer de usuario al buffer del kernel
    if (copy_from_user(&kbuf, user_buf, 1)) {
        printk(KERN_ERR "rpi_led: Fallo al copiar datos desde el espacio de usuario en write. \n");
        return -EFAULT; // Bad address
    }
    // convertir el caracter a un estado ON/OFF
    if (kbuf == '1') {
        gpio_set_value(LED_GPIO_PIN, 1); // ENCENDER EL LED
        printk(KERN_INFO "rpi_led: LED encendido. \n");
    } else if (kbuf == '0') {
        gpio_set_value(LED_GPIO_PIN, 0); // Apagar el led
        printk(KERN_INFO "rpi_led: LED apagado.\n");
    } else {
        printk(KERN_WARNING "rpi_led: Comando de escritura desconocido. use '0' o '1'.\n");
        return -EINVAL; // Argumento invalido
    }
    return 1; // se procesó 1 byte
}

// estructura que define las operaciones de archivo para el dispositivo
static const struct file_operations rpi_led_fops = {
    .owner = THIS_MODULE,
    .open = rpi_led_open,
    .release = rpi_led_release,
    .read = rpi_led_read,
    .write = rpi_led_write,
};

// =====================================================
// Funciones de inicializacion y limpieza del módulo
// =====================================================

/**
 * @brief Funcion de inicializacion del modulo. se llama cuando el modulo es cargado
 * @return int 0 en exito, codigo de error negativo en fallo.
 */
static int __init rpi_led_init(void)
{
    int ret;
    struct device *dev_ret; // Para almacenar el resultado de device_create

    printk(KERN_INFO "rpi_led: Iniciando controlador LED GPIO...\n");
    // 1. asignar un numero mayor (major number) dinamicamente
    // se debe hacer para evitar conflictos con otros dispositivos
    ret = alloc_chrdev_region(&rpi_led_dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "rpi_led: Fallo al asignar el numero mayor. Error: %d\n", ret);
        return ret;
    }
    printk(KERN_INFO "rpi_led: Numero mayor asignado: %d. Numero menor: %d\n",
           MAJOR(rpi_led_dev_num), MINOR(rpi_led_dev_num));

    // 2.inicializar la estructura cdev y vincular las operaciones de archivo
    cdev_init(&rpi_led_cdev, &rpi_led_fops);
    rpi_led_cdev.owner = THIS_MODULE;

    // 3. añadir el dispositivo de caracteres al sistema
    ret = cdev_add(&rpi_led_cdev, rpi_led_dev_num, 1);
    if (ret < 0) {
        printk(KERN_ERR "rpi_led: Fallo al añadir cdev. Error: %d\n", ret);
        unregister_chrdev_region(rpi_led_dev_num, 1); //limpiar el numero asignado
        return ret;
    }

    // 4. crear una clase de dispositivo para que udev cree el nodo automaticamente
    rpi_led_class = class_create(DEVICE_NAME);
    if (IS_ERR(rpi_led_class)) {
        ret = PTR_ERR(rpi_led_class);
        printk(KERN_ERR "rpi_led: Fallo al crear la clase de dispositivo. Error: %d\n", ret);
        cdev_del(&rpi_led_cdev); //limpiar cdev
        unregister_chrdev_region(rpi_led_dev_num, 1); // limpiar numero
        return ret;
    }

    // 5. crear el nodo de dispositivo, se crea en /dev/rpi_led
    dev_ret = device_create(rpi_led_class, NULL, rpi_led_dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(dev_ret)) {
        ret = PTR_ERR(dev_ret);
        printk(KERN_ERR "rpi_led: Fallo al crear el dispositivo. Error: %d\n", ret);
        class_destroy(rpi_led_class); // limpiar clase
        cdev_del(&rpi_led_cdev); // limpiar dispositivo
        unregister_chrdev_region(rpi_led_dev_num, 1); // limpiar numero
        return ret;
    }

    // 6. solicitar el pin GPIO y configurarlo como salida
    ret = gpio_request(LED_GPIO_PIN, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "rpi_led: Fallo al solicitar el GPIO %d. Error: %d\n", LED_GPIO_PIN, ret);
        device_destroy(rpi_led_class, rpi_led_dev_num); // limpiar dispositivo
        class_destroy(rpi_led_class);
        cdev_del(&rpi_led_cdev);
        unregister_chrdev_region(rpi_led_dev_num, 1);
        return ret;
    }

    ret = gpio_direction_output(LED_GPIO_PIN, 0); //configurarlo como salida e iniciar en low (LED apagado)
    if (ret < 0) {
        printk(KERN_ERR "rpi_led: Fallo al configurar GPIO %d como salida. Error: %d\n", LED_GPIO_PIN, ret);
        gpio_free(LED_GPIO_PIN); // Liberar el pin gpio
        device_destroy(rpi_led_class, rpi_led_dev_num);
        class_destroy(rpi_led_class);
        cdev_del(&rpi_led_cdev);
        unregister_chrdev_region(rpi_led_dev_num, 1);
        return ret;
    }
    printk(KERN_INFO "rpi_led: Driver cargado con exito. Controlando GPIO %d\n", LED_GPIO_PIN);
    printk(KERN_INFO "rpi_led: Puedes interactuar con /dev/%s\n", DEVICE_NAME);

    return 0; // exito
}


/**
 * @brief Funcion de limpieza del modulo, se llama cuendo el modulo es decargado
 */
static void __exit rpi_led_exit(void)
{
    printk(KERN_INFO "rpi_led: Descargando controlador de LED GPIO... \n");

    // CAMBIO 2: Eliminar la verificación de gpio_cansleep()
    // Asegurarse de apagar el led al descargar el modulo
    if (gpio_is_valid(LED_GPIO_PIN)) { // Solo verificamos si el GPIO es válido
        gpio_set_value(LED_GPIO_PIN, 0);
        printk(KERN_INFO "rpi_led: LED del GPIO %d apagado. \n", LED_GPIO_PIN);
        // Liberar el pin GPIO
        gpio_free(LED_GPIO_PIN);
        printk(KERN_INFO "rpi_led: LED conectado al pin GPIO %d liberado.\n", LED_GPIO_PIN);
    } else {
         printk(KERN_WARNING "rpi_led: GPIO %d no válido o no se pudo liberar (ya libre o error en init).\n", LED_GPIO_PIN);
    }

    // El orden de liberación es inverso al de asignación para asegurar una limpieza adecuada
    // 1. Destruir el nodo del dispositivo en /dev
    device_destroy(rpi_led_class, rpi_led_dev_num);
    printk(KERN_INFO "rpi_led: Dispositivo /dev/%s eliminado. \n", DEVICE_NAME);

    // 2. Destruir la clase del dispositivo
    class_destroy(rpi_led_class);
    printk(KERN_INFO "rpi_led: Clase de dispositivo eliminada.\n");

    // 3. Eliminar el dispositivo de caracteres
    cdev_del(&rpi_led_cdev);
    printk(KERN_INFO "rpi_led: cdev eliminado. \n");

    // 4. Desregistrar el numero mayor/menor
    unregister_chrdev_region(rpi_led_dev_num, 1);
    printk(KERN_INFO "rpi_led: Número mayor/menor desregistrado con éxito. \n");
    printk(KERN_INFO "rpi_led: Controlador descargado con éxito. \n");
}

// Registro de las funciones de inicializacion y limpieza del modulo
module_init(rpi_led_init);
module_exit(rpi_led_exit);