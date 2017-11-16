/* 
 * File:   shiftregister.h
 * Author: Joaquín Alan Alvidrez Soto
 *
 * Created on 29 de septiembre de 2017, 04:09 PM
 */

#ifndef SHIFTREGISTER_H
#define	SHIFTREGISTER_H

#define  PIN_CLOCK                      PIN_A3      //CLK
#define  PIN_DATA                       PIN_A0    //NEAR R
#define  PIN_STROBE                     PIN_A1      //SCLK

void ShiftRegister_closeStrobe() {
    output_high(PIN_STROBE); // transfiere los datos a las salidas del registro 4094
    output_low(PIN_STROBE); // un pulso de strobe 
}

void ShiftRegister_sendData(int charactersArray [], int arraySize) {
    int i = 1;
    int j = 0;

    output_low(PIN_CLOCK); // EL pin clk se pone en bajo antes de enviar bits
    output_low(PIN_STROBE); // strobe en bajo

    for (j = 0; j < arraySize; j++) {
        int character = charactersArray[j];

        for (i = 1; i <= 8; i++) {
            output_bit(PIN_DATA, (character & 0x01)); // operación and para sacar bit a bit
            character >>= 1; //desplazo un bit a la derecha a datob y lo almaceno en dato 
            output_high(PIN_CLOCK); // saca un pulso de reloj
            output_low(PIN_CLOCK);
        }
    }

    ShiftRegister_closeStrobe();
}




#endif	/* SHIFTREGISTER_H */

