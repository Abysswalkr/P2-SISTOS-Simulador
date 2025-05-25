#ifndef GUI_H
#define GUI_H

#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include "simulador_sincronizacion.h"
#include "parser.h"

class GUI {
private:
    sf::RenderWindow window;
    sf::Font font;
    
    // Simulador
    SimuladorSincronizacion simulador;
    TipoAlgoritmo algoritmoActual;
    
    // Configuración visual
    const int GANTT_START_X = 50;
    const int GANTT_START_Y = 200;
    const int GANTT_HEIGHT = 60;
    const int PIXELS_PER_CYCLE = 40;
    const int WINDOW_WIDTH = 1200;
    const int WINDOW_HEIGHT = 600;
    
    // Scroll
    float scrollOffset = 0;
    
    // Quantum actual
    int quantumActual = 2;
    
    // Colores para procesos
    std::map<std::string, sf::Color> coloresProcesos;
    std::vector<sf::Color> paletaColores;
    
    // Estado de carga de archivos
    bool procesosLoaded = false;
    bool recursosLoaded = false;
    bool accionesLoaded = false;
    
    // Botones simples
    struct Boton {
        sf::RectangleShape shape;
        sf::Text texto;
        std::string id;
    };
    std::vector<Boton> botones;
    
public:
    GUI() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
                   "Simulador de Procesos - Sistemas Operativos"),
            algoritmoActual(FIFO) {
        
        // Inicializar paleta de colores
        paletaColores = {
            sf::Color(255, 99, 71),   // Tomato
            sf::Color(60, 179, 113),  // MediumSeaGreen
            sf::Color(106, 90, 205),  // SlateBlue
            sf::Color(255, 165, 0),   // Orange
            sf::Color(218, 165, 32),  // GoldenRod
            sf::Color(255, 20, 147),  // DeepPink
            sf::Color(0, 206, 209),   // DarkTurquoise
            sf::Color(255, 218, 185)  // PeachPuff
        };
        
        // Cargar fuente (usar fuente del sistema)
        if (!font.loadFromFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf") &&
            !font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf") &&
            !font.loadFromFile("/System/Library/Fonts/Helvetica.ttc")) {
            // Fuente de respaldo
        }
        
        inicializarBotones();
    }
    
    void inicializarBotones() {
        // Botones de algoritmos
        crearBoton(50, 50, 100, 30, "FIFO", "fifo");
        crearBoton(160, 50, 100, 30, "SJF", "sjf");
        crearBoton(270, 50, 100, 30, "SRTF", "srtf");
        crearBoton(380, 50, 100, 30, "RR", "rr");
        crearBoton(490, 50, 100, 30, "Priority", "priority");
        
        // Botones de control
        crearBoton(650, 50, 140, 30, "Cargar Procesos", "cargar_procesos");
        crearBoton(800, 50, 140, 30, "Cargar Recursos", "cargar_recursos");
        crearBoton(950, 50, 140, 30, "Cargar Acciones", "cargar_acciones");
        crearBoton(50, 90, 120, 30, "Ejecutar", "ejecutar");
        crearBoton(180, 90, 120, 30, "Set Quantum", "set_quantum");
    }
    
    void crearBoton(float x, float y, float w, float h, 
                    const std::string& texto, const std::string& id) {
        Boton btn;
        btn.shape.setPosition(x, y);
        btn.shape.setSize(sf::Vector2f(w, h));
        btn.shape.setFillColor(sf::Color(200, 200, 200));
        btn.shape.setOutlineColor(sf::Color::Black);
        btn.shape.setOutlineThickness(1);
        
        btn.texto.setFont(font);
        btn.texto.setString(texto);
        btn.texto.setCharacterSize(14);
        btn.texto.setFillColor(sf::Color::Black);
        
        // Centrar texto
        sf::FloatRect textBounds = btn.texto.getLocalBounds();
        btn.texto.setPosition(x + (w - textBounds.width) / 2, 
                             y + (h - textBounds.height) / 2 - 5);
        
        btn.id = id;
        botones.push_back(btn);
    }
    
    void ejecutar() {
        while (window.isOpen()) {
            manejarEventos();
            dibujar();
        }
    }
    
    void manejarEventos() {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            
            // Scroll con rueda del mouse
            if (event.type == sf::Event::MouseWheelScrolled) {
                scrollOffset -= event.mouseWheelScroll.delta * 20;
                if (scrollOffset < 0) scrollOffset = 0;
            }
            
            // Click en botones
            if (event.type == sf::Event::MouseButtonPressed) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                
                for (auto& btn : botones) {
                    if (btn.shape.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        manejarBoton(btn.id);
                    }
                }
            }
        }
    }
    
    void manejarBoton(const std::string& id) {
        if (id == "fifo") algoritmoActual = FIFO;
        else if (id == "sjf") algoritmoActual = SJF;
        else if (id == "srtf") algoritmoActual = SRTF;
        else if (id == "rr") algoritmoActual = ROUND_ROBIN;
        else if (id == "priority") algoritmoActual = PRIORITY;
        else if (id == "cargar_procesos") {
            cargarProcesos();
        }
        else if (id == "cargar_recursos") {
            cargarRecursos();
        }
        else if (id == "cargar_acciones") {
            cargarAcciones();
        }
        else if (id == "ejecutar") {
            ejecutarSimulacion();
        }
        else if (id == "set_quantum") {
            // Ciclar entre valores de quantum comunes
            quantumActual = (quantumActual % 5) + 1;
            simulador.setQuantum(quantumActual);
        }
    }
    
    void cargarProcesos() {
        try {
            std::vector<Proceso> procs = Parser::cargarProcesos("procesos.txt");
            simulador.cargarProcesos(procs);
            
            // Asignar colores a procesos
            int colorIndex = 0;
            for (const auto& p : procs) {
                coloresProcesos[p.pid] = paletaColores[colorIndex % paletaColores.size()];
                colorIndex++;
            }
            
            procesosLoaded = true;
        } catch (const std::exception& e) {
            // Manejar error
            procesosLoaded = false;
        }
    }
    
    void cargarRecursos() {
        try {
            std::vector<Recurso> recs = Parser::cargarRecursos("recursos.txt");
            simulador.cargarRecursos(recs);
            recursosLoaded = true;
        } catch (const std::exception& e) {
            // Manejar error
            recursosLoaded = false;
        }
    }
    
    void cargarAcciones() {
        try {
            std::vector<Accion> acts = Parser::cargarAcciones("acciones.txt");
            simulador.cargarAcciones(acts);
            accionesLoaded = true;
        } catch (const std::exception& e) {
            // Manejar error
            accionesLoaded = false;
        }
    }
    
    void ejecutarSimulacion() {
        if (procesosLoaded) {
            simulador.ejecutar(algoritmoActual);
        }
    }
    
    void dibujar() {
        window.clear(sf::Color::White);
        
        // Dibujar botones
        for (const auto& btn : botones) {
            // Cambiar color según estado
            if ((btn.id == "cargar_procesos" && procesosLoaded) ||
                (btn.id == "cargar_recursos" && recursosLoaded) ||
                (btn.id == "cargar_acciones" && accionesLoaded)) {
                btn.shape.setFillColor(sf::Color(150, 255, 150)); // Verde claro
            }
            
            window.draw(btn.shape);
            window.draw(btn.texto);
        }
        
        // Dibujar título
        sf::Text titulo;
        titulo.setFont(font);
        titulo.setString("Simulador de Algoritmos de Calendarizacion y Sincronizacion");
        titulo.setCharacterSize(22);
        titulo.setFillColor(sf::Color::Black);
        titulo.setPosition(WINDOW_WIDTH / 2 - 300, 10);
        window.draw(titulo);
        
        // Dibujar estado de carga
        dibujarEstadoCarga();
        
        // Dibujar diagrama de Gantt
        dibujarGantt();
        
        // Dibujar métricas
        dibujarMetricas();
        
        window.display();
    }
    
    void dibujarEstadoCarga() {
        sf::Text estado;
        estado.setFont(font);
        estado.setCharacterSize(12);
        estado.setPosition(50, 130);
        
        std::string textoEstado = "Estado: ";
        textoEstado += procesosLoaded ? "Procesos OK " : "Procesos NO ";
        textoEstado += recursosLoaded ? "| Recursos OK " : "| Recursos NO ";
        textoEstado += accionesLoaded ? "| Acciones OK" : "| Acciones NO";
        
        estado.setString(textoEstado);
        estado.setFillColor(sf::Color::Black);
        window.draw(estado);
        
        // Mostrar quantum actual si es Round Robin
        if (algoritmoActual == ROUND_ROBIN) {
            sf::Text quantumText;
            quantumText.setFont(font);
            quantumText.setString("Quantum: " + std::to_string(quantumActual) + " (Click 'Set Quantum' para cambiar)");
            quantumText.setCharacterSize(12);
            quantumText.setFillColor(sf::Color::Blue);
            quantumText.setPosition(50, 150);
            window.draw(quantumText);
        }
    }
    
    void dibujarGantt() {
        // Área de visualización con clipping
        sf::View ganttView(sf::FloatRect(scrollOffset, 0, WINDOW_WIDTH - 100, WINDOW_HEIGHT));
        ganttView.setViewport(sf::FloatRect(0, 0.3f, 1, 0.5f));
        
        // Dibujar línea de tiempo
        sf::RectangleShape lineaTiempo(sf::Vector2f(2000, 2));
        lineaTiempo.setPosition(GANTT_START_X, GANTT_START_Y + GANTT_HEIGHT + 10);
        lineaTiempo.setFillColor(sf::Color::Black);
        window.draw(lineaTiempo);
        
        // Dibujar eventos
        for (const auto& evento : simulador.getEventos()) {
            float x = GANTT_START_X + evento.inicio * PIXELS_PER_CYCLE - scrollOffset;
            float width = (evento.fin - evento.inicio) * PIXELS_PER_CYCLE;
            
            // Rectángulo del proceso
            sf::RectangleShape rect(sf::Vector2f(width, GANTT_HEIGHT));
            rect.setPosition(x, GANTT_START_Y);
            
            // Color según estado
            if (evento.estado == "RUNNING") {
                rect.setFillColor(coloresProcesos[evento.pid]);
            } else if (evento.estado == "WAITING") {
                rect.setFillColor(sf::Color(255, 200, 200)); // Rojo claro
            } else if (evento.estado == "ACCESSED") {
                rect.setFillColor(sf::Color(200, 255, 200)); // Verde claro
            }
            
            rect.setOutlineColor(sf::Color::Black);
            rect.setOutlineThickness(1);
            window.draw(rect);
            
            // Nombre del proceso
            sf::Text nombre;
            nombre.setFont(font);
            nombre.setString(evento.pid);
            nombre.setCharacterSize(12);
            nombre.setFillColor(sf::Color::Black);
            nombre.setPosition(x + width/2 - 10, GANTT_START_Y + GANTT_HEIGHT/2 - 10);
            window.draw(nombre);
        }
        
        // Dibujar marcas de tiempo
        for (int i = 0; i <= 50; i += 5) {
            float x = GANTT_START_X + i * PIXELS_PER_CYCLE - scrollOffset;
            
            sf::Text marca;
            marca.setFont(font);
            marca.setString(std::to_string(i));
            marca.setCharacterSize(12);
            marca.setFillColor(sf::Color::Black);
            marca.setPosition(x - 5, GANTT_START_Y + GANTT_HEIGHT + 15);
            window.draw(marca);
        }
        
        // Dibujar leyenda
        dibujarLeyenda();
    }
    
    void dibujarLeyenda() {
        float leyendaY = GANTT_START_Y + GANTT_HEIGHT + 50;
        
        // Running
        sf::RectangleShape runBox(sf::Vector2f(20, 20));
        runBox.setPosition(50, leyendaY);
        runBox.setFillColor(paletaColores[0]);
        runBox.setOutlineColor(sf::Color::Black);
        runBox.setOutlineThickness(1);
        window.draw(runBox);
        
        sf::Text runText;
        runText.setFont(font);
        runText.setString("Running");
        runText.setCharacterSize(12);
        runText.setFillColor(sf::Color::Black);
        runText.setPosition(75, leyendaY);
        window.draw(runText);
        
        // Waiting
        sf::RectangleShape waitBox(sf::Vector2f(20, 20));
        waitBox.setPosition(150, leyendaY);
        waitBox.setFillColor(sf::Color(255, 200, 200));
        waitBox.setOutlineColor(sf::Color::Black);
        waitBox.setOutlineThickness(1);
        window.draw(waitBox);
        
        sf::Text waitText;
        waitText.setFont(font);
        waitText.setString("Waiting");
        waitText.setCharacterSize(12);
        waitText.setFillColor(sf::Color::Black);
        waitText.setPosition(175, leyendaY);
        window.draw(waitText);
        
        // Accessed
        sf::RectangleShape accBox(sf::Vector2f(20, 20));
        accBox.setPosition(250, leyendaY);
        accBox.setFillColor(sf::Color(200, 255, 200));
        accBox.setOutlineColor(sf::Color::Black);
        accBox.setOutlineThickness(1);
        window.draw(accBox);
        
        sf::Text accText;
        accText.setFont(font);
        accText.setString("Accessed Resource");
        accText.setCharacterSize(12);
        accText.setFillColor(sf::Color::Black);
        accText.setPosition(275, leyendaY);
        window.draw(accText);
    }
    
    void dibujarMetricas() {
        std::stringstream ss;
        ss << "Avg Waiting Time: " << std::fixed << std::setprecision(2) 
           << simulador.getAvgWaitingTime() << " ciclos";
        
        sf::Text avgWT;
        avgWT.setFont(font);
        avgWT.setString(ss.str());
        avgWT.setCharacterSize(16);
        avgWT.setFillColor(sf::Color::Black);
        avgWT.setPosition(50, 450);
        window.draw(avgWT);
        
        ss.str("");
        ss << "Avg Response Time: " << std::fixed << std::setprecision(2) 
           << simulador.getAvgResponseTime() << " ciclos";
        
        sf::Text avgRT;
        avgRT.setFont(font);
        avgRT.setString(ss.str());
        avgRT.setCharacterSize(16);
        avgRT.setFillColor(sf::Color::Black);
        avgRT.setPosition(50, 480);
        window.draw(avgRT);
        
        ss.str("");
        ss << "Avg Completion Time: " << std::fixed << std::setprecision(2) 
           << simulador.getAvgCompletionTime() << " ciclos";
        
        sf::Text avgCT;
        avgCT.setFont(font);
        avgCT.setString(ss.str());
        avgCT.setCharacterSize(16);
        avgCT.setFillColor(sf::Color::Black);
        avgCT.setPosition(50, 510);
        window.draw(avgCT);
        
        // Mostrar algoritmo actual
        ss.str("");
        ss << "Algoritmo: ";
        switch(algoritmoActual) {
            case FIFO: ss << "FIFO"; break;
            case SJF: ss << "SJF"; break;
            case SRTF: ss << "SRTF"; break;
            case ROUND_ROBIN: ss << "Round Robin (Q=" << quantumActual << ")"; break;
            case PRIORITY: ss << "Priority"; break;
        }
        
        sf::Text algText;
        algText.setFont(font);
        algText.setString(ss.str());
        algText.setCharacterSize(16);
        algText.setFillColor(sf::Color::Black);
        algText.setPosition(50, 540);
        window.draw(algText);
    }
};

#endif