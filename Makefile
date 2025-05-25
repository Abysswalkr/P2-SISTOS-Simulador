CXX = g++
CXXFLAGS = -std=c++11 -Wall
SFML_FLAGS = -lsfml-graphics -lsfml-window -lsfml-system

# Nombre del ejecutable
TARGET = simulador

# Archivos fuente
SOURCES = main.cpp

# Compilación
$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(SFML_FLAGS)

# Limpiar archivos compilados
clean:
	rm -f $(TARGET)

# Ejecutar
run: $(TARGET)
	./$(TARGET)

.PHONY: clean run