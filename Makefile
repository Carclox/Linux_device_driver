obj-m := rpi_led_gpio_driver.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean


# ESTE SOLO ES UN PROTOTIPO, HAY QUE MEJORARLO