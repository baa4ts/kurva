#ifndef MOVIMIENTO_HPP
#define MOVIMIENTO_HPP

#include "Movment.hpp"
#include <iostream>
#include <chrono>
#include <tuple>
#include <random>
#include <thread>

namespace Movimiento
{

    // Mueve el cursor aleatoriamente un número específico de veces
    void MoverAleatoriamenteCursor(int veces)
    {
        for (int i = 0; i < veces; ++i)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(250));
            Maus::Mouse mouse;
            mouse.MoverAleatoriamenteCursor();
        }
    }

    // Mueve el cursor a una posición específica
    void MoverCursor(int x, int y)
    {
        Maus::Mouse mouse;
        mouse.MoverCursor(x, y);
    }

    // Genera una posición aleatoria dentro de un rango
    std::tuple<int, int> GenerarPosicionRandom(int xMin, int yMin, int xMax, int yMax)
    {
        static std::random_device rd;
        static std::mt19937 gen(rd());

        std::uniform_int_distribution<> disX(xMin, xMax);
        std::uniform_int_distribution<> disY(yMin, yMax);

        return {disX(gen), disY(gen)};
    }

    // Mueve una ventana identificada por su PID aleatoriamente en la pantalla
    void MoverVentanaAleatoriamente(int pid)
    {
        bool check;
        Vetana::Ventana ventana(pid, check, true);

        auto [x, y] = ventana.VentanaMinimos();
        auto [mx, my] = ventana.ResolucionPantalla();

        const int xMin = x / 2;
        const int yMin = y / 2;
        const int xMax = mx - xMin;
        const int yMax = my - yMin;

        ventana.VentanaRedimensionar(x, y);

        for (size_t i = 0; i < 80; ++i)
        {
            auto [destX, destY] = GenerarPosicionRandom(xMin, yMin, xMax, yMax);
            auto [curX, curY] = ventana.ObtenerCordenadas();

            auto ruta = Algoritmos::AlgoritmoAEstrella::GenerarRuta(curX, curY, destX, destY);
            if (ruta.size() < 2)
                return;

            for (const auto &[stepX, stepY] : ruta)
            {
                ventana.VentanaMover(stepX, stepY);
                std::this_thread::sleep_for(std::chrono::microseconds(250));
            }
        }
    }

} // namespace Movimiento

#endif // MOVIMIENTO_HPP