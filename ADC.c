#include "ADC.h"

// Function to configure and read from the ADC on AN5 (RA3)
uint16_t do_ADC(void) {
    uint16_t ADCvalue;  // Variable to hold the ADC result
    // ------------- ADC INITIALIZATION ------------------
    AD1CON1 = 0;                // Clear ADC control register
    AD1CON1bits.FORM = 0;       // Integer output format
    AD1CON1bits.SSRC = 0b111;   // Auto conversion
    AD1CON1bits.ASAM = 0;       // Manual sampling
    
    AD1CON2 = 0;                // AVdd and AVss as voltage reference
    AD1CON2bits.VCFG = 0b000;   // AVdd, AVss as Vref
    
    AD1CON3 = 0;
    AD1CON3bits.ADRC = 0;       // Use system clock
    AD1CON3bits.SAMC = 16;      // Sample time = 16 TAD
    AD1CON3bits.ADCS = 2;       // TAD = 2*TCY (adjust as needed)
    
    AD1CHS = 0;                 // Channel select register
    AD1CHSbits.CH0SA = 5;       // Select AN5 as the input for A/D channel
    
    AD1PCFG = 0xFFFF;           // Set all pins to digital by default
    AD1PCFGbits.PCFG5 = 0;      // Set AN5 (RA3) as analog input

    AD1CON1bits.ADON = 1;       // Turn on ADC module

    // ------------- ADC SAMPLING AND CONVERSION ------------------
    AD1CON1bits.SAMP = 1;       // Start sampling
    while (!AD1CON1bits.DONE);  // Wait for conversion to complete
    
    ADCvalue = ADC1BUF0;        // Read the conversion result
    AD1CON1bits.SAMP = 0;       // Stop sampling
    AD1CON1bits.ADON = 0;       // Turn off ADC

    return ADCvalue;            // Return the 10-bit ADC result
}