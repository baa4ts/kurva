#include <winsock2.h>
#include <windows.h>
#include <tchar.h>
#include <thread>
#include <atomic>

/////////////////
// Definición para soporte de OpenSSL en el servidor HTTP
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "server.hpp"

// Variables
const char *NombreServicio = "Windows Runtime";

std::atomic<bool> Conexion{false};
std::atomic<bool> ServidorActivo{false};
std::atomic<bool> Interruptor{false};

SERVICE_STATUS g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_ServiceHandle = nullptr;

// Declaración de funciones
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
void WINAPI ServiceCtrlHandler(DWORD control);

// Función que maneja el servidor TCP y la lógica del servicio
void ServidorTCP()
{
    // El servidor se ejecuta mientras Interruptor esté activado
    while (Interruptor.load(std::memory_order_seq_cst))
    {
        if (!ServidorActivo.load())
        {
            std::thread TCP(Comunicacion::iniciarServidor);
            TCP.detach();
        }
        Sleep(10000);
    }
}

// Función principal que inicia el servicio
int main()
{
    SERVICE_TABLE_ENTRY serviceTable[] = {
        {(LPSTR)NombreServicio, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
        {NULL, NULL}};

    if (!StartServiceCtrlDispatcher(serviceTable))
    {
        return 1;
    }

    return 0;
}

// Función que maneja la ejecución principal del servicio
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
    g_ServiceHandle = RegisterServiceCtrlHandler(NombreServicio, ServiceCtrlHandler);
    if (!g_ServiceHandle)
    {
        return;
    }

    // Inicia el servicio y establece su estado
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwControlsAccepted = 0;
    SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);

    // Cambia el estado a "en ejecución" cuando el servicio está listo
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);

    Interruptor.store(true, std::memory_order_seq_cst);

    std::thread hiloServidor(ServidorTCP);
    hiloServidor.detach();

    while (Interruptor.load(std::memory_order_seq_cst))
    {
        Sleep(1600);
    }

    // Detiene el servicio
    Interruptor.store(false, std::memory_order_seq_cst);

    g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);

    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);
}

// Función que maneja las señales de control enviadas al servicio
void WINAPI ServiceCtrlHandler(DWORD control)
{
    switch (control)
    {
    case SERVICE_CONTROL_STOP:
        // Detiene el servidor y la conexión
        Conexion.store(false, std::memory_order_seq_cst);
        std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        Interruptor.store(false, std::memory_order_seq_cst);

        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);

        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(g_ServiceHandle, &g_ServiceStatus);
        break;
    default:
        break;
    }
}
