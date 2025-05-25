#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include "estructuras.h"

class Parser {
public:
    // Cargar procesos desde archivo
    static std::vector<Proceso> cargarProcesos(const std::string& archivo) {
        std::vector<Proceso> procesos;
        std::ifstream file(archivo);
        std::string linea;
        
        if (!file.is_open()) {
            throw std::runtime_error("No se pudo abrir el archivo: " + archivo);
        }
        
        while (std::getline(file, linea)) {
            if (linea.empty()) continue;
            
            std::stringstream ss(linea);
            std::string pid, bt_str, at_str, pr_str;
            
            // Leer: <PID>, <BT>, <AT>, <Priority>
            std::getline(ss, pid, ',');
            std::getline(ss, bt_str, ',');
            std::getline(ss, at_str, ',');
            std::getline(ss, pr_str, ',');
            
            // Limpiar espacios
            pid = trim(pid);
            bt_str = trim(bt_str);
            at_str = trim(at_str);
            pr_str = trim(pr_str);
            
            int bt = std::stoi(bt_str);
            int at = std::stoi(at_str);
            int pr = std::stoi(pr_str);
            
            procesos.push_back(Proceso(pid, bt, at, pr));
        }
        
        file.close();
        return procesos;
    }
    
    // Cargar recursos desde archivo
    static std::vector<Recurso> cargarRecursos(const std::string& archivo) {
        std::vector<Recurso> recursos;
        std::ifstream file(archivo);
        std::string linea;
        
        if (!file.is_open()) {
            throw std::runtime_error("No se pudo abrir el archivo: " + archivo);
        }
        
        while (std::getline(file, linea)) {
            if (linea.empty()) continue;
            
            std::stringstream ss(linea);
            std::string nombre, contador_str;
            
            // Leer: <NOMBRE RECURSO>, <CONTADOR>
            std::getline(ss, nombre, ',');
            std::getline(ss, contador_str, ',');
            
            nombre = trim(nombre);
            contador_str = trim(contador_str);
            
            int contador = std::stoi(contador_str);
            
            recursos.push_back(Recurso(nombre, contador));
        }
        
        file.close();
        return recursos;
    }
    
    // Cargar acciones desde archivo
    static std::vector<Accion> cargarAcciones(const std::string& archivo) {
        std::vector<Accion> acciones;
        std::ifstream file(archivo);
        std::string linea;
        
        if (!file.is_open()) {
            throw std::runtime_error("No se pudo abrir el archivo: " + archivo);
        }
        
        while (std::getline(file, linea)) {
            if (linea.empty()) continue;
            
            std::stringstream ss(linea);
            std::string pid, tipo, recurso, ciclo_str;
            
            // Leer: <PID>, <ACCION>, <RECURSO>, <CICLO>
            std::getline(ss, pid, ',');
            std::getline(ss, tipo, ',');
            std::getline(ss, recurso, ',');
            std::getline(ss, ciclo_str, ',');
            
            pid = trim(pid);
            tipo = trim(tipo);
            recurso = trim(recurso);
            ciclo_str = trim(ciclo_str);
            
            int ciclo = std::stoi(ciclo_str);
            
            acciones.push_back(Accion(pid, tipo, recurso, ciclo));
        }
        
        file.close();
        return acciones;
    }
    
private:
    // Función auxiliar para eliminar espacios en blanco
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (std::string::npos == first) {
            return str;
        }
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
};

#endif