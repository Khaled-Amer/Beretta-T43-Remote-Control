/* ENEL 500
 * 
 * File: main.c 
 * 
 * Authors: 
 */

#include "xc.h"
#include <p24fxxxx.h>
#include <p24F16KA101.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <libpic30.h>

#include "ChangeClk.h"

//// CONFIGURATION BITS ////

// Code protection 
#pragma config BSS = OFF // Boot segment code protect disabled
#pragma config BWRP = OFF // Boot sengment flash write protection off
#pragma config GCP = OFF // general segment code protecion off
#pragma config GWRP = OFF

// CLOCK CONTROL 
#pragma config IESO = OFF    // 2 Speed Startup disabled
#pragma config FNOSC = FRC  // Start up CLK = 8 MHz
#pragma config FCKSM = CSECMD // Clock switching is enabled, clock monitor disabled
#pragma config SOSCSEL = SOSCLP // Secondary oscillator for Low Power Operation
#pragma config POSCFREQ = MS  //Primary Oscillator/External clk freq betwn 100kHz and 8 MHz. Options: LS, MS, HS
#pragma config OSCIOFNC = ON  //CLKO output disabled on pin 8, use as IO. 
#pragma config POSCMOD = NONE  // Primary oscillator mode is disabled

// WDT
#pragma config FWDTEN = OFF // WDT is off
#pragma config WINDIS = OFF // STANDARD WDT/. Applicable if WDT is on
#pragma config FWPSA = PR32 // WDT is selected uses prescaler of 32
#pragma config WDTPS = PS1 // WDT postscler is 1 if WDT selected

// MCLR/RA5 CONTROL
#pragma config MCLRE = OFF // RA5 pin configured as input, MCLR reset on RA5 diabled

// BOR  - FPOR Register
#pragma config BORV = LPBOR // LPBOR value=2V is BOR enabled
#pragma config BOREN = BOR0 // BOR controlled using SBOREN bit
#pragma config PWRTEN = OFF // Powerup timer disabled
#pragma config I2C1SEL = PRI // Default location for SCL1/SDA1 pin

// JTAG FICD Register
#pragma config BKBUG = OFF // Background Debugger functions disabled
#pragma config ICS = PGx2 // PGC2 (pin2) & PGD2 (pin3) are used to connect PICKIT3 debugger

// Deep Sleep RTCC WDT
#pragma config DSWDTEN = OFF // Deep Sleep WDT is disabled
#pragma config DSBOREN = OFF // Deep Sleep BOR is disabled
#pragma config RTCOSC = LPRC// RTCC uses LPRC 32kHz for clock
#pragma config DSWDTOSC = LPRC // DeepSleep WDT uses Lo Power RC clk
#pragma config DSWDTPS = DSWDTPS7 // DSWDT postscaler set to 32768 

// MACROS
#define Nop() {__asm__ volatile ("nop");}
#define ClrWdt() {__asm__ volatile ("clrwdt");}
#define Sleep() {__asm__ volatile ("pwrsav #0");}   // set sleep mode
#define Idle() {__asm__ volatile ("pwrsav #1");}
#define dsen() {__asm__ volatile ("BSET DSCON, #15");}

// Definition for the built-in delay function
#define FCY 4000000UL // 8 MHz/2 Instruction cycle frequency, Hz

#define CHNUP 0xE0E048B7 
#define CHNDWN 0xE0E008F7 
#define VOLUP 0xE0E0E01F 
#define VOLDWN 0xE0E0D02F 
#define PWRSW 0xE0E040BF

void init_timer(void){
    // Timer 1 config bits - Used for Data
    IEC0bits.T1IE = 1;
    T1CONbits.TSIDL = 0;
    T1CONbits.TCS = 0;
    IPC0bits.T1IP = 0b100;
}
//delay in us
void delay_us(uint16_t time_us)
{
    //NewClk(8);
    PR1 = time_us << 2;
    T1CONbits.TON = 1;
    Idle();
}

//delay in ms (max delay: 260 ms)
void delay_ms(uint16_t time_ms){
    NewClk(500);
    PR1 = time_ms * 250;
    T1CONbits.TON = 1;
    Idle();
    NewClk(8);
}

void start_bit(void){
    LATBbits.LATB4 = 1; 
    delay_us(4500);
    LATBbits.LATB4 = 0;  
    delay_us(4500);
}

void zero_bit(void){
    LATBbits.LATB4 = 1;  
    delay_us(560);
    LATBbits.LATB4 = 0;  
    delay_us(560);
}

void one_bit(void){
    LATBbits.LATB4 = 1;  
    delay_us(560);
    LATBbits.LATB4 = 0;  
    delay_us(1690);
}

void send_signal(uint32_t number){
   
    uint8_t i = 0;   // Aux variable
    
    start_bit(); // Start bit
    
    // Loop for sending 
    for(i = 0; i < 32; i++){    
        if(((number>>(31-i)) & 1) == 1){
            one_bit();
        }
        else{
            zero_bit();
        }
    }   
    
    zero_bit(); // end bit
}

int main(void) {
    
    // Analog ports to digital
    AD1PCFG = 0xFFFF; 
     
    init_timer();
    
    TRISAbits.TRISA4 = 0;
    TRISBbits.TRISB4 = 0;
    LATAbits.LATA4 = 1;  

    while(1) {
        send_signal(0xE0E048B7);
        delay_ms(250);
    }
    return 0;
}

void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void){

}

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
    T1CONbits.TON = 0;
    IFS0bits.T1IF=0; //Clear timer 1 interrupt flag
}
