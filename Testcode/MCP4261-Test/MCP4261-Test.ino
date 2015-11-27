
// inslude the SPI library:
#include <SPI.h>

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin0 = 10;
const int slaveSelectPin1 = 9;
const int shutdownPin = 7;
const int wiper0writeAddr = B00000000;
const int wiper1writeAddr = B00010000;
const int tconwriteAddr = B01000000;
const int tcon_0off_1on = B11110000;
const int tcon_0on_1off = B00001111;
const int tcon_0off_1off = B00000000;
const int tcon_0on_1on = B11111111;

int warten = 3000;


void setup() {
  Serial.begin(9600);
  // set the slaveSelectPin as an output:
  pinMode (slaveSelectPin0, OUTPUT);
  pinMode (slaveSelectPin1, OUTPUT);
  // set the shutdownPin as an output:
  pinMode (shutdownPin, OUTPUT);
  // start with all the pots shutdown
  digitalWrite(shutdownPin,LOW);
  // initialize SPI:
  SPI.begin();
  Serial.println("Setup complete");
}

void loop() {
  Serial.println("Starting loop");
  digitalWrite(shutdownPin,HIGH); //Turn off shutdown

    for (int x = 0; x < 255; x++) {  
    Serial.print("Value sent to pot: ");
    Serial.println(x);
    digitalPotWrite0(wiper0writeAddr, x);
    digitalPotWrite0(wiper1writeAddr, x);
    digitalPotWrite1(wiper0writeAddr, x);
    digitalPotWrite1(wiper1writeAddr, x);

    }

    delay(warten);
    digitalPotWrite0(wiper0writeAddr, 0);
    digitalPotWrite0(wiper1writeAddr, 0);
    digitalPotWrite1(wiper0writeAddr, 0);
    digitalPotWrite1(wiper1writeAddr, 0);
    delay(250);

    for (int x = 0; x < 255; x++) {  
    Serial.print("Value sent to pot: ");
    Serial.println(255-x);
    digitalPotWrite0(wiper0writeAddr, 255-x);
    digitalPotWrite0(wiper1writeAddr, 255-x);
    digitalPotWrite1(wiper0writeAddr, 255-x);
    digitalPotWrite1(wiper1writeAddr, 255-x);

    }
    
    delay(warten);
    digitalPotWrite0(wiper0writeAddr, 255);
    digitalPotWrite0(wiper1writeAddr, 255);
    digitalPotWrite1(wiper0writeAddr, 255);
    digitalPotWrite1(wiper1writeAddr, 255);
    delay(250);

}


void digitalPotWrite0(int address, int value) {
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin0,LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin0,HIGH); 
}

void digitalPotWrite1(int address, int value) {
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin1,LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin1,HIGH); 
}
