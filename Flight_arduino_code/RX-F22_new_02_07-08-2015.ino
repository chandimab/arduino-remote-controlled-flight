/* Receiver ver1.0
Chandima B Samarasinghe
http://about.me/chandima.b.samarasinghe
working// led for A4 attached
*/
#include <SPI.h> //to handle the communication interface with the modem
#include "nRF24L01.h" //to handle this particular modem driver
#include "RF24.h" //the library which helps us to control the radio modem
#include <Servo.h>


/* RF */
const uint64_t addresses[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
RF24 radio(9,10); // signals CE_PIN,CSE_PIN

int TXDATA[4] = {1,700,1490,1490}; //codex,esc,ML,MR

/* controllers */
Servo serEsc,serML,serMR;

void setup() {
  Serial.begin(9600);
  Serial.println("PLANE-F22_Raptor Powered On");
  
  radio.begin(); //activate the modem
  radio.setDataRate(RF24_2MBPS);
  radio.openWritingPipe(addresses[0]); //sets the address of the receiver to which the program will send data.
  radio.openReadingPipe(1,addresses[1]); //channel(0-upto7), this device?
  
  radio.startListening(); // switch the modem to data receiving mode
  
  /* controllers*/
  serEsc.attach(5);
  serML.attach(14);
  serMR.attach(15);
  
  /* esc calibration */
 calibEsc(true); //auto calibration

}

void loop() {
  if(radio.available()){
    radio.read(&TXDATA, sizeof(TXDATA));
    
    if(TXDATA[0]=1993){ //noiseDetection CODE; geniune
      serEsc.writeMicroseconds(TXDATA[1]);
      serML.writeMicroseconds(TXDATA[2]);
      serMR.writeMicroseconds(TXDATA[3]);
    }else{ //noiseAvailable
        //writing previous value to controllers
      serEsc.writeMicroseconds(TXDATA[1]);
      serML.writeMicroseconds(TXDATA[2]);
      serMR.writeMicroseconds(TXDATA[3]);
    }  
  }else{
    Serial.println("NO-SIGNAL");
     //writing default values to controllers
        serEsc.writeMicroseconds(700);
        serML.writeMicroseconds(1490);
        serMR.writeMicroseconds(1490);
  }
 
}


/*custom functions */
void calibEsc(bool isAuto){
  if(isAuto){
    int calibSignal;
    //here is esc calibration part
    Serial.print("Esc auto calibration started");
    Serial.println("Sending MAX Signal");
    serEsc.writeMicroseconds(2000);
    //delay(2000);
    //waiting for radio
    while(!radio.available()); //stop here until radio available
    radio.read(&calibSignal,sizeof(calibSignal)); //read radio, once available
    while(!(calibSignal=700)){}; //until caibMin signal recieves wait here */
    Serial.println("Sending MIN Signal");
    serEsc.writeMicroseconds(700);
    delay(2000);
    Serial.println("Calibration Complete");
  }else{
    //manual calibration process 
  }
}
