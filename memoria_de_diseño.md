# Electrónica IV - TP - HAL Parte 1 - Memoria de Diseño

Alumno: Fernando Alberto Miranda Bonomi

## 1. Especificación

Desarrollaremos una capa de abstracción de hardware que permita controlar el
temporizador y los pines de entrada/salida del bluepill. Luego la emplearemos
para realizar un programa que haga parpadear el led integrado a la placa con
frecuencia de 1 Hz

## 2. Pines de entrada/salida

Implementaremos en primer lugar las características estrictamente necesarias
para completar el programa de parpadeo, por lo cual averiguamos el puerto y pin
donde está conectado el led integrado a la bluepill. En la Figura 2.1 vemos el
pinout del bluepill. El LED está conectado al pin PC13, osea el pin 13 del
puerto C.

![Pinout de la bluepill, LED es PC13][fig-1]\
Figura 2.1. Pinout de la bluepill (Tolocka, 2024)

### 2.1. Habilitación de reloj del puerto

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
operación de máscara. El Listado 2.1 muestra una operación de máscara donde se
pone en 0 los bit marcados como unos en MASCARA_RESET y luego se ponen en 1
los bit marcados como unos en MASCARA_SET. (...)

Listado 2.1: Operaciones de máscara.

```c
    enum{MASCARA_RESET = 0xF << (3*4),
         MASCARA_SET   = 0b0100 << (3*4)};
    palabra = (palabra & ~MASCARA_RESET) | MASCARA_SET ;
```

El Listado 2.2 muestra el código de habilitación de reloj del puerto GPIOC
usando las definiciones en `stm32f1xx.h`.

Listado 2.2. Habilitación de reloj para los puertos

```c
static void habilitaRelojPuertoC(void)
{
    RCC->APB2ENR = RCC->APB2ENR | RCC_APB2ENR_IOPCEN;
}
```

### 2.2. Interfaz abstracta para controlar un pin

Según el enunciado, nuestra interfaz abstracta no puede incluir ningún detalle
sobre el hardware (fuera de las funciones mismas) o las librerías del fabricante. Para lograr este cometido utilizaremos el concepto de *handle* para
referirnos a recursos en forma abstracta (Handle, 2021). De esta manera
definiremos los handle que permiten acceder a los pines como miembros de un
*enum* de C. En el Listado 2.3 vemos las definiciones mínimas necesarias para
lograr completar nuestro práctico.

Listado 2.3. Handles y operaciones para referirse a los pines

```c
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
```

### 2.4. Descriptor de pin

En la interfaz abstracta los pines son descritos por *handles*, números enteros
arbitrarios. En la implementación debemos contar con un *descriptor* de pin,
una estructura de datos que reúne la información necesaria para implementar las
operaciones definidas por la interfaz. Cada periférico tiene un conjunto de registros que lo controlan y las librerías de soporte definen un tipo de dato
*struct* de C que describe la organización de esos registros y un conjunto
de constantes que describen los campos de bit dentro de cada registro en
particular. Haremos uso de estas definiciones para desarrollar nuestra
implementación.

Para implementar las operaciones sobre un pin necesitamos acceso a sus registros
de control (un puntero a la estructura de control de tipo GPIO_TypeDef), el
número de pin (un entero) y acceso a la rutina de habilitación de reloj para el
periférico GPIO desarrollada en la [Sección 2.1][sec-2-1]. En el Listado 2.4
vemos la definición del tipo *Pin*, descriptor de pin, y la implementación de la
tabla de descriptores que asocia cada *handle* a un *descriptor*. La tabla es
un arreglo calificado *const* (de solo lectura) con almacenamiento estático y
vinculación interna (especificado por *static*). Las variables con
almacenamiento estático calificadas *const* son ubicadas por el compilador en
memoria de programa, evitando así ocupar memoria RAM. Utilizamos *inicializador*
*designado* en el arreglo para asegurar la correspondencia entre *descriptores*
y *handles*, como así también para explicitar la inicialización de los distintos
miembros del *descriptor* (Array initialization, 2022; Struct and union
initialization 2023).

Listado 2.4. Tipo *Pin* y tabla de descriptores

```c
typedef struct Pin{
    /**
     * @brief Puntero a los registros del puerto
     * 
     */
    GPIO_TypeDef * puerto;
    /**
     * @brief Número del pin
     * 
     */
    int pin;
    /**
     * @brief Función que habilita el reloj, llamar antes de configurar
     * 
     */
    void (*habilitaReloj)(void);
}Pin;

// Tabla de descriptores de pin (en ROM)
static const Pin descriptores[Pin_NUM_HANDLES]={
    [Pin_LED] = {.puerto=GPIOC,.pin=13,.habilitaReloj=habilitaRelojPuertoC}
};
```

### 2.5. Configuración de modo de pin

En un primer momento implementamos solamente la configuración como salida,
puesto que es la necesaria para resolver el práctico. En primer lugar debemos
recuperar el *descriptor* a partir del *handle*. Antes de esto validamos el
*handle*, que debe estar comprendido entre 0 y el número de *handles* menos uno.
Luego de validar el *handle* obtenemos con el mismo un puntero a *descriptor*
mediante la tabla de descriptores. Una vez obtenido el descriptor habilitamos el
reloj del puerto (si ya está habilitado no genera cambios) y procedemos a
modificar el registro de configuración. De acuerdo al manual de referencia (RM0008, 2021, secciones 9.2.1 y 9.2.2) la configuración del pin se establece
mediante un campo de 4 bit que está ubicado, para el pin $n$ en $\mathrm{CRL}_{<4n\ldots4n+3>}$ si $n<8$ o $\mathrm{CRH}_{<4(n-8)\ldots4(n-8)+3>}$ para $n>8$.
De analizar las expresiones podemos ver que el offset (en bits) será siempre
$4(n \mod 8)$, y el registro será CRL si $n<8$ y CRH si $n>=8$. Recordamos
que los puertos pueden tener hasta 16 pines osea que $n$ varía entre 0 y 15.
De la misma documentación vemos que la configuración de salida *lenta* (baja
tasa de subida, apropiada para esta aplicación), *push-pull* (puede suministrar
o tomar corriente) y de *propósito general* (controlada por el registro de
salida de GPIO y no por otro periférico) corresponde al patron de bits de
configuración `0b0010`. En el Listado 2.5 mostramos la implementación de la
función de configuración de acuerdo a lo investigado.

Listado 2.5. Función de configuración

```c
void Pin_ponModoSalida(HPin pin)
{
    enum{MODO_SALIDA = 0b0010};
    if (pin >= Pin_NUM_HANDLES) return;
    const Pin *desc = descriptores+pin; // equivalente a &descriptores[pin]

    desc->habilitaReloj();

    const int offset = ((desc->pin % 8) * 4);
    const uint32_t mascaraReset = 0xF << offset;
    const uint32_t mascaraSet = MODO_SALIDA << offset;

    if (desc->pin < 8){
        desc->puerto->CRL = (desc->puerto->CRL & ~mascaraReset) | mascaraSet;
    }else{
        desc->puerto->CRH = (desc->puerto->CRH & ~mascaraReset) | mascaraSet;
    }
}
```

### 2.6. Establecer salida en alto y bajo

Para establecer en *alto* el estado de salida del pin $n$ debemos poner en uno
el bit $\mathrm{ODR}_{<n>}$ en el registro ODR. Esta acción requeriría un
proceso de lectura, modificación con máscara y escritura. Dicho proceso es
vulnerable a cambios realizados durante interrupciones, que podrían afectar el
valor del registro luego de ser leido y antes de ser escrito resultando en una
*condición de carrera de datos* (Condición de carrera, 2022) y la consiguiente
corrupción de datos. El periférico GPIO cuenta con un mecanismo de *bit set* y
*bit reset* que permite hacer este tipo de cambios en forma atómica (en una
única operación del bus). Para ello, si necesitamos establecer el estado en uno
escribimos un uno en el bit $\mathrm{BSRR}_{<n>}$ de BSRR (Bit Set and Reset
Register). Y si necesitamos establecer el estado en cero escribimos un uno en
$\mathrm{BSRR}_{<n+16>}$ o bien en $\mathrm{BRR}_{<n>}$ (este último Bit Reset
Register) (RM0008, 2021, secciones 9.1.2, 9.1.5 y 9.1.6). El Listado 2.6 muestra
nuestra implementación de las funciones para establecer el estado de salida de
un pin.

Listado 2.6. Implementación de `Pin_ponEstadoBajo` y `Pin_ponEstadoAlto`

```c
void Pin_ponEstadoBajo(HPin pin)
{
    if (pin >= Pin_NUM_HANDLES) return;
    const Pin *desc = descriptores + pin; // &descriptores[pin]
    const uint32_t mascaraPin = 1 << desc->pin;
    desc->puerto->BRR = mascaraPin;
}

void Pin_ponEstadoAlto(HPin pin)
{
    if (pin >= Pin_NUM_HANDLES) return;
    const Pin *desc = descriptores + pin; // &descriptores[pin]
    const uint32_t mascaraPin = 1 << desc->pin;
    desc->puerto->BSRR = mascaraPin;
}
```

### 2.7. Determinar el estado de salida de un pin

El estado de salida del pin $n$ corresponde al estado del bit
$\mathrm{ODR}_{<n>}$ en el registro ODR. Para consultarlo realizamos una
operación de máscara. El resultado de la operación de máscara puede retornarse
directamente, dado que al ser convertido en *bool* cualquier valor distinto de
cero se convierte en *true* (1). El Listado 2.7 muestra la implementación de
la función `Pin_consultaEstado`.

Listado 2.7. Implementación de `Pin_consultaEstado`

```c
bool Pin_consultaEstado(HPin pin)
{
    if (pin >= Pin_NUM_HANDLES) return false;
    const Pin *desc = descriptores + pin; // &descriptores[pin]
    const uint32_t mascaraPin = 1 << desc->pin;
    return desc->puerto->ODR & mascaraPin; 
}
```

### 2.8. Invertir el estado de salida de un pin

Para invertir el estado de salida de un pin basta con consultar su estado actual
y establecer el estado opuesto utilizando las funciones ya implementadas. El
Listado 2.8 muestra nuestra implementación.

Listado 2.8. Implementación de `Pin_invierteEstado`

```c
void Pin_invierteEstado(HPin pin)
{
    if (Pin_consultaEstado(pin)){
        Pin_ponEstadoBajo(pin);
    }else{
        Pin_ponEstadoAlto(pin);
    }
}
```

## 3. Temporizador

El temporizador debe permitir una espera bloqueante (mediante un lazo de espera)
de un número prescrito de milisegundos. El enunciado además indica utilizar el
temporizador del sistema SysTick como referencia de tiempo. El temporizador del
sistema es un contador descendente que puede configurarse para generar una
interrupción periódica (ARM®v7-M Architecture Reference Manual, 2017, Sección B3.3). Las librerías CMSIS-Core para Cortex-M (CMSIS-Core, 2024) proveen
funciones para la determinación de la frecuencia de reloj del sistema y para la
configuración del timer del sistema. En particular en (CMSIS-Core, 2024, API
Reference/Systick Timer) hay un ejemplo mínimo de configuración del timer,
incluyendo la rutina de servicio de interrupción para mantener un contador de
milisegundos. Utilizamos ese ejemplo como guía para implementar nuestra función
de temporización.

### 3.1. Interfaz abstracta

Implementamos una interfaz mínima que contempla una función de inicialización,
una función para obtener el valor de una cuenta de milisegundos actual como
entero sin signo y una función para generar un retardo bloqueante (detiene el
flujo del programa) de una cantidad prescrita de milisegundos. El entero sin
signo en formato binario natural tiene comportamiento módulo $2^{32}$, lo que
permite calcular el tiempo transcurrido como $T_f-T_i$, donde $T_f$ y $T_i$ son
los valores final e inicial del contador, sin que el cálculo sea afectado por
el desborde de la cuenta. El Listado 3.1 muestra la interfaz abstracta diseñada.
No hemos determinado un *handle* explícito porque en el contexto de este
práctico no contemplamos el uso de otros temporizadores distintos al
temporizador del sistema. En caso de que deba utilizarse un *handle* explícito,
todas las funciones tendrían como primer argumento dicho *handle*. Se utiliza el
un tipo definido por el usuario para permitir flexibilidad en la elección del
tipo subyacente al cambiar de arquitectura.

Listado 3.1. Interfaz abstracta para el temporizador

```c
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

```

### 3.2. Inicialización

Es necesario inicializar el temporizador del sistema antes de poder ejecutar
las funciones que de él dependen. La función de inicialización provista por la
librería CMSIS acepta como parámetro la cantidad de cuentas entre
interrupciones. Para configurar una interrupción periódica determinamos la
frecuencia de reloj y calculamos el número de ciclos necesarios para completar
un milisegundo, $N=\dfrac{f_\mathrm{ck}}{1000}$ donde la $f_\mathrm{ck}$ es la
frecuencia de reloj en Hertz. La frecuencia de reloj está disponible en la
variable `SystemCoreClock` que es inicializada por la función
`SystemCoreClockUpdate`, y ARM recomienda llamar dicha función antes de usar la
variable (CMSIS-Core, 2024, API Reference/System and Clock Configuration/
SystemCoreClock). El Listado 3.2. muestra la rutina de inicialización del
temporizador del sistema realizada en base a la documentación de CMSIS. La
rutina de inicialización habilita la interrupción de SysTick, por lo tanto es
necesario asegurarse que esté presente la rutina de servicio de interrupción
`SysTick_Handler` (de lo contrario el handler por defecto es un lazo infinito).

Listado 3.2. Inicialización del temporizador

```c
void Temporizador_inicializa(void)
{
    SystemCoreClockUpdate();

    const uint32_t ciclosPorMilisegundo = SystemCoreClock/1000;

    SysTick_Config(ciclosPorMilisegundo);
}
```

### 3.3. Rutina de servicio de interrupción

La rutina de servicio de interrupción será ejecutada una vez por milisegundo por
el timer del sistema. En ella actualizamos una variable contador de milisegundos
de tipo entero sin signo de 32 bit que corre en forma libre. Mantenemos la cuenta en una variable calificada *volatile*, lo que garantiza que su valor es
leido en cada uso de la variable en lugar de mantenerse en un registro como
optimización. Esto es necesario siempre que una variable puede ser modificada
por algún proceso externo al programa principal, tal como una rutina de
servicio de interrupción.

> *Nota*: la variable *volatile* no garantiza consistencia en caso de carrera
de datos. Sin embargo aquí existe un único punto en que puede ser modificada
dicha variable. Esta garantía es fuerte debido al enlace interno (especificador
*static*) de la variable y que el acceso a la misma desde fuera es posible solo
a través de las funciones provistas.

El Listado 3.3. muestra nuestra implementación de rutina de servicio de interrupción.

Listado 3.3. Rutina de servicio de interrupción y variable de cuenta de
milisegundos

```c
static volatile Milisegundos cuenta;

void SysTick_Handler(void)
{
    ++cuenta;
}
```

### 3.4 Espera y obtención de la cuenta de milisegundos

El valor del contador de milisegundos solo es accesible mediante la función `Temporizador_obtCuenta`, esto previene que el código cliente altere el
contador, lo que podría resultar en una carrera de datos. La rutina de espera
admite como parámetro un tiempo en milisegundos y retorna control al programa
solo luego de que haya transcurrido dicho tiempo. No retorna valor puesto que
fue diseñada para emplearse por su efecto secundario únicamente. El Listado 3.4
muestra la implementación de estas funciones.

> *Nota*: La función `Temporizador_espera` no funcionaría si *cuenta* no fuese
calificada *volatile*, puesto que en ese caso su valor sería cargado en un
registro al principio y nunca más se leería desde memoria (operaría sobre su
propia copia de la variable, que no responde a cambios en el original).

Listado 3.4. Funciones para obtener cuenta y esperar

```c
Milisegundos Temporizador_obtCuenta(void)
{
    return cuenta;
}

void Temporizador_espera(Milisegundos tiempo)
{
    const Milisegundos inicial = cuenta;
    while(cuenta - inicial < tiempo);
}
```

## 4. Programa principal

El programa principal implementado utilizando la capa de abstracción de hardware
desarrollada se muestra en el Listado 4.1. Simplemente leer el código permite
imaginar *que* hace, pero no *como* lo hace. Esta es la característica principal
de una interfaz abstracta.

Listado 4.1. Programa principal

```c
#include "hal.h"

int main(void)
{
    Temporizador_inicializa();
    Pin_ponModoSalida(Pin_LED);
    Pin_ponEstadoAlto(Pin_LED);
    for(;;){
        Temporizador_espera(500);
        Pin_invierteEstado(Pin_LED);
    }
    return 0;
}
```

## Referencias

- *ARM®v7-M Architecture Reference Manual*. (19 de mayo de 2017) ARM Limited.
- *Array initialization*. (16 de octubre de 2022) cppreference.com. Accedido el 10/05/2024 en <https://en.cppreference.com/w/c/language/array_initialization>
- *CMSIS-Core*. (8 de mayo de 2024) ARM Limited. Accedido el 10/05/2024 en <https://arm-software.github.io/CMSIS_6/latest/Core/index.html>
- *Condición de carrera*. (2 de febrero de 2022) Wikipedia. Accedido el 10/05/2024 en <https://es.wikipedia.org/wiki/Condici%C3%B3n_de_carrera>
- *Handle* (29 de octubre de 2021) Wikipedia, Wikimedia Foundation. Accedido el 10/5/2024 en <https://es.wikipedia.org/wiki/Handle>
- *Hoja de datos STM32F103x* (agosto 2015) ST.
- *RM0008* (febrero 2021) ST.
- *Struct and union inicialization*. (26 de enero de 2023) cppreference.com. Accedido el 10/05/2024 en <https://en.cppreference.com/w/c/language/struct_initialization>
- Tolocka,E (2024) *BluePill_STM32F103_Pinout*. Accedido el 10/05/2024 en <https://www.profetolocka.com.ar/2021/04/12/primeros-pasos-con-la-stm32-blue-pill/bluepill_stm32f103_pinout/>

[fig-1]: figuras/pinout-bluepill.jpg
[sec-2-1]: #21-habilitación-de-reloj-del-puerto