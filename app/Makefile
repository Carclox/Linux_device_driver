## para el subdirectorio app
## la tarea de compilar se delega a este makefile

## Ruta al directorio raiz
TOP_DIR := ..


#directorios de salida definidos en el makefile principal
OBJ_DIR := $(TOP_DIR)/obj
BIN_DIR := $(TOP_DIR)/bin

TARGET := app

SRCS := user_app.c
OBJS := $(SRCS:.C=.O)


## ESTA REGLA ALL NO ES REALMENTE NESESARIA EN ESTE MAKEFILE
# pero se define por si acaso hay problemas con la compilacion

all: $(BIN_DIR)/$(TARGET)

$(BIN_DIR)/$(TARGET): $(OBJ_DIR)/app.o | $(BIN_DIR)
	$(CC) $(OBJ_DIR)/app.o -o $@
	@echo "Ejecutable $(TARGET) creado en $(BIN_DIR)"

$(OBJ_DIR)/app.o: $(SRCS) | $(OBJ_DIR)
	$(CC) -c $< -o $@
	@echo "Objeto app.o creado en $(OBJ_DIR)"

.PHONY: clean
clean:
	rm -f $(OBJ_DIR)/$(OBJS) $(BIN_OBJ_DIR)/$(TARGET)
	@echo "Limpieza del modulo app completa."

# reglas para crear directorios si no existen
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)