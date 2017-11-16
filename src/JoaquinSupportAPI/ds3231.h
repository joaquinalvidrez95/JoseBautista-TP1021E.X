#ifndef DS3231_H
#define DS3231_H

#define RTC_SDA  PIN_C4
#define RTC_SCL  PIN_C3 
#use i2c(master, sda=RTC_SDA, scl=RTC_SCL,fast=400000) 

#define DS3231_Address              0x68

#define DS3231_Read_addr            ((DS3231_Address << 1) | 0x01)
#define DS3231_Write_addr           ((DS3231_Address << 1) & 0xFE)

#define secondREG                   0x00
#define minuteREG                   0x01
#define hourREG                     0x02
#define dayREG                      0x03
#define dateREG                     0x04                             
#define monthREG                    0x05                           
#define yearREG                     0x06                   
#define alarm1secREG                0x07       
#define alarm1minREG                0x08
#define alarm1hrREG                 0x09           
#define alarm1dateREG               0x0A 
#define alarm2minREG                0x0B   
#define alarm2hrREG                 0x0C
#define alarm2dateREG               0x0D
#define controlREG                  0x0E
#define statusREG                   0x0F
#define ageoffsetREG                0x10
#define tempMSBREG                  0x11
#define tempLSBREG                  0x12

#define _24_hour_format             0
#define _12_hour_format             1
#define am                          0
#define pm                          1


int bcd_to_decimal(int d);
int decimal_to_bcd(int d);
int DS3231_Read(int address);
void DS3231_Write(int address, int value);
void DS3231_init();

int bcd_to_decimal(int d) {
    return ((d & 0x0F) + (((d & 0xF0) >> 4) * 10));
}

int decimal_to_bcd(int d) {
    return (((d / 10) << 4) & 0xF0) | ((d % 10) & 0x0F);
}

int DS3231_Read(int address) {
    int value = 0;
    I2C_start();
    I2C_write(DS3231_Write_addr);
    I2C_write(address);
    I2C_start();
    I2C_write(DS3231_Read_addr);
    value = I2C_read(0);
    I2C_stop();
    return value;
}

void DS3231_Write(int address, int value) {
    I2C_start();
    I2C_write(DS3231_Write_addr);
    I2C_write(address);
    I2C_write(value);
    I2C_stop();
}

void DS3231_init() {
    DS3231_Write(controlREG, 0x00);
    DS3231_Write(statusREG, 0x08);
}

void ds3231_get_Time(int &hora, int &min, int &sec) {
    sec = DS3231_Read(secondREG);
    sec = bcd_to_decimal(sec);
    min = DS3231_Read(minuteREG);
    min = bcd_to_decimal(min);
    hora = (0x3F & DS3231_Read(hourREG));
    hora = bcd_to_decimal(hora);
}

void DS3231_get_Date(int &date, int &month, int &year, int &dow) {
    year = DS3231_Read(yearREG);
    year = bcd_to_decimal(year);
    month = (0x1F & DS3231_Read(monthREG));
    month = bcd_to_decimal(month);
    date = (0x3F & DS3231_Read(dateREG));
    date = bcd_to_decimal(date);
    dow = (0x07 & DS3231_Read(dayREG));
    dow = bcd_to_decimal(dow);
}

void setTime(int hSet, int mSet, int sSet) {
    DS3231_Write(secondREG, (decimal_to_bcd(sSet)));
    DS3231_Write(minuteREG, (decimal_to_bcd(mSet)));
    DS3231_Write(hourREG, (0x3F & (decimal_to_bcd(hSet))));

}

void setDate(int daySet, int dateSet, int monthSet, int yearSet) {
    DS3231_Write(dayREG, (decimal_to_bcd(daySet)));
    DS3231_Write(dateREG, (decimal_to_bcd(dateSet)));
    DS3231_Write(monthREG, (decimal_to_bcd(monthSet)));
    DS3231_Write(yearREG, (decimal_to_bcd(yearSet)));
}

void DS3231_set_date_time(int dia, int mes, int year, int dow, int hora, int min, int sec) {
    settime(hora, min, sec);
    setdate(dow, dia, mes, year);
}

float getTemp() {
    register float t = 0.0;
    int lowByte = 0;
    signed char highByte = 0;
    lowByte = DS3231_Read(tempLSBREG);
    highByte = DS3231_Read(tempMSBREG);
    lowByte >>= 6;
    lowByte &= 0x03;
    t = ((float) lowByte);
    t *= 0.25;
    t += highByte;
    return t;

}
#endif