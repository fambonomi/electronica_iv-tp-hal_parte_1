#ifndef HAL_H
#define HAL_H
#include <stdbool.h>

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

#endif