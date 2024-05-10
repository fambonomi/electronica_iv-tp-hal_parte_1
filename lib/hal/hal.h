#ifndef HAL_H
#define HAL_H
#include <stdbool.h>
#include <stdint.h>

/* GPIO */

typedef enum HPin{
    Pin_LED,
    Pin_NUM_HANDLES
}HPin;

/**
 * @brief Pone un pin en modo salida
 * @note Modo lento, push-pull
 * @param pin el pin
 */
void Pin_ponModoSalida(HPin pin);

/**
 * @brief Pone el estado del pin en bajo
 * @note Si el pin está configurado como entrada el buffer de salida está
 * desconectado
 * @param pin el pin 
 */
void Pin_ponEstadoBajo(HPin pin);

/**
 * @brief Pone el estado del pin en alto
 * @note Si el pin está configurado como entrada el buffer de salida está
 * desconectado
 * @param pin el pin 
 */
void Pin_ponEstadoAlto(HPin pin);

/**
 * @brief Consulta el estado de salida del pin
 * 
 * @param pin el pin
 * @return true estado Alto
 * @return false estado Bajo
 */
bool Pin_consultaEstado(HPin pin);

/**
 * @brief Invierte el estado del pin (lo pone en alto si estaba bajo y
 * viceversa)
 * @note Se basa en el estado de salida, no en el estado lógico actual del pin
 * físico
 * @param pin el pin
 */
void Pin_invierteEstado(HPin pin);

/* Temporizador */

typedef uint32_t Milisegundos;

/**
 * @brief Configura el temporizador del sistema. Es necesario llamar a esta
 * rutina antes de cualquier otra función del temporizador.
 * 
 */
void Temporizador_inicializa(void);

/**
 * @brief Obtiene la cuenta actual de milisegundos.
 * @note Debe inicializarse el temporizador con Temporizador_incializa antes de
 * usar esta función
 * @return Milisegundos Valor de la cuenta (en milisegundos) 
 */
Milisegundos Temporizador_obtCuenta(void);

/**
 * @brief Bloquea la ejecución del programa durante el tiempo indicado
 * @note Debe inicializarse el temporizador con Temporizador_incializa antes de
 * usar esta función
 * @param tiempo Tiempo en milisegundos
 */
void Temporizador_espera(Milisegundos tiempo);

#endif