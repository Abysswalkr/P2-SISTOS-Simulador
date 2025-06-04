# -------------------------------------------------------------
# Makefile para compilar Simulador (SFML + ImGui + ImGui-SFML)
# -------------------------------------------------------------

# ----------------------------------------------------------------
# 1) Definimos compilador y banderas básicos
# ----------------------------------------------------------------
CXX      := g++
CXXFLAGS := -std=c++17 -Wall \
            -Iinclude \
            -Iimgui \
            -Iimgui-sfml \
            -I/usr/include \
            -DIMGUI_SFML_DEBUG_LOG

# ----------------------------------------------------------------
# 2) Definimos las bibliotecas a enlazar (SFML y dependencias)
# ----------------------------------------------------------------
LDFLAGS  := -lsfml-graphics \
            -lsfml-window   \
            -lsfml-system   \
            -lGL            \
            -ldl            \
            -lpthread

# ----------------------------------------------------------------
# 3) Listado de todos los .cpp que debe compilar
#    - src/main.cpp           : tu código principal
#    - imgui/*.cpp            : todos los archivos fuente de ImGui
#    - imgui-sfml/imgui-SFML.cpp : binding ImGui→SFML
# ----------------------------------------------------------------
SRCS := \
    src/main.cpp \
    imgui/imgui.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_widgets.cpp \
    imgui/imgui_tables.cpp \
    imgui/imgui_demo.cpp \
    imgui-sfml/imgui-SFML.cpp

# ----------------------------------------------------------------
# 4) Generamos la lista de .o a partir de los .cpp
# ----------------------------------------------------------------
OBJS := $(SRCS:.cpp=.o)

# ----------------------------------------------------------------
# 5) Nombre final del ejecutable
# ----------------------------------------------------------------
TARGET := simulador

# ----------------------------------------------------------------
.PHONY: all clean

# ----------------------------------------------------------------
# Regla principal: compilar y enlazar
# ----------------------------------------------------------------
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# ----------------------------------------------------------------
# Regla genérica para compilar cada .cpp en su .o correspondiente
# ----------------------------------------------------------------
# Ejemplo: src/main.cpp → src/main.o
#          imgui/imgui.cpp → imgui/imgui.o
#          imgui-sfml/imgui-SFML.cpp → imgui-sfml/imgui-SFML.o
# ----------------------------------------------------------------
%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ----------------------------------------------------------------
# Limpiar todo: elimina .o y el ejecutable
# ----------------------------------------------------------------
clean:
	rm -f $(OBJS) $(TARGET)
