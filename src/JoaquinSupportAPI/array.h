/* 
 * File:   array.h
 * Author: DISENO4
 *
 * Created on 29 de septiembre de 2017, 04:01 PM
 */

#ifndef ARRAY_H
#define	ARRAY_H

#define Array_getArraySize(array)(sizeof((array))/sizeof((array)[0]))

void Array_clearArray(int array[], int arraySize) {
    int i = 0;

    for (i = 0; i < arraySize; i++) {
        array[i] = 0;
    }
}

void Array_splitNumberIntoDigits(int16 number, int digits[], int arraySize) {
    Array_clearArray(digits, arraySize);

    int index = 0;
    while (number > 0) {
        int digit = number % 10;
        digits[index] = digit;
        index++;
        number /= 10;
    }
}

#endif	/* ARRAY_H */

