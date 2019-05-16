Neue Fassung ohne SD card und mit EEPROM.
Die Idee ist : 
Schalter Nummer 2 (repo["p"]==2) auf dem Arduino Nummer 1 (repo["r"]==1) ist ein Input (repo["n"]==1) und befiehl LED Nummer 5 (repo["c"] == 5)  auf dem Arduino 1 (repo["d"]==1) und die Funktion Nummer 1 (repo["a"]==1) wird eingesetzt.

  "p"  |  "r"  |  "n"  |  "c"  |  "a"  |  "d"  |
================================================
   2   |   1   |   1   |   5   |   1   |   1   |           ====>>> Schalter 2 befiehl LED 5
------------------------------------------------
    5  |    1  |   2   |       |       |       |            ====>>> LED 5
------------------------------------------------
 die anderen ....
------------------------------------------------

"p" == 2, 3 , 4 , ....  : Pin
"r" == 1, 2, 3 : Arduino Nummer 1, 2 oder 3
"n" == 1 :  (INPUT) 2 (OUTPUT) oder 3 (Sensor)
"c" == 5 6 7 8 ...  : durch "p" befohlene Leds
"a" == 0, 1, 2 : durch Eindruck von pin eingesetzte Funktion
"d" == 0 1 2 : Arduino Nummer wo LED gestochen wird

Sehen wiring.jpg für Anschluss