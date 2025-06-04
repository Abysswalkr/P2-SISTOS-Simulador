// main.cpp
// Interfaz gráfica completa para el Simulador de Calendarización y Sincronización
// Utiliza SFML 2.5, ImGui-SFML, y las clases de simulación ya existentes

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>
#include <cmath>
#include "parser.h"
#include "simulador_calendarizacion.h"
#include "simulador_sincronizacion.h"

// ------------------------------------------------------------
// Helper Functions
// ------------------------------------------------------------

// Genera un color único a partir de un PID (hash simple)
ImU32 ColorFromPID(const std::string& pid) {
    std::hash<std::string> hasher;
    size_t h = hasher(pid);
    uint8_t r = static_cast<uint8_t>((h & 0xFF0000) >> 16);
    uint8_t g = static_cast<uint8_t>((h & 0x00FF00) >> 8);
    uint8_t b = static_cast<uint8_t>(h & 0x0000FF);
    return IM_COL32(r, g, b, 255);
}

// Convierte ciclos en coordenadas X en pantalla
float CycleToX(int ciclo, float pixelsPerCycle) {
    return static_cast<float>(ciclo) * pixelsPerCycle;
}

// ------------------------------------------------------------
// Estructuras para GUI
// ------------------------------------------------------------

enum class SimulationType {
    Calendarizacion,
    Sincronizacion
};

enum class SyncMode {
    Mutex,
    Semaforo
};

// ------------------------------------------------------------
// Main
// ------------------------------------------------------------

int main() {
    // Crear ventana SFML
    sf::RenderWindow window({1280, 768}, "Simulador de Calendarizacion y Sincronizacion");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    // Variables de GUI
    SimulationType simType = SimulationType::Calendarizacion;
    SyncMode syncMode = SyncMode::Mutex;

    // Archivos de entrada (por defecto dentro de data/)
    char procesosFile[256]    = "data/procesos.txt";
    char recursosFile[256]    = "data/recursos.txt";
    char accionesFile[256]    = "data/acciones.txt";

    // Selección de algoritmos para Calendarización
    bool algoFIFO   = true;
    bool algoSJF    = false;
    bool algoSRTF   = false;
    bool algoRR     = false;
    bool algoPRIO   = false;
    int  quantum    = 2;  // Valor por defecto

    // Contenedor de procesos, recursos y acciones
    std::vector<Proceso> procesos;
    std::vector<Recurso> recursos;
    std::vector<Accion> acciones;

    // Instancias de simuladores
    SimuladorCalendarizacion simuladorCal;
    SimuladorSincronizacion simuladorSync;

    // Eventos de Gantt resultantes
    std::vector<EventoGantt> eventosGantt;

    // Métricas (tras simulación)
    float avgWaitingTime    = 0.0f;
    float avgCompletionTime = 0.0f;
    float avgResponseTime   = 0.0f;

    // Estados de animación
    bool datosCargados      = false;
    bool simulacionListo    = false;
    bool corriendo           = false;
    float velocidad          = 1.0f;       // Factor de velocidad (1.0 = normal)
    float tiempoSimulacion   = 0.0f;       // Tiempo acumulado en segundos
    int   currentCycle       = 0;          // Ciclo actual de la animación
    int   maxCycle           = 0;          // Ciclo máximo de la simulación
    const float segundosPorCiclo = 0.1f;   // 0.1 seg = 1 ciclo (ajustable)
    float pixelsPerCycle     = 20.0f;      // Zoom horizontal (20 pixeles = 1 ciclo)

    // Mapear PID a índice de fila (para dibujar cada proceso en su propia fila)
    std::unordered_map<std::string, int> pidToRow;

    // Temporizador SFML para ImGui-SFML
    sf::Clock deltaClock;

    while (window.isOpen()) {
        // 1) Procesar eventos SFML
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        // 2) Actualizar ImGui-SFML
        ImGui::SFML::Update(window, deltaClock.restart());

        // ----------------------------------------
        // 3) VENTANA DE CONTROLES (25% DERECHO)
        // ----------------------------------------
        ImGui::SetNextWindowPos(ImVec2(window.getSize().x * 0.75f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(window.getSize().x * 0.25f, window.getSize().y), ImGuiCond_Always);
        ImGui::Begin("Controles del Simulador", nullptr,
                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove);

        // Selección de tipo de simulación
        ImGui::Text("Tipo de Simulación:");
        ImGui::SameLine();
        if (ImGui::RadioButton("Calendarización", simType == SimulationType::Calendarizacion)) {
            simType = SimulationType::Calendarizacion;
            datosCargados = false;
            simulacionListo = false;
            corriendo = false;
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Sincronización", simType == SimulationType::Sincronizacion)) {
            simType = SimulationType::Sincronizacion;
            datosCargados = false;
            simulacionListo = false;
            corriendo = false;
        }

        ImGui::Separator();

        // Dependiendo del tipo, mostrar opciones específicas
        if (simType == SimulationType::Calendarizacion) {
            // Algoritmos disponibles
            ImGui::Text("Seleccionar Algoritmos:");
            ImGui::Checkbox("FIFO", &algoFIFO);
            ImGui::Checkbox("SJF", &algoSJF);
            ImGui::Checkbox("SRTF", &algoSRTF);
            ImGui::Checkbox("Round Robin", &algoRR);
            if (algoRR) {
                ImGui::SameLine();
                ImGui::Text("Quantum:");
                ImGui::SameLine();
                ImGui::PushItemWidth(50);
                ImGui::SliderInt("##quantum_rr", &quantum, 1, 10);
                ImGui::PopItemWidth();
            }
            ImGui::Checkbox("Priority (Envejecimiento)", &algoPRIO);

            ImGui::Separator();
            // Archivo de procesos
            ImGui::Text("Archivo de Procesos (.txt):");
            ImGui::PushItemWidth(-1);
            ImGui::InputText("##procesosPath", procesosFile, IM_ARRAYSIZE(procesosFile));
            ImGui::PopItemWidth();

        } else {
            // Simulador de Sincronización
            ImGui::Text("Modo de Sincronización:");
            ImGui::SameLine();
            if (ImGui::RadioButton("Mutex", syncMode == SyncMode::Mutex)) {
                syncMode = SyncMode::Mutex;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Semáforo", syncMode == SyncMode::Semaforo)) {
                syncMode = SyncMode::Semaforo;
            }
            ImGui::Separator();
            // Archivos de recursos y acciones
            ImGui::Text("Archivo de Procesos (.txt):");
            ImGui::PushItemWidth(-1);
            ImGui::InputText("##procesosPath", procesosFile, IM_ARRAYSIZE(procesosFile));
            ImGui::PopItemWidth();

            ImGui::Text("Archivo de Recursos (.txt):");
            ImGui::PushItemWidth(-1);
            ImGui::InputText("##recursosPath", recursosFile, IM_ARRAYSIZE(recursosFile));
            ImGui::PopItemWidth();

            ImGui::Text("Archivo de Acciones (.txt):");
            ImGui::PushItemWidth(-1);
            ImGui::InputText("##accionesPath", accionesFile, IM_ARRAYSIZE(accionesFile));
            ImGui::PopItemWidth();
        }

        ImGui::Separator();

        // Botón para cargar datos
        if (ImGui::Button("Cargar Archivos")) {
            try {
                // Cargar procesos
                procesos = Parser::cargarProcesos(procesosFile);

                if (simType == SimulationType::Sincronizacion) {
                    // Cargar recursos y acciones
                    recursos = Parser::cargarRecursos(recursosFile);
                    acciones = Parser::cargarAcciones(accionesFile);
                }

                // Verificar al menos un algoritmo seleccionado (para Calendarización)
                if (simType == SimulationType::Calendarizacion) {
                    if (!(algoFIFO || algoSJF || algoSRTF || algoRR || algoPRIO)) {
                        throw std::runtime_error("Debe seleccionar al menos un algoritmo.");
                    }
                }

                datosCargados = true;
                simulacionListo = false;
                corriendo = false;
                ImGui::OpenPopup("Éxito");
            } catch (const std::exception& e) {
                ImGui::OpenPopup("Error");
            }
        }

        // Pop-up de éxito/error
        if (ImGui::BeginPopupModal("Éxito", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Archivos cargados correctamente.");
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Error al cargar archivos. Verifique las rutas y formatos.");
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Separator();

        // Mostrar información cargada para revisión
        if (datosCargados) {
            ImGui::Text("Información Cargada:");
            ImGui::Text("Procesos:");
            ImGui::BeginChild("##listaProcesos", ImVec2(0, 80), true);
            for (const auto& p : procesos) {
                ImGui::Text("- %s (BT=%d, AT=%d, PRIO=%d)", p.pid.c_str(), p.burstTime, p.arrivalTime, p.priority);
            }
            ImGui::EndChild();

            if (simType == SimulationType::Sincronizacion) {
                ImGui::Text("Recursos:");
                ImGui::BeginChild("##listaRecursos", ImVec2(0, 60), true);
                for (const auto& r : recursos) {
                    ImGui::Text("- %s (Cnt=%d)", r.nombre.c_str(), r.contadorOriginal);
                }
                ImGui::EndChild();

                ImGui::Text("Acciones:");
                ImGui::BeginChild("##listaAcciones", ImVec2(0, 80), true);
                for (const auto& a : acciones) {
                    ImGui::Text("- %s %s %s en ciclo %d", a.pid.c_str(), a.tipo.c_str(), a.recurso.c_str(), a.ciclo);
                }
                ImGui::EndChild();
            }
        }

        ImGui::Separator();

        // Botón para ejecutar simulación
        if (ImGui::Button(simulacionListo ? "Reiniciar Simulación" : "Ejecutar Simulación")) {
            if (!datosCargados) {
                ImGui::OpenPopup("ErrorSim");
            } else {
                // Preparar simuladores y ejecutar
                eventosGantt.clear();
                pidToRow.clear();
                currentCycle = 0;
                tiempoSimulacion = 0.0f;
                corriendo = false;

                // Mapear PIDs a filas
                for (size_t i = 0; i < procesos.size(); ++i) {
                    pidToRow[procesos[i].pid] = static_cast<int>(i);
                }

                if (simType == SimulationType::Calendarizacion) {
                    simuladorCal.cargarProcesos(procesos);
                    simuladorCal.setQuantum(quantum);

                    int offsetFila = 0;
                    std::vector<Proceso> originalProcesos = procesos; // guardamos copia

                    if (algoFIFO) {
                        simuladorCal.cargarProcesos(originalProcesos);
                        simuladorCal.ejecutar(TipoAlgoritmo::FIFO);
                        auto ev = simuladorCal.getEventos();
                        for (auto& e : ev) {
                            int row = pidToRow[e.pid] + offsetFila * originalProcesos.size();
                            e.estado = "RUNNING";
                            e.pid = e.pid + "|FIFO";
                            procesos.push_back(Proceso(e.pid, 0, 0, 0));
                            pidToRow[e.pid] = row;
                            eventosGantt.push_back(e);
                        }
                        offsetFila++;
                    }
                    if (algoSJF) {
                        simuladorCal.cargarProcesos(originalProcesos);
                        simuladorCal.ejecutar(TipoAlgoritmo::SJF);
                        auto ev = simuladorCal.getEventos();
                        for (auto& e : ev) {
                            int row = pidToRow[e.pid] + offsetFila * originalProcesos.size();
                            e.estado = "RUNNING";
                            e.pid = e.pid + "|SJF";
                            procesos.push_back(Proceso(e.pid, 0, 0, 0));
                            pidToRow[e.pid] = row;
                            eventosGantt.push_back(e);
                        }
                        offsetFila++;
                    }
                    if (algoSRTF) {
                        simuladorCal.cargarProcesos(originalProcesos);
                        simuladorCal.ejecutar(TipoAlgoritmo::SRTF);
                        auto ev = simuladorCal.getEventos();
                        for (auto& e : ev) {
                            int row = pidToRow[e.pid] + offsetFila * originalProcesos.size();
                            e.estado = "RUNNING";
                            e.pid = e.pid + "|SRTF";
                            procesos.push_back(Proceso(e.pid, 0, 0, 0));
                            pidToRow[e.pid] = row;
                            eventosGantt.push_back(e);
                        }
                        offsetFila++;
                    }
                    if (algoRR) {
                        simuladorCal.cargarProcesos(originalProcesos);
                        simuladorCal.setQuantum(quantum);
                        simuladorCal.ejecutar(TipoAlgoritmo::ROUND_ROBIN);
                        auto ev = simuladorCal.getEventos();
                        for (auto& e : ev) {
                            int row = pidToRow[e.pid] + offsetFila * originalProcesos.size();
                            e.estado = "RUNNING";
                            e.pid = e.pid + "|RR";
                            procesos.push_back(Proceso(e.pid, 0, 0, 0));
                            pidToRow[e.pid] = row;
                            eventosGantt.push_back(e);
                        }
                        offsetFila++;
                    }
                    if (algoPRIO) {
                        simuladorCal.cargarProcesos(originalProcesos);
                        simuladorCal.ejecutar(TipoAlgoritmo::PRIORITY);
                        auto ev = simuladorCal.getEventos();
                        for (auto& e : ev) {
                            int row = pidToRow[e.pid] + offsetFila * originalProcesos.size();
                            e.estado = "RUNNING";
                            e.pid = e.pid + "|PRIO";
                            procesos.push_back(Proceso(e.pid, 0, 0, 0));
                            pidToRow[e.pid] = row;
                            eventosGantt.push_back(e);
                        }
                        offsetFila++;
                    }

                    // Métricas promedio (basado en el último algoritmo ejecutado)
                    avgWaitingTime    = simuladorCal.getAvgWaitingTime();
                    avgCompletionTime = simuladorCal.getAvgCompletionTime();
                    avgResponseTime   = simuladorCal.getAvgResponseTime();

                } else {
                    // Sincronización
                    simuladorSync.cargarProcesos(procesos);
                    simuladorSync.setQuantum(quantum);
                    simuladorSync.cargarRecursos(recursos);
                    simuladorSync.cargarAcciones(acciones);
                    simuladorSync.ejecutar(TipoAlgoritmo::FIFO); // FIFO para calendarización previa
                    eventosGantt = simuladorSync.getEventos();

                    // Métricas promedio (usa valores de calendarización interna)
                    avgWaitingTime    = simuladorSync.getAvgWaitingTime();
                    avgCompletionTime = simuladorSync.getAvgCompletionTime();
                    avgResponseTime   = simuladorSync.getAvgResponseTime();
                }

                // Encontrar ciclo máximo
                maxCycle = 0;
                for (const auto& e : eventosGantt) {
                    if (e.fin > maxCycle) {
                        maxCycle = e.fin;
                    }
                }
                simulacionListo = true;
                corriendo = false;
                ImGui::OpenPopup("SimListo");
            }
        }

        if (ImGui::BeginPopupModal("ErrorSim", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Primero debe cargar los datos correctamente.");
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopupModal("SimListo", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextUnformatted("Simulación lista para visualizar.");
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Separator();

        // Mostrar métricas una vez simulado
        if (simulacionListo) {
            ImGui::Text("Métricas de Eficiencia:");
            ImGui::Text("Avg Waiting Time    : %.2f", avgWaitingTime);
            ImGui::Text("Avg Completion Time : %.2f", avgCompletionTime);
            ImGui::Text("Avg Response Time   : %.2f", avgResponseTime);

            ImGui::Separator();
            // Controles de animación
            ImGui::Text("Animación:");
            ImGui::SliderFloat("Velocidad", &velocidad, 0.1f, 10.0f, "%.1f");
            if (ImGui::Button(corriendo ? "Pausar" : "Iniciar")) {
                corriendo = !corriendo;
                if (!corriendo) {
                    // Si se pausó, no reiniciar el tiempo
                } else {
                    // Si inicia por primera vez, resetear tiempo
                    if (currentCycle >= maxCycle) {
                        currentCycle = 0;
                        tiempoSimulacion = 0.0f;
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Reiniciar")) {
                corriendo = false;
                currentCycle = 0;
                tiempoSimulacion = 0.0f;
            }
            ImGui::Text("Ciclo Actual: %d / %d", currentCycle, maxCycle);
        }

        ImGui::End(); // << Fin de "Controles del Simulador"

        // ---------------------------------------------------
        // 4) VENTANA DE GANTT (75% IZQUIERDO)
        // ---------------------------------------------------
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(window.getSize().x * 0.75f, window.getSize().y), ImGuiCond_Always);
        ImGui::Begin("Diagrama de Gantt", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

        if (simulacionListo) {
            // Actualizar animación si está corriendo
            if (corriendo) {
                tiempoSimulacion += deltaClock.getElapsedTime().asSeconds() * velocidad;
                int cicloAvanzado = static_cast<int>(tiempoSimulacion / segundosPorCiclo);
                if (cicloAvanzado > currentCycle) {
                    currentCycle = cicloAvanzado;
                    if (currentCycle > maxCycle) {
                        currentCycle = maxCycle;
                        corriendo = false;
                    }
                }
            }

            ImGui::BeginChild("##GanttScrollArea", ImVec2(0, 0), true,
                              ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove);

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 origin = ImGui::GetCursorScreenPos();

            // Tamaños de filas
            float rowHeight = 24.0f;
            float labelWidth = 60.0f;
            float contentHeight = (static_cast<float>(procesos.size()) * rowHeight) + 30.0f;
            float contentWidth = CycleToX(maxCycle + 1, pixelsPerCycle) + labelWidth + 50.0f;

            // Dibujar líneas horizontales por fila
            for (size_t i = 0; i < procesos.size(); ++i) {
                float y = origin.y + 5 + i * rowHeight;
                drawList->AddLine(
                    ImVec2(origin.x + labelWidth, y),
                    ImVec2(origin.x + contentWidth, y),
                    IM_COL32(70, 70, 70, 255), 1.0f
                );
                // Etiqueta de PID
                drawList->AddText(
                    ImVec2(origin.x + 2, y - (rowHeight / 2) + 6),
                    IM_COL32(200, 200, 200, 255),
                    procesos[i].pid.c_str()
                );
            }

            // Dibujar eventos de Gantt hasta currentCycle
            for (const auto& e : eventosGantt) {
                if (e.fin <= currentCycle) {
                    int fila = pidToRow[e.pid];
                    float y = origin.y + 5 + fila * rowHeight;
                    float x0 = origin.x + labelWidth + CycleToX(e.inicio, pixelsPerCycle);
                    float x1 = origin.x + labelWidth + CycleToX(e.fin, pixelsPerCycle);

                    ImU32 col;
                    if (e.estado == "RUNNING") {
                        col = ColorFromPID(e.pid);
                    } else if (e.estado == "WAITING") {
                        col = IM_COL32(200, 50, 50, 200);
                    } else if (e.estado == "ACCESSED") {
                        col = IM_COL32(50, 200, 50, 200);
                    } else {
                        col = ColorFromPID(e.pid);
                    }

                    drawList->AddRectFilled(
                        ImVec2(x0, y - (rowHeight / 2) + 4),
                        ImVec2(x1, y + (rowHeight / 2) - 4),
                        col
                    );
                    drawList->AddText(
                        ImVec2(x0 + 2, y - 5),
                        IM_COL32(0, 0, 0, 255),
                        std::to_string(e.inicio).c_str()
                    );
                }
            }

            // Línea vertical que marca currentCycle
            float xLine = origin.x + labelWidth + CycleToX(currentCycle, pixelsPerCycle);
            drawList->AddLine(
                ImVec2(xLine, origin.y + 2),
                ImVec2(xLine, origin.y + contentHeight),
                IM_COL32(255, 255, 0, 200), 2.0f
            );

            // Espacio final para scrollbar
            ImGui::Dummy(ImVec2(contentWidth, contentHeight));
            ImGui::EndChild();
        } else {
            ImGui::Text("Ejecute la simulacion para visualizar el diagrama de Gantt.");
        }

        ImGui::End(); // << Fin de "Diagrama de Gantt"

        // ---------------------------------------------------
        // 5) Renderizar
        // ---------------------------------------------------
        window.clear({30, 30, 30});
        ImGui::SFML::Render(window);
        window.display();
    }

    // Shutdown ImGui-SFML
    ImGui::SFML::Shutdown();
    return 0;
}
