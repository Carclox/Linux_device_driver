#directorios de salida

OBJ_DIR := obj
BIN_DIR := bin

.PHONY: all app driver tests clean

all: app driver
		@echo "Construccion completa de la aplicacion y el driver."
app: $(BIN_DIR)/app
		@echo "Aplicacion 'app' lista en $(BIN_DIR)."

driver: $(OBJ_DIR)/rpi_led_driver.ko
		@echo "Modulo del driver 'rpi_led_driver' listo en $(OBJ_DIR)"

tests: app driver $(OBJ_DIR)/test_driver.o $(OBJ_DIR)/test_app.o
		$(MAKE) -C tests all
		@echo "Todos los componentes, incluyendo los tests han sido construidos."

$(BIN_DIR)/app:
		$(MAKE) -C app

$(OBJ_DIR)/rpi_led_driver.ko:
		$(MAKE) -C driver

$(OBJ_DIR)/test_driver.o $(OBJ_DIR)/test_app.o
		$(MAKE) -C tests


clean:
		@echo "Limpiando archivos actuales..."
		$(MAKE) -C app clean
		$(MAKE) -C driver clean
		$(MAKE) -C tests clean
		rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.ko $(BIN_DIR)/*
		@echo "Limpieza completa"



## crear directorios si no existen

$(OBJ_DIR):
		mkdir -p $(OBJ_DIR)

$(BIN_DIR):
		mkdir -p $(BIN_DIR)