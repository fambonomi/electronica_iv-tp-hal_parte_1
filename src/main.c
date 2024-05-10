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