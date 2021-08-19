/*
 Arduino Mysensors Smart Keypad and Password Acess Control

 August 2, 2021

 Version 1.1
 Arduino Mysensors Smart Keypad and Password Acess Control


 This sketch features the following:
 * Signing and  maybe encryption needs enabled before build.
 * Passwords and User access must be set through the controller or serial monitor for keypad to be usable.
 * 6 different passwords stored in the eeprom and load on power loss
 * 6 user Passwords can be changed via the controller or mysensors gateway serial monitor
 * Passwords must be 6 digits long  8; 161 or 167 or 173 or 179 or 185 or 192;1;0;47;------to load passwords
 * 6 User names can be stored in the eeprom and are set from the controller or mysensors gateway serial monitir
 * User names max 8 characters   8;207 or 215 or 223 or 231 or 239 or 247;1;0;47;-------- to load user name
 * Access control for 20 different zones or other smart things attatched to your controller
 * Access to the 20 zones  can be turned off/on for each of the 6 users from  the controller of serial monitor
 * Access permission is stored in the eeprom and load on reboot
 * To set each access permission 8;41 to 160;1;0;2;0 or 1 to turn a zone on or off, see defines for zone numbers. u1e1 is user 1 zone/equipment 1,  u3e9 is user 3 equipment/zone 9 etc
 * Displays a greeting message on 16x2 lcd--This will be saved in eeprom and load on reboot, after setting from controller 
 * Needs to run on an Arduino Mega 2560
 * Has a motion sensor SR505 currently but looking to get adjustable SR602
 * Has a door open/close sensor
 *The keypad I have used also has a back light
 *Password will automatically reset after 12 seconds, prevent half a correct password remianing in the buffer m=by mistake
 *Zone names can not be loaded from the controller but can be types manaully in the sketch before uploading


 -Pins doorsnensor 23,GND
 -Pins motion sensor 23,GND,5V
 -Keypad rowpins top to bottom 3,4,5,6 column pins left to right 7,8,9
 -LCD 16x2 I2C SDA20,SCL21,GND,5V
 -NRF24L01 
  CE_PIN 49
  CS_PIN 53
  MISO 50
  MOSI  51
  SCK  52
  IRQ 19 
  GND
  3.3V or I used the 5V power adapter thingy
 by Joshua Fitzgerald
 
 Change Log
 Developed whilst in covid hotel isolation Perth WA Jul 2021
 Version 1.0 Release 2 Aug 2021
 1.1  4/8/2021 fix double ups in child zone names
 1.15 8/8/2021 fix update names function to work with adjusted child ids for zone names, all included my buffer feature and IRQ pin is 19

 Done but not tested
 - Aug 2021 addedEEPROM_LOAD so it can be test, plug and play. comment out to use hard coddded password, username and zone activations

 To Do
 - Allow greeting message to be stored in eeprom and load from controller also
 -  add signing and security
 - power off keyboard and lcd back light, wake on motion


 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows
#define LCD_ON  // message to i2c lcd for debug or display?
#define EEPROM_LOAD // comment this out to use the coded passwords,usernames and set the zone active statements.
//#define DEBUG_ON // my own serial messages for debug
#define MY_DEBUG
#define MY_RF24_IRQ_PIN 19
#define MY_RX_MESSAGE_BUFFER_FEATURE 
#define MY_NODE_ID 118 //uncomment this line to assign a static ID
#define MY_RF24_CE_PIN 49
#define MY_RF24_CS_PIN 53
// Enable and select radio type attached
#define MY_RADIO_RF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95
#include <String.h>
#include <MySensors.h>
// password correct message to gateway/controller entry extension equipment child ids
#define CHILD_ID_E1 1   // Id of the sensor child
#define CHILD_ID_E2 2
#define CHILD_ID_E3 3
#define CHILD_ID_E4 4
#define CHILD_ID_E5 5
#define CHILD_ID_E6 6
#define CHILD_ID_E7 7
#define CHILD_ID_E8 8
#define CHILD_ID_E9 9
#define CHILD_ID_E10 10
#define CHILD_ID_E11 11
#define CHILD_ID_E12 12
#define CHILD_ID_E13 13
#define CHILD_ID_E14 14
#define CHILD_ID_E15 15
#define CHILD_ID_E16 16
#define CHILD_ID_E17 17
#define CHILD_ID_E18 18
#define CHILD_ID_E19 19
#define CHILD_ID_E20 20
// cild id for door open/close state and motion sensor
#define CHILD_ID_DOORSTATE 23
#define CHILD_ID_DOORMOTION 22
// mcd display greeting message , not used currently
//#define CHILD_ID_MINFO1 31  
//#define CHILD_ID_MINFO2 32
// child is for recieving password updates
// do not chnage these numbers, they are used in message.receive to save in the eeprom address 6 bytes/character password
#define CHILD_ID_PW1 161
#define CHILD_ID_PW2 167
#define CHILD_ID_PW3 173
#define CHILD_ID_PW4 179    ////////   eeprom addresses used 161 - 197 ///////////////////////////// 
#define CHILD_ID_PW5 185
#define CHILD_ID_PW6 192
//Child id for user names, also first eeprom address for storage of names 8 bytes/characters each name 
#define CHILD_ID_N1 207
#define CHILD_ID_N2 215
#define CHILD_ID_N3 223     ////////////  eeprom addresses  207 - 254   ////////////////////////////////////
#define CHILD_ID_N4 231
#define CHILD_ID_N5 239
#define CHILD_ID_N6 247
// child id for receiving user zone active permissions these id number also coresponde to the eeprom memory location in eeprom for the bool data
// user 1 equipment 1-20 permissions
#define CHILD_ID_U1E1 41
#define CHILD_ID_U1E2 42
#define CHILD_ID_U1E3 43
#define CHILD_ID_U1E4 44
#define CHILD_ID_U1E5 45
#define CHILD_ID_U1E6 46
#define CHILD_ID_U1E7 47
#define CHILD_ID_U1E8 48
#define CHILD_ID_U1E9 49
#define CHILD_ID_U1E10 50
#define CHILD_ID_U1E11 51
#define CHILD_ID_U1E12 52
#define CHILD_ID_U1E13 53
#define CHILD_ID_U1E14 54
#define CHILD_ID_U1E15 55
#define CHILD_ID_U1E16 56
#define CHILD_ID_U1E17 57
#define CHILD_ID_U1E18 58
#define CHILD_ID_U1E19 59
#define CHILD_ID_U1E20 60
// user 2 equipment 1-20 permissions
#define CHILD_ID_U2E1 61
#define CHILD_ID_U2E2 62
#define CHILD_ID_U2E3 63
#define CHILD_ID_U2E4 64
#define CHILD_ID_U2E5 65
#define CHILD_ID_U2E6 66
#define CHILD_ID_U2E7 67
#define CHILD_ID_U2E8 68
#define CHILD_ID_U2E9 69
#define CHILD_ID_U2E10 70
#define CHILD_ID_U2E11 71
#define CHILD_ID_U2E12 72
#define CHILD_ID_U2E13 73
#define CHILD_ID_U2E14 74
#define CHILD_ID_U2E15 75
#define CHILD_ID_U2E16 76
#define CHILD_ID_U2E17 77
#define CHILD_ID_U2E18 78
#define CHILD_ID_U2E19 79
#define CHILD_ID_U2E20 80
// user 3 equipment 1-20 permissions
#define CHILD_ID_U3E1 81
#define CHILD_ID_U3E2 82
#define CHILD_ID_U3E3 83
#define CHILD_ID_U3E4 84
#define CHILD_ID_U3E5 85
#define CHILD_ID_U3E6 86
#define CHILD_ID_U3E7 87
#define CHILD_ID_U3E8 88
#define CHILD_ID_U3E9 89
#define CHILD_ID_U3E10 90
#define CHILD_ID_U3E11 91
#define CHILD_ID_U3E12 92
#define CHILD_ID_U3E13 93
#define CHILD_ID_U3E14 94
#define CHILD_ID_U3E15 95
#define CHILD_ID_U3E16 96
#define CHILD_ID_U3E17 97
#define CHILD_ID_U3E18 98
#define CHILD_ID_U3E19 99
#define CHILD_ID_U3E20 100
// user 4 equipment 1-20 permissions
#define CHILD_ID_U4E1 101
#define CHILD_ID_U4E2 102
#define CHILD_ID_U4E3 103
#define CHILD_ID_U4E4 104
#define CHILD_ID_U4E5 105
#define CHILD_ID_U4E6 106
#define CHILD_ID_U4E7 107
#define CHILD_ID_U4E8 108
#define CHILD_ID_U4E9 109
#define CHILD_ID_U4E10 110
#define CHILD_ID_U4E11 111
#define CHILD_ID_U4E12 112
#define CHILD_ID_U4E13 113
#define CHILD_ID_U4E14 114
#define CHILD_ID_U4E15 115
#define CHILD_ID_U4E16 116
#define CHILD_ID_U4E17 117
#define CHILD_ID_U4E18 118
#define CHILD_ID_U4E19 119
#define CHILD_ID_U4E20 120
// user 5 equipment 1-20 permissions
#define CHILD_ID_U5E1 121
#define CHILD_ID_U5E2 122
#define CHILD_ID_U5E3 123
#define CHILD_ID_U5E4 124
#define CHILD_ID_U5E5 125
#define CHILD_ID_U5E6 126
#define CHILD_ID_U5E7 127
#define CHILD_ID_U5E8 128
#define CHILD_ID_U5E9 129
#define CHILD_ID_U5E10 130
#define CHILD_ID_U5E11 131
#define CHILD_ID_U5E12 132
#define CHILD_ID_U5E13 133
#define CHILD_ID_U5E14 134
#define CHILD_ID_U5E15 135
#define CHILD_ID_U5E16 136
#define CHILD_ID_U5E17 137
#define CHILD_ID_U5E18 138
#define CHILD_ID_U5E19 139
#define CHILD_ID_U5E20 140
// user 6 equipment 1-20 permissions
#define CHILD_ID_U6E1 141
#define CHILD_ID_U6E2 142
#define CHILD_ID_U6E3 143
#define CHILD_ID_U6E4 144
#define CHILD_ID_U6E5 145
#define CHILD_ID_U6E6 146
#define CHILD_ID_U6E7 147
#define CHILD_ID_U6E8 148
#define CHILD_ID_U6E9 149
#define CHILD_ID_U6E10 150
#define CHILD_ID_U6E11 151
#define CHILD_ID_U6E12 152
#define CHILD_ID_U6E13 153
#define CHILD_ID_U6E14 154
#define CHILD_ID_U6E15 155
#define CHILD_ID_U6E16 156
#define CHILD_ID_U6E17 157
#define CHILD_ID_U6E18 158
#define CHILD_ID_U6E19 159
#define CHILD_ID_U6E20 160
// child id for updating zone names 16 bytes/characters

/*

#define CHILD_ID_PW2 167
#define CHILD_ID_PW3 173
#define CHILD_ID_PW4 179    ////////   eeprom addresses used 161 - 197 ///////////////////////////// 
#define CHILD_ID_PW5 185
#define CHILD_ID_PW6 192

*/
#define CHILD_ID_ZN1 168 
#define CHILD_ID_ZN2 169
#define CHILD_ID_ZN3 170
#define CHILD_ID_ZN4 171
#define CHILD_ID_ZN5 172
#define CHILD_ID_ZN6 174
#define CHILD_ID_ZN7 175
#define CHILD_ID_ZN8 176
#define CHILD_ID_ZN9 177
#define CHILD_ID_ZN10 178
#define CHILD_ID_ZN11 180
#define CHILD_ID_ZN12 181
#define CHILD_ID_ZN13 182
#define CHILD_ID_ZN14 184
#define CHILD_ID_ZN15 186
#define CHILD_ID_ZN16 187
#define CHILD_ID_ZN17 188
#define CHILD_ID_ZN18 189
#define CHILD_ID_ZN19 190
#define CHILD_ID_ZN20 191
// Initialize motion message
MyMessage statemsg(CHILD_ID_DOORSTATE, V_STATUS); //single message type as they are all s_binary/v_status  type
MyMessage passwordmsg(CHILD_ID_U1E1, V_TRIPPED); //single message type as they are all s_door/v_tripped  type for correct password entry

#include <Password.h> //http://www.arduino.cc/playground/uploads/Code/Password.zip
#include <Keypad.h> //http://www.arduino.cc/playground/uploads/Code/Keypad.zip

// Greeting message for display on lcd
char greetingline1[17]="   Welcome to   ";  //16 bytes/characters
char greetingline2[17]="   The Chateau  "; //16 bytes/characters

//Various passwords, these passwords can  updated from the controller and are stored in the eeprom, these password are nut used, loads from eeprom on reboot
char userpw[7][7]={"0000000",  // 0 index not used
                   "0123456", // user 1 password 
                   "0234567",  // user 2 password 
                   "0345678", // user 3 password base
                   "0456789", // user 4 password base
                   "0567890", // user 5 password base
                   "0678901"}; // user 6 password base

// username can be update from the gateway serial monitor or controller
 char username[7][9]={"0000000",  // 0 index not used
                   "USER: 1", // user 1 password 
                   "USER: 2",  // user 2 password 
                   "USER: 3", // user 3 password base
                   "USER: 4", // user 4 password base
                   "USER: 5", // user 5 password base
                   "USER: 6"}; // user 6 password base

                   
 // initialise user zone activated perminssions, these need to be activated from the controller to use the zones. these are no longer relevent.
  bool pz[7][21]={
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},  //0 index not used
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  //user 1 zones
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // user 2 zones
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  //user 3 zones
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  //user 4 zones
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // user 5 zones
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},  // user 6 zones
  };  //permitted zones
  
//0 index not used
const char zones [21][3]={"000",  "001", "002","003","004","005", "006","007","008","009","010","011","012","013","014","015","016","017","018","019","020"}; 

char zonename [21][17]={"0000000","-----ZONE 1----", "-----ZONE 2----", "-----ZONE 3----", "-----ZONE 4----","-----ZONE 5----","-----ZONE 6----","-----ZONE 7----","-----ZONE 8----",
"-----ZONE 9----","-----ZONE 10---","-----ZONE 11---","-----ZONE 12---","-----ZONE 13---","-----ZONE 14---","-----ZONE 15---","-----ZONE 16---","-----ZONE 17---","-----ZONE 18---",
"-----ZONE 19---","-----ZONE 20---",};  // zones names 1 to 20 id number                         

char passzone[9];
//char* pointer=passzone;


   //char* join(int p,int e,char *poin ){ //concantante the user password with the equipment zone
     void join(int p,int e){passzone[0]=userpw[p][1];passzone[1]=userpw[p][2];passzone[2]=userpw[p][3];passzone[3]=userpw[p][4];passzone[4]=userpw[p][5];passzone[5]=userpw[p][6];
        passzone[6]=zones[e][1];passzone[7]=zones[e][2]; }  // return  poin;
     
// user 1 equipment 1-20 passwords
Password u1e1 = Password((passzone)); Password u1e2 = Password((passzone)); Password u1e3 = Password((passzone));Password u1e4 = Password((passzone));Password u1e5 = Password((passzone));
Password u1e6 = Password((passzone));Password u1e7 = Password((passzone));Password u1e8 = Password((passzone));Password u1e9 = Password((passzone));Password u1e10 = Password((passzone));
Password u1e11 = Password((passzone));Password u1e12 = Password((passzone));Password u1e13 = Password((passzone));Password u1e14 = Password((passzone));Password u1e15 = Password((passzone));
Password u1e16 = Password((passzone));Password u1e17 = Password((passzone));Password u1e18 = Password((passzone));Password u1e19 = Password((passzone));Password u1e20 = Password((passzone));
// User 2 equipment 1-20 passwords
Password u2e1 = Password((passzone));Password u2e2 = Password((passzone));Password u2e3 = Password((passzone));Password u2e4 = Password((passzone));Password u2e5 = Password((passzone));
Password u2e6 = Password((passzone));Password u2e7 = Password((passzone));Password u2e8 = Password((passzone));Password u2e9 = Password((passzone));Password u2e10 = Password((passzone));
Password u2e11 = Password((passzone));Password u2e12 = Password((passzone));Password u2e13 = Password((passzone));Password u2e14 = Password((passzone));Password u2e15 = Password((passzone));
Password u2e16 = Password((passzone));Password u2e17 = Password((passzone));Password u2e18 = Password((passzone));Password u2e19 = Password((passzone));Password u2e20 = Password((passzone));
// User 3 equipment 1-20 passwords
Password u3e1 = Password((passzone));Password u3e2 = Password((passzone));Password u3e3 = Password((passzone));Password u3e4 = Password((passzone));Password u3e5 = Password((passzone));
Password u3e6 = Password((passzone));Password u3e7 = Password((passzone));Password u3e8 = Password((passzone));Password u3e9 = Password((passzone));Password u3e10 = Password((passzone));
Password u3e11 = Password((passzone));Password u3e12 = Password((passzone));Password u3e13 = Password((passzone));Password u3e14 = Password((passzone));Password u3e15 = Password((passzone));
Password u3e16 = Password((passzone));Password u3e17 = Password((passzone));Password u3e18 = Password((passzone));Password u3e19 = Password((passzone));Password u3e20 = Password((passzone));
// User 4 equipment 1-20 passwords
Password u4e1 = Password((passzone));Password u4e2 = Password((passzone));Password u4e3 = Password((passzone));Password u4e4 = Password((passzone));Password u4e5 = Password((passzone));
Password u4e6 = Password((passzone));Password u4e7 = Password((passzone));Password u4e8 = Password((passzone));Password u4e9 = Password((passzone));Password u4e10 = Password((passzone));
Password u4e11 = Password((passzone));Password u4e12 = Password((passzone));Password u4e13 = Password((passzone));Password u4e14 = Password((passzone));Password u4e15 = Password((passzone));
Password u4e16 = Password((passzone));Password u4e17 = Password((passzone));Password u4e18 = Password((passzone));Password u4e19 = Password((passzone));Password u4e20 = Password((passzone));
// User 5 equipment 1-20 passwords
Password u5e1 = Password((passzone));Password u5e2 = Password((passzone));Password u5e3 = Password((passzone));Password u5e4 = Password((passzone));Password u5e5 = Password((passzone));
Password u5e6 = Password((passzone));Password u5e7 = Password((passzone));Password u5e8 = Password((passzone));Password u5e9 = Password((passzone));Password u5e10 = Password((passzone));
Password u5e11 = Password((passzone));Password u5e12 = Password((passzone));Password u5e13 = Password((passzone));Password u5e14 = Password((passzone));Password u5e15 = Password((passzone));
Password u5e16 = Password((passzone));Password u5e17 = Password((passzone));Password u5e18 = Password((passzone));Password u5e19 = Password((passzone));Password u5e20 = Password((passzone));
// User 6 equipment 1-20 passwords
Password u6e1 = Password((passzone));Password u6e2 = Password((passzone));Password u6e3 = Password((passzone));Password u6e4 = Password((passzone));Password u6e5 = Password((passzone));
Password u6e6 = Password((passzone));Password u6e7 = Password((passzone));Password u6e8 = Password((passzone));Password u6e9 = Password((passzone));Password u6e10 = Password((passzone));
Password u6e11 = Password((passzone));Password u6e12 = Password((passzone));Password u6e13 = Password((passzone));Password u6e14 = Password((passzone));Password u6e15 = Password((passzone));
Password u6e16 = Password((passzone));Password u6e17 = Password((passzone));Password u6e18 = Password((passzone));Password u6e19 = Password((passzone));Password u6e20 = Password((passzone));

const byte ROWS = 4; // 4 rows
const byte COLS = 3; // 3 columns
// Define the Keymap
char keys[ROWS][COLS] = {
{'1','2','3'},{'4','5','6'},{'7','8','9'},{'*','0','#'}};
//keypad pin connections below, also a ground and a 3.3v for backlight led
byte rowPins[ROWS] = { 3,4,5,6};   // pin connection for vertical keys top to bottom
byte colPins[COLS] = { 7,8,9
};  // pin connection for keys keys left to right

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// some function definitions from the end of page
 void updatepw(int,char*);
 void updatezone(int ,bool);
 void updatename(int ,char*);
 void updatezn(int ,char*);
 void loadza();
 void loadpw();
 void loadun();
 void resetpassword(); 
 void appendpassword();

// for motion report time interval
const unsigned long reportinterval= 30000;   // SR505 motion sensors resets after 8sec +-30%  5.6 - 11.4sec
      unsigned long currentmilli;
      unsigned long previousmilli;
      unsigned long lcdonmilli;
const unsigned long lcdoffinterval=30000;
 #define MOTION_SENSOR_INPUT 22 // pin connection for motion sensor
  bool tripped=0;
   bool lastmotionsent=1;
   bool motionsent=0;

 // for door open/closed sensor
  #include <Bounce2.h>
  Bounce debouncer = Bounce(); 
 int oldValue=-1;
#define DOOR_SENSOR_PIN  23 
//already previously defined CHILD_ID 23 for door sensor

// keypad timeout variables....... to clear password buffer if remains incomplete
int keypressedcount=0;
unsigned long sincekeypressed;
unsigned long nowmilli;
unsigned long resetinterval=12000;  // if a key is pressed, the password reset function will run after 12seconds



void presentation()
{
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("keypad,mulitple passwords", "1.0");

  // Register all sensors to gw (they will be created as child devices)
 //keypad extensions
 present(CHILD_ID_E1, S_DOOR);present( CHILD_ID_E2, S_DOOR);present(CHILD_ID_E3, S_DOOR);present(CHILD_ID_E4, S_DOOR);present(CHILD_ID_E5, S_DOOR);present(CHILD_ID_E6, S_DOOR);
 present(CHILD_ID_E7, S_DOOR);present(CHILD_ID_E8, S_DOOR);present(CHILD_ID_E8, S_DOOR);present(CHILD_ID_E10, S_DOOR);present(CHILD_ID_E11, S_DOOR);present(CHILD_ID_E12, S_DOOR);
 present(CHILD_ID_E13, S_DOOR);present(CHILD_ID_E14, S_DOOR);present(CHILD_ID_E15, S_DOOR);present(CHILD_ID_E15, S_DOOR);present(CHILD_ID_E17, S_DOOR);present(CHILD_ID_E18, S_DOOR);
 present(CHILD_ID_E19, S_DOOR);present(CHILD_ID_E20, S_DOOR);
// door open/closed and motion sensors
 present(CHILD_ID_DOORSTATE, S_BINARY);present(CHILD_ID_DOORMOTION, S_BINARY);// doormotion was s_door
 //present(CHILD_ID_MINFO1, S_INFO);present(CHILD_ID_MINFO2, S_INFO); //not used currently for greeting message
 // to receive the PASSWORD updates   
present(CHILD_ID_PW1, S_INFO);present(CHILD_ID_PW2, S_INFO);present(CHILD_ID_PW3, S_INFO);present(CHILD_ID_PW4, S_INFO);present(CHILD_ID_PW5, S_INFO);present(CHILD_ID_PW6, S_INFO);
// User name child to allow update of user names from gateway/controller
present(CHILD_ID_N1, S_INFO);present(CHILD_ID_N2, S_INFO);present(CHILD_ID_N3, S_INFO);present(CHILD_ID_N4, S_INFO);present(CHILD_ID_N5, S_INFO);present(CHILD_ID_N6, S_INFO);

present( CHILD_ID_U1E1 , S_BINARY);present( CHILD_ID_U1E2 ,  S_BINARY);present( CHILD_ID_U1E3 , S_BINARY);present( CHILD_ID_U1E4 , S_BINARY);present( CHILD_ID_U1E5 , S_BINARY);
present( CHILD_ID_U1E6 , S_BINARY);present( CHILD_ID_U1E7 , S_BINARY);present( CHILD_ID_U1E8 , S_BINARY);present( CHILD_ID_U1E9 , S_BINARY);present( CHILD_ID_U1E10 , S_BINARY);
present( CHILD_ID_U1E11 , S_BINARY);present( CHILD_ID_U1E12 , S_BINARY);present( CHILD_ID_U1E13 , S_BINARY);present( CHILD_ID_U1E14 , S_BINARY);present( CHILD_ID_U1E15 , S_BINARY);
present( CHILD_ID_U1E16 , S_BINARY);present( CHILD_ID_U1E17 , S_BINARY);present( CHILD_ID_U1E18 , S_BINARY);present( CHILD_ID_U1E19 , S_BINARY);present( CHILD_ID_U1E20 , S_BINARY);
// user 2 equipment 1-20 permissions
present( CHILD_ID_U2E1 , S_BINARY);present( CHILD_ID_U2E2 , S_BINARY);present( CHILD_ID_U2E3 , S_BINARY);present( CHILD_ID_U2E4 , S_BINARY);present( CHILD_ID_U2E5 , S_BINARY);
present( CHILD_ID_U2E6 , S_BINARY);present( CHILD_ID_U2E7 , S_BINARY);present( CHILD_ID_U2E8 , S_BINARY);present( CHILD_ID_U2E9 , S_BINARY);present( CHILD_ID_U2E10 , S_BINARY);
present( CHILD_ID_U2E11 , S_BINARY);present( CHILD_ID_U2E12 , S_BINARY);present( CHILD_ID_U2E13 , S_BINARY);present( CHILD_ID_U2E14 , S_BINARY);present( CHILD_ID_U2E15 , S_BINARY);
present( CHILD_ID_U2E16 , S_BINARY);present( CHILD_ID_U2E17 , S_BINARY);present( CHILD_ID_U2E18 , S_BINARY);present( CHILD_ID_U2E19 , S_BINARY);present( CHILD_ID_U2E20 , S_BINARY);
// user 3 equipment 1-20 permissions
present( CHILD_ID_U3E1 , S_BINARY);present( CHILD_ID_U3E2 , S_BINARY);present( CHILD_ID_U3E3 , S_BINARY);present( CHILD_ID_U3E4 , S_BINARY);present( CHILD_ID_U3E5 , S_BINARY);
present( CHILD_ID_U3E6 , S_BINARY);present( CHILD_ID_U3E7 , S_BINARY);present( CHILD_ID_U3E8 , S_BINARY);present( CHILD_ID_U3E9 , S_BINARY);present( CHILD_ID_U3E10 , S_BINARY);
present( CHILD_ID_U3E11 , S_BINARY);present( CHILD_ID_U3E12 , S_BINARY);present( CHILD_ID_U3E13 , S_BINARY);present( CHILD_ID_U3E14 , S_BINARY);present( CHILD_ID_U3E15 , S_BINARY);
present( CHILD_ID_U3E16 , S_BINARY);present( CHILD_ID_U3E17 , S_BINARY);present( CHILD_ID_U3E18 , S_BINARY);present( CHILD_ID_U3E19 , S_BINARY);present( CHILD_ID_U3E20 , S_BINARY);
// user 4 equipment 1-20 permissions
present( CHILD_ID_U4E1 , S_BINARY);present( CHILD_ID_U4E2 , S_BINARY);present( CHILD_ID_U4E3 , S_BINARY);present( CHILD_ID_U4E4 , S_BINARY);present( CHILD_ID_U4E5 , S_BINARY);
present( CHILD_ID_U4E6 , S_BINARY);present( CHILD_ID_U4E7 , S_BINARY);present( CHILD_ID_U4E8 , S_BINARY);present( CHILD_ID_U4E9 , S_BINARY);present( CHILD_ID_U4E10 , S_BINARY);
present( CHILD_ID_U4E11 , S_BINARY);present( CHILD_ID_U4E12 , S_BINARY);present( CHILD_ID_U4E13 , S_BINARY);present( CHILD_ID_U4E14 , S_BINARY);present( CHILD_ID_U4E15 , S_BINARY);
present( CHILD_ID_U4E16 , S_BINARY);present( CHILD_ID_U4E17 , S_BINARY);present( CHILD_ID_U4E18 , S_BINARY);present( CHILD_ID_U4E19 , S_BINARY);present( CHILD_ID_U4E20 , S_BINARY);
// user 5 equipment 1-20 permissions
present( CHILD_ID_U5E1 , S_BINARY);present( CHILD_ID_U5E2 , S_BINARY);present( CHILD_ID_U5E3 , S_BINARY);present( CHILD_ID_U5E4 , S_BINARY);present( CHILD_ID_U5E5 , S_BINARY);
present( CHILD_ID_U5E6 , S_BINARY);present( CHILD_ID_U5E7 , S_BINARY);present( CHILD_ID_U5E8 , S_BINARY);present( CHILD_ID_U5E9 , S_BINARY);present( CHILD_ID_U5E10 , S_BINARY);
present( CHILD_ID_U5E11 , S_BINARY);present( CHILD_ID_U5E12 , S_BINARY);present( CHILD_ID_U5E13 , S_BINARY);present( CHILD_ID_U5E14 , S_BINARY);present( CHILD_ID_U5E15 , S_BINARY);
present( CHILD_ID_U5E16 , S_BINARY);present( CHILD_ID_U5E17 , S_BINARY);present( CHILD_ID_U5E18 , S_BINARY);present( CHILD_ID_U5E19 , S_BINARY);present( CHILD_ID_U5E20 , S_BINARY);
// user 6 equipment 1-20 permissions
present( CHILD_ID_U6E1 , S_BINARY);present( CHILD_ID_U6E2 , S_BINARY);present( CHILD_ID_U6E3 , S_BINARY);present( CHILD_ID_U6E4 , S_BINARY);present( CHILD_ID_U6E5 , S_BINARY);
present( CHILD_ID_U6E6 , S_BINARY);present( CHILD_ID_U6E7 , S_BINARY);present( CHILD_ID_U6E8 , S_BINARY);present( CHILD_ID_U6E9 , S_BINARY);present( CHILD_ID_U6E10 , S_BINARY);
present( CHILD_ID_U6E11 , S_BINARY);present( CHILD_ID_U6E12 , S_BINARY);present( CHILD_ID_U6E13 , S_BINARY);present( CHILD_ID_U6E14 , S_BINARY);present( CHILD_ID_U6E15 , S_BINARY);
present( CHILD_ID_U6E16 , S_BINARY);present( CHILD_ID_U6E17 , S_BINARY);present( CHILD_ID_U6E18 , S_BINARY);present( CHILD_ID_U6E19 , S_BINARY);present( CHILD_ID_U6E20 , S_BINARY);
// child id for updating zone names 16 bytes/characters
present( CHILD_ID_ZN1 , S_INFO);present( CHILD_ID_ZN2 , S_INFO);present( CHILD_ID_ZN3 , S_INFO);present( CHILD_ID_ZN4 , S_INFO);present( CHILD_ID_ZN5 , S_INFO);
present( CHILD_ID_ZN6 , S_INFO);present( CHILD_ID_ZN7 , S_INFO);present( CHILD_ID_ZN8 , S_INFO);present( CHILD_ID_ZN9 , S_INFO);present( CHILD_ID_ZN10 , S_INFO);
present( CHILD_ID_ZN11 , S_INFO);present( CHILD_ID_ZN12 , S_INFO);present( CHILD_ID_ZN13 , S_INFO);present( CHILD_ID_ZN14 , S_INFO);present( CHILD_ID_ZN15 , S_INFO);
present( CHILD_ID_ZN16 , S_INFO);present( CHILD_ID_ZN17 , S_INFO);present( CHILD_ID_ZN18 , S_INFO);present( CHILD_ID_ZN19 , S_INFO);present( CHILD_ID_ZN20 , S_INFO);
}  // end of void presentation
void setup(){
  keypad.addEventListener(keypadEvent); //add an event listener for this keypad
 #ifdef EEPROM_LOAD
 loadza();   // load zones active state from eeprom on reboot comment these out to use coded names
 loadpw();  // load passwords from eeprom function on reboot comment these out to use coded names
 loadun(); // load user names stored in eeprom on reboot  comment these out to use coded names
 #endif

//lcd display start---------------------
#ifdef LCD_ON
   lcd.init(); // initialize the lcd
 // lcd.noBacklight();
 lcd.backlight();
 lcd.clear();
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("  Fresh Reboot  ");        // print message at (0, 0)
 lcd.setCursor(0, 1);         // move cursor to   (2, 1)
  lcd.print("  Power Outage? "); // print message at (2, 1)
  ///lcd display end
#endif
pinMode(MOTION_SENSOR_INPUT, INPUT); 
//door sensor
  pinMode(DOOR_SENSOR_PIN,INPUT);
  // Activate internal pull-up
  digitalWrite(DOOR_SENSOR_PIN,HIGH); 
  // After setting up the button, setup debouncer
  debouncer.attach(DOOR_SENSOR_PIN);
  debouncer.interval(5);}  // end of setup

void loop(){
  
  keypad.getKey();
   currentmilli=millis(); 
   if(motionsent!=1){
                       tripped= digitalRead(MOTION_SENSOR_INPUT);
      if(tripped==1&lastmotionsent!=1){   
     send(statemsg.setSensor(CHILD_ID_DOORMOTION).set(1));   //was passwordmsg
       lastmotionsent=1; 
       motionsent=1;      
       }// end of if tripped ==1
         if(tripped==0&lastmotionsent!=0){
         send(statemsg.setSensor(CHILD_ID_DOORMOTION).set(0));
          lastmotionsent=0;  
 }
  previousmilli=currentmilli;     
}// if motionsent section
 if(currentmilli-previousmilli>reportinterval&motionsent==1){ // only update state if reportinterval time has passed 30 sec to allow keypad use on motion detection
    motionsent=0;
   }
// door sensor section
 debouncer.update();
  // Get the update value
  int value = debouncer.read();
  if (value != oldValue) {
     // Send in the new value
     send(statemsg.setSensor(CHILD_ID_DOORSTATE).set(value==HIGH ? 1 : 0));
     oldValue = value;
  }
       nowmilli=millis();
      if(nowmilli-sincekeypressed>resetinterval&keypressedcount>=1) resetpassword();
} //end of void loop

//take care of some special events
void keypadEvent(KeypadEvent eKey){
         nowmilli=millis();
  switch (keypad.getState()){
        case PRESSED:
        keypressedcount++;
       if(keypressedcount==1)sincekeypressed=millis();
        //  nowmilli=millis();  
  
   Serial.print("Pressed: ");
   Serial.println(eKey);
   Serial.print(" key count ");
   Serial.print(keypressedcount);
      
                  #ifdef LCD_ON
                  lcd.clear();
                    lcd.setCursor(0, 0);         // move cursor to   (0, 0)
                 lcd.print("  KEY PRESSED"); 
                  lcd.setCursor(0, 1);         // move cursor to   (0, 0)
                 lcd.print(">>>>>>    <<<<<<");
                 lcd.setCursor(7, 1);         // move cursor to   (0, 0)
                 lcd.print(eKey);               
                  #endif

                     
   switch (eKey){
     case '#': 
     checkpassword();             
                     break;                   
     case '*':    
                 resetpassword();               

                    break;
     default: 
             
u1e1.append(eKey);u1e2.append(eKey);u1e3.append(eKey);u1e4.append(eKey);u1e5.append(eKey);u1e6.append(eKey);u1e7.append(eKey);u1e8.append(eKey);u1e9.append(eKey);u1e10.append(eKey);
u1e11.append(eKey);u1e12.append(eKey);u1e13.append(eKey);u1e14.append(eKey);u1e15.append(eKey);u1e16.append(eKey);u1e17.append(eKey);u1e18.append(eKey);u1e19.append(eKey);u1e20.append(eKey);
u2e1.append(eKey);u2e2.append(eKey);u2e3.append(eKey);u2e4.append(eKey);u2e5.append(eKey);u2e6.append(eKey);u2e7.append(eKey);u2e8.append(eKey);u2e9.append(eKey);u2e10.append(eKey);
u2e11.append(eKey);u2e12.append(eKey);u2e13.append(eKey);u2e14.append(eKey);u2e15.append(eKey);u2e16.append(eKey);u2e17.append(eKey);u2e18.append(eKey);u2e19.append(eKey);u2e20.append(eKey);
u3e1.append(eKey);u3e2.append(eKey);u3e3.append(eKey);u3e4.append(eKey);u3e5.append(eKey);u3e6.append(eKey);u3e7.append(eKey);u3e8.append(eKey);u3e9.append(eKey);u3e10.append(eKey);
u3e11.append(eKey);u3e12.append(eKey);u3e13.append(eKey);u3e14.append(eKey);u3e15.append(eKey);u3e16.append(eKey);u3e17.append(eKey);u3e18.append(eKey);u3e19.append(eKey);u3e20.append(eKey);
u4e1.append(eKey);u4e2.append(eKey);u4e3.append(eKey);u4e4.append(eKey);u4e5.append(eKey);u4e6.append(eKey);u4e7.append(eKey);u4e8.append(eKey);u4e9.append(eKey);u4e10.append(eKey);
u4e11.append(eKey);u4e12.append(eKey);u4e13.append(eKey);u4e14.append(eKey);u4e15.append(eKey);u4e16.append(eKey);u4e17.append(eKey);u4e18.append(eKey);u4e19.append(eKey);u4e20.append(eKey);
u5e1.append(eKey);u5e2.append(eKey);u5e3.append(eKey);u5e4.append(eKey);u5e5.append(eKey);u5e6.append(eKey);u5e7.append(eKey);u5e8.append(eKey);u5e9.append(eKey);u5e10.append(eKey);
u5e11.append(eKey);u5e12.append(eKey);u5e13.append(eKey);u5e14.append(eKey);u5e15.append(eKey);u5e16.append(eKey);u5e17.append(eKey);u5e18.append(eKey);u5e19.append(eKey);u5e20.append(eKey);
u6e1.append(eKey);u6e2.append(eKey);u6e3.append(eKey);u6e4.append(eKey);u6e5.append(eKey);u6e6.append(eKey);u6e7.append(eKey);u6e8.append(eKey);u6e9.append(eKey);u6e10.append(eKey);
u6e11.append(eKey);u6e12.append(eKey);u6e13.append(eKey);u6e14.append(eKey);u6e15.append(eKey);u6e16.append(eKey);u6e17.append(eKey);u6e18.append(eKey);u6e19.append(eKey);u6e20.append(eKey);
}}}
void checkpassword(){  //create 2 of these check password functions one for individual password and another for the exstension
   bool pass=0;
   int u,z;
  // user 1 password and equipment evaluate
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=1; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e1.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif
   
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=2; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e2.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif
   
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=3; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e3.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=4; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e4.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=5; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e5.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=6; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e6.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=7; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e7.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=8; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e8.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=9; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e9.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=10; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e10.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=11; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e11.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=12; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e12.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=13; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e13.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=14; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e14.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=15; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e15.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=16; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e16.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=17; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e17.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=18; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e18.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=19; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e19.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=1;z=20; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e20.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
 //// user 2 password and equimpent evaluate----------------------------------------------------------------------------------------------------------------------------
if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=1; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e1.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=2; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e2.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=3; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e3.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=4; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e4.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=5; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e5.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=6; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e6.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=7; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e7.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=8; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e8.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=9; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e9.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=10; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e10.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=11; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e11.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=12; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e12.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=13; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e13.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=14; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e14.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=15; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e15.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=16; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e16.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=17; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e17.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=18; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e18.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=19; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e19.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=2;z=20; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u2e20.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
 //// user 3 password and equimpent evaluate------------------------------------------------------------------------------------------------------------------------
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=1; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e1.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=2; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e2.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=3; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e3.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif   
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=4; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e4.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=5; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e5.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=6; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e6.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=7; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e7.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=8; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e8.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=9; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e9.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=10; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e10.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=11; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e11.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=12; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e12.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=13; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e13.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=14; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e14.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif   
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=15; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e15.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=16; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e16.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=17; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e17.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=18; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e18.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=19; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u3e19.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=3;z=20; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u1e20.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
 //// user 4 password and equimpent evaluate------------------------------------------------------------------------------------------------------------------------
 if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=1; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e1.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=2; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e2.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=3; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e3.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=4; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e4.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=5; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e5.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=6; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e6.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=7; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e7.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=8; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e8.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=9; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e9.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=10; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e10.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif   
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=11; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e11.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=12; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e12.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=13; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e13.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=14; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e14.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=15; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e15.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=16; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e16.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif   
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=17; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e17.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=18; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e18.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=19; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e19.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=4;z=20; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u4e20.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  } }}
 //// user 5 password and equimpent evaluate--------------------------------------------------------------------------------------------------------------------------------------------
 if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=1; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e1.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=2; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e2.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=3; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e3.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=4; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e4.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=5; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e5.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=6; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e6.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=7; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e7.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif   
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=8; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e8.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif   
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=9; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e9.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=10; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e10.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=11; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e11.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=12; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e12.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=13; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e13.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=14; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e14.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=15; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e15.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=16; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e16.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=17; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e17.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=18; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e18.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=19; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e19.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  } }}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=5;z=20; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u5e20.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
 //// user 6 password and equimpent evaluate-----------------------------------------------------------------------------------------------------------------------------
 if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=1; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e1.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=2; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e2.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
  if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=3; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e3.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=4; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e4.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=5; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e5.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=6; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e6.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=7; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e7.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif   
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=8; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e8.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=9; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e9.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=10; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e10.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif   
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=11; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e11.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=12; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e12.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=13; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e13.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif   
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=14; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e14.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=15; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e15.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=16; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e16.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=17; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e17.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif  
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=18; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e18.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=19; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e19.evaluate()){    //u1_e1.password needs to be set here for each password section
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
   if(pass==0){   // u,v and u_e_.password needs to be set in these sections
    u=6;z=20; // this needs to be set here u user number and z zone for each password
  join(u,z);
 if (u6e20.evaluate()){//u1_e1.password needs to be set here for each password section
   
   pass=1;   if(pz[u][z]==1){
  #ifdef DEBUG_ON
   Serial.println("E1 message sending"); 
   #endif
send(passwordmsg.setSensor(z).set(1));  // Send tripped value to gw
wait(20);
send(passwordmsg.setSensor(z).set(0));  // Send tripped value to gw
wait(20);
#ifdef LCD_ON
  lcd.setCursor(0, 0);   // move cursor to   (0, 0)
 lcd.print((zonename[z]));
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
   lcd.print("USER:"); 
   lcd.setCursor(7, 1);         // move cursor to   (0, 0)
   lcd.print((username[u])); 
   wait(3000);
   lcd.clear();
  #endif 
 }else{
  #ifdef LCD_ON
    lcd.setCursor(1, 0);         // move cursor to   (0, 0)
  lcd.print("USER    ZONE ");
    lcd.setCursor(6, 0);         // move cursor to   (0, 0)
  lcd.print(u); 
   lcd.setCursor(14, 0);         // move cursor to   (0, 0)
  lcd.print(z);  
    lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("ACCESS DENIED"); 
   wait(3000);
   lcd.clear();  
  #endif
  }}}
if(pass==0){
   Serial.println("Wrong");
  //add code to run if it did not work
  #ifdef LCD_ON
 lcd.clear();
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("INVALID PASSWORD"); 
   lcd.setCursor(1, 1);         // move cursor to   (0, 0)
  lcd.print("   TRY AGAIN"); 
  wait(3000);
  #endif
 }
pass=0;
 lcd.clear();    
                  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
                 lcd.print(greetingline1); 
                 lcd.setCursor(0, 1);         // move cursor to   (0, 0)
                 lcd.print(greetingline2);
                 
}  // end of check password function


/// ----------------------------------void receive---------------------------------------------------------------------------------------------------------------
void receive(const MyMessage &message)
{
  if (message.getType()==V_TEXT) {   //v_text type number is 47   
     int s=message.getSensor();
    if(s==CHILD_ID_PW1|s==CHILD_ID_PW2|s==CHILD_ID_PW3|s==CHILD_ID_PW4|s==CHILD_ID_PW5|s==CHILD_ID_PW6){  // make sure the correct child-id arriving
    updatepw(message.getSensor(),message.getString());
    }
     if(s==CHILD_ID_N1|s==CHILD_ID_N2|s==CHILD_ID_N3|s==CHILD_ID_N4|s==CHILD_ID_N5|s==CHILD_ID_N6){     
    updatename(message.getSensor(),message.getString());
    }
    if(s==CHILD_ID_ZN1|s==CHILD_ID_ZN2|s==CHILD_ID_ZN3|s==CHILD_ID_ZN4|s==CHILD_ID_ZN5|
    s==CHILD_ID_ZN6|s==CHILD_ID_ZN7|s==CHILD_ID_ZN8|s==CHILD_ID_ZN9|s==CHILD_ID_ZN10|
    s==CHILD_ID_ZN11|s==CHILD_ID_ZN12|s==CHILD_ID_ZN13|s==CHILD_ID_ZN14|s==CHILD_ID_ZN15|
    s==CHILD_ID_ZN16|s==CHILD_ID_ZN17|s==CHILD_ID_ZN18|s==CHILD_ID_ZN19|s==CHILD_ID_ZN20){
      updatezn(message.getSensor(),message.getString());
    }
          
  }
if (message.getType()==V_STATUS) {  // v_status type number is 2
    updatezone(message.getSensor(),message.getBool());  
}
#ifdef LCD_ON
lcd.clear();
  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
  lcd.print("    Good Day");   
#endif
}
//  User name load from eeprom function----------------------------------------------------------------------------------------------------------------------------------------
void loadun(){  
int i;
int j;
int mempos;
for(i=0,j=1,mempos=CHILD_ID_N1;i<8;i++,mempos++){ //user      
       username[j][i]  =loadState(mempos);
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
i++;
username[j][i]=0;
for(i=0,j=2,mempos=CHILD_ID_N2;i<8;i++,mempos++){ //user      
       username[j][i]  =loadState(mempos);
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
i++;
username[j][i]=0;
for(i=0,j=3,mempos=CHILD_ID_N3;i<8;i++,mempos++){ //user    
       username[j][i]  =loadState(mempos);    
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
i++;
username[j][i]=0;

for(i=0,j=4,mempos=CHILD_ID_N4;i<8;i++,mempos++){ //user      
       username[j][i]  =loadState(mempos);    
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
i++;
username[j][i]=0;

for(i=0,j=5,mempos=CHILD_ID_N5;i<8;i++,mempos++){ //user      
       username[j][i]  =loadState(mempos);      
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
i++;
username[j][i]=0;
for(i=0,j=6,mempos=CHILD_ID_N6;i<8;i++,mempos++){ //user      
       username[j][i]  =loadState(mempos);      
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
i++;
username[j][i]=0;
  } // end of load password from eeprom
  //------------------------------------------------------------update ZONE NAMES-----------------------------------------------------------------------------------------
// Zone names  update, these are not stored in eeprom reboot will set the names back to original coded name--------------------------------------------------------------------
 void updatezn(int n,char *p){    
      int i=0;
      char b=*p;
    
if(n==CHILD_ID_ZN1)n=1;
else if(n==CHILD_ID_ZN2)n=2;
else if(n==CHILD_ID_ZN3)n=3;
else if(n==CHILD_ID_ZN4)n=4;
else if(n==CHILD_ID_ZN5)n=5;
else if(n==CHILD_ID_ZN6)n=6;
else if(n==CHILD_ID_ZN7)n=7;
else if(n==CHILD_ID_ZN8)n=8;
else if(n==CHILD_ID_ZN9)n=9;
else if(n==CHILD_ID_ZN10)n=10;
else if(n==CHILD_ID_ZN11)n=11;
else if(n==CHILD_ID_ZN12)n=12;
else if(n==CHILD_ID_ZN13)n=13;
else if(n==CHILD_ID_ZN14)n=14;
else if(n==CHILD_ID_ZN15)n=15;
else if(n==CHILD_ID_ZN16)n=16;
else if(n==CHILD_ID_ZN17)n=17;
else if(n==CHILD_ID_ZN18)n=18;
else if(n==CHILD_ID_ZN19)n=19;
else if(n==CHILD_ID_ZN20)n=20;
      
#ifdef LCD_ON
lcd.clear();
#endif
while(b!=0&i<16){
  #ifdef LCD_ON
       lcd.setCursor(i, 0);         // move cursor to   (0, 0)
  lcd.print(*p);        // print message at (0, 0)
  #endif
       zonename[n][i]  = b;  
       i++;
       p++;
       b=*p;
    }
    zonename[n][i]=0;   
      #ifdef LCD_ON
          lcd.setCursor(0, 1);         // move cursor to   (0, 0)
  lcd.print(zonename[n]);  
      wait(3000);
        lcd.clear();
  #endif 
  #ifdef DEBUG_ON
    Serial.print(" User: ");
        Serial.print(n);
          Serial.print(" Name updated to: ");
        Serial.print(zonename[n]);
        #endif
  }
// username update from controller section----------------------------------------------------------------------------------------------------------------
 void updatename(int n,char *p){
      int a=0;
      int i=0;
      char b=*p;
   if(n==CHILD_ID_N1)a=1; 
   else if (n==CHILD_ID_N2) a=2;
    else if(n==CHILD_ID_N3)a=3; 
    else if(n==CHILD_ID_N4)a=4; 
    else if (n==CHILD_ID_N5)a=5; 
   else if(n==CHILD_ID_N6)a=6;    
#ifdef LCD_ON
lcd.clear();
#endif
while(b!=0&i<8){
  #ifdef LCD_ON
       lcd.setCursor(i, 0);         // move cursor to   (0, 0)
  lcd.print(*p);        // print message at (0, 0)
  #endif
       username[a][i]  = b;  
       saveState(n+i,b);
       i++;
       p++;
       b=*p;
    }
    username[a][i]=0;
      #ifdef LCD_ON
          lcd.setCursor(0, 1);         // move cursor to   (0, 0)
          lcd.print(username[a]);
              wait(3000);
        lcd.clear();
    #endif
  
  #ifdef DEBUG_ON
    Serial.print(" User: ");
        Serial.print(a);
          Serial.print(" Name updated to: ");
        Serial.print(username[a]);
        #endif
  }
/// password update from controller/gateway section-----------------------------------------------------------------------------------------------------------
 void updatepw(int n,char *p){
int i=1;
int u;
     if(n==CHILD_ID_PW1)u=1;
else if(n==CHILD_ID_PW2)u=2;
else if(n==CHILD_ID_PW3)u=3;
else if(n==CHILD_ID_PW4)u=4;
else if(n==CHILD_ID_PW5)u=5;
else if(n==CHILD_ID_PW6)u=6;
int mempos=n; // eeprom memory location CHILD_ID_PW_
#ifdef LCD_ON;
lcd.clear();
#endif
for(i=1;i<7;i++){ 
  #ifdef LCD_ON
       lcd.setCursor(i, 0);         // move cursor to   (0, 0)
  lcd.print(*(p-1+i));        // print message at (0, 0)
  #endif
       userpw[u][i] = *(p-1+i);  
  saveState(mempos,*(p+i-1));
   mempos++;
    }
      #ifdef LCD_ON
          lcd.setCursor(0, 1);         // move cursor to   (0, 0)
  lcd.print(userpw[u]);  
  #endif
  #ifdef DEBUG_ON
    Serial.print(" User: ");
        Serial.print(u);
          Serial.print(" Password updated to: ");
        Serial.print(userpw[u]);
        #endif
  
 }  
//end of updatepw
void loadpw(){
int i;
int j;
int mempos;
for(i=1,j=1,mempos=CHILD_ID_PW1;i<7;i++,mempos++){ //user
       
       userpw[j][i]  =loadState(mempos);
    
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
for(i=1,j=2,mempos=CHILD_ID_PW2;i<7;i++,mempos++){ //user     
       userpw[j][i]  =loadState(mempos);  
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
for(i=1,j=3,mempos=CHILD_ID_PW3;i<7;i++,mempos++){ //user     
       userpw[j][i]  =loadState(mempos);
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
for(i=1,j=4,mempos=CHILD_ID_PW4;i<7;i++,mempos++){ //user      
       userpw[j][i]  =loadState(mempos);   
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
for(i=1,j=5,mempos=CHILD_ID_PW5;i<7;i++,mempos++){ //user     
       userpw[j][i]  =loadState(mempos); 
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
for(i=1,j=6,mempos=CHILD_ID_PW6;i<7;i++,mempos++){ //user     
       userpw[j][i]  =loadState(mempos);   
       #ifdef DEBUG_ON
         Serial.print(" User: ");
        Serial.print(i);
        Serial.print(" Password: ");
       Serial.print(userpw[i]);
       Serial.println();
       #endif
}
 } // end load pw 
  // update zone permit use from eeprom-----------------------------------------------------------------------------------------------------------------------------
   void updatezone(int n,bool s){
  int u,e;
    if(n>=41&&n<=60){
      u=1;e=n-40;
      saveState(n,s);  // eeprom save state
      pz[u][e]=s;  //user zone active 
      }
    if(n>=61&&n<=80){
        u=2;e=n-60;
         saveState(n,s);  // eeprom save state
      pz[u][e]=s;  //user zone active
      }
       if(n>=81&&n<=100){
        u=3;e=n-80;
         saveState(n,s);  // eeprom save state
      pz[u][e]=s;  //user zone active
      }
          if(n>=101&&n<=120){
        u=4;e=n-100;
         saveState(n,s);  // eeprom save state
      pz[u][e]=s;  //user zone active
      }
           if(n>=121&&n<=140){
        u=5;e=n-120;
         saveState(n,s);  // eeprom save state
      pz[u][e]=s;  //user zone active
      }
           if(n>=141&&n<=160){
        u=6;e=n-140;
         saveState(n,s); // eeprom save state
      pz[u][e]=s;  //user zone active
      }
  }  // end of update equipment user permissions
   void loadza()
   {   // zones activated, load from eeprom memory----------------------------------------------------------------------------------------------------------------------------
  int u,e,n;
  for(n=40;n<161;n++){ 
    if(n>=41&&n<=60){
      u=1;e=n-40;
      pz[u][e]=  loadState(n);  // eeprom save state 

      #ifdef DEBUG_ON
              Serial.print("User: ");
           Serial.print(u);
           Serial.print(" Zone: ");
           Serial.print(e);
           Serial.print(" Activation state: ");
          Serial.print(pz[u][e]);
          Serial.print(" ");
          #endif
      }
      #ifdef DEBUG_ON
      Serial.println("\n");
      #endif
      
      if(n>=61&&n<=80){
        u=2;e=n-60;
      pz[u][e]=loadState(n);  // eeprom save state

      #ifdef DEBUG_ON
         Serial.print("User: ");
           Serial.print(u);
           Serial.print(" Zone: ");
           Serial.print(e);
           Serial.print(" Activation state: ");
          Serial.print(pz[u][e]);
          Serial.print(" ");
      #endif
      }
      #ifdef DEBUG_ON
       Serial.println("\n");
       #endif

       if(n>=81&&n<=100){
        u=3;e=n-80;
      pz[u][e]=loadState(n);  // eeprom save state
     
      #ifdef DEBUG_ON
         Serial.print("User: ");
           Serial.print(u);
           Serial.print(" Zone: ");
           Serial.print(e);
           Serial.print(" Activation state: ");
          Serial.print(pz[u][e]);
          Serial.print(" ");
          #endif
      }
      #ifdef DEBUG_ON
       Serial.println("\n");
       #endif

          if(n>=101&&n<=120){
        u=4;e=n-100;
      pz[u][e]=loadState(n);  // eeprom save state

      #ifdef DEBUG_ON
        Serial.print("User: ");
           Serial.print(u);
           Serial.print(" Zone: ");
           Serial.print(e);
           Serial.print(" Activation state: ");
          Serial.print(pz[u][e]);
          Serial.print(" ");
          #endif
      }
      #ifdef DEBUG_ON
       Serial.println("\n");
       #endif

           if(n>=121&&n<=140){
        u=5;e=n-120;
      pz[u][e]=pz[u][e]=loadState(n);  //user zone active

      #ifdef DEBUG_ON
         Serial.print("User: ");
           Serial.print(u);
           Serial.print(" Zone: ");
           Serial.print(e);
           Serial.print(" Activation state: ");
          Serial.print(pz[u][e]);
          Serial.print(" ");
          #endif
      }
      #ifdef DEBUG_ON
       Serial.println("\n");
       #endif

           if(n>=141&&n<=160){
        u=6;e=n-140;
      pz[u][e]=pz[u][e]=loadState(n);  //user zone active

      #ifdef DEBUG_ON
        Serial.print("User: ");
           Serial.print(u);
           Serial.print(" Zone: ");
           Serial.print(e);
           Serial.print(" Activation state: ");
          Serial.print(pz[u][e]);
          Serial.print(" ");
          #endif
      }
      #ifdef DEBUG_ON
       Serial.println("\n");
       #endif 
  } // end of update zone
   }
void resetpassword(){
  keypressedcount=0;  //reset the keypressed counter
                 #ifdef LCD_ON
                  lcd.clear();
                 lcd.setCursor(0, 0);         // move cursor to   (0, 0)
                 lcd.print(">---PASSWORD---<"); 
                 lcd.setCursor(0, 1);         // move cursor to   (0, 0)
                 lcd.print(">----RESET-----<");   
                 wait(1000);
                 lcd.clear();    
                  lcd.setCursor(0, 0);         // move cursor to   (0, 0)
                 lcd.print(greetingline1); 
                 lcd.setCursor(0, 1);         // move cursor to   (0, 0)
                 lcd.print(greetingline2);                        
                  #endif
     
u1e1.reset();u1e2.reset();u1e3.reset();u1e4.reset();u1e5.reset();u1e6.reset();u1e7.reset();u1e8.reset();u1e9.reset();u1e10.reset();u1e11.reset();u1e12.reset();u1e13.reset();u1e14.reset();
u1e15.reset();u1e16.reset();u1e17.reset();u1e18.reset();u1e19.reset();u1e20.reset();u2e1.reset();u2e2.reset();u2e3.reset();u2e4.reset();u2e5.reset();u2e6.reset();u2e7.reset();u2e8.reset();
u2e9.reset();u2e10.reset();u2e11.reset();u2e12.reset();u2e13.reset();u2e14.reset();u2e15.reset();u2e16.reset();u2e17.reset();u2e18.reset();u2e19.reset();u2e20.reset();u3e1.reset();
u3e2.reset();u3e3.reset();u3e4.reset();u3e5.reset();u3e6.reset();u3e7.reset();u3e8.reset();u3e9.reset();u3e10.reset();u3e11.reset();u3e12.reset();u3e13.reset();u3e14.reset();u3e15.reset();
u3e16.reset();u3e17.reset();u3e18.reset();u3e19.reset();u3e20.reset();u4e1.reset();u4e2.reset();u4e3.reset();u4e4.reset();u4e5.reset();u4e6.reset();u4e7.reset();u4e8.reset();u4e9.reset();
u4e10.reset();u4e11.reset();u4e12.reset();u4e13.reset();u4e14.reset();u4e15.reset();u4e16.reset();u4e17.reset();u4e18.reset();u4e19.reset();u4e20.reset();u5e1.reset();u5e2.reset();u5e3.reset();
u5e4.reset();u5e5.reset();u5e6.reset();u5e7.reset();u5e8.reset();u5e9.reset();u5e10.reset();u5e11.reset();u5e12.reset();u5e13.reset();u5e14.reset();u5e15.reset();u5e16.reset();u5e17.reset();
u5e18.reset();u5e19.reset();u5e20.reset();u6e1.reset();u6e2.reset();u6e3.reset();u6e4.reset();u6e5.reset();u6e6.reset();u6e7.reset();u6e8.reset();u6e9.reset();u6e10.reset();u6e11.reset();
u6e12.reset();u6e13.reset();u6e14.reset();u6e15.reset();u6e16.reset();u6e17.reset();u6e18.reset();u6e19.reset();u6e20.reset();}



   
