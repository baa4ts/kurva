#include <filesystem>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

namespace explorer
{
    namespace fs = std::filesystem;
    fs::path PATH = "C:\\";

    class Acciones
    {
    private:
        bool Verificar(const fs::path &path)
        {
            return fs::is_directory(path);
        }

        std::string ListarDirectorio(fs::path path)
        {
            std::string archivos;

            try
            {
                if (fs::exists(path) && this->Verificar(path))
                {
                    for (const auto &entry : fs::directory_iterator(path))
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

    public:
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

        std::string Evaluar(std::vector<std::string> Args)
        {
            if (Args[0] == "ls")
            {
                return this->ListarDirectorio(PATH) + "\n";
            }
            else if (Args[0] == "cd")
            {
                return "CD\n";
            }
            else if (Args[0] == "rm")
            {
                return "RM\n";
            }
            else if (Args[0] == "mkdir")
            {
                return "mkdir\n";
            }
            else if (Args[0] == "rmdir")
            {
                return "rmdir\n";
            }
            else if (Args[0] == "mv")
            {
                return "mv\n";
            }
            else if (Args[0] == "cp")
            {
                return "cp\n";
            }

            return "";
        }
    };
}

int main()
{
    explorer::Acciones acciones;
    std::string input;

    while (true)
    {
        input.clear();
        std::cout << "explorer> ";
        std::getline(std::cin, input);

        std::cout << "Input: " << acciones.Evaluar(acciones.Split(input));
    }
}