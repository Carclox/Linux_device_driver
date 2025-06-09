# Makefile para el subdirectorio 'tests'

# Ruta al directorio raíz del proyecto
TOP_DIR := ..

# Directorio de salida para los objetos
OBJ_DIR := $(TOP_DIR)/obj

SRCS := test_driver.c test_app.c
OBJS := $(SRCS:.c=.o)

all: $(OBJ_DIR)/test_driver.o $(OBJ_DIR)/test_app.o
	@echo "Archivos de test objeto creados en $(OBJ_DIR)"

$(OBJ_DIR)/test_driver.o: test_driver.c | $(OBJ_DIR)
	$(CC) -c $< -o $@
	@echo "Objeto test_driver.o creado en $(OBJ_DIR)"

$(OBJ_DIR)/test_app.o: test_app.c | $(OBJ_DIR)
	$(CC) -c $< -o $@
	@echo "Objeto test_app.o creado en $(OBJ_DIR)"

.PHONY: clean

clean:
	rm -f $(OBJ_DIR)/$(OBJS)
	@echo "Limpieza de los archivos de test objeto completa."

# Regla para crear directorio si no existe
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)