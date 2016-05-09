/* Transmitter ver1.0
Chandima B Samarasinghe
fine calibrated , main wings angle increased , upperwing angle increased
4 potentometers included
  LeftUp(brown)18             RightUp(orange)20
  LeftBottom(purple)19        RightBottom(blue)21
2 toggle switches
  rUp(yellow)2             lUp(blue)11
  rBottom(green)4           lBottom(purple)12

  
successful code


- modification 1 ,( no variables, functions) ,2MBPS 
analog 14(a0) 15 16 17 18 19(a5)
nano pwm 3,5,6,9,10,11 / normal digital 0,1,2,4,7,8,12,13
*/
#include <SPI.h> //to handle the communication interface with the modem
#include "nRF24L01.h" //to handle this particular modem driver
#include "RF24.h" //the library which helps us to control the radio modem

/* inputs */
#define JOYSTICK_R_X 16 //right-left
#define JOYSTICK_R_Y 17 //elivator
#define JOYSTICK_R_K 7

#define JOYSTICK_L_X 14 //A0=14 no-function
#define JOYSTICK_L_Y 15 //esc
#define JOYSITCK_L_K 8

#define esc 0
#define ml 1
#define mr 2

#define codex 1993 //noiseDetectionCode

int neutJoyRX,neutJoyRY,neutJoyLX,neutJoyLY;
int valJoyRX,valJoyRY,valJoyLX,valJoyLY; //to hold joystick values

/* 880~1490~2100
       610  610

*/
int elivatorR=1490;
int elivatorL=1490;
#define p (30*10) // 5 degree  (1 degree angle==> 10 points)

#define minElivator (880+p)
#define maxElivator (2100-p)

const uint64_t addresses[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
RF24 radio(9,10); // signals CE_PIN 9,CSE_PIN10

int TXDATA[4] = {codex,700,1490,1490}; //codex,esc,ML,MR
//TXDATA[5]=codex;

void setup() {

  pinMode(2 ,INPUT);
  pinMode(11 ,INPUT);
  pinMode(4 ,INPUT);
  pinMode(12 ,INPUT);
 /* pinMode( ,INPUT);
  pinMode( ,INPUT);
  pinMode( ,INPUT);
  pinMode( ,INPUT); */

  
  Serial.begin(9600);
  Serial.println("Transmitter Powered On");
  
  radio.begin(); //activate the modem
  radio.setDataRate(RF24_2MBPS);
  radio.setRetries(0,0);
  
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
  radio.openWritingPipe(addresses[1]); //sets the address of the receiver to which the program will send data.
  radio.openReadingPipe(1,addresses[0]); //channel(0-upto7), this device?
  
  radio.stopListening(); // switch the modem to data transmission mode
  
  /*getting neutral positions */
  Serial.println("Neutral-Mode Calibration : Start");
    calibInitial();
  Serial.println("Neutral-Mode Calibration : Complete");
  
  /*sending min pwm signal to calibrate esc */
  Serial.println("waiting for esc min");
  while(!(analogRead(JOYSTICK_L_Y)<100)); //wait until JoyLY---> reach down to send min PWM signal to esc
   calibEsc(true);
  Serial.println("MIN esc sent");
  
}

void loop() {
 //read joysticks and assign appropriate data to transmit
    //getting row values
  valJoyRX=1023-analogRead(16);
  valJoyRY=1023-analogRead(17);
  //valJoyLX=analogRead(JOYSTICK_L_X);
  valJoyLY=analogRead(15);
  
  //calculations & TXDATA SETUP
  /* ESC */
  if(valJoyLY>=neutJoyLY){
    TXDATA[1]=map(valJoyLY,neutJoyLY,1023,700,2000); // pwm esc, for joystick upper-half
  }

  /*  30deg 90 150
   *  880~1490~2100
       310  310

*/

  /* Elivator */ 
  if(valJoyRY>=neutJoyRY){ //upper positions
    elivatorR=map(valJoyRY,neutJoyRY,1023,1490,maxElivator); // pwm servo, for joystick upper-half 
    elivatorL=map(valJoyRY,neutJoyRY,1023,1490,minElivator);
   }else{
    elivatorR=map(valJoyRY,neutJoyRY,0,1490,minElivator); //pwm servo, bottom-half 
    elivatorL=map(valJoyRY,neutJoyRY,0,1490,maxElivator);
   }
  
  /* rotation */ 
  if(valJoyRX>=neutJoyRX){ //right-side
    TXDATA[2]=elivatorR + map(valJoyRX,neutJoyRX,1023,0,p); // pwm servo, for joystick right-half  :mr fine working
    TXDATA[3]=elivatorL+ map(valJoyRX,neutJoyRX,1023,0,p); // pwm servo, for joystick right-half : ml
    /* two servos are attached to plane invertially. so no need to invert the code */
   }else{
    TXDATA[2]=elivatorR - map(valJoyRX,neutJoyRX,0,0,p); // pwm servo, for joystick left-half  : mr fine working
    TXDATA[3]=elivatorL - map(valJoyRX,neutJoyRX,0,0,p); // pwm servo, for joystick left-half 
  }
 // Serial.println(" esc   ml    mr   tw   uw");

  /*
  Serial.print(TXDATA[0]);
    Serial.print(" ");
  Serial.print(TXDATA[1]);
    Serial.print(" ");
  Serial.print(TXDATA[2]);
    Serial.print(" ");
  Serial.println(TXDATA[3]);
 */
   
  radio.write(&TXDATA,sizeof(TXDATA));
  //Serial.println("sent");
 //delay(100);
}


/* custom functions */
void calibInitial(){
  neutJoyRX=1023-analogRead(JOYSTICK_R_X);
  neutJoyRY=1023-analogRead(JOYSTICK_R_Y);
  neutJoyLX=analogRead(JOYSTICK_L_X);
  neutJoyLY=analogRead(JOYSTICK_L_Y);
}

void calibEsc(bool isAuto){
  int calibSignal;
  if(isAuto){
    //auto calibration process.
    Serial.println("Sending MIN Signal");
      calibSignal=700;
      radio.write(&calibSignal,sizeof(calibSignal));
      delay(2000);
    Serial.println("Calibration Complete");
  }else{
   //manual calibration process. 
  }
}

/* read potentometers 

4 potentometers included
  LeftUp(brown)18            RightUp(orange)20
  LeftBottom(purple)19        RightBottom(blue)21 */
void readPotentometers(){
  
}

