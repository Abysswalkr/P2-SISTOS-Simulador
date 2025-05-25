#ifndef GUI_H
#define GUI_H

#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include "simulador_calendarizacion.h"

class GUI {
private:
    sf::RenderWindow window;
    sf::Font font;
    
    // Simulador
    SimuladorCalendarizacion simulador;
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
    
    // Colores para procesos
    std::map<std::string, sf::Color> coloresProcesos;
    std::vector<sf::Color> paletaColores;
    
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
        crearBoton(700, 50, 120, 30, "Cargar", "cargar");
        crearBoton(830, 50, 120, 30, "Ejecutar", "ejecutar");
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
        btn.texto.setCharacterSize(16);
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
        else if (id == "cargar") {
            cargarArchivo();
        }
        else if (id == "ejecutar") {
            ejecutarSimulacion();
        }
    }
    
    void cargarArchivo() {
        try {
            // Por ahora, cargar archivo fijo "procesos.txt"
            std::vector<Proceso> procs = Parser::cargarProcesos("procesos.txt");
            simulador.cargarProcesos(procs);
            
            // Asignar colores a procesos
            int colorIndex = 0;
            for (const auto& p : procs) {
                coloresProcesos[p.pid] = paletaColores[colorIndex % paletaColores.size()];
                colorIndex++;
            }
        } catch (const std::exception& e) {
            // Manejar error
        }
    }
    
    void ejecutarSimulacion() {
        simulador.ejecutar(algoritmoActual);
    }
    
    void dibujar() {
        window.clear(sf::Color::White);
        
        // Dibujar botones
        for (const auto& btn : botones) {
            window.draw(btn.shape);
            window.draw(btn.texto);
        }
        
        // Dibujar título
        sf::Text titulo;
        titulo.setFont(font);
        titulo.setString("Simulador de Algoritmos de Calendarizacion");
        titulo.setCharacterSize(24);
        titulo.setFillColor(sf::Color::Black);
        titulo.setPosition(WINDOW_WIDTH / 2 - 250, 10);
        window.draw(titulo);
        
        // Dibujar diagrama de Gantt
        dibujarGantt();
        
        // Dibujar métricas
        dibujarMetricas();
        
        window.display();
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
            rect.setFillColor(coloresProcesos[evento.pid]);
            rect.setOutlineColor(sf::Color::Black);
            rect.setOutlineThickness(1);
            window.draw(rect);
            
            // Nombre del proceso
            sf::Text nombre;
            nombre.setFont(font);
            nombre.setString(evento.pid);
            nombre.setCharacterSize(14);
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
        ss << "Avg Completion Time: " << std::fixed << std::setprecision(2) 
           << simulador.getAvgCompletionTime() << " ciclos";
        
        sf::Text avgCT;
        avgCT.setFont(font);
        avgCT.setString(ss.str());
        avgCT.setCharacterSize(16);
        avgCT.setFillColor(sf::Color::Black);
        avgCT.setPosition(50, 480);
        window.draw(avgCT);
    }
};

#endif