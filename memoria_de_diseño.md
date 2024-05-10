# Electrónica IV - TP - HAL Parte 1 - Memoria de Diseño

Alumno: Fernando Alberto Miranda Bonomi

## Especificación

Desarrollaremos una capa de abstracción de hardware que permita controlar el
temporizador y los pines de entrada/salida del bluepill. Luego la emplearemos
para realizar un programa que haga parpadear el led integrado a la placa con
frecuencia de 1 Hz

## Pines de entrada/salida

Implementaremos en primer lugar las características estrictamente necesarias
para completar el programa de parpadeo, por lo cual averiguamos el puerto y pin
donde está conectado el led integrado a la bluepill. En la Figura 1 vemos el
pinout del bluepill. El LED está conectado al pin PC13, osea el pin 13 del
puerto C.

![Pinout de la bluepill, LED es PC13][fig-1]\
Figura 1. Pinout de la bluepill (Tolocka, 2024)

### Habilitación de reloj del puerto

Los pines de entrada/salida son controlados por el periférico GPIO conectado
al bus periférico APB2 del microcontrolador. Los periféricos de los buses APB1
y APB2 cuentan con una habilitación de reloj comandada por el periférico RCC
(Reset and Clock Control, control de reloj y reset). Para utilizar el puerto C
debemos en primer lugar habilitar su reloj. Según el manual de referencia de la
familia del microcontrolador (RM0008, 2021, sección 7.3.7) el reloj del puerto
GPIOC se habilita poniendo en uno el bit 4 (IOCEN) del registro de offset
0x18 (APB2ENR) en el periférico RCC, de dirección base 0x40021000 según
(Hoja de datos STM32F103x, 2015).

Para poner en 0 o en 1 bits individuales en un registro es necesario hacer una
operación de máscara. El Listado 1 muestra una operación de máscara donde se
pone en 0 los bit marcados como unos en MASCARA_RESET y luego se ponen en 1
los bit marcados como unos en MASCARA_SET. (...)

Listado 1: Operaciones de máscara.

```c
    enum{MASCARA_RESET = 0xF << (3*4),
         MASCARA_SET   = 0b0100 << (3*4)};
    palabra = (palabra & ~MASCARA_RESET) | MASCARA_SET ;
```

El Listado 2 muestra el código de habilitación de reloj del puerto GPIOC
usando las definiciones en `stm32f1xx.h`.

Listado 2. Habilitación de reloj para los puertos

```c
static void habilitaRelojPuertoC(void)
{
    RCC->APB2ENR = RCC->APB2ENR | RCC_APB2ENR_IOPCEN;
}
```

### Interfaz abstracta para controlar un pin

Según el enunciado, nuestra interfaz abstracta no puede incluir ningún detalle
sobre el hardware (fuera de las funciones mismas) o las librerías del fabricante. Para lograr este cometido utilizaremos el concepto de *handle* para
referirnos a recursos en forma abstracta (Handle, 2021). De esta manera
definiremos los handle que permiten acceder a los pines como miembros de un
*enum* de C. En el Listado 3 vemos las definiciones mínimas necesarias para
lograr completar nuestro práctico.

Listado 3. Handles y operaciones para referirse a los pines

```c
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
 * @brief Invierte el estado del pin (lo pone en alto si estaba bajo y
 * viceversa)
 * @note Se basa en el estado de salida, no en el estado lógico actual del pin
 * físico
 * @param pin el pin
 */
void Pin_invierteEstado(HPin pin);
```

## Referencias

- Tolocka,E (2024) *BluePill_STM32F103_Pinout*. Accedido el 10/05/2024 en <https://www.profetolocka.com.ar/2021/04/12/primeros-pasos-con-la-stm32-blue-pill/bluepill_stm32f103_pinout/>
- *RM0008* (febrero 2021) ST.
- *Hoja de datos STM32F103x* (agosto 2015) ST.
- *Handle* (29 de octubre de 2021) Wikipedia, Wikimedia Foundation. Accedido el 10/5/2024 en <https://es.wikipedia.org/wiki/Handle>

[fig-1]: figuras/pinout-bluepill.jpg