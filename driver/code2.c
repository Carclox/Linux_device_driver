#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/gpio.h>

// Definicion del nombre del dispositivo
#define         DEVICE_NAME         "rpi_led"

// Metadatos del modulo
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos S rangel, Valeria Garcia Rodas");
MODULE_DESCRIPTION("GPIO LED driver");
MODULE_ALIAS("platform:rpi-LED-ctrl");


//--------------------------------//
// Definiciones globales //

// Numero de pin GPIO BCM para el LED
#define         LED_GPIO_PIN        21 // pin numero 40, 21 en BCM

static  dev_t   rpi_led_dev_num;  // numero mayor / menor del dispositivo
static struct cdev rpi_led_cdev; // estructura cdev para el dispositivo
static struct class *rpi_led_class; // clase de dispositivo para sys/class (añadido ; al final)


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
static ssize_t rpi_led_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos) // CORREGIDO: char __user *user_buf
{
    char state_str[2]; // para "0\n" o "1\n"
    int current_state;
    ssize_t len;

    // leer el estado actual del pin GPIO
    current_state = gpio_get_value(LED_GPIO_PIN);

    // formatear el estado como una cadena
    len = snprintf(state_str, sizeof(state_str), "%d", current_state); // solo '0' o '1'

    // copiar el estado al buffer de usuario
    if (copy_to_user(user_buf, state_str, len)) { // CORREGIDO: len
        printk(KERN_ERR "rpi_led: Fallo al copiar al espacio de usuario en read. \n");
        return -EFAULT; // bad address
    }
    printk(KERN_INFO "rpi_led: Leido estado del LED: %d \n", current_state);
    return len; // devolver la cantidad de bytes copiados
}

/**
 * @brief Se llama cuando el usuario escribe en el nodo del dispositivo /dev/rpi_led
 * Controla el LED (0 para OFF, cualquier otro valor para ON)
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
        printk(KERN_INFO "rpi_led: Apagado.\n"); // CORREGIDO: /n a \n
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

    printk(KERN_INFO "rpi_led: Iniciando controlador LED GPIO...\n");
    // 1. asignar un numero mayor (major number) dinamicamente
    // se debe hacer para evitar conflictos con otros dispositivos
    ret = alloc_chrdev_region(&rpi_led_dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "rpi_led: Fallo al asignar el numero mayor. Error: %d\n", ret); // CORREGIDO: /n a \n
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
    // esta no es una clase de poo, es mas bien un tipo de categoria logica para el dispositivo
    rpi_led_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(rpi_led_class)) {
        ret = PTR_ERR(rpi_led_class);
        printk(KERN_ERR "rpi_led: Fallo al crear la clase de dispositivo. Error: %d\n", ret);
        cdev_del(&rpi_led_cdev); //limpiar cdev
        unregister_chrdev_region(rpi_led_dev_num, 1); // limpiar numero
        return ret;
    }

    // 5. crear el nodo de dispositivo, se crea en /dev/rpi_led
    // CORREGIDO: device_create sin IS_ERR alrededor y sin re-invocación en el mensaje de error
    if (IS_ERR(device_create(rpi_led_class, NULL, rpi_led_dev_num, NULL, DEVICE_NAME))) {
        ret = PTR_ERR(device_create(rpi_led_class, NULL, rpi_led_dev_num, NULL, DEVICE_NAME));
        printk(KERN_ERR "rpi_led: fallo al crear el dispositivo. error: %d\n", ret);
        class_destroy(rpi_led_class); // limpiar clase
        cdev_del(&rpi_led_cdev); // limpiar dispositivo
        unregister_chrdev_region(rpi_led_dev_num, 1); // limpiar numero
        return ret;
    }

    // 6. solicitar el pin GPIO y configuararlo como salida
    ret = gpio_request(LED_GPIO_PIN, DEVICE_NAME);
    if (ret < 0) {
        printk(KERN_ERR "rpi_led: Fallo al solicitar el GPIO %d. Error: %d\n", LED_GPIO_PIN, ret); // CORREGIDO: ',' faltante
        device_destroy(rpi_led_class, rpi_led_dev_num); // limpiar dispositivo
        class_destroy(rpi_led_class); // CORREGIDO: typo
        cdev_del(&rpi_led_cdev);
        unregister_chrdev_region(rpi_led_dev_num, 1);
        return ret;
    }

    ret = gpio_direction_output(LED_GPIO_PIN, 0); //configurarlo como salida e iniciar en low
    if (ret < 0) {
        printk(KERN_ERR "rpi_led: Fallo al configurar GPIO  %d como salida. Error: %d\n", LED_GPIO_PIN, ret);
        gpio_free(LED_GPIO_PIN); // Liberar el pin gpio
        device_destroy(rpi_led_class, rpi_led_dev_num);
        class_destroy(rpi_led_class); // CORREGIDO: typo
        cdev_del(&rpi_led_cdev);
        unregister_chrdev_region(rpi_led_dev_num, 1);
        return ret;
    }
    printk(KERN_INFO "rpi_led: Driver cargado con exito. controlando GPIO %d\n", LED_GPIO_PIN); // CORREGIDO: carggado a cargado
    printk(KERN_INFO "rpi_led: Puedes interactuar con /dev/%s\n", DEVICE_NAME);

    return 0; // exito
}


/**
 * @brief Funcion de limpieza del modulo, se llama cuendo el modulo es decargado
 */
static void __exit rpi_led_exit(void)
{
    printk(KERN_INFO "rpi_led: Descargando controlador de LED GPIO... \n");

    // asegurarse de apagar el led al descargar el modulo
    gpio_set_value(LED_GPIO_PIN, 0);
    printk(KERN_INFO "rpi_led: LED del GPIO %d apagado. \n", LED_GPIO_PIN);

    // Liberar el pin GPIO
    gpio_free(LED_GPIO_PIN);
    printk(KERN_INFO "rpi_led: LED conectado al pin GPIO %d liberado.\n", LED_GPIO_PIN);
    // 2. Destruir el nodo del dispositivo en /dev
    device_destroy(rpi_led_class, rpi_led_dev_num); // CORREGIDO: device-destroy a device_destroy
    printk(KERN_INFO "rpi_led: Dispositivo /dev/%s eliminado. \n", DEVICE_NAME); // CORREGIDO: %s

    //3. destruir la clase del dispositivo
    class_destroy(rpi_led_class); // CORREGIDO: class_destroy: a class_destroy;
    printk(KERN_INFO "rpi_led: Clase de dispositivo eliminada.\n");

    // 4. eliminar el dispositivo de caracteres
    cdev_del(&rpi_led_cdev);
    printk(KERN_INFO "rpi_led: cdev eliminado. \n");

    // 5. desregstrar el numero mayor
    unregister_chrdev_region(rpi_led_dev_num, 1);
    printk(KERN_INFO "rpi_led: Numero mayor descargado con exito. \n"); // CORREGIDO: desacrgado a descargado
    printk(KERN_INFO "rpi_led: Controlador descargado con exito. \n");
}

// Registro de las funciones de inicializacion y limpieza del modulo

module_init(rpi_led_init);
module_exit(rpi_led_exit);