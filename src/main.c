#include "hal.h"

int main(void)
{
    Pin_ponModoSalida(Pin_LED);

    Pin_ponEstadoBajo(Pin_LED);

    for(;;){}

    return 0;
}