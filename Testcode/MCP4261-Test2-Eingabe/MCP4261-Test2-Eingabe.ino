
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
int eingabe = 0;


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
  digitalWrite(shutdownPin,HIGH); //Turn off shutdown

      if (Serial.available() > 0) {
                // read the incoming byte:
                eingabe = Serial.parseInt();
                Serial.print("Eingabe: ");
                Serial.println(eingabe);
                digitalPotWrite0(wiper0writeAddr, eingabe);
                digitalPotWrite0(wiper1writeAddr, eingabe);
                digitalPotWrite1(wiper0writeAddr, eingabe);
                digitalPotWrite1(wiper1writeAddr, eingabe);
      }


                

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
