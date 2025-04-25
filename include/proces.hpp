#pragma once
#ifndef PROCES_HPP
#define PROCES_HPP

#include <string>
#include <windows.h>
#include <tlhelp32.h>

namespace proces
{

    /**
     * @brief Lista los procesos activos en el sistema.
     *
     * @return std::string Cadena con nombre y PID de cada proceso, o mensaje de error.
     */
    inline std::string ListarProcesos()
    {
        std::string procesos;

        // Crear snapshot de procesos
        HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnap == INVALID_HANDLE_VALUE)
        {
            return "Error al crear el snapshot de procesos.";
        }

        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        // Recorrer la lista de procesos
        if (Process32First(hProcessSnap, &pe32))
        {
            do
            {
                procesos += "Nombre: " + std::string(pe32.szExeFile) +
                            "  PID: " + std::to_string(pe32.th32ProcessID) + "\n";

            } while (Process32Next(hProcessSnap, &pe32));
        }
        else
        {
            procesos = "No se pudo obtener el primer proceso.";
        }

        // Cerrar el handle al snapshot
        CloseHandle(hProcessSnap);
        return procesos;
    }

} // namespace proces

#endif // PROCES_HPP