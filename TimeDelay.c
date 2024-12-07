#include "xc.h"
#include "TimeDelay.h"

uint8_t delay = 0;

void delay_ms(float time_ms){    
    //T2CON configuration
    T2CONbits.T32 = 0; // disable 32-bit timer mode to operate in 16-bit
    T2CONbits.TCKPS = 3; // prescalar value of 256
    T2CONbits.TSIDL = 0; // operates in idle mode
    T2CONbits.TCS = 0; // use internal clock
    
    //Timer 2 interrupt configuration
    IPC1bits.T2IP = 4; // priority level
    IFS0bits.T2IF = 0; // clears timer 2 interrupt flag
    IEC0bits.T2IE = 1; // enable time interrupt

    PR2 = time_ms*15.625; // PR2 calculation
    TMR2= 0;

    delay = 0;  // Reset delay flag
    T2CONbits.TON = 1;
}   

// Timer 2 interrupt subroutine
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    delay = 1;
    IFS0bits.T2IF=0; // Clear timer 2 interrupt flag
    TMR2 = 0; // Reset TMR2
    T2CONbits.TON=0; // Stop timer
    return;

}