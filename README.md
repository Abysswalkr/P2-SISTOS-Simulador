# Simulador de Calendarización y Sincronización

Este proyecto implementa un simulador de algoritmos de planificación de procesos y mecanismos de sincronización en C++ utilizando SFML y la biblioteca ImGui-SFML para la interfaz gráfica. A continuación encontrarás todas las instrucciones para clonar el repositorio, instalar dependencias, compilar y probar el simulador.

---

## Contenido del Repositorio

```
Simulador/
├── data/
│   └── procesos.txt           <-- Archivo de ejemplo para procesos (se debe crear)
│   └── recursos.txt           <-- Archivo de ejemplo para recursos (para sincronización)
│   └── acciones.txt           <-- Archivo de ejemplo para acciones (para sincronización)
├── imgui/                     <-- Biblioteca ImGui (código fuente)
│   ├── imgui.cpp
│   ├── imgui.h
│   ├── imgui_draw.cpp
│   ├── imgui_widgets.cpp
│   ├── imgui_tables.cpp
│   └── imgui_demo.cpp
├── imgui-sfml/                <-- Biblioteca ImGui-SFML (binding para SFML)
│   ├── imgui-SFML.cpp
│   ├── imgui-SFML.h
│   ├── imconfig-SFML.h
│   ├── imgui-SFML_export.h
│   └── CMakeLists.txt         <-- (no se usa en este proyecto)
├── include/                   <-- Cabeceras propias del proyecto
│   ├── estructuras.h
│   ├── parser.h
│   ├── simulador_calendarizacion.h
│   └── simulador_sincronizacion.h
├── src/                       <-- Código fuente principal
│   └── main.cpp               <-- Interfaz gráfica con ImGui-SFML y lógica de simulación
├── Makefile                   <-- Archivo para compilar el proyecto
└── README.md                  <-- Este archivo
```

---

## 1. Clonar el Repositorio

Para obtener el código fuente, clona el repositorio desde GitHub (reemplaza `<usuario>` y `<repositorio>` con la URL real):

```bash
cd /home/usuario
# Clona el repositorio principal
git clone https://github.com/<usuario>/<repositorio>.git Simulador
cd Simulador
```

Esto creará la carpeta `Simulador/` con todo el contenido.

---

## 2. Requisitos Previos

Antes de compilar y ejecutar el simulador, asegúrate de tener instalado lo siguiente en tu sistema (se asume distribución basada en Debian/Ubuntu/Kali):

1. **Compilador C++** (g++ con soporte C++17).

   ```bash
   sudo apt-get update
   sudo apt-get install build-essential
   ```

2. **SFML 2.5+** (bibliotecas y cabeceras de desarrollo).

   ```bash
   sudo apt-get install libsfml-dev
   ```

3. **pkg-config** (opcionalmente, aunque el Makefile ha sido adaptado para no usarlo directamente).

   ```bash
   sudo apt-get install pkg-config
   ```

4. **Git** (para clonar y actualizar ImGui e ImGui-SFML, si no vienen ya en el repositorio).

   ```bash
   sudo apt-get install git
   ```

5. **Dependencias de ImGui-SFML** (normalmente ya instaladas con SFML, pero si hay errores de OpenGL, instala):

   ```bash
   sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev libx11-dev libxrandr-dev libxi-dev
   ```

---

## 3. Estructura de Carpetas: Descargar ImGui e ImGui-SFML

> **Nota Importante**: Este proyecto se ha probado con **ImGui v1.82** y **ImGui-SFML v2.3**, que funcionan correctamente con SFML 2.5.

Si tu repositorio **no** incluye ya las carpetas `imgui/` e `imgui-sfml/`, puedes clonarlas manualmente. Si ya existen, asegúrate de que sean las versiones compatibles (v1.82 y v2.3). A continuación se explica cómo obtener esas versiones:

```bash
# Estando en /home/usuario/Simulador
mkdir imgui
mkdir imgui-sfml

# Clonar ImGui v1.82 (etiqueta release)
cd imgui
git clone https://github.com/ocornut/imgui.git .
git checkout v1.82

# Clonar ImGui-SFML v2.3 (etiqueta release)
cd ../imgui-sfml
git clone https://github.com/eliasdaler/imgui-sfml.git .
git checkout v2.3
```

Verifica que queden estos archivos principales:

```
imgui/
├── imgui.cpp
├── imgui.h
├── imgui_draw.cpp
├── imgui_widgets.cpp
├── imgui_tables.cpp
└── imgui_demo.cpp

imgui-sfml/
├── ImGui-SFML.cpp
├── ImGui-SFML.h
├── imconfig-SFML.h
└── imgui-SFML_export.h
```

> Si ya tenías una versión distinta instalada, borra la carpeta correspondiente y repite estos pasos para asegurarte de usar ImGui v1.82 e ImGui-SFML v2.3.

---

## 4. Configurar Archivos de Datos (Carpeta `data/`)

El simulador lee de archivos de texto planos para los procesos (y, si usas el modo de sincronización, también recursos y acciones). Debes crear la carpeta `data/` en la raíz del proyecto y allí colocar tres archivos de ejemplo (o propios):

```bash
cd /home/usuario/Simulador
mkdir data
```

### 4.1. Archivo de Procesos (`data/procesos.txt`)

Cada línea debe tener el formato:

```
<PID>, <BurstTime>, <ArrivalTime>, <Priority>
```

donde:

* **PID**: identificador único (ej. "P1").
* **BurstTime (BT)**: tiempo de CPU que necesitará el proceso (entero).
* **ArrivalTime (AT)**: ciclo de llegada al sistema (entero).
* **Priority**: prioridad numérica (1 = mayor prioridad) (entero).

Ejemplo mínimo:

```txt
P1, 6, 0, 2
P2, 4, 1, 3
P3, 8, 2, 1
P4, 3, 3, 4
P5, 5, 4, 2
```

Guarda esto en `data/procesos.txt`.

### 4.2. (Opcional) Archivos para Sincronización

Si deseas probar el modo "Sincronización", crea también:

#### `data/recursos.txt`

Formato por línea:

```
<Recurso>, <CantidadInicial>
```

ejemplo:

```
mutex1, 1
mutex2, 1
semaforo1, 3
semaforo2, 2
```

#### `data/acciones.txt`

Formato por línea:

```
<PID>, <TIPO>, <Recurso>, <Ciclo>
```

ejemplo:

```
P1, READ, mutex1, 2
P1, WRITE, mutex1, 3
P2, READ, mutex1, 1
P2, WRITE, semaforo1, 2
P3, READ, semaforo1, 3
P3, WRITE, mutex2, 5
P4, READ, mutex2, 1
P5, WRITE, semaforo2, 2
```

Guárdalos en su carpeta: `data/recursos.txt` y `data/acciones.txt`.

> **Importante**: Si cualquiera de estos archivos no existe o no respeta el formato, el simulador mostrará un error al cargar. Usa estrictamente el formato indicado.

---

## 5. Compilar el Proyecto

En la raíz del proyecto (`/home/usuario/Simulador`), verifica que tengas el siguiente `Makefile`. Si no, créalo o reemplázalo con el contenido de abajo.

### 5.1. Makefile

```makefile
# -------------------------------------------------------------
# Makefile para compilar Simulador (SFML + ImGui + ImGui-SFML)
# -------------------------------------------------------------

# Compilador y banderas de compilación (modo debug: -g, sin optimizar)
CXX      := g++
CXXFLAGS := -std=c++17 -g -O0 -Wall \
            -Iinclude \
            -Iimgui \
            -Iimgui-sfml \
            -I/usr/include \
            -DIMGUI_SFML_DEBUG_LOG

# Bibliotecas para enlazar (SFML y dependencias de OpenGL)
LDFLAGS  := -lsfml-graphics \
            -lsfml-window   \
            -lsfml-system   \
            -lGL            \
            -ldl            \
            -lpthread       \
            -fsanitize=address

# Archivos fuente:
SRCS := \
    src/main.cpp \
    imgui/imgui.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_widgets.cpp \
    imgui/imgui_tables.cpp \
    imgui/imgui_demo.cpp \
    imgui-sfml/ImGui-SFML.cpp

# Objetos correspondientes (cambia .cpp a .o)
OBJS := $(SRCS:.cpp=.o)

# Nombre del ejecutable
TARGET := simulador

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Regla genérica: compilar .cpp a .o
%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
```

### 5.2. Pasos para compilar

```bash
cd /home/usuario/Simulador
make clean   # Elimina binarios y objetos previos
make         # Compila todo en modo debug con AddressSanitizer
```

* El ejecutable resultante se llamará `simulador` y estará en `/home/usuario/Simulador/simulador`.
* Si hay errores de compilación, revisa los mensajes en la terminal: pueden faltar dependencias (p. ej., SFML o bibliotecas de OpenGL).

---

## 6. Ejecutar y Probar el Simulador

Continuar con las secciones anteriores...
Configurar Archivos de Datos (Carpeta `data/`)

El simulador lee de archivos de texto planos para los procesos (y, si usas el modo de sincronización, también recursos y acciones). Debes crear la carpeta `data/` en la raíz del proyecto y allí colocar tres archivos de ejemplo (o propios):

```bash
cd /home/usuario/Simulador
mkdir data
```

### 4.1. Archivo de Procesos (`data/procesos.txt`)

Cada línea debe tener el formato:

```
<PID>, <BurstTime>, <ArrivalTime>, <Priority>
```

donde:

* **PID**: identificador único (ej. "P1").
* **BurstTime (BT)**: tiempo de CPU que necesitará el proceso (entero).
* **ArrivalTime (AT)**: ciclo de llegada al sistema (entero).
* **Priority**: prioridad numérica (1 = mayor prioridad) (entero).

Ejemplo mínimo:

```txt
P1, 6, 0, 2
P2, 4, 1, 3
P3, 8, 2, 1
P4, 3, 3, 4
P5, 5, 4, 2
```

Guarda esto en `data/procesos.txt`.

### 4.2. (Opcional) Archivos para Sincronización

Si deseas probar el modo "Sincronización", crea también:

#### `data/recursos.txt`

Formato por línea:

```
<Recurso>, <CantidadInicial>
```

ejemplo:

```
mutex1, 1
mutex2, 1
semaforo1, 3
semaforo2, 2
```

#### `data/acciones.txt`

Formato por línea:

```
<PID>, <TIPO>, <Recurso>, <Ciclo>
```

ejemplo:

```
P1, READ, mutex1, 2
P1, WRITE, mutex1, 3
P2, READ, mutex1, 1
P2, WRITE, semaforo1, 2
P3, READ, semaforo1, 3
P3, WRITE, mutex2, 5
P4, READ, mutex2, 1
P5, WRITE, semaforo2, 2
```

Guárdalos en su carpeta: `data/recursos.txt` y `data/acciones.txt`.

> **Importante**: Si cualquiera de estos archivos no existe o no respeta el formato, el simulador mostrará un error al cargar. Usa estrictamente el formato indicado.

---

## 5. Compilar el Proyecto

En la raíz del proyecto (`/home/usuario/Simulador`), verifica que tengas el siguiente `Makefile`. Si no, créalo o reemplázalo con el contenido de abajo.

### 5.1. Makefile

```makefile
# -------------------------------------------------------------
# Makefile para compilar Simulador (SFML + ImGui + ImGui-SFML)
# -------------------------------------------------------------

# Compilador y banderas de compilación (modo debug: -g, sin optimizar)
CXX      := g++
CXXFLAGS := -std=c++17 -g -O0 -Wall \
            -Iinclude \
            -Iimgui \
            -Iimgui-sfml \
            -I/usr/include/SFML \
            -DIMGUI_SFML_DEBUG_LOG

# Bibliotecas para enlazar (SFML y dependencias de OpenGL)
LDFLAGS  := -lsfml-graphics \
            -lsfml-window   \
            -lsfml-system   \
            -lGL            \
            -ldl            \
            -lpthread       \
            -fsanitize=address

# Archivos fuente:
SRCS := \
    src/main.cpp \
    imgui/imgui.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_widgets.cpp \
    imgui/imgui_tables.cpp \
    imgui/imgui_demo.cpp \
    imgui-sfml/imgui-SFML.cpp

# Objetos correspondientes (cambia .cpp a .o)
OBJS := $(SRCS:.cpp=.o)

# Nombre del ejecutable
TARGET := simulador

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Regla genérica: compilar .cpp a .o
%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
```

### 5.2. Pasos para compilar

```bash
cd /home/usuario/Simulador
make clean   # Elimina binarios y objetos previos
make         # Compila todo en modo debug con AddressSanitizer
```

* El ejecutable resultante se llamará `simulador` y estará en `/home/usuario/Simulador/simulador`.
* Si hay errores de compilación, revisa los mensajes en la terminal: pueden faltar dependencias (p. ej., SFML o bibliotecas de OpenGL).

---

## 6. Ejecutar y Probar el Simulador

### 6.1. Ejecutar desde la carpeta correcta

1. Abre una terminal y sitúate en la carpeta del proyecto:

   ```bash
   cd /home/usuario/Simulador
   ```
2. Ejecuta el simulador:

   ```bash
   ./simulador
   ```

> **Importante**: Si ejecutas `./simulador` desde otra carpeta, las rutas relativas (`data/procesos.txt`) no funcionarán. Asegúrate siempre de estar en `/home/usuario/Simulador`.

### 6.2. Interfaz Gráfica

La ventana se divide en dos paneles:

* **Panel Izquierdo (75 %)**: "Diagrama de Gantt". Al inicio muestra:

  > Ejecute la simulación para visualizar el diagrama de Gantt.
* **Panel Derecho (25 %)**: "Controles del Simulador", que contiene:
