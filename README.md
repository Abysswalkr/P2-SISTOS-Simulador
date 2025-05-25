# Simulador de Algoritmos de Calendarización y Sincronización

Este proyecto implementa un simulador visual de algoritmos de calendarización de procesos con soporte para sincronización mediante mutex y semáforos.

## Características

### Algoritmos de Calendarización Implementados
- **FIFO (First In First Out)**: Los procesos se ejecutan en orden de llegada
- **SJF (Shortest Job First)**: Se ejecuta primero el proceso más corto (no apropiativo)
- **SRTF (Shortest Remaining Time First)**: Versión apropiativa de SJF
- **Round Robin**: Los procesos se ejecutan en turnos con un quantum de tiempo
- **Priority**: Los procesos se ejecutan según su prioridad (con envejecimiento)

### Sincronización
- Soporte para **mutex** (exclusión mutua)
- Soporte para **semáforos** con contador
- Visualización de estados: Running, Waiting, Accessed Resource

## Estructura del Proyecto
```
P2-SISTOS-Simulador/
├── src/                # Código fuente
│   └── main.cpp
├── include/            # Archivos de cabecera
│   ├── estructuras.h
│   ├── parser.h
│   ├── simulador_calendarizacion.h
│   ├── simulador_sincronizacion.h
│   └── gui.h
├── data/               # Archivos de datos
│   ├── procesos.txt
│   ├── recursos.txt
│   └── acciones.txt
├── docs/               # Documentación
│   └── Definición de Proyecto Simulador 2025.pdf
├── Makefile
└── README.md
```

## Requisitos
- C++11 o superior
- SFML 2.5 o superior
- Make

## Instalación

### Ubuntu/Debian
```bash
sudo apt-get install libsfml-dev g++ make
```

### macOS
```bash
brew install sfml
```

## Compilación
```bash
make
```

## Ejecución
```bash
make run
# o directamente
./simulador
```

## Formato de Archivos de Entrada

Los archivos de entrada deben estar ubicados en el directorio `data/`.

### data/procesos.txt
```
<PID>, <BT>, <AT>, <Priority>
```
- PID: Identificador del proceso
- BT: Burst Time (tiempo de ejecución)
- AT: Arrival Time (tiempo de llegada)
- Priority: Prioridad (1 = más alta)

Ejemplo:
```
P1, 6, 0, 2
P2, 4, 1, 3
P3, 8, 2, 1
```

### data/recursos.txt
```
<Nombre>, <Contador>
```
- Nombre: Identificador del recurso
- Contador: 1 para mutex, >1 para semáforo

Ejemplo:
```
mutex1, 1
semaforo1, 3
```

### data/acciones.txt
```
<PID>, <Acción>, <Recurso>, <Ciclo>
```
- PID: Proceso que realiza la acción
- Acción: READ o WRITE
- Recurso: Nombre del recurso a acceder
- Ciclo: Ciclo en el que se realiza la acción

Ejemplo:
```
P1, READ, mutex1, 2
P2, WRITE, semaforo1, 3
```

## Uso del Simulador

1. **Cargar archivos**: Usar los botones para cargar procesos.txt, recursos.txt y acciones.txt
2. **Seleccionar algoritmo**: Click en el botón del algoritmo deseado
3. **Configurar quantum**: Para Round Robin, usar el botón "Set Quantum" (cicla entre valores 1-5)
4. **Ejecutar**: Click en "Ejecutar" para iniciar la simulación
5. **Navegar**: Usar la rueda del mouse para hacer scroll en el diagrama de Gantt

## Interpretación del Diagrama de Gantt
- **Colores de procesos**: Cada proceso tiene un color único cuando está ejecutándose
- **Rojo claro**: Proceso esperando por un recurso
- **Verde claro**: Proceso accediendo a un recurso
- **Métricas**: Se muestran el tiempo promedio de espera, respuesta y finalización

## Estructura del Código
- `include/estructuras.h`: Define las estructuras de datos principales
- `include/parser.h`: Parseo de archivos de entrada
- `include/simulador_calendarizacion.h`: Implementación de algoritmos de calendarización
- `include/simulador_sincronizacion.h`: Extensión con soporte para sincronización
- `include/gui.h`: Interfaz gráfica con SFML
- `src/main.cpp`: Punto de entrada del programa

## Notas de Implementación
- El simulador ejecuta ciclo por ciclo para SRTF y Round Robin
- El envejecimiento en Priority ocurre cada 5 ciclos
- Los recursos se liberan automáticamente después de 1 ciclo de uso
- La sincronización se simula sobre el resultado del algoritmo de calendarización
- Los archivos de datos deben estar en el directorio `data/`

# Guía Rápida de Compilación y Ejecución

## Compilar el proyecto
```bash
make
```

## Ejecutar el simulador
```bash
make run
```

## Limpiar archivos compilados
```bash
make clean
```

## Estructura de archivos de datos

Los archivos de entrada deben estar en el directorio `data/`:
- `data/procesos.txt` - Lista de procesos
- `data/recursos.txt` - Lista de recursos (mutex/semáforos)
- `data/acciones.txt` - Acciones de sincronización

## Uso del simulador

1. Al ejecutar, se abrirá una ventana gráfica
2. Cargar los archivos usando los botones correspondientes
3. Seleccionar el algoritmo deseado
4. Para Round Robin, ajustar el quantum con el botón "Set Quantum"
5. Hacer clic en "Ejecutar" para ver la simulación

## Solución de problemas

Si hay errores de compilación relacionados con SFML:
- En Ubuntu/Debian: `sudo apt-get install libsfml-dev`
- En macOS: `brew install sfml`
- En Windows: Descargar SFML de https://www.sfml-dev.org/ 