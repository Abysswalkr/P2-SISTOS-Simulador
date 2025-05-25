#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <string>
#include <vector>

// Estructura para un Proceso
struct Proceso {
    std::string pid;
    int burstTime;      // BT - Tiempo de ejecución
    int arrivalTime;    // AT - Tiempo de llegada
    int priority;       // Prioridad (1 = más alta)
    
    // Variables para el algoritmo
    int remainingTime;  // Para SRTF y Round Robin
    int waitingTime;    // Tiempo de espera
    int completionTime; // Tiempo de finalización
    int startTime;      // Primera vez que se ejecuta
    int responseTime;   // Tiempo de respuesta (startTime - arrivalTime)
    bool started;       // Si ya empezó a ejecutarse
    
    // Constructor
    Proceso(std::string p, int bt, int at, int pr) 
        : pid(p), burstTime(bt), arrivalTime(at), priority(pr),
          remainingTime(bt), waitingTime(0), completionTime(0), 
          startTime(-1), responseTime(0), started(false) {}
};

// Estructura para un Recurso (para sincronización)
struct Recurso {
    std::string nombre;
    int contador;       // 1 para mutex, >1 para semáforo
    int contadorOriginal;
    std::vector<std::string> colaEspera; // PIDs esperando
    
    Recurso(std::string n, int c) 
        : nombre(n), contador(c), contadorOriginal(c) {}
};

// Estructura para una Acción (para sincronización)
struct Accion {
    std::string pid;
    std::string tipo;   // READ o WRITE
    std::string recurso;
    int ciclo;
    
    Accion(std::string p, std::string t, std::string r, int c)
        : pid(p), tipo(t), recurso(r), ciclo(c) {}
};

// Estructura para un evento en el diagrama de Gantt
struct EventoGantt {
    std::string pid;
    int inicio;
    int fin;
    std::string estado; // "RUNNING", "WAITING", "ACCESSED"
    
    EventoGantt(std::string p, int i, int f, std::string e = "RUNNING")
        : pid(p), inicio(i), fin(f), estado(e) {}
};

#endif