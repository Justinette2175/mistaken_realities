/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "/Users/justinegagnepain/Documents/concordia_classes/CART_461_Tangible_Media_Studio/mistaken_realities_latest/src/miskatken_realities.ino"
#include <Wire.h>

#include "simple-OSC.h"
#include "math.h"

/* MPR121 CONSTANTS */
// const int irqpins[2] = {7, 8};
// const unsigned char addresses[2] = {0x5A, 0x5C};

void connectToLAN();
void sendOSCData(float x);
void setup();
void loop();
void setupTouchDevices();
void readAllTouchInputs();
void readTouchInputs();
void mpr121_setup(unsigned char address);
boolean checkInterrupt(int pin);
void set_register(int address, unsigned char r, unsigned char v);
#line 10 "/Users/justinegagnepain/Documents/concordia_classes/CART_461_Tangible_Media_Studio/mistaken_realities_latest/src/miskatken_realities.ino"
const int irqpin = 7;
const unsigned char MPR121_address = 0x5A;
boolean touchStates[12]; //to keep track of the previous touch states
int lastTouchedSensorIndex = 0;

/*
    MPR121.h
	April 8, 2010
	by: Jim Lindblom
*/

// MPR121 Register Defines
#define MHD_R 0x2B
#define NHD_R 0x2C
#define NCL_R 0x2D
#define FDL_R 0x2E
#define MHD_F 0x2F
#define NHD_F 0x30
#define NCL_F 0x31
#define FDL_F 0x32
#define ELE0_T 0x41
#define ELE0_R 0x42
#define ELE1_T 0x43
#define ELE1_R 0x44
#define ELE2_T 0x45
#define ELE2_R 0x46
#define ELE3_T 0x47
#define ELE3_R 0x48
#define ELE4_T 0x49
#define ELE4_R 0x4A
#define ELE5_T 0x4B
#define ELE5_R 0x4C
#define ELE6_T 0x4D
#define ELE6_R 0x4E
#define ELE7_T 0x4F
#define ELE7_R 0x50
#define ELE8_T 0x51
#define ELE8_R 0x52
#define ELE9_T 0x53
#define ELE9_R 0x54
#define ELE10_T 0x55
#define ELE10_R 0x56
#define ELE11_T 0x57
#define ELE11_R 0x58
#define FIL_CFG 0x5D
#define ELE_CFG 0x5E
#define GPIO_CTRL0 0x73
#define GPIO_CTRL1 0x74
#define GPIO_DATA 0x75
#define GPIO_DIR 0x76
#define GPIO_EN 0x77
#define GPIO_SET 0x78
#define GPIO_CLEAR 0x79
#define GPIO_TOGGLE 0x7A
#define ATO_CFG0 0x7B
#define ATO_CFGU 0x7D
#define ATO_CFGL 0x7E
#define ATO_CFGT 0x7F

// Global Constants
#define TOU_THRESH 0x06
#define REL_THRESH 0x0A

#define B_TN D2 // MOMENTARY BUTTON

/* SIMPLE-OSC USES UDP AS TRANSPORT LAYER */
UDP Udp;

IPAddress argonIP;
char argonIPAddress[16];
/* EXPLICIT REMOTE ADDRESS DECLARATION IF IS KNOWN - 
 * REMOTE ADDRESS CAN ALSO BE RETRIEVED FROM RECEIVED 
 * OSC/UDP PACKET  */
IPAddress remoteIP(192, 168, 1, 9);
/* PORTS FOR INCOMING & OUTGOIN9 DATA */
unsigned int outPort = 8000;

void connectToLAN()
{
    /* IF ARGON ALREADY CONFIGURED FOR SSID/ROUTER - THEN THIS */
    /* TRY CONNECT TO SSID - ROUTER */
    WiFi.connect();
    /* WAIT UNTIL DHCP SERVICE ASSIGNS ARGON IPADDRESS */
    while (!WiFi.ready())
        ;

    delay(5);
    /* GET HOST (ARGON) ASSIGNED IP */
    Serial.print("ARGON IP (DHCP): ");
    //argonIP = WiFi.localIP();
    //sprintf(argonIPAddress, "%d.%d.%d.%d", argonIP[0], argonIP[1], argonIP[2], argonIP[3]);
    Serial.println(WiFi.localIP());
}

void sendOSCData(float x)
{
    OSCMessage outMessage("/touch");
    outMessage.addFloat(x);
    outMessage.send(Udp, remoteIP, outPort);
}

void setup()
{

    uint8_t ver;

    // Initialize Serial port
    Serial.begin(9600);
    Serial.println();
    Serial.println("-----------------------------------");
    Serial.println("Mistaken Realities");
    Serial.println("-----------------------------------");

    /* INPUT: PUSH BUTTON */
    pinMode(B_TN, INPUT_PULLUP); // NO RESISTOR

    connectToLAN();
    /* START UDP SERVICE - USED BY SIMPLE-OSC */

    delay(5); // Force Serial.println in void setup()
    Serial.println("Completed void setup");
}

void loop()
{
    if (digitalRead(B_TN) == LOW)
    {
        Serial.println("I'm pressing");
        sendOSCData(1);
        delay(1000);
    }

    sendOSCData(1.0);
    // If there is position data available, read and print it
}

/****************** MPR121 FUNCTIONS ********************************************
This project uses 2 MPR121 breakout boards. One is adressed to 0x5A, and the other to 0x5C.
This allows for 24 independent touch sensors rendings. 
*/

void setupTouchDevices()
{
    pinMode(irqpin, INPUT);
    digitalWrite(irqpin, HIGH);

    Wire.begin();

    mpr121_setup(0x5A);
}

void readAllTouchInputs()
{
    readTouchInputs();
}

void readTouchInputs()
{
    if (!checkInterrupt(irqpin))
    {
        //read the touch state from the MPR121
        Wire.requestFrom(MPR121_address, 2);

        byte LSB = Wire.read();
        byte MSB = Wire.read();

        uint16_t touched = ((MSB << 8) | LSB); //16bits that make up the touch states

        for (int i = 0; i < 12; i++)
        { // Check what electrodes were pressed
            int globalIndex = i;
            if (touched & (1 << i))
            {
                touchStates[globalIndex] = 1;
            }
            else
            {
                touchStates[globalIndex] = 0;
            }
            Serial.println("Status of touch states");
            Serial.print(touchStates[globalIndex]);
            Serial.println("");
        }
    }
}

void mpr121_setup(unsigned char address)
{

    set_register(address, ELE_CFG, 0x00);

    // Section A - Controls filtering when data is > baseline.
    set_register(address, MHD_R, 0x01);
    set_register(address, NHD_R, 0x01);
    set_register(address, NCL_R, 0x00);
    set_register(address, FDL_R, 0x00);

    // Section B - Controls filtering when data is < baseline.
    set_register(address, MHD_F, 0x01);
    set_register(address, NHD_F, 0x01);
    set_register(address, NCL_F, 0xFF);
    set_register(address, FDL_F, 0x02);

    // Section C - Sets touch and release thresholds for each electrode
    set_register(address, ELE0_T, TOU_THRESH);
    set_register(address, ELE0_R, REL_THRESH);

    set_register(address, ELE1_T, TOU_THRESH);
    set_register(address, ELE1_R, REL_THRESH);

    set_register(address, ELE2_T, TOU_THRESH);
    set_register(address, ELE2_R, REL_THRESH);

    set_register(address, ELE3_T, TOU_THRESH);
    set_register(address, ELE3_R, REL_THRESH);

    set_register(address, ELE4_T, TOU_THRESH);
    set_register(address, ELE4_R, REL_THRESH);

    set_register(address, ELE5_T, TOU_THRESH);
    set_register(address, ELE5_R, REL_THRESH);

    set_register(address, ELE6_T, TOU_THRESH);
    set_register(address, ELE6_R, REL_THRESH);

    set_register(address, ELE7_T, TOU_THRESH);
    set_register(address, ELE7_R, REL_THRESH);

    set_register(address, ELE8_T, TOU_THRESH);
    set_register(address, ELE8_R, REL_THRESH);

    set_register(address, ELE9_T, TOU_THRESH);
    set_register(address, ELE9_R, REL_THRESH);

    set_register(address, ELE10_T, TOU_THRESH);
    set_register(address, ELE10_R, REL_THRESH);

    set_register(address, ELE11_T, TOU_THRESH);
    set_register(address, ELE11_R, REL_THRESH);

    // Section D
    // Set the Filter Configuration
    // Set ESI2
    set_register(address, FIL_CFG, 0x04);

    // Section E
    // Electrode Configuration
    // Set ELE_CFG to 0x00 to return to standby mode
    set_register(address, ELE_CFG, 0x0C); // Enables all 12 Electrodes

    // Section F
    // Enable Auto Config and auto Reconfig
    /*set_register(0x5A, ATO_CFG0, 0x0B);
  set_register(0x5A, ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   set_register(0x5A, ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
  set_register(0x5A, ATO_CFGT, 0xB5);*/
    // Target = 0.9*USL = 0xB5 @3.3V

    set_register(address, ELE_CFG, 0x0C);
}

boolean checkInterrupt(int pin)
{
    return digitalRead(pin);
}

void set_register(int address, unsigned char r, unsigned char v)
{
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}
