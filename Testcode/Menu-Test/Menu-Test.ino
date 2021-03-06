/*
  Porsche Klimasteuergeräte Automatik Bedienteil

  Das PKAB übernimmt die Einstellung des Bedienteils für die
  verschiedenen Prüfschritte. Dabei werden die Tasten mittels
  4-Kanal-Relais geschaltet und die Potentiometer mittels zwei
  2-Kanal-Digital-Potentionmeter MCP4261 gesteuert.

  Das MCP4261-Digital-Potentionmeter ist SPI-gesteuert und die
  Steuerung funktioniert mit zwei Bytes. Das erste sendet die
  Adresse des Poti-Kanals (Poti 0 oder Poti 1) und das zweite
  Byte sendet den Widerstandswert, der von 0 bis 255 eingestellt
  werden kann.

  Arduino Pinbelegung:
  * CS für Chip A (SS Pin) - Digital Pin 10
  * CS für Chip B (SS Pin) - Digital Pin 9
  * SDI (MOSI Pin)         - Digital Pin 11
  * CLK (SCK Pin)          - Digital Pin 13


 August 2015
 Sebastian Schult

*/


/*################### Libraries ###################*/

#include <SPI.h>
#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

// 0x27 ist die I2C Bus-Adresse für das Serial-Backpack am LCD
LiquidCrystal_I2C  lcd(0x27,2,1,0,4,5,6,7); 


/*################### Variablen ###################*/

const int an             = 1;
const int aus            = 0;
const int links          = 0;
const int mitte          = 128;
const int rechts         = 255;

const int acmax          = 4;   //Pin 3: Relay 1 steuert ACMax
const int ac             = 5;   //Pin 4: Relay 2 steuert AC
const int umluft         = 6;   //Pin 5: Relay 3 steuert Umluft
const int defrost        = 7;   //Pin 6: Relay 4 steuert Defrost

const int mausklick      = 8;

//Initialisiert die Variablen für die Potis
const int geblaese       = 101;
const int temp           = 102;
const int defrost_poti   = 103;
const int fussraum_poti  = 104;

const int ChipSelectA    = 10;    //Pin 10 steuert das Poti A (Defrost- und Fussraumpoti)
const int ChipSelectB    = 9;     //Pin  9 steuert das Poti B (Geblaese- und Temperaturpoti)

const int wiper0writeAddr= B00000000; //Schreibadresse für Poti 0 der jeweiligen Chips (A, B)
const int wiper1writeAddr= B00010000; //Schreibadresse für Poti 1 der jeweiligen Chips (A, B)

const int button1        = 2;     //Pin 2: Taste 1
const int button2        = 3;     //Pin 3: Taste 2
const int drehpoti       = A1;    //Analog Pin 1: Dreh-Potentiometer (5 Stufen)

int aktueller_schritt    = -2;     //Speichert den aktuellen Prüfschritt
int schrittspeicher      = -2;    //Zwischenspeicher für den Wert aus "aktueller_schritt"
int pruef_auswahl        = -1;     //Variable fuer Pruefauswahlzaehler
int pruefspeicher        = -1;     //Zwischenspeicher für den Wert aus "pruef_auswahl"
//int menu                 = 0;
long time                = 0;     //Zeitmessung startet bei 0s.
long entprellen          = 100;   //250ms Taste entprellen

int flag                 = 0;     //wird gesetzt, um "Pruefauswahl" nur am Anfang anzuzeigen

const int countdown      = 5000;  //5 Sekunden bis zum nächsten Schritt


/*################### Setup ###################*/

void setup() {
  // Setzt Chip-Select als AUSGANG und deaktviert beide:
  pinMode (ChipSelectA, OUTPUT);
  pinMode (ChipSelectB, OUTPUT);
  pinMode (ChipSelectA, HIGH);
  pinMode (ChipSelectB, HIGH);

  // Initialisiert SPI und Serial:
  SPI.begin();
  Serial.begin(9600);
  Serial.println("Starte Programm\n");
  
  // Initialisiert die Relais-Pins als AUSGANG
  pinMode(acmax,    OUTPUT);       
  pinMode(ac,       OUTPUT);
  pinMode(umluft,   OUTPUT);
  pinMode(defrost,  OUTPUT);
  pinMode(mausklick,OUTPUT);

  // Schaltet zu Beginn alle Relais aus
  digitalWrite(acmax,    HIGH);
  digitalWrite(ac,       HIGH);
  digitalWrite(umluft,   HIGH);
  digitalWrite(defrost,  HIGH);
  digitalWrite(mausklick,HIGH);

  // Initialisiert das LCD und schaltet die Hintergrundbeleuchtung ein
  lcd.begin (16,2); // für das 16 x 2 LCD Modul
  lcd.setBacklightPin(3,POSITIVE);
  lcd.setBacklight(HIGH);

  //Startsequenz
  Startsequenz();

  //Aktiviert den internen Pull-Up-Widerstand für die Eingänge
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(drehpoti,INPUT_PULLUP);

  //Interrupt für den Schrittzaehler
  attachInterrupt(0, Pruefauswahlzaehler, LOW); //Taste 1
  attachInterrupt(1, Schrittzaehler,      LOW); //Taste 2

}


/*################### Loop ###################*/

void loop() {

// Schleife läuft endlos bis Arduino neu gestartet wird

  if(aktueller_schritt == -2 && flag == 0){
    lcd.clear();
    lcd.home(); // Setzt Cursor auf 0,0
    lcd.print("Pruefauswahl");
    Serial.println("Pruefauswahl\n");
    flag = 1;
  }
  if(pruef_auswahl > -2 && aktueller_schritt == -1){
    lcd.setCursor (0,1);
    lcd.print("Wiederholung: ");
    lcd.print(Wiederholung());
  }

  if(schrittspeicher != aktueller_schritt || pruefspeicher != pruef_auswahl){
    Pruefauswahl(pruef_auswahl); 
    schrittspeicher = aktueller_schritt;
    pruefspeicher = pruef_auswahl;

    //Löst Mausklick aus und startet nächsten Prüfschritt am Tester
    if(aktueller_schritt >= 0){ 
     MausklickSteuerung();
     flag = 0;
    }
    
  }


//  if(aktueller_schritt <= 0){     
//    lcd.setCursor (0,1);       
//    lcd.print("                ");  // löscht die zweite LCD Zeile im Schritt 0
//  }


 //Automatisch nächster Schritt nach Ablauf des Countdown und nach Start-Tastendruck
// if(millis() - time > countdown && aktueller_schritt > 0){
//  aktueller_schritt++;
//  time = millis();
//  }

}


/*################### Funktionen ###################*/

void Pruefauswahl(int nummer){

  switch (nummer) {
    case -1:
      break;
    case 0:
      if(aktueller_schritt == -2){
      Serial.println("Pruefung 04\n");
      }
      Pruefung_04();
      lcd.home(); // Setzt Cursor auf 0,0
      lcd.print("Pruefung 04 ");
      break;
    case 1:
      if(aktueller_schritt == -2){
      Serial.println("Pruefung 06\n");
      }
      Pruefung_06();
      lcd.home(); // Setzt Cursor auf 0,0
      lcd.print("Pruefung 06 "); 
      break;
    default:
      Serial.print("Error!\n");
      lcd.home(); // Setzt Cursor auf 0,0
      lcd.print("Error Pruefauswahl");
   }
}

void TastenSteuerung(int relayx, int onoff){
  digitalWrite(relayx, !onoff);
   switch (relayx) {
    case acmax:
      Serial.print("ACMax");    //Relay 1
      lcd.setCursor (0,1);      // Schreibt auf der zweiten Zeile
      lcd.print("                ");
      lcd.setCursor (0,1);       
      lcd.print("ACMax");
      break;
    case ac:
      Serial.print("AC");       //Relay 2
      lcd.setCursor (0,1);      // Schreibt auf der zweiten Zeile
      lcd.print("                ");
      lcd.setCursor (0,1);       
      lcd.print("AC");
      break;
    case umluft:
      Serial.print("Umluft");    //Relay 3
      lcd.setCursor (0,1);      // Schreibt auf der zweiten Zeile
      lcd.print("                ");
      lcd.setCursor (0,1);       
      lcd.print("Umluft");
      break;
    case defrost:
      Serial.print("Defrost");  //Relay 4
      lcd.setCursor (0,1);      // Schreibt auf der zweiten Zeile
      lcd.print("                ");
      lcd.setCursor (0,1);       
      lcd.print("Defrost");
      break;
    default:
      Serial.print("Error!\n");
      lcd.home(); // Setzt Cursor auf 0,0
      lcd.print("Error  ");
   }
   switch (onoff) {
    case an:
      Serial.println(" an");
      lcd.setCursor (13,1);      // Schreibt auf der zweiten Zeile       
      lcd.print(" an");
      break;
    case aus:
      Serial.println(" aus");
      lcd.setCursor (13,1);      // Schreibt auf der zweiten Zeile       
      lcd.print("aus");
      break;
    }
}

void PotiSteuerung(int potix, int stellung) {
  int adresse = 0;   //Poti 0 = wiper0writeAddr // Poti 1 = wiper1writeAddr
  
  switch (potix) {
    case geblaese: 
      Serial.print("Geblaese");
      lcd.setCursor (0,1);      // Schreibt auf der zweiten Zeile       
      lcd.print("                ");
      lcd.setCursor (0,1);
      lcd.print("Geblaese");
      // Geblaese wird mit Chip B, Poti 1 gesteuert
      adresse = wiper1writeAddr;
      digitalWrite(ChipSelectB,LOW);
      break;
    case temp:
      Serial.print("Temperatur");
      lcd.setCursor (0,1);      // Schreibt auf der zweiten Zeile
      lcd.print("                ");
      lcd.setCursor (0,1);       
      lcd.print("Temperatur");
      // Temperatur wird mit Chip B, Poti 0 gesteuert
      adresse = wiper0writeAddr;
      digitalWrite(ChipSelectB,LOW);
      break;
    case defrost_poti:
      Serial.print("Defrost-Poti");
      lcd.setCursor (0,1);      // Schreibt auf der zweiten Zeile       
      lcd.print("                ");
      lcd.setCursor (0,1);
      lcd.print("Defr.-Poti");
      // Defrost-Poti wird mit Chip A, Poti 1 gesteuert
      adresse = wiper1writeAddr;
      digitalWrite(ChipSelectA,LOW);
      break;
    case fussraum_poti:
      Serial.print("Fussraum-Poti");
      lcd.setCursor (0,1);      // Schreibt auf der zweiten Zeile       
      lcd.print("                ");
      lcd.setCursor (0,1);
      lcd.print("Fussr.-Poti");
      // Fussraum-Poti wird mit Chip A, Poti 0 gesteuert
      adresse = wiper0writeAddr;
      digitalWrite(ChipSelectA,LOW);
      break;
    default:
      Serial.print("Error!\n");
      lcd.home(); // Setzt Cursor auf 0,0
      lcd.print("Error  ");
  }

  switch (stellung) {
    case links:
      Serial.println(" links");
      lcd.setCursor (13,1);      // Schreibt auf der zweiten Zeile       
      lcd.print("li.");
      break;
    case mitte:
      Serial.println(" mitte");
      lcd.setCursor (13,1);      // Schreibt auf der zweiten Zeile       
      lcd.print("mi.");
      break;
    case rechts:
      Serial.println(" rechts");
      lcd.setCursor (13,1);      // Schreibt auf der zweiten Zeile       
      lcd.print("re.");
      break;
    } 
  
  SPI.transfer(adresse);
  SPI.transfer(stellung);
  digitalWrite(ChipSelectA,HIGH);
  digitalWrite(ChipSelectB,HIGH);
  
}


void Schrittzaehler() { 
  // Taste wird entprellt und zählt den aktuellen Prüfschritt hoch
  if(millis() - time > entprellen){
    if(pruef_auswahl > -1){
    aktueller_schritt++;
    }
  }
  time = millis();
}

void Pruefauswahlzaehler() { 
  // Taste wird entprellt und zählt den aktuellen Prüfschritt hoch
  if(millis() - time > entprellen){
    if(aktueller_schritt < 0){
      pruef_auswahl++;
    }
  }
  time = millis();
  if(pruef_auswahl > 1){
      pruef_auswahl = 0;
  }
}


void Startsequenz(){
  lcd.home();
  lcd.print("Start");
  delay(500);
  lcd.home();
  lcd.print("Start.");
  delay(500);
  lcd.home();
  lcd.print("Start..");
  delay(500);
  lcd.home();
  lcd.print("Start...");
  delay(500);
}

void Reset(){
  Serial.println("Reset auf Pruefschritt 0");
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor (0,1);
  lcd.print("Reset");
  aktueller_schritt = -1;
  pruef_auswahl = -1;
}

void MausklickSteuerung(){
  delay(150);
  digitalWrite(mausklick, LOW);
  delay(150);
  digitalWrite(mausklick, HIGH);
}

void Pruefschrittanzeige(){
  Serial.print("Pruefschritt: ");
  Serial.println(aktueller_schritt);
  lcd.setCursor (14,0);      // Schreibt auf der zweiten Zeile
  lcd.print("  ");
  if(aktueller_schritt >= 0){
    lcd.setCursor (14,0);       
    lcd.print(aktueller_schritt);
  }
  
}

int Wiederholung(){
  int drehpoti_stellung = map(analogRead(drehpoti), 0, 1023, 0, 4);
  return drehpoti_stellung;
}   


void Pruefung_06(){
  Pruefschrittanzeige();

  switch (aktueller_schritt) {
    case -1:
      break;
    case 0:
      TastenSteuerung(acmax, aus);
      TastenSteuerung(ac, aus);
      TastenSteuerung(umluft, aus);
      TastenSteuerung(defrost, aus);
      PotiSteuerung(temp, links);
      PotiSteuerung(geblaese, links);
      PotiSteuerung(defrost_poti, mitte);
      PotiSteuerung(fussraum_poti, mitte);
      break;
    case 1:
      PotiSteuerung(defrost_poti, rechts);
      break;
    case 2:
      PotiSteuerung(defrost_poti, links);
      break;
    case 3:
      PotiSteuerung(fussraum_poti, rechts);
      break;
    case 4:
      PotiSteuerung(fussraum_poti, links);
      break;
    case 5:
      PotiSteuerung(fussraum_poti, rechts);
      break;
    case 6:
      PotiSteuerung(fussraum_poti, links);
      break;
    case 7:
      TastenSteuerung(umluft, an);
      break;
    case 8:
      TastenSteuerung(umluft, aus);
      break;
    case 9:
      TastenSteuerung(ac, an);
      break;
    case 10:
      TastenSteuerung(ac, aus);
      break;
    case 11:
      PotiSteuerung(temp, rechts);
      break;
    case 12:
      PotiSteuerung(temp, links);
      break;
    case 13:
      PotiSteuerung(temp, rechts);
      break;
    case 14:
      TastenSteuerung(ac, an);
      break;
    case 15:
      TastenSteuerung(ac, aus);
      break;
    case 16:
      PotiSteuerung(temp, links);
      break;
    case 17:
      TastenSteuerung(acmax, an);
      break;
    case 18:
      TastenSteuerung(acmax, aus);
      TastenSteuerung(ac, an);
      break;
    case 19:
      PotiSteuerung(geblaese, mitte);
      break;
    case 20:
      TastenSteuerung(defrost, an);
      break;
    case 21:
      TastenSteuerung(defrost, aus);
      break;
    case 22:
      TastenSteuerung(acmax, an);
      break;
    case 23:
      TastenSteuerung(acmax, aus);
      PotiSteuerung(geblaese, rechts);
      break;
    case 24:
      PotiSteuerung(temp, mitte);
      break;      
    default:
      Reset();
  }
  Serial.println();
}

void Pruefung_04(){
  Pruefschrittanzeige();

  switch (aktueller_schritt) {
    case -1:
      break;
    case 0:
      TastenSteuerung(acmax, aus);
      TastenSteuerung(ac, aus);
      TastenSteuerung(umluft, aus);
      TastenSteuerung(defrost, aus);
      PotiSteuerung(temp, links);
      PotiSteuerung(geblaese, links);
      PotiSteuerung(defrost_poti, mitte);
      PotiSteuerung(fussraum_poti, mitte);
      break;
    case 1:
      PotiSteuerung(defrost_poti, rechts);
      break;
    case 2:
      PotiSteuerung(defrost_poti, links);
      break;
    case 3:
      PotiSteuerung(fussraum_poti, rechts);
      break;
    case 4:
      PotiSteuerung(fussraum_poti, links);
      break;
    case 5:
      TastenSteuerung(umluft, an);
      break;
    case 6:
      TastenSteuerung(umluft, aus);
      break;
    case 7:
      PotiSteuerung(temp, rechts);
      break;
    case 8:
      PotiSteuerung(temp, links);
      break;
    case 9:
      PotiSteuerung(temp, rechts);
      break;
    case 10:
      TastenSteuerung(ac, an);
      break;
    case 11:
      TastenSteuerung(ac, aus);
      break;
    case 12:
      PotiSteuerung(temp, links);
      break;
    case 13:
      TastenSteuerung(ac, an);
      break;
    case 14:
      PotiSteuerung(geblaese, mitte);
      break;
    case 15:
      TastenSteuerung(defrost, an);
      break;
    case 16:
      TastenSteuerung(defrost, aus);
      break;
    case 17:
      TastenSteuerung(acmax, an);
      break;
    case 18:
      TastenSteuerung(acmax, aus);
      PotiSteuerung(geblaese, rechts);
      break;
    case 19:
      PotiSteuerung(temp, mitte);
      break;      
    default:
      Reset();
  }
  Serial.println();
}

