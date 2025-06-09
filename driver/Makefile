# makefile para compilar el modulo del kernel

TOP_DIR := ..


#DIRECTORIO DE SALIDA PARA LOS OBJETOS  Y EL MODIULO

OBJ_DIR := $(TOP_DIR)/obj

#Nombre del modulo del kernel
obj-m := rpi_led_driver.o

KDIR := /lib/modules/$(shell uname -r)/build
PW := $(shell pwd)


all: $(OBJ_DIR)/rpi_led_driver.ko

$(OBJ_DIR)/rpi_led_driver.ko:
		$(MAKE) -C $(KDIR) M=$(PWD) modules
		mv *.ko $(OBJ_DIR)/
		mv *.o $(OBJ_DIR)/
		mv *.mod.c $(OBJ_DIR)/ 2>/dev/null || true
		mv *.cmd $(OBJ_DIR)/ 2>/dev/null || true
		@echo  "Modulo del kernel rpi_led_driver.ko creado en $(OBJ_DIR)"


.PHONY: clean
clean:
		$(MAKE) -C $(KDIR) M=$(PWD) clean
		rm -f $(OBJ_DIR)/rpi_led_driver.o $(OBJ_DIR)/rpi_led_driver.ko $(OBJ_DIR)/rpi_led_driver.mod.c $(OBJ_DIR)/modules.order $(OBJ_DIR)/Module.symvers
		rm -f $(OBJ_DIR)/.rpi_led_driver.o.cmd $(OBJ_DIR)/.rpi_led_driver.ko.cmd
		@echo "Limpieza del modulo driver completa"

# Regla para crear directorio si no existe
$(OBJ_DIR):
		mkdir -p $(OBJ_DIR)