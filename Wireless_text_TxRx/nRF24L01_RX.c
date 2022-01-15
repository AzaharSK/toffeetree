
#include <SPI.h>
#include <nRF24L01p.h>

nRF24L01p receiver(7,8);   //CSN,CE

void setup(){
  delay(150);
  Serial.begin(115200);
  SPI.begin();
  //SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.setBitOrder(MSBFIRST);
  receiver.channel(90);
  receiver.RXaddress("Artur");
  receiver.init();
}

String message;
unsigned long myLong,i,key=12321;

void loop(){ 
  if(receiver.available()){
    receiver.read();
    receiver.rxPL(message);
    myLong=message.toInt();
    
   
  
    myLong^=key;
    Serial.println(myLong);
   
  
   
    message="";
  }
}
