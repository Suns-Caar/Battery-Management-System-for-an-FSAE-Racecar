#include <FlexCAN_T4.h>
#include "arduino_freertos.h"
#include "avr/pgmspace.h"
#include <semphr.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;
CAN_message_t msg;
CAN_message_t receivedMsg;
char buf[40];
String checks;
int8_t  dataTherm[10];
int value1, adcValue;
unsigned long value3;
SemaphoreHandle_t adcMutex;
bool adcValueReady = false;
QueueHandle_t canQueue;

static void task1(void*) {
    while (true) {
        if (adcValueReady) {
            xSemaphoreTake(adcMutex, portMAX_DELAY);
            int adcValue = value1;
            adcValueReady = false;
            xSemaphoreGive(adcMutex);

            msg.id = 1232;
            for (uint8_t i = 0; i < 8; i++) {
                msg.buf[i] = (uint8_t)((adcValue >> (i * 8)) & 0xFF);
            }
//            ::Serial.println(adcValue);
//            can1.write(msg);
//            sprintf(buf, "CAN sent with id: %lu \n", msg.id); // Use %lu for uint32_t
//            ::Serial.println(buf);
        }
        ::vTaskDelay(pdMS_TO_TICKS(500));
    }
}

static void task2(void*) {
    while (true) {
        xSemaphoreTake(adcMutex, portMAX_DELAY);
         value1 = arduino::analogRead(A1);
         int adcValue= value1;
            adcValueReady = true;
        xSemaphoreGive(adcMutex);
        checks= Serial.readString();
      if(checks == "Axx1"){
        ::Serial.print(adcValue);
        ::Serial.print(" Voltage: ");
        ::Serial.println(adcValue * 3.3 / 4095, arduino::DEC);
      }
        ::vTaskDelay(pdMS_TO_TICKS(20));
    }
}

 void canint(const CAN_message_t &receivedMsg) {
//  can1.events();
            value3 = receivedMsg.id;
          if(receivedMsg.id == 0x1838F380){
            ::Serial.print("CAN ID:  ");
            ::Serial.print(receivedMsg.id,arduino:: HEX );
            ::Serial.print(" ");
//            dataTherm = recievedMs
            for(int k=0;k<8;k++){
             dataTherm[k] =  (int8_t)receivedMsg.buf[k];
              ::Serial.print(dataTherm[k]);
               ::Serial.print(" ");
              }
              ::Serial.println();
//            ::vTaskDelay(pdMS_TO_TICKS(30));
//        }
          }
}

FLASHMEM __attribute__((noinline)) void setup() {
    ::Serial.begin(115'200);
    ::pinMode(arduino::LED_BUILTIN, arduino::OUTPUT);
    ::digitalWriteFast(arduino::LED_BUILTIN, arduino::HIGH);
    ::pinMode(A1, arduino::INPUT);
    ::arduino::analogReadResolution(12);
    ::can1.begin();
     ::can1.enableFIFO();
     ::can1.enableFIFOInterrupt();
    ::can1.onReceive(canint);
    ::can1.setBaudRate(500000);
      
    adcMutex = xSemaphoreCreateMutex();
    canQueue = xQueueCreate(10, sizeof(CAN_message_t)); // Adjust the queue size as needed

    ::delay(5'000);

    if (CrashReport) {
        ::Serial.print(CrashReport);
        ::Serial.println();
        ::Serial.flush();
    }

    ::Serial.println(PSTR("\r\nBooting FreeRTOS kernel " tskKERNEL_VERSION_NUMBER ". Built by gcc " __VERSION__ " (newlib " _NEWLIB_VERSION ") on " __DATE__ ". ***\r\n"));

    ::xTaskCreate(task1, "task1", 512, nullptr, 3, nullptr); // Increase priority to 3
    ::xTaskCreate(task2, "task2", 512, nullptr, 2, nullptr);
//    ::xTaskCreate(task3, "task3", 512, nullptr, 1, nullptr); // Increase priority to 1

    ::Serial.println("setup(): starting scheduler...");
    ::Serial.flush();

    ::vTaskStartScheduler();
}

void loop() {
//  can1.events();
  }
//
//void FlexCAN_T4_userRxCallback(uint32_t id, uint8_t len, uint8_t* buf, uint8_t bus) {
//    receivedMsg.id = id;
//    receivedMsg.len = len;
//    memcpy(receivedMsg.buf, buf, len);
//    ::Serial.println("Can Recieved");
//    ::digitalWriteFast(arduino::LED_BUILTIN, arduino::LOW);
//    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//    xQueueSendFromISR(canQueue, &receivedMsg, &xHigherPriorityTaskWoken);
//
////    if (xHigherPriorityTaskWoken != pdFALSE) {
////        portYIELD_FROM_ISR();
////    }
//}
