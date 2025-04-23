#pragma once
#ifndef SERVER_HPP
#define SERVER_HPP

#define ASIO_STANDALONE

#include <asio.hpp>
#include <thread>
#include "REPL.hpp"

// Variables
extern std::atomic<bool> ServidorActivo;
extern std::atomic<bool> Conexion;
namespace Comunicacion
{

    // Maneja la sesión: lee datos y responde "hola\n"
    inline void manejarSesion(asio::ip::tcp::socket sock)
    {
        try
        {
            REPL::Evaluacion Evaluador;
            char buffer[4096];
            Conexion.store(true, std::memory_order_seq_cst);

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
                std::string respuesta = "-----------------------------------------------------\n" + Evaluador.Evaluar(Instrucciones) + "\n-----------------------------------------------------\n";
                asio::write(sock, asio::buffer(respuesta), asio::transfer_all());
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
