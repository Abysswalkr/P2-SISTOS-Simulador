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
    
    // Implementación de SJF (Shortest Job First)
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
        std::vector<Proceso*> pendientes;
        std::vector<Proceso*> listos;
        Proceso* procesoActual = nullptr;
        
        // Crear punteros a los procesos
        for (auto& p : procesos) {
            pendientes.push_back(&p);
        }
        
        tiempoActual = 0;
        
        while (!pendientes.empty() || !listos.empty() || procesoActual) {
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
            
            // Si hay un proceso en ejecución, verificar si debe ser reemplazado
            if (procesoActual && procesoActual->remainingTime > 0) {
                // Buscar si hay un proceso con menor tiempo restante
                Proceso* menorTiempo = procesoActual;
                for (auto* p : listos) {
                    if (p->remainingTime < menorTiempo->remainingTime) {
                        menorTiempo = p;
                    }
                }
                
                // Si hay un proceso con menor tiempo, hacer cambio de contexto
                if (menorTiempo != procesoActual) {
                    listos.push_back(procesoActual);
                    procesoActual = menorTiempo;
                    listos.erase(std::find(listos.begin(), listos.end(), menorTiempo));
                }
            }
            
            // Si no hay proceso actual, seleccionar el de menor tiempo restante
            if (!procesoActual && !listos.empty()) {
                // Ordenar por tiempo restante
                std::sort(listos.begin(), listos.end(), 
                    [](Proceso* a, Proceso* b) {
                        return a->remainingTime < b->remainingTime;
                    });
                
                procesoActual = listos.front();
                listos.erase(listos.begin());
            }
            
            // Si hay un proceso para ejecutar
            if (procesoActual) {
                // Marcar inicio si es la primera vez
                if (!procesoActual->started) {
                    procesoActual->startTime = tiempoActual;
                    procesoActual->started = true;
                }
                
                // Ejecutar por 1 unidad de tiempo
                int inicioEvento = tiempoActual;
                procesoActual->remainingTime--;
                tiempoActual++;
                
                // Registrar evento
                eventos.push_back(EventoGantt(procesoActual->pid, inicioEvento, tiempoActual));
                
                // Si el proceso terminó
                if (procesoActual->remainingTime == 0) {
                    procesoActual->completionTime = tiempoActual;
                    procesoActual->waitingTime = procesoActual->completionTime - 
                                               procesoActual->arrivalTime - 
                                               procesoActual->burstTime;
                    procesoActual = nullptr;
                }
            } else if (!pendientes.empty()) {
                // CPU idle - avanzar al siguiente arrival
                tiempoActual = (*std::min_element(pendientes.begin(), 
                    pendientes.end(), [](Proceso* a, Proceso* b) {
                        return a->arrivalTime < b->arrivalTime;
                    }))->arrivalTime;
            }
        }
    }
    
    // Implementación de Round Robin
    void ejecutarRoundRobin() {
        std::queue<Proceso*> colaListos;
        std::vector<Proceso*> pendientes;
        Proceso* procesoActual = nullptr;
        int tiempoQuantum = 0;
        
        // Crear punteros a los procesos
        for (auto& p : procesos) {
            pendientes.push_back(&p);
        }
        
        // Ordenar pendientes por tiempo de llegada
        std::sort(pendientes.begin(), pendientes.end(), 
            [](Proceso* a, Proceso* b) {
                return a->arrivalTime < b->arrivalTime;
            });
        
        tiempoActual = 0;
        
        while (!pendientes.empty() || !colaListos.empty() || procesoActual) {
            // Mover procesos que han llegado a la cola de listos
            auto it = pendientes.begin();
            while (it != pendientes.end()) {
                if ((*it)->arrivalTime <= tiempoActual) {
                    colaListos.push(*it);
                    it = pendientes.erase(it);
                } else {
                    ++it;
                }
            }
            
            // Si el proceso actual agotó su quantum o terminó
            if (procesoActual && (tiempoQuantum >= quantum || procesoActual->remainingTime == 0)) {
                if (procesoActual->remainingTime > 0) {
                    // Volver a la cola si no ha terminado
                    colaListos.push(procesoActual);
                }
                procesoActual = nullptr;
                tiempoQuantum = 0;
            }
            
            // Seleccionar siguiente proceso si no hay uno actual
            if (!procesoActual && !colaListos.empty()) {
                procesoActual = colaListos.front();
                colaListos.pop();
                tiempoQuantum = 0;
                
                // Marcar inicio si es la primera vez
                if (!procesoActual->started) {
                    procesoActual->startTime = tiempoActual;
                    procesoActual->started = true;
                }
            }
            
            // Ejecutar proceso actual
            if (procesoActual) {
                int inicioEvento = tiempoActual;
                int tiempoEjecucion = std::min(quantum - tiempoQuantum, 
                                              procesoActual->remainingTime);
                
                procesoActual->remainingTime -= tiempoEjecucion;
                tiempoActual += tiempoEjecucion;
                tiempoQuantum += tiempoEjecucion;
                
                // Registrar evento
                eventos.push_back(EventoGantt(procesoActual->pid, inicioEvento, tiempoActual));
                
                // Si el proceso terminó
                if (procesoActual->remainingTime == 0) {
                    procesoActual->completionTime = tiempoActual;
                    procesoActual->waitingTime = procesoActual->completionTime - 
                                               procesoActual->arrivalTime - 
                                               procesoActual->burstTime;
                }
            } else if (!pendientes.empty()) {
                // CPU idle - avanzar al siguiente arrival
                tiempoActual = pendientes.front()->arrivalTime;
            }
        }
    }
    
    // Implementación de Priority con envejecimiento
    void ejecutarPriority() {
        std::vector<Proceso*> pendientes;
        std::vector<Proceso*> listos;
        
        // Crear punteros a los procesos y agregar edad inicial
        for (auto& p : procesos) {
            pendientes.push_back(&p);
        }
        
        tiempoActual = 0;
        int tiempoUltimaActualizacion = 0;
        
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
            
            // Aplicar envejecimiento cada 5 unidades de tiempo
            if (tiempoActual - tiempoUltimaActualizacion >= 5) {
                for (auto* p : listos) {
                    if (p->priority > 1) {
                        p->priority--;  // Mejorar prioridad (1 es la más alta)
                    }
                }
                tiempoUltimaActualizacion = tiempoActual;
            }
            
            if (listos.empty() && !pendientes.empty()) {
                // CPU idle - avanzar al siguiente arrival
                tiempoActual = (*std::min_element(pendientes.begin(), 
                    pendientes.end(), [](Proceso* a, Proceso* b) {
                        return a->arrivalTime < b->arrivalTime;
                    }))->arrivalTime;
                continue;
            }
            
            // Ordenar por prioridad (1 = más alta prioridad)
            // En caso de empate, usar FIFO (arrival time)
            std::sort(listos.begin(), listos.end(), 
                [](Proceso* a, Proceso* b) {
                    if (a->priority == b->priority) {
                        return a->arrivalTime < b->arrivalTime;
                    }
                    return a->priority < b->priority;
                });
            
            // Ejecutar el proceso de mayor prioridad
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
                
                // Restaurar prioridad original para futuras simulaciones
                for (auto& proc : procesos) {
                    if (proc.pid == p->pid) {
                        p->priority = proc.priority;
                        break;
                    }
                }
            }
        }
    }
};

#endif