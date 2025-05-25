CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Nombre del ejecutable
TARGET = simulador

# Archivos fuente
SOURCES = main.cpp
HEADERS = estructuras.h parser.h simulador_calendarizacion.h simulador_sincronizacion.h gui.h

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

.PHONY: all clean run