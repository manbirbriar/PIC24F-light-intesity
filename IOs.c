#include "IOs.h"          
#include "stdio.h"        
#include "ADC.h"          
#include "TimeDelay.h"    
#include <xc.h>           
#include <p24F16KA101.h>  
#include "UART2.h"        

#define LED LATBbits.LATB8  // Macro to control the LED output using pin RB8

// External delay flag from TimeDelay.c
extern int delay;

// Global variables to store previous button states (debouncing)
uint8_t prevStatePB1 = 1; // For RA4 (PB1) - initial state is released (high)
uint8_t prevStatePB2 = 1; // For RB4 (PB2) - initial state is released (high)
uint8_t prevStatePB3 = 1; // For RA2 (PB3) - initial state is released (high)

// Flags for tracking system modes
uint8_t mode = 0;       // Flag to track ON/OFF mode
uint8_t blink = 0;      // Flag to track blink state
uint8_t transmit = 0;   // Flag to track UART transmission state

// Variables for LED brightness and ADC values
uint8_t intensity;      // Holds light intensity percentage (0-100)
uint16_t adc_value;      // Holds raw ADC value
float brightness;       // Brightness value scaled from 0 to 1


// Function to initialize Input/Output configurations
void IOinit(){
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */

    // Configure RB8 as an Output and Initialize It Low
    TRISBbits.TRISB8 = 0;   // Set RB8 as an output
    LATBbits.LATB8 = 0;     // Initialize RB8 to low (0)
    
    // Configure RA4 as an Input with Pull-Up and Change Notification
    TRISAbits.TRISA4 = 1;   // Set RA4 as an input
    CNPU1bits.CN0PUE = 1;   // Enable pull-up resistor on CN0 (RA4)
    CNEN1bits.CN0IE = 1;    // Enable Change Notification Interrupt on CN0
    
     // Configure RB4 as an Input with Pull-Up and Change Notification
    TRISBbits.TRISB4 = 1;   // Set RB4 as an input
    CNPU1bits.CN1PUE = 1;   // Enable pull-up resistor on CN1 (RB4)
    CNEN1bits.CN1IE = 1;    // Enable Change Notification Interrupt on CN1

    // Configure RA2 as an Input with Pull-Up and Change Notification
    TRISAbits.TRISA2 = 1;   // Set RA2 as an input
    CNPU2bits.CN30PUE = 1;  // Enable pull-up resistor on CN30 (RA2)
    CNEN2bits.CN30IE = 1;   // Enable Change Notification Interrupt on CN30
    
    // Configure Interrupt Priorities and Enable Interrupts
    IPC4bits.CNIP = 6;  // Set Change Notification Interrupt Priority to 6
    IFS1bits.CNIF = 0;  // Clear the Change Notification Interrupt Flag
    IEC1bits.CNIE = 1;  // Enable the Change Notification Interrupt
}


// Function to configure and start the PWM (Pulse Width Modulation) timer
void PWM() {
    T2CONbits.T32 = 0;      // Disable 32-bit timer mode to operate in 16-bit
    T3CONbits.TCKPS = 1;    // Set prescaler to 8
    T3CONbits.TSIDL = 0;    // Continue operation in idle mode
    T3CONbits.TCS = 0;      // Use internal clock

    // Configure Timer 3 interrupt
    IPC2bits.T3IP = 2;      // Set priority level
    IFS0bits.T3IF = 0;      // Clear Timer 3 interrupt flag
    IEC0bits.T3IE = 1;      // Enable Timer 3 interrupt

    // Set PWM period based on LED state and brightness
    if (LED) {
        PR3 = (10000 * brightness) + 1; // Brightness ON period
    } else {
        PR3 = (10000 * (1 - brightness)) + 1; // Brightness OFF period
    }

    TMR3 = 0;               // Reset Timer 3 counter
    T3CONbits.TON = 1;      // Enable Timer 3
}

    
// Timer 3 interrupt service routine (ISR) for PWM control
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void) {
    IFS0bits.T3IF = 0;      // Clear Timer 3 interrupt flag
    T3CONbits.TON = 0;      // Disable Timer 3

    // Toggle LED state and update PWM period
    if (LED || brightness == 0) {
        LED = 0;
        PR3 = (10000 * (1 - brightness)) + 1;
    } else {
        LED = 1;
        PR3 = (10000 * brightness) + 1;
    }

    TMR3 = 0;               // Reset Timer 3 counter
    T3CONbits.TON = 1;      // Enable Timer 3
}

// Function to check and handle Input/Output events
void IOcheck(){
    
    // If in on mode and blink off
    if (mode && blink == 0){
        PWM();  // Start PWM timer 3
        
        while(mode && blink ==0){   // Only continue while in same mode
            adc_value = do_ADC();    // Get ADC value
            //brightness = ADCvalue * 0.0009776; 
            brightness = adc_value * 0.0009775171;   // Calculate brightness value 
            
            // If transmit on, transmit adc and brightness value
            if(transmit){   
                if(adc_value == 1023){
                    intensity = 100;
                }else{
                    intensity = brightness*100;
                }
                XmitUART2('P',1);
                Disp2Dec(intensity);
                XmitUART2('A',1);
                Disp2Dec(adc_value);
                XmitUART2('\n',1); 
            }
        }
        TMR3 = 0;   // Reset timer
        T3CONbits.TON = 0;  // Turn timer off
    }
    
    // If in on mode blink on
    else if (mode && blink){ 
        PWM();  // Start PWM timer 3
        
        while(mode && blink){   // Only continue while in same mode
            if(brightness == 0){    //If the LED off turn it on
                adc_value = do_ADC();    // Get ADC value
                //brightness = ADCvalue * 0.0009776; 
                brightness = adc_value * 0.0009775171;   // Calculate brightness value 
            }
            else{   // If the LED was ON turn it off
                brightness = 0;
            }
            delay_ms(500);  // delay for 0.5 seconds
            while(delay==0){    // can change brightness level until 0.5 seconds is over
                if(brightness){ 
                    adc_value = do_ADC();    // Get ADC value
                    //brightness = ADCvalue * 0.0009776;
                    brightness = adc_value * 0.0009775171;   // Calculate brightness value 
                }
            }
            
            // If transmit on, transmit adc and brightness value
            if(transmit){
                if(adc_value == 1023 && LED==1){
                    intensity = 100;
                }else{
                    intensity = brightness*100;
                }
                XmitUART2('P',1);
                Disp2Dec(intensity);
                XmitUART2('A',1);
                Disp2Dec(adc_value);
                XmitUART2('\n',1); 
            }
        }
        TMR3 = 0;   // Reset timer
        T3CONbits.TON = 0;  // Turn timer off    
    }
    
    // If in off mode and blink on
    else if (mode == 0 && blink){
        LED = 1;    // Turn LED on first
        delay_ms(500);  // Delay for 0.5 seconds
        brightness = 1; // Light on so brightness = 1
        while(mode == 0 && blink){  // Only continue while in same mode
            if(delay==1){  // If 0.5 seconds has passed
                LED = !LED; // Toggle LED
                delay_ms(500);  // Delay for 0.5 seconds
            }
        }
    }
    
    // If not in any mode turn off light and go into idle
    else{
        LED = 0;
        Idle();
    }

}


void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void) {
    // Don't forget to clear the CN interrupt flag!
    IFS1bits.CNIF = 0;

    // Check the state of each button and implement debouncing (only toggle on release)
    if (PORTAbits.RA4 == 1 && prevStatePB1 == 0) {  // PB1 released (active low)
        mode = !mode;  // Toggle mode flag
    }
    prevStatePB1 = PORTAbits.RA4;  // Update previous state of PB1

    if (PORTBbits.RB4 == 1 && prevStatePB2 == 0) {  // PB2 released (active low)
        blink = !blink;  // Toggle blink flag
    }
    prevStatePB2 = PORTBbits.RB4;  // Update previous state of PB2

    if (PORTAbits.RA2 == 1 && prevStatePB3 == 0 && mode ==1) {  // PB3 released (active low) only turn transmission on when mode is on
        transmit = !transmit;  // Toggle transmit flag
    }
    prevStatePB3 = PORTAbits.RA2;  // Update previous state of PB3
}
