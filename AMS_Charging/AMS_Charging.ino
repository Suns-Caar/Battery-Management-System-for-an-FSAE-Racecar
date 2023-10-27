#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsgs;
struct can_frame canMsgr;
MCP2515 mcp2515(10);
MCP2515::ERROR t;
 uint8_t a;

void setup()
{
  while (!Serial);
  Serial.begin(115200);
  SPI.begin();
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_1000KBPS);
  mcp2515.setNormalMode();  


  canMsgs.can_id  = 0x2FF; //767
  canMsgs.can_dlc = 7; //7
  canMsgs.data[0] = 0x1; //1
  canMsgs.data[1] = 0xE8;//232
  canMsgs.data[2] = 0x3; //3
  canMsgs.data[3] = 0xAC; //172
  canMsgs.data[4] = 0xD; //13
  canMsgs.data[5] = 0x1E; //30
  canMsgs.data[6] = 0x0; //0
  //canMsgs.data[7] = 0x0;

  
}

void loop()
{

 t = mcp2515.sendMessage(&canMsgs); 
 Serial.println(t);
  
int currentmillis=millis();
while((millis()-currentmillis)<990)
{

  if (mcp2515.readMessage(&canMsgr) == MCP2515::ERROR_OK)
  {
    Serial.print(canMsgr.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canMsgr.can_dlc, HEX); // print DLC
    Serial.print(" ");
    
    for (int i = 0; i<canMsgr.can_dlc; i++) 
    {  // print the data
        
      Serial.print(canMsgr.data[i],HEX);
      Serial.print(" ");

    }

    Serial.println();      
  } 
}
a=mcp2515.readRegister(0x2D);
if((a==64)||(a==128))
{
mcp2515.clearInterrupts();
//mcp2515.reset();
}
Serial.println(a); 

}
