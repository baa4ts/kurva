#pragma once
#ifndef EXPLORER_HPP
#define EXPLORER_HPP

#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <thread>
#include <mutex>

namespace explorer
{
    namespace fs = std::filesystem;

    class Acciones
    {
    private:
        fs::path current_path;
        mutable std::mutex path_mutex;

        bool Verificar(const fs::path &path) const
        {
            return fs::is_directory(path);
        }

        std::string ListarDirectorio() const
        {
            std::lock_guard<std::mutex> lock(path_mutex);
            std::string archivos;

            try
            {
                if (fs::exists(current_path) && Verificar(current_path))
                {
                    for (const auto &entry : fs::directory_iterator(current_path))
                    {
                        if (fs::is_directory(entry.path()))
                        {
                            archivos += "[DIR]  " + entry.path().filename().string() + "\n";
                        }
                        else if (fs::is_regular_file(entry.path()))
                        {
                            archivos += "[FILE] " + entry.path().filename().string() + "\n";
                        }
                    }
                    if (archivos.empty())
                        archivos = "Directorio vacío\n";
                }
                else
                {
                    archivos = "El directorio no existe o es un archivo\n";
                }
            }
            catch (const std::exception &e)
            {
                archivos = "Error: " + std::string(e.what()) + "\n";
            }

            return archivos;
        }

        bool Retroceder()
        {
            std::lock_guard<std::mutex> lock(path_mutex);
            if (current_path == "C:\\")
            {
                return false;
            }
            if (current_path.has_parent_path())
            {
                current_path = current_path.parent_path();
                return true;
            }
            return false;
        }

        bool CambiarDirectorio(const fs::path &path)
        {
            std::lock_guard<std::mutex> lock(path_mutex);
            fs::path tmPath = current_path / path;
            if (fs::exists(tmPath) && Verificar(tmPath))
            {
                current_path = tmPath;
                return true;
            }
            return false;
        }

        bool Borrar(const fs::path &archivo)
        {
            std::lock_guard<std::mutex> lock(path_mutex);
            fs::path full_path = current_path / archivo;

            if (!fs::exists(full_path))
            {
                return false;
            }

            std::uintmax_t borrados = fs::is_directory(full_path)
                                          ? fs::remove_all(full_path)
                                          : (fs::remove(full_path) ? 1 : 0);

            return borrados > 0;
        }

        bool CrearDirectorio(const fs::path &path)
        {
            std::lock_guard<std::mutex> lock(path_mutex);
            fs::path full_path = current_path / path;
            return fs::create_directories(full_path);
        }

        bool lanzarAsync(const fs::path &path)
        {
            fs::path tmp;
            {
                std::lock_guard<std::mutex> lock(path_mutex);
                tmp = current_path / path;
            }

            if (!fs::exists(tmp) || !fs::is_regular_file(tmp))
                return false;

            try
            {
                std::thread([tmp]()
                            { 
                                std::string command = "\"" + tmp.string() + "\"";
                                std::system(command.c_str()); })
                    .detach();
                return true;
            }
            catch (const std::system_error &)
            {
                return false;
            }
        }

    public:
        Acciones() : current_path("C:\\") {}

        std::vector<std::string> Split(const std::string &entrada) const
        {
            std::vector<std::string> args;
            std::string current;
            bool in_quotes = false;

            for (size_t i = 0; i < entrada.length(); ++i)
            {
                if (entrada[i] == '\"')
                {
                    in_quotes = !in_quotes;
                }
                else if (entrada[i] == ' ' && !in_quotes)
                {
                    if (!current.empty())
                    {
                        args.push_back(current);
                        current.clear();
                    }
                }
                else
                {
                    current += entrada[i];
                }
            }

            if (!current.empty())
            {
                args.push_back(current);
            }

            return args;
        }

        std::string Evaluar(const std::vector<std::string> &Args)
        {
            if (Args.empty())
                return "Comando vacío";

            if (Args[0] == "ls")
            {
                return this->ListarDirectorio();
            }
            else if (Args[0] == "cd")
            {
                if (Args.size() < 2)
                    return "Uso: cd <directorio>";

                if (Args[1] == "..")
                {
                    if (this->Retroceder())
                    {
                        std::lock_guard<std::mutex> lock(path_mutex);
                        return "Retrocediendo a: " + current_path.string();
                    }
                    else
                    {
                        return "No se puede retroceder más allá de la raíz";
                    }
                }
                else
                {
                    if (this->CambiarDirectorio(Args[1]))
                    {
                        std::lock_guard<std::mutex> lock(path_mutex);
                        return "Cambiando a: " + current_path.string();
                    }
                    else
                    {
                        return "No se puede cambiar al directorio: " + Args[1];
                    }
                }
            }
            else if (Args[0] == "rm")
            {
                if (Args.size() < 2)
                {
                    return "Uso: rm <archivo o directorio>";
                }
                else
                {
                    if (this->Borrar(Args[1]))
                    {
                        return "Eliminando: " + Args[1];
                    }
                    else
                    {
                        return "No se puede eliminar: " + Args[1];
                    }
                }
            }
            else if (Args[0] == "mkdir")
            {
                if (Args.size() < 2)
                    return "Uso: mkdir <nombre del directorio>";

                if (this->CrearDirectorio(Args[1]))
                    return "Directorio creado: " + Args[1];
                else
                    return "No se pudo crear el directorio: " + Args[1];
            }
            else if (Args[0] == "path")
            {
                std::lock_guard<std::mutex> lock(path_mutex);
                return "Path: " + current_path.string();
            }
            else if (Args[0] == "start")
            {
                if (Args.size() < 2)
                    return "Uso: start <archivo>";

                if (this->lanzarAsync(Args[1]))
                    return "Iniciando: " + Args[1];
                else
                    return "No se puede iniciar: " + Args[1];
            }
            else if (Args[0] == "help" || Args[0] == "ayuda")
            {
                return "Ayuda:\n"
                       "ls: Listar contenido del directorio actual\n"
                       "cd <dir>: Cambiar directorio\n"
                       "rm <target>: Eliminar archivo/directorio\n"
                       "mkdir <dir>: Crear directorio\n"
                       "path: Mostrar ruta actual\n"
                       "start <archivo>: Ejecutar archivo\n"
                       "help/ayuda: Mostrar ayuda";
            }
            return "Comando no encontrado: " + Args[0] +
                   "\nEscriba 'ayuda' para ver comandos disponibles";
        }
    };
}

#endif // EXPLORER_HPP