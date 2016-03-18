# porsche-automatik-bedienteil
GitHub repository for the automation project "Porsche Automatik Bedienteil".

==============================================
Porsche Klimasteuergeräte Automatik Bedienteil

Das PKAB übernimmt die Einstellung des Bedienteils für die
verschiedenen Prüfschritte des Porsche Testers. Dabei werden
die Tasten mittels 4-Kanal-Relais geschaltet und die
Potentiometer mittels zwei Digital-Potentionmetern (MCP4261)
gesteuert.
  
Das MCP4261-Digital-Potentionmeter hat zwei Kanäle, ist
SPI-gesteuert und die Steuerung funktioniert mit zwei Bytes.
Das erste sendet die Adresse des Poti-Kanals (Poti 0 oder
Poti 1) und das zweite Byte sendet den Widerstandswert,
der von 0 bis 255 eingestellt werden kann. Es hat einen 
Einstellbereich von 0 bis 10kOhm.

Arduino Pinbelegung:
* CS für Chip A (SS Pin) - Digital Pin 10
* CS für Chip B (SS Pin) - Digital Pin 9
* SDI (MOSI Pin)         - Digital Pin 11
* CLK (SCK Pin)          - Digital Pin 13


Digital-Potentiometer:
* Chip A - Defrost- und Fussraumpoti
* Chip B - Geblaese- und Temperaturpoti


August 2015
Sebastian Schult

==============================================