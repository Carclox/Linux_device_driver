# directorios de salida

OBJ_DIR := obj/
BIN_DIR := bin/

.PHONY: all app driver tests clean

all: app driver
	@echo "Construccion completa de la aplicacion y el driver."

app: $(BIN_DIR)app
	@echo "Aplicacion 'app' lista en $(BIN_DIR)."


driver: $(BIN_DIR)rpi_led_driver.ko
	@echo "Modulo del driver 'rpi_led_driver' listo en $(BIN_DIR)"

tests: app driver $(OBJ_DIR)test_driver.o $(OBJ_DIR)test_app.o
	$(MAKE) -C tests all
	@echo "Todos los componentes, incluyendo los tests han sido construidos."

$(BIN_DIR)app: $(BIN_DIR)
	$(MAKE) -C app

$(BIN_DIR)rpi_led_driver.ko: $(BIN_DIR) $(OBJ_DIR)
	$(MAKE) -C driver
	mv driver/*.ko $(BIN_DIR)/
	# Movemos los archivos .o y otros intermedios a OBJ_DIR
	mv driver/*.o $(OBJ_DIR)/ 2>/dev/null || true
	mv driver/*.mod.c $(OBJ_DIR)/ 2>/dev/null || true
	mv driver/*.cmd $(OBJ_DIR)/ 2>/dev/null || true
	mv driver/Module.symvers $(OBJ_DIR)/ 2>/dev/null || true
	mv driver/modules.order $(OBJ_DIR)/ 2>/dev/null || true
	@echo "Moviendo archivos del driver a sus directorios destino."

clean:
	@echo "Limpiando archivos actuales..."
	$(MAKE) -C app clean
	$(MAKE) -C driver clean
	$(MAKE) -C tests clean
	rm -f $(OBJ_DIR)* $(BIN_DIR)*
	@echo "Limpieza completa"


## crear directorios si no existen

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)