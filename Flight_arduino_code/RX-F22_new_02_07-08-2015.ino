/* Receiver ver1.0
Chandima B Samarasinghe
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
  /*
  radio.setChannel(1);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  radio.setAutoAck(1);                     // Ensure autoACK is enabled
  radio.setRetries(2,15);                  // Optionally, increase the delay between retries & # of retries
                                            It shows how many times the modem will retry to the send data in case of not receiving by another modem.
                                            The first argument sets how often modem will retry. It is a multiple of 250 microseconds. 15 * 250 = 3750. So, if the recipient does not receive data, 
                                            modem will try to send them every 3.75 milliseconds. Second argument is the number of attempts. So in our example,modem will try to send 15 times before
                                            it will stop and finds that the receiver is out of range, or is turned off.
  radio.setCRCLength(RF24_CRC_8);          // Use 8-bit CRC for performance
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  
  radio.startListening(); //OR stopListening
  */
  radio.openWritingPipe(addresses[0]); //sets the address of the receiver to which the program will send data.
  radio.openReadingPipe(1,addresses[1]); //channel(0-upto7), this device?
  
  radio.startListening(); // switch the modem to data receiving mode
  
  /* controllers*/
  serEsc.attach(5);
  serML.attach(14);
  serMR.attach(15);

    
  /* esc calibration */
 calibEsc(true); //auto calibration
  
  /*led */
//  analogWrite(18,128); //A4  
}

void loop() {
  if(radio.available()){
    radio.read(&TXDATA, sizeof(TXDATA));
    
    if(TXDATA[0]=1993){ //noiseDetection CODE; geniune
    /*  Serial.print(TXDATA[1]);
          Serial.print(" ");
        Serial.print(TXDATA[2]);
          Serial.print(" ");
        Serial.println(TXDATA[3]);/*
          Serial.print(" ");
        Serial.print(TXDATA[tw]);
          Serial.print(" ");
        Serial.println(TXDATA[uw]);
        Serial.println(""); */
      //Serial.println("genuine recieved");
      serEsc.writeMicroseconds(TXDATA[1]);
      serML.writeMicroseconds(TXDATA[2]);
      serMR.writeMicroseconds(TXDATA[3]);
    }else{ //noiseAvailable
        //writing previous value to controllers
       // Serial.println("noise recieved");
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
