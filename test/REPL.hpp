#pragma once
#ifndef REPL_HPP
#define REPL_HPP

#include <iostream>
#include <sstream>
#include <thread>
#include <string>
#include <vector>
#include <atomic>
#include <chrono>
#include <thread>
#include "movimiento.hpp"
// #include "explorer.hpp"

extern std::atomic<bool> Conexion;

namespace REPL
{

    class Evaluacion
    {
    private:
        // Separar por espacios
        std::vector<std::string> Split(const std::string &input)
        {
            std::istringstream iss(input);
            std::vector<std::string> inputs;
            std::string commando;

            while (iss >> commando)
            {
                inputs.push_back(commando);
            }

            return inputs;
        }

        bool ModoGestor{false};
        bool ModoMovimientos{false};

        static void CerrrarServidor()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
            Conexion.store(false, std::memory_order_seq_cst);
        }

        // Función auxiliar para formatear las respuestas
        std::string FormatearRespuesta(const std::string &mensaje)
        {
            return mensaje;
        }

        std::string Activadores(std::string &input)
        {

            std::vector<std::string> Args = this->Split(input);
            if (Args.empty())
            {
                return FormatearRespuesta("No hay comandos para evaluar");
            }

            if (Args[0] == "gestor")
            {
                ModoGestor = true;
                return FormatearRespuesta("Modo gestor activado, escriba 'exit' para salir del modo gestor");
            }
            else if (Args[0] == "movimiento")
            {
                ModoMovimientos = true;
                return FormatearRespuesta("Modo movimiento activado, escriba 'exit' para salir del modo movimiento");
            }
            else if (Args[0] == "exit")
            {
                std::thread hiloCierre(CerrrarServidor);
                hiloCierre.detach();
                return FormatearRespuesta("Saliendo del programa...");
            }
            else if (Args[0] == "ayuda" || Args[0] == "help")
            {
                return FormatearRespuesta("Comandos disponibles: \n"
                                          "movimiento - Activar modo movimiento\n"
                                          "gestor - Activar modo gestor\n"
                                          "ayuda - Mostrar esta ayuda\n"
                                          "exit - Salir del programa\n");
            }

            return FormatearRespuesta("Comando no encontrado: [ " + Args[0] + " ]\n" +
                                      "Escriba 'ayuda' o 'help' para ver la lista de comandos disponibles");
        }

    public:
        std::string Evaluar(std::string &input)
        {
            explorer::Acciones Acciones;
            if (!input.empty() && input.back() == '\n')
            {
                input.pop_back();
            }

            if (ModoGestor)
            {
                if (input == "exit")
                {
                    ModoGestor = false;
                    return FormatearRespuesta("Modo gestor desactivado, escriba 'gestor' para volver a activarlo");
                }
                return Acciones.Evaluar(Acciones.Split(input));
            }

            /**
             *
             * Evaluacion de comandos
             *
             */

            else if (ModoMovimientos)
            {
                return "hola";
            }

            return this->Activadores(input);
        }
    };

} // namespace REPL

#endif // REPL_HPP