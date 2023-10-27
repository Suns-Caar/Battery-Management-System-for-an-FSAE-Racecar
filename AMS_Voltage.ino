#include <LTC6812.h>

#include <LT_SPI.h>

#include <Arduino.h>
#include <stdint.h>
#include <SPI.h>
#include "Linduino.h"
#include "UserInterface.h"
#include "LTC681x.h"
#include "LTC6812.h"



#define ENABLED 1
#define DISABLED 0
#define DATALOG_ENABLED 1
#define DATALOG_DISABLED 0

void check_error(int error);

const uint8_t TOTAL_IC = 1; //!< Number of ICs in the daisy chain

const uint8_t ADC_OPT = ADC_OPT_DISABLED; //!< ADC Mode option bit
const uint8_t ADC_CONVERSION_MODE = MD_7KHZ_3KHZ; //!< ADC Mode
const uint8_t ADC_DCP = DCP_DISABLED; //!< Discharge Permitted 
const uint8_t CELL_CH_TO_CONVERT = CELL_CH_ALL; //!< Channel Selection for ADC conversion
const uint8_t AUX_CH_TO_CONVERT = AUX_CH_ALL;  //!< Channel Selection for ADC conversion
const uint8_t STAT_CH_TO_CONVERT = STAT_CH_ALL;  //!< Channel Selection for ADC conversion
const uint8_t SEL_ALL_REG = REG_ALL; //!< Register Selection 
const uint8_t SEL_REG_A = REG_1; //!< Register Selection 
const uint8_t SEL_REG_B = REG_2; //!< Register Selection 

const uint16_t MEASUREMENT_LOOP_TIME = 500; //!< Loop Time in milliseconds(ms)

//Under Voltage and Over Voltage Thresholds
const uint16_t OV_THRESHOLD = 42000; //!< Over voltage threshold ADC Code. LSB = 0.0001 ---(4.1V)
const uint16_t UV_THRESHOLD = 28000; //!< Under voltage threshold ADC Code. LSB = 0.0001 ---(3V)

cell_asic BMS_IC[TOTAL_IC]; //!< Global Battery Variable

/*************************************************************************
 Set configuration register. Refer to the data sheet
**************************************************************************/
bool REFON = true; //!< Reference Powered Up Bit
bool ADCOPT = false; //!< ADC Mode option bit
bool GPIOBITS_A[5] = {true,true,true,true,true}; //!< GPIO Pin Control // Gpio 1,2,3,4,5
bool GPIOBITS_B[4] = {true,true,true,true}; //!< GPIO Pin Control // Gpio 6,7,8,9
uint16_t UV=UV_THRESHOLD; //!< Under voltage Comparison Voltage
uint16_t OV=OV_THRESHOLD; //!< Over voltage Comparison Voltage
bool DCCBITS_A[12] = {false,false,false,false,false,false,false,false,false,false,false,false}; //!< Discharge cell switch //Dcc 1,2,3,4,5,6,7,8,9,10,11,12
bool DCCBITS_B[7]= {false,false,false,false}; //!< Discharge cell switch //Dcc 0,13,14,15
bool DCTOBITS[4] = {true,false,true,false}; //!< Discharge time value //Dcto 0,1,2,3  // Programed for 4 min 
/*Ensure that Dcto bits are set according to the required discharge time. Refer to the data sheet */
bool FDRF = false; //!< Force Digital Redundancy Failure Bit
bool DTMEN = true; //!< Enable Discharge Timer Monitor
bool PSBits[2]= {false,false}; //!< Digital Redundancy Path Selection//ps-0,1

int error;
double c_v[TOTAL_IC][14];
double min_c_v=4.2000;
int min_c_v_la;
int min_c_v_lb;
int c_v_la[(TOTAL_IC*14)-1];
int c_v_lb[(TOTAL_IC*14)-1];



void setup()
{
  Serial.begin(115200);
  quikeval_SPI_connect();
  spi_enable(SPI_CLOCK_DIV16); // This will set the Linduino to have a 1MHz Clock
  LTC6812_init_cfg(TOTAL_IC, BMS_IC);
  LTC6812_init_cfgb(TOTAL_IC,BMS_IC);
  for (uint8_t current_ic = 0; current_ic<TOTAL_IC;current_ic++) 
  {
    LTC6812_set_cfgr(current_ic,BMS_IC,REFON,ADCOPT,GPIOBITS_A,DCCBITS_A, DCTOBITS, UV, OV);
    LTC6812_set_cfgrb(current_ic,BMS_IC,FDRF,DTMEN,PSBits,GPIOBITS_B,DCCBITS_B);   
  }   
  LTC6812_reset_crc_count(TOTAL_IC,BMS_IC);
  LTC6812_init_reg_limits(TOTAL_IC,BMS_IC);
   wakeup_sleep(TOTAL_IC);

}




void check_error(int error)
{
  if (error == -1)
    Serial.println(F("A PEC error was detected in the received data"));
}


void loop()
{ 

  
 

    
//Cell Voltages  
    wakeup_idle(TOTAL_IC);
    LTC6812_wrcfg(TOTAL_IC,BMS_IC); // Write into Configuration Register
    LTC6812_wrcfgb(TOTAL_IC,BMS_IC); // Write into Configuration Register B

    LTC6812_adcv(ADC_CONVERSION_MODE,ADC_DCP,CELL_CH_TO_CONVERT);

    error = LTC6812_rdcv(SEL_ALL_REG, TOTAL_IC,BMS_IC); // Set to read back all cell voltage registers
    check_error(error);
    for (int current_ic = 0 ; current_ic < TOTAL_IC; current_ic++)
    {
      Serial.print("IC");
      Serial.print(current_ic+1,DEC);
      Serial.print(":-");
      for (int i=0; i<((BMS_IC[0].ic_reg.cell_channels)-1); i++)
      {
        Serial.print(" C");
        Serial.print(i+1,DEC);
        Serial.print(":");
        c_v[current_ic][i]=BMS_IC[current_ic].cells.c_codes[i]*0.0001;
        Serial.print(c_v[current_ic][i],4);
        Serial.print(" ");
      }
      Serial.println();
     }
    Serial.println("\n");


//Finding the Minimum Cell Voltage
   for(int i=0; i<TOTAL_IC; i++)
   for(int j=0; j<14; j++)
   {
    if(c_v[i][j]<min_c_v)
    {
     min_c_v=c_v[i][j];
     min_c_v_la=i+1;
     min_c_v_lb=j+1;
    }   
   }
   Serial.print("Minimum Cell Voltage :- ");
   Serial.print(min_c_v,4);
   Serial.print("@ IC=");
   Serial.print(min_c_v_la);
   Serial.print(" and Cell No.=");
   Serial.print(min_c_v_lb);
   Serial.println("\n");
  


//Aux Voltages (GPIO Pins)
    wakeup_idle(TOTAL_IC);
     LTC6812_adax(ADC_CONVERSION_MODE, AUX_CH_TO_CONVERT);

     error = LTC6812_rdaux(SEL_ALL_REG,TOTAL_IC,BMS_IC); // Set to read back all aux registers
     check_error(error);
     for (int current_ic =0 ; current_ic < TOTAL_IC; current_ic++)
     {
       Serial.print("IC");
      Serial.print(current_ic+1,DEC);
      Serial.print(":-");
      for (int i = 0; i < 5; i++)
      {
        Serial.print(F(" GPIO"));
        Serial.print(i+1,DEC);
        Serial.print(":");
        Serial.print(BMS_IC[current_ic].aux.a_codes[i]*0.0001,4);
        Serial.print(" ");
      }
      for (int i=6; i < 10; i++)
      {
        Serial.print(F(" GPIO"));
        Serial.print(i,DEC);
        Serial.print(":");
        Serial.print(BMS_IC[current_ic].aux.a_codes[i]*0.0001,4);
        Serial.print(" ");
      }
      Serial.println();
     }
     Serial.println("\n");


//Sum of Cells and Internal Temperature
     wakeup_idle(TOTAL_IC);
     LTC6812_adstat(ADC_CONVERSION_MODE, STAT_CH_TO_CONVERT);
     
     error = LTC6812_rdstat(SEL_ALL_REG,TOTAL_IC,BMS_IC); // Set to read back all stat registers
     check_error(error);
     double itmp;
     for (int current_ic =0 ; current_ic < TOTAL_IC; current_ic++)
     {
      Serial.print(F("IC"));
      Serial.print(current_ic+1,DEC);
      Serial.print(":-");
      Serial.print(F(" SOC:"));
      Serial.print(BMS_IC[current_ic].stat.stat_codes[0]*0.0001*30,4);
      Serial.print(F("   "));
      Serial.print(F("Itemp:"));
      itmp = (double)((BMS_IC[current_ic].stat.stat_codes[1] * (0.0001 / 0.0076)) - 276);   //Internal Die Temperature(°C) = ITMP • (100 µV / 7.6mV)°C - 276°C
      Serial.print(itmp,4);  
       Serial.println();    
     }
      Serial.println("\n");


/*
 
 int m=0;
//Balancing Cells
   wakeup_idle(TOTAL_IC);
   for(int a=0; a<TOTAL_IC; a++)
   {
   for(int b=0; b<14; b++)
   {
     double difference=c_v[a][b]-min_c_v;
    // Serial.println(difference);
    if(difference>0.01)
    {
     m++;
     c_v_la[m]=a;
     c_v_lb[m]=b;
     Serial.print(c_v_la[m]+1);
     Serial.print(" , ");
     Serial.print(c_v_lb[m]+1);
     Serial.print("     ");
    }
   }
   }
        Serial.println("\n");
for(int j=0; j<50; j++)
{
  delay(100);
   for(int i=1;i<=m;i++)
   {
     wakeup_idle(TOTAL_IC);
     if(c_v_la[i]==0)
     {   
     LTC6812_set_discharge(c_v_lb[i]+1, 1,BMS_IC); 
     LTC6812_wrcfg(TOTAL_IC,BMS_IC);
     LTC6812_wrcfgb(TOTAL_IC,BMS_IC);
     }
     if(c_v_la[i]==1)
     {   
     LTC6812_set_discharge(c_v_lb[i]+1, 0,BMS_IC); 
     LTC6812_wrcfg(TOTAL_IC,BMS_IC);
     LTC6812_wrcfgb(TOTAL_IC,BMS_IC);
     }
}
}

   
LTC6812_clear_discharge(TOTAL_IC,BMS_IC);   
LTC6812_wrcfg(TOTAL_IC,BMS_IC);
LTC6812_wrcfgb(TOTAL_IC,BMS_IC);
*/

}
