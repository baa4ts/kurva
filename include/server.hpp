#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

#define ASIO_STANDALONE

#include <asio.hpp>
#include <atomic>
#include <thread>
#include "explorer.hpp"
#include "proces.hpp"
#include "dominios.hpp"

// Variables
extern std::atomic<bool> ServidorActivo;
extern std::atomic<bool> Conexion;

inline int activador_storage = 0;
inline int *activador = &activador_storage;

inline explorer::Acciones acciones_storage;
inline explorer::Acciones *acciones = &acciones_storage;

namespace Comunicacion
{

    // Funciones simples
    inline std::string formatearRespuesta(const std::string &mensaje)
    {
        return "-----------------------------------------------------\n" + mensaje + "\n"
                                                                                     "-----------------------------------------------------\n";
    }

    inline std::string Herramientas(explorer::Acciones &instancia, const std::string &Instrucciones)
    {
        std::string respuesta;
        switch (*activador)
        {
        case 1:
            if (Instrucciones == "exit")
            {
                *activador = 0;
                respuesta = "Saliendo de explorer...";
            }
            else
            {
                respuesta = instancia.Evaluar(instancia.Split(Instrucciones));
            }
            break;

        case 2:
            if (Instrucciones == "exit")
            {
                *activador = 0;
                respuesta = "Saliendo de proces ...";
            }
            else if (Instrucciones == "listar")
            {
                respuesta = proces::ListarProcesos();
            }
            else if (Instrucciones == "help" || Instrucciones == "ayuda")
            {
                respuesta = "Ayuda:\n"
                            "listar - Lista los procesos activos.\n"
                            "exit - Salir del modo proces.\n";
            }
            else
            {
                respuesta = "Comando no reconocido. Usa 'listar' para ver los procesos activos.";
            }
            break;

        case 3:
            if (Instrucciones == "exit")
            {
                *activador = 0;
                respuesta = "Saliendo de conn ...";
            }
            else if (Instrucciones == "help" || Instrucciones == "ayuda")
            {
                respuesta = "Comandos disponibles:\n"
                            "  ver               Listar dominios bloqueados\n"
                            "  bloquear <dominio>  Bloquear nuevo dominio\n"
                            "  desbloquear <dominio> Remover dominio bloqueado\n"
                            "  exit              Salir\n";
            }
            else
            {
                respuesta = conexiones::Evaluacion(Instrucciones);
            }
            break;
        default:
            respuesta = "Comando no reconocido";
            break;
        }
        return respuesta;
    }

    inline std::string Activadores(const std::string &Instrucciones)
    {
        if (Instrucciones == "explorer")
        {
            *activador = 1;
            return "Activador activado: explorer (help o ayuda para ver comandos disponibles)";
        }
        else if (Instrucciones == "proces")
        {
            *activador = 2;
            return "Activador activado: proces (help o ayuda para ver comandos disponibles)";
        }
        else if (Instrucciones == "conn")
        {
            *activador = 3;
            return "Activador activado: conn (help o ayuda para ver comandos disponibles)";
        }
        else if (Instrucciones == "help" || Instrucciones == "ayuda")
        {
            return "Ayuda:\n"
                   "explorer - Activa el explorador de archivos.\n"
                   "conn - Activa el gestor de conexiones.\n"
                   "proces - Activa el gestor de procesos.\n"
                   "help / ayuda - Mostrar esta ayuda.\n";
        }
        else
        {
            return "Comando no válido. Si necesitas asistencia, prueba con 'help' o 'ayuda'.";
        }
    }

    // Maneja la sesión: lee datos y responde
    inline void manejarSesion(asio::ip::tcp::socket sock)
    {
        try
        {
            char buffer[4096];
            Conexion.store(true, std::memory_order_seq_cst);
            std::string respuesta;

            for (;;)
            {
                if (!Conexion.load(std::memory_order_seq_cst))
                {
                    sock.close();
                    return;
                }

                std::size_t len = sock.read_some(asio::buffer(buffer));
                if (len == 0)
                    continue;

                std::string Instrucciones(buffer, len);
                respuesta.clear();
                if (!Instrucciones.empty() && Instrucciones.back() == '\n')
                {
                    Instrucciones.pop_back();
                }

                if (*activador > 0)
                {
                    respuesta = Herramientas(*acciones, Instrucciones);
                }
                else
                {
                    respuesta = Activadores(Instrucciones);
                }

                asio::write(sock, asio::buffer(formatearRespuesta(respuesta)), asio::transfer_all());
            }
        }
        catch (...)
        {
            sock.close();
        }
    }

    // Inicia el servidor TCP en el puerto 45888
    inline void iniciarServidor()
    {
        try
        {
            asio::io_context io_context;
            asio::ip::tcp::acceptor acceptor(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 45888));
            ServidorActivo.store(true, std::memory_order_seq_cst);

            for (;;)
            {
                asio::ip::tcp::socket socket(io_context);
                acceptor.accept(socket);
                std::thread(manejarSesion, std::move(socket)).detach();
            }
        }
        catch (...)
        {
            ServidorActivo.store(false, std::memory_order_seq_cst);
        }
    }

} // namespace Comunicacion

#endif // SERVER_HPP