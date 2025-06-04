#ifndef SIMULADOR_SINCRONIZACION_H
#define SIMULADOR_SINCRONIZACION_H

#include <vector>
#include <map>
#include <algorithm>
#include <queue>
#include "estructuras.h"
#include "simulador_calendarizacion.h"

class SimuladorSincronizacion : public SimuladorCalendarizacion {
private:
    std::vector<Recurso> recursos;
    std::vector<Accion> acciones;
    std::map<std::string, int> estadoProcesos; // Ciclo actual de cada proceso
    std::map<std::string, std::vector<Accion>> accionesPorProceso;
    
public:
    SimuladorSincronizacion() : SimuladorCalendarizacion() {}
    
    // Cargar recursos y acciones
    void cargarRecursos(const std::vector<Recurso>& recs) {
        recursos = recs;
        // Resetear contadores
        for (auto& r : recursos) {
            r.contador = r.contadorOriginal;
            r.colaEspera.clear();
        }
    }
    
    void cargarAcciones(const std::vector<Accion>& acts) {
        acciones = acts;
        accionesPorProceso.clear();
        
        // Organizar acciones por proceso
        for (const auto& a : acciones) {
            accionesPorProceso[a.pid].push_back(a);
        }
        
        // Ordenar acciones por ciclo para cada proceso
        for (auto& pair : accionesPorProceso) {
            std::sort(pair.second.begin(), pair.second.end(),
                [](const Accion& a, const Accion& b) {
                    return a.ciclo < b.ciclo;
                });
        }
    }
    
    // Override ejecutar para incluir sincronización
    void ejecutar(TipoAlgoritmo tipo) {
        // Primero ejecutar el algoritmo base
        SimuladorCalendarizacion::ejecutar(tipo);
        
        // Si hay recursos y acciones, simular sincronización
        if (!recursos.empty() && !acciones.empty()) {
            simularSincronizacion();
        }
    }
    
private:
    void simularSincronizacion() {
        // Crear nuevo conjunto de eventos con sincronización
        std::vector<EventoGantt> eventosConSinc;
        std::map<std::string, bool> procesoBloqueado;
        
        // Inicializar estado
        for (auto& p : procesos) {
            estadoProcesos[p.pid] = 0;
            procesoBloqueado[p.pid] = false;
        }
        
        // Resetear recursos
        for (auto& r : recursos) {
            r.contador = r.contadorOriginal;
            r.colaEspera.clear();
        }
        
        // Procesar eventos del Gantt original
        for (const auto& evento : eventos) {
            std::string pid = evento.pid;
            int duracion = evento.fin - evento.inicio;
            int cicloInicial = estadoProcesos[pid];
            
            // Para cada ciclo del evento
            for (int ciclo = 0; ciclo < duracion; ciclo++) {
                int cicloActual = cicloInicial + ciclo;
                bool bloqueado = false;
                
                // Verificar si hay acciones en este ciclo
                if (accionesPorProceso.find(pid) != accionesPorProceso.end()) {
                    for (const auto& accion : accionesPorProceso[pid]) {
                        if (accion.ciclo == cicloActual) {
                            // Intentar ejecutar la acción
                            if (accion.tipo == "READ" || accion.tipo == "WRITE") {
                                Recurso* recurso = encontrarRecurso(accion.recurso);
                                if (recurso) {
                                    if (recurso->contador > 0) {
                                        // Adquirir recurso
                                        recurso->contador--;
                                        
                                        // Agregar evento de acceso
                                        eventosConSinc.push_back(
                                            EventoGantt(pid, evento.inicio + ciclo, 
                                                      evento.inicio + ciclo + 1, "ACCESSED")
                                        );
                                    } else {
                                        // Bloquear proceso
                                        bloqueado = true;
                                        procesoBloqueado[pid] = true;
                                        recurso->colaEspera.push_back(pid);
                                        
                                        // Agregar evento de espera
                                        eventosConSinc.push_back(
                                            EventoGantt(pid, evento.inicio + ciclo, 
                                                      evento.inicio + ciclo + 1, "WAITING")
                                        );
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
                
                // Si no está bloqueado, ejecutar normalmente
                if (!bloqueado && !procesoBloqueado[pid]) {
                    eventosConSinc.push_back(
                        EventoGantt(pid, evento.inicio + ciclo, 
                                  evento.inicio + ciclo + 1, "RUNNING")
                    );
                    estadoProcesos[pid]++;
                } else if (procesoBloqueado[pid]) {
                    // Verificar si se puede desbloquear
                    if (intentarDesbloquear(pid)) {
                        procesoBloqueado[pid] = false;
                        eventosConSinc.push_back(
                            EventoGantt(pid, evento.inicio + ciclo, 
                                      evento.inicio + ciclo + 1, "RUNNING")
                        );
                        estadoProcesos[pid]++;
                    } else {
                        eventosConSinc.push_back(
                            EventoGantt(pid, evento.inicio + ciclo, 
                                      evento.inicio + ciclo + 1, "WAITING")
                        );
                    }
                }
                
                // Liberar recursos al final del ciclo si es necesario
                liberarRecursosCompletados(pid, cicloActual);
            }
        }
        
        // Actualizar eventos con la sincronización
        eventos = eventosConSinc;
    }
    
    Recurso* encontrarRecurso(const std::string& nombre) {
        for (auto& r : recursos) {
            if (r.nombre == nombre) {
                return &r;
            }
        }
        return nullptr;
    }
    
    bool intentarDesbloquear(const std::string& pid) {
        // Verificar si el proceso puede adquirir algún recurso esperado
        for (auto& r : recursos) {
            auto it = std::find(r.colaEspera.begin(), r.colaEspera.end(), pid);
            if (it != r.colaEspera.end() && r.contador > 0) {
                // Puede adquirir el recurso
                r.contador--;
                r.colaEspera.erase(it);
                return true;
            }
        }
        return false;
    }
    
    void liberarRecursosCompletados(const std::string& pid, int ciclo) {
        // Aquí se podría implementar la lógica para liberar recursos
        // después de cierto tiempo o cuando se complete una operación
        // Por simplicidad, asumimos que los recursos se liberan después de 1 ciclo
        
        // Buscar acciones completadas
        if (accionesPorProceso.find(pid) != accionesPorProceso.end()) {
            for (const auto& accion : accionesPorProceso[pid]) {
                if (accion.ciclo == ciclo - 1) {
                    Recurso* recurso = encontrarRecurso(accion.recurso);
                    if (recurso && recurso->contador < recurso->contadorOriginal) {
                        recurso->contador++;
                        
                        // Despertar proceso en espera si hay alguno
                        if (!recurso->colaEspera.empty()) {
                            // El siguiente proceso en la cola será desbloqueado
                            // en la próxima iteración
                        }
                    }
                }
            }
        }
    }
};

#endif 