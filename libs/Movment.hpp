#ifndef MOV_HPP
#define MOV_HPP

#include <windows.h>
#include <vector>
#include <queue>
#include <memory>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <atomic>
#include <string>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <tuple>
#include <ctime>

namespace Algoritmos
{
    class AlgoritmoAEstrella
    {
    public:
        /// Genera una ruta desde el punto (InicioX, InicioY) hasta (FinalX, FinalY) utilizando el algoritmo A*.
        /// @param InicioX Coordenada X de inicio.
        /// @param InicioY Coordenada Y de inicio.
        /// @param FinalX Coordenada X de destino.
        /// @param FinalY Coordenada Y de destino.
        /// @return Vector de pares de coordenadas que representan la ruta calculada.
        static std::vector<std::pair<int, int>> GenerarRuta(int InicioX, int InicioY, int FinalX, int FinalY)
        {
            // Determinar límites del área de trabajo
            int minX = std::min(InicioX, FinalX);
            int minY = std::min(InicioY, FinalY);
            int maxX = std::max(InicioX, FinalX);
            int maxY = std::max(InicioY, FinalY);

            int ancho = maxX - minX + 1;
            int alto = maxY - minY + 1;

            // Crear la matriz visitada del tamaño necesario
            std::vector<std::vector<bool>> Visitado(alto, std::vector<bool>(ancho, false));

            struct Nodo
            {
                int x, y, g, h;
                std::shared_ptr<Nodo> previo;
                Nodo(int x, int y, int g, int h, std::shared_ptr<Nodo> previo = nullptr)
                    : x(x), y(y), g(g), h(h), previo(previo) {}
                int f() const { return g + h; }
            };

            struct NodoComparador
            {
                bool operator()(const std::shared_ptr<Nodo> &a, const std::shared_ptr<Nodo> &b)
                {
                    return a->f() > b->f();
                }
            };

            // Convertir a coordenadas relativas
            int sx = InicioX - minX;
            int sy = InicioY - minY;
            int ex = FinalX - minX;
            int ey = FinalY - minY;

            std::priority_queue<std::shared_ptr<Nodo>, std::vector<std::shared_ptr<Nodo>>, NodoComparador> Abierto;
            Abierto.push(std::make_shared<Nodo>(sx, sy, 0, Heuristica(sx, sy, ex, ey)));

            int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
            int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

            while (!Abierto.empty())
            {
                auto Actual = Abierto.top();
                Abierto.pop();

                if (Visitado[Actual->y][Actual->x])
                    continue;
                Visitado[Actual->y][Actual->x] = true;

                if (Actual->x == ex && Actual->y == ey)
                {
                    std::vector<std::pair<int, int>> Ruta;
                    for (auto p = Actual; p != nullptr; p = p->previo)
                    {
                        // Convertimos de nuevo a coordenadas absolutas
                        Ruta.emplace_back(p->x + minX, p->y + minY);
                    }
                    std::reverse(Ruta.begin(), Ruta.end());
                    return Ruta;
                }

                for (int i = 0; i < 8; ++i)
                {
                    int nx = Actual->x + dx[i];
                    int ny = Actual->y + dy[i];

                    if (nx >= 0 && ny >= 0 && nx < ancho && ny < alto && !Visitado[ny][nx])
                    {
                        Abierto.push(std::make_shared<Nodo>(
                            nx, ny, Actual->g + 1, Heuristica(nx, ny, ex, ey), Actual));
                    }
                }
            }

            return {};
        }

        /// Calcula la heurística Manhattan entre dos puntos.
        /// @param x1 Coordenada X del primer punto.
        /// @param y1 Coordenada Y del primer punto.
        /// @param x2 Coordenada X del segundo punto.
        /// @param y2 Coordenada Y del segundo punto.
        /// @return Distancia heurística.
        static int Heuristica(int x1, int y1, int x2, int y2)
        {
            return std::abs(x1 - x2) + std::abs(y1 - y2);
        }
    };
} // namespace Algoritmos

namespace Vetana
{
    template <typename T>
    inline void EstablecerEstado(std::atomic<T> &estado, T valor)
    {
        estado.store(valor, std::memory_order_seq_cst);
    }

#define VETANA_VALIDADOR()                                                             \
    if (!(EstadoValido.load(std::memory_order_seq_cst) && (Identificador != nullptr))) \
    {                                                                                  \
        return false;                                                                  \
    }

    class Ventana
    {
    private:
        HWND Identificador = nullptr;
        bool AcoplesVentana = true;
        std::atomic<bool> EstadoValido{true};

        struct Monitor
        {
            LONG SUPERIOR, INFERIOR, DERECHA, ISQUIERDA;
            Monitor() : SUPERIOR(0), INFERIOR(0), DERECHA(0), ISQUIERDA(0) {}
        };

        /// Busca la ventana principal de un proceso dado mediante su PID y guarda el HWND si la encuentra.
        /// @param PID ID del proceso.
        /// @return Verdadero si se encontró la ventana.
        bool ObtenerHandlePID(DWORD PID)
        {
            HWND VentanaActual = GetTopWindow(nullptr);
            while (VentanaActual)
            {
                DWORD pidActual;
                GetWindowThreadProcessId(VentanaActual, &pidActual);
                if (pidActual == PID && IsWindowVisible(VentanaActual) && GetWindow(VentanaActual, GW_OWNER) == nullptr)
                {
                    Identificador = VentanaActual;
                    return true;
                }
                VentanaActual = GetNextWindow(VentanaActual, GW_HWNDNEXT);
            }
            return false;
        }

        /// Actualiza los valores de la estructura `Monitor` con los límites del área de trabajo del monitor actual.
        /// @param Monitor Referencia a la estructura `Monitor` que se actualizará.
        /// @return Verdadero si la información fue obtenida con éxito.
        bool ActualizarCoordenadas(Monitor &Monitor)
        {
            VETANA_VALIDADOR();
            HMONITOR MR = MonitorFromWindow(Identificador, MONITOR_DEFAULTTONEAREST);
            MONITORINFO InformacionMonitor{sizeof(MONITORINFO)};
            return GetMonitorInfo(MR, &InformacionMonitor) ? (Monitor.SUPERIOR = InformacionMonitor.rcWork.top,
                                                              Monitor.INFERIOR = InformacionMonitor.rcWork.bottom,
                                                              Monitor.DERECHA = InformacionMonitor.rcWork.right,
                                                              Monitor.ISQUIERDA = InformacionMonitor.rcWork.left, true)
                                                           : false;
        }

    public:
        Ventana(DWORD PID, bool &Confirmador, bool Acoples = true) : AcoplesVentana(Acoples)
        {
            bool estado = ObtenerHandlePID(PID);
            EstablecerEstado(EstadoValido, estado);
            Confirmador = estado;
        }

        ~Ventana() { Identificador = nullptr; }

        /// Cambia el estado de la ventana a minimizar o maximizar.
        /// @param Minimizar Si es verdadero, minimiza; de lo contrario, maximiza.
        /// @return Verdadero si tuvo éxito, falso si falló.
        bool VentanaCambiarEstado(bool Minimizar)
        {
            VETANA_VALIDADOR();
            bool exito = ShowWindow(Identificador, Minimizar ? SW_MINIMIZE : SW_MAXIMIZE);
            if (!exito)
                EstablecerEstado(EstadoValido, false);
            return exito;
        }

        /// Restaura la ventana a su estado normal si está minimizada o maximizada.
        /// @return Verdadero si fue restaurada, falso en caso contrario.
        bool VentanaRestaurar()
        {
            VETANA_VALIDADOR();
            if (IsIconic(Identificador) || IsZoomed(Identificador))
            {
                bool exito = ShowWindow(Identificador, SW_RESTORE);
                EstablecerEstado(EstadoValido, exito);
                return exito;
            }
            EstablecerEstado(EstadoValido, false);
            return false;
        }

        /// Mueve la ventana a una nueva posición (X, Y), dentro de los límites del monitor.
        /// @param X Nueva coordenada X.
        /// @param Y Nueva coordenada Y.
        /// @return Verdadero si el movimiento fue exitoso.
        bool VentanaMover(int X, int Y)
        {
            VETANA_VALIDADOR();
            Monitor M;
            if (!ActualizarCoordenadas(M))
                return false;

            LONG posX = std::clamp<LONG>(X, M.ISQUIERDA, M.DERECHA - GetSystemMetrics(SM_CXMINTRACK));
            LONG posY = std::clamp<LONG>(Y, M.SUPERIOR, M.INFERIOR - GetSystemMetrics(SM_CYMINTRACK));

            return SetWindowPos(Identificador, nullptr, posX, posY, 0, 0,
                                SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }

        /// Cambia el tamaño de la ventana según las dimensiones proporcionadas, respetando los límites del monitor.
        /// @param X Nueva anchura.
        /// @param Y Nueva altura.
        /// @return Verdadero si el cambio de tamaño fue exitoso.
        bool VentanaRedimensionar(int X, int Y)
        {
            VETANA_VALIDADOR();
            Monitor M;
            if (!ActualizarCoordenadas(M))
                return false;

            LONG NuevoX = std::clamp<LONG>(X, GetSystemMetrics(SM_CXMINTRACK), M.DERECHA - M.ISQUIERDA);
            LONG NuevoY = std::clamp<LONG>(Y, GetSystemMetrics(SM_CYMINTRACK), M.INFERIOR - M.SUPERIOR);

            return SetWindowPos(Identificador, nullptr, 0, 0, NuevoX, NuevoY,
                                SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
        }

        /// Devuelve los valores mínimos permitidos para el tamaño de una ventana.
        /// @return tupla con dimensiones mínimas en X y Y.
        static std::tuple<int, int> VentanaMinimos()
        {
            return std::tuple<int, int>(GetSystemMetrics(SM_CXMINTRACK),
                                        GetSystemMetrics(SM_CYMINTRACK));
        }

        /// Devuelve los valores máximos permitidos para el tamaño de una ventana.
        /// @return Cadena con dimensiones máximas en X y Y.
        static std::string VentanaMaximos()
        {
            std::ostringstream rts;
            rts << "X: " << GetSystemMetrics(SM_CXSCREEN) - GetSystemMetrics(SM_CXMINTRACK)
                << " | Y: " << GetSystemMetrics(SM_CYSCREEN) - GetSystemMetrics(SM_CYMINTRACK);
            return rts.str();
        }

        static std::tuple<int, int> ResolucionPantalla()
        {
            return {
                GetSystemMetrics(SM_CXSCREEN), // Ancho
                GetSystemMetrics(SM_CYSCREEN)  // Alto
            };
        }

        std::tuple<int, int> ObtenerCordenadas()
        {
            RECT rect;
            if (Identificador != nullptr && GetWindowRect(Identificador, &rect))
            {
                return {rect.left, rect.top};
            }
            else
            {
                return {-1, -1};
            }
        }
    };
#undef VETANA_VALIDADOR
} // namespace Vetana

namespace Maus
{
    class Mouse
    {
    private:
        int WIDTH, HEIGHT;

    public:
        Mouse() : WIDTH(GetSystemMetrics(SM_CXSCREEN)),
                  HEIGHT(GetSystemMetrics(SM_CYSCREEN))
        {
            std::srand(static_cast<unsigned>(std::time(nullptr)));
        }

        /// Obtiene la posición actual del cursor.
        /// @return Tupla (X, Y) con la posición actual o (-1, -1) si falla.
        std::tuple<int, int> PosicionActualCursor()
        {
            POINT p;
            if (GetCursorPos(&p))
            {
                return std::make_tuple(p.x, p.y);
            }
            else
            {
                return std::make_tuple(-1, -1);
            }
        }

        /// Genera una posición aleatoria dentro de la pantalla.
        /// @return Tupla (X, Y) con una posición aleatoria.
        std::tuple<int, int> PosicionAleatoriaPantalla()
        {
            return {std::rand() % WIDTH, std::rand() % HEIGHT};
        }

        /// Mueve el cursor del ratón desde su posición actual a una aleatoria siguiendo una ruta tipo A*.
        void MoverAleatoriamenteCursor()
        {
            auto [x1, y1] = PosicionActualCursor();
            auto [x2, y2] = PosicionAleatoriaPantalla();

            auto ruta = Algoritmos::AlgoritmoAEstrella::GenerarRuta(x1, y1, x2, y2);
            if (ruta.size() < 2)
                return;

            for (const auto &[x, y] : ruta)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(1000));
                this->MoverCursor(x, y);
            }
        }

        /// Mueve el cursor del ratón a una posición específica dentro de los límites de la pantalla.
        /// @param X Coordenada X de destino.
        /// @param Y Coordenada Y de destino.
        /// @return Verdadero si el movimiento fue exitoso.
        bool MoverCursor(int X, int Y)
        {
            if (X >= 0 && X <= WIDTH && Y >= 0 && Y <= HEIGHT)
            {
                return SetCursorPos(X, Y);
            }
            return false;
        }
    };
} // namespace Maus

#endif // MOV_HPP