#include <stm32f1xx.h>
#include "hal.h"

/* Aquí tu implementación de controlador para el timer SysTick */

void Temporizador_inicializa(void)
{
    SystemCoreClockUpdate();

    const uint32_t ciclosPorMilisegundo = SystemCoreClock/1000;

    SysTick_Config(ciclosPorMilisegundo);
}

static volatile Milisegundos cuenta;

void SysTick_Handler(void)
{
    ++cuenta;
}

Milisegundos Temporizador_obtCuenta(void)
{
    return cuenta;
}

void Temporizador_espera(Milisegundos tiempo)
{
    const Milisegundos inicial = cuenta;
    while(cuenta - inicial < tiempo);
}