#ifndef SIMULADOR_CALENDARIZACION_H
#define SIMULADOR_CALENDARIZACION_H

#include <vector>
#include <queue>
#include <algorithm>
#include "estructuras.h"

enum TipoAlgoritmo {
    FIFO,
    SJF,
    SRTF,
    ROUND_ROBIN,
    PRIORITY
};

class SimuladorCalendarizacion {
protected:
    std::vector<Proceso> procesos;
    std::vector<EventoGantt> eventos;
    int tiempoActual;
    int quantum;
    
public:
    SimuladorCalendarizacion() : tiempoActual(0), quantum(2) {}
    
    // Cargar procesos
    void cargarProcesos(const std::vector<Proceso>& procs) {
        procesos = procs;
        eventos.clear();
        tiempoActual = 0;
    }
    
    // Configurar quantum para Round Robin
    void setQuantum(int q) { quantum = q; }
    
    // Ejecutar simulación según el algoritmo
    void ejecutar(TipoAlgoritmo tipo) {
        eventos.clear();
        tiempoActual = 0;
        
        // Resetear valores de los procesos
        for (auto& p : procesos) {
            p.remainingTime = p.burstTime;
            p.waitingTime = 0;
            p.completionTime = 0;
            p.startTime = -1;
            p.started = false;
        }
        
        switch(tipo) {
            case FIFO:
                ejecutarFIFO();
                break;
            case SJF:
                ejecutarSJF();
                break;
            case SRTF:
                ejecutarSRTF();
                break;
            case ROUND_ROBIN:
                ejecutarRoundRobin();
                break;
            case PRIORITY:
                ejecutarPriority();
                break;
        }
    }
    
    // Obtener eventos para el diagrama de Gantt
    std::vector<EventoGantt> getEventos() const { return eventos; }
    
    // Calcular métricas
    float getAvgWaitingTime() const {
        float total = 0;
        for (const auto& p : procesos) {
            total += p.waitingTime;
        }
        return procesos.empty() ? 0 : total / procesos.size();
    }
    
    float getAvgCompletionTime() const {
        float total = 0;
        for (const auto& p : procesos) {
            total += p.completionTime;
        }
        return procesos.empty() ? 0 : total / procesos.size();
    }
    
private:
    // Implementación de FIFO (First In First Out)
    void ejecutarFIFO() {
        // Ordenar por tiempo de llegada
        std::sort(procesos.begin(), procesos.end(), 
            [](const Proceso& a, const Proceso& b) {
                return a.arrivalTime < b.arrivalTime;
            });
        
        tiempoActual = 0;
        
        for (auto& p : procesos) {
            // Si el CPU está idle, avanzar al tiempo de llegada
            if (tiempoActual < p.arrivalTime) {
                tiempoActual = p.arrivalTime;
            }
            
            // Marcar inicio
            p.startTime = tiempoActual;
            p.waitingTime = tiempoActual - p.arrivalTime;
            
            // Ejecutar proceso completo
            eventos.push_back(EventoGantt(p.pid, tiempoActual, 
                                         tiempoActual + p.burstTime));
            
            tiempoActual += p.burstTime;
            p.completionTime = tiempoActual;
            p.remainingTime = 0;
        }
    }
    
    // Implementación de SJF (Shortest Job First) - No apropiativo
    void ejecutarSJF() {
        std::vector<Proceso*> listos;
        std::vector<Proceso*> pendientes;
        
        // Crear punteros a los procesos
        for (auto& p : procesos) {
            pendientes.push_back(&p);
        }
        
        tiempoActual = 0;
        
        while (!pendientes.empty() || !listos.empty()) {
            // Mover procesos que han llegado a la cola de listos
            auto it = pendientes.begin();
            while (it != pendientes.end()) {
                if ((*it)->arrivalTime <= tiempoActual) {
                    listos.push_back(*it);
                    it = pendientes.erase(it);
                } else {
                    ++it;
                }
            }
            
            if (listos.empty() && !pendientes.empty()) {
                // CPU idle - avanzar al siguiente arrival
                tiempoActual = (*std::min_element(pendientes.begin(), 
                    pendientes.end(), [](Proceso* a, Proceso* b) {
                        return a->arrivalTime < b->arrivalTime;
                    }))->arrivalTime;
                continue;
            }
            
            // Ordenar por burst time (shortest first)
            std::sort(listos.begin(), listos.end(), 
                [](Proceso* a, Proceso* b) {
                    return a->burstTime < b->burstTime;
                });
            
            // Ejecutar el proceso más corto
            if (!listos.empty()) {
                Proceso* p = listos.front();
                listos.erase(listos.begin());
                
                p->startTime = tiempoActual;
                p->waitingTime = tiempoActual - p->arrivalTime;
                
                eventos.push_back(EventoGantt(p->pid, tiempoActual, 
                                             tiempoActual + p->burstTime));
                
                tiempoActual += p->burstTime;
                p->completionTime = tiempoActual;
                p->remainingTime = 0;
            }
        }
    }
    
    // Implementación de SRTF (Shortest Remaining Time First)
    void ejecutarSRTF() {
        // TODO: Implementar en la siguiente iteración
    }
    
    // Implementación de Round Robin
    void ejecutarRoundRobin() {
        // TODO: Implementar en la siguiente iteración
    }
    
    // Implementación de Priority con envejecimiento
    void ejecutarPriority() {
        // TODO: Implementar en la siguiente iteración
    }
};

#endif