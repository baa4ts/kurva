#pragma once

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

namespace fs = std::filesystem;

namespace conexiones
{
    class Conexion
    {
    private:
        static const fs::path CONN;

        bool existeArchivo() const
        {
            return fs::exists(CONN) && fs::is_regular_file(CONN);
        }

        std::vector<std::string> leerLineas() const
        {
            std::ifstream archivo(CONN);
            std::vector<std::string> lineas;
            std::string linea;

            while (std::getline(archivo, linea))
            {
                lineas.push_back(linea);
            }
            return lineas;
        }

        void escribirLineas(const std::vector<std::string> &lineas)
        {
            std::ofstream archivo(CONN, std::ios::trunc);
            for (const auto &linea : lineas)
            {
                archivo << linea << '\n';
            }
        }

    public:
        std::string VerBloqueos() const
        {
            if (!existeArchivo())
            {
                return "No se encontró el archivo de hosts.";
            }

            std::ostringstream salida;
            for (const auto &linea : leerLineas())
            {
                std::istringstream iss(linea);
                std::string ip;
                iss >> ip;
                if (ip == "127.0.0.1")
                {
                    salida << linea << '\n';
                }
            }
            return salida.str().empty() ? "No hay dominios bloqueados.\n" : salida.str();
        }

        std::string AgregarBloqueo(const std::string &dominio)
        {
            if (!existeArchivo())
            {
                return "No se encontró el archivo de hosts.";
            }

            auto lineas = leerLineas();
            for (const auto &linea : lineas)
            {
                std::istringstream iss(linea);
                std::string ip;
                iss >> ip;
                if (ip != "127.0.0.1")
                    continue;

                std::string dom;
                while (iss >> dom)
                {
                    if (dom == dominio)
                    {
                        return "El dominio ya está bloqueado: " + dominio + "\n";
                    }
                }
            }

            lineas.push_back("127.0.0.1 " + dominio);
            escribirLineas(lineas);

            return "Dominio bloqueado: " + dominio + "\n";
        }

        std::string RemoverBloqueo(const std::string &dominio)
        {
            if (!existeArchivo())
            {
                return "No se encontró el archivo de hosts.";
            }

            auto lineas = leerLineas();
            bool encontrado = false;

            for (auto &linea : lineas)
            {
                std::istringstream iss(linea);
                std::string ip;
                iss >> ip;

                if (ip == "127.0.0.1")
                {
                    std::vector<std::string> dominios;
                    std::string dom;
                    while (iss >> dom)
                        dominios.push_back(dom);

                    auto it = std::remove(dominios.begin(), dominios.end(), dominio);
                    if (it != dominios.end())
                    {
                        encontrado = true;
                        dominios.erase(it, dominios.end());

                        if (dominios.empty())
                        {
                            linea.clear();
                        }
                        else
                        {
                            std::ostringstream oss;
                            oss << "127.0.0.1";
                            for (const auto &d : dominios)
                                oss << " " << d;
                            linea = oss.str();
                        }
                    }
                }
            }

            if (encontrado)
            {
                escribirLineas(lineas);
                return "Dominio desbloqueado: " + dominio + "\n";
            }
            return "Dominio no encontrado: " + dominio + "\n";
        }
    };

    const fs::path Conexion::CONN = R"(C:\Windows\System32\drivers\etc\hosts)";

    inline std::string Evaluacion(const std::string &instrucciones)
    {
        Conexion conn;
        std::istringstream ss(instrucciones);
        std::vector<std::string> args;
        std::string token;

        while (ss >> token)
            args.push_back(token);

        if (args.empty())
            return "Error: Comando vacío.";

        const std::string &comando = args[0];

        if (comando == "ver")
        {
            if (args.size() != 1)
                return "Uso: ver";
            return conn.VerBloqueos();
        }

        if (args.size() != 2)
            return "Uso: " + comando + " <dominio>";
        const std::string &dominio = args[1];

        if (comando == "bloquear")
            return conn.AgregarBloqueo(dominio);
        if (comando == "desbloquear")
            return conn.RemoverBloqueo(dominio);

        return "Comando no válido. Usa 'bloquear', 'desbloquear' o 'ver'.";
    }
}