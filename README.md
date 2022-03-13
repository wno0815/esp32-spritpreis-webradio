# esp32-spritpreis-webradio

*Note for readers outside Germany: This project uses the API from [Tankerkönig](https://creativecommons.tankerkoenig.de/) for fuel/gas prices - only available in Germany.*

Webradio mit Benzinpreisanzeige und -Überwachung basierend auf ESP32, einem VS1053 MP3-Decoder und Nextion 3.5"-Display.

Funktionen:
- Webradio
- Benzinpreisüberwachung mit Warnton und Sprachausgabe
- Uhr

Sonderfunktionen:
- Konfiguration über JSON-Dateien
- Flashen des Nextion aus dem Dateisystem (LITTLEFS) des ESP
- kein Webserver!

Code: `radio/`
Konfigurationsdateien: `radio/data`
HMI: `hmi/`
Dokumentation: `doc/`
