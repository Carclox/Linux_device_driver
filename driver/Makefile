# makefile para compilar el modulo del kernel

# No necesitamos TOP_DIR, OBJ_DIR ni BIN_DIR aquí, el Makefile principal se encargará del movimiento.


obj-m := rpi_led_driver.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

.PHONY: clean
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	#Asegurarse de limpiar los archivos generados por kbuild en este directorio (driver/)
	rm -f *.o *.ko *.mod.c *.cmd *.symvers *.order
	@echo "Limpieza del modulo driver completa"

