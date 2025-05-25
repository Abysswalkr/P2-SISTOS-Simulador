CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -Iinclude
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Nombre del ejecutable
TARGET = simulador

# Directorios
SRCDIR = src
INCDIR = include
DATADIR = data

# Archivos fuente
SOURCES = $(SRCDIR)/main.cpp
HEADERS = $(wildcard $(INCDIR)/*.h)

# Compilación
all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(SFML_FLAGS)

# Limpiar archivos compilados
clean:
	rm -f $(TARGET)

# Ejecutar
run: $(TARGET)
	./$(TARGET)

# Crear estructura de directorios si no existe
dirs:
	@mkdir -p $(SRCDIR) $(INCDIR) $(DATADIR) docs

.PHONY: all clean run dirs