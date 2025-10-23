#include <rgb_lcd.h>    // Bibliothek zum Ansteuern des Grove-LED Display's

/*------------------- Definition der PIN's -------------------*/
#define interruptPin 2  // Interrupt PIN2 (D2)
#define ledPin 4        // Output LED-PIN4 (D4) 

/*------------------- Globale Variablen -------------------*/
rgb_lcd lcd;          // Definition des LED-Displays (Ansteuerung ueber I2C-Bus)
int currentFinger;     // Definiert den Messzustand des Systems
enum fingerState {littleFinger= 0, ringFinger,middleFinger,indexFinger,thumb};  // Enum --> Zu besseren Lesbarkeit des Codes (Um Zustand zu definieren)
static const char *fingerTypes[] ={(char*)"little Finger",(char*)"ring Finger",(char*)"middle Finger",(char*)"index Finger",(char*)"thumb",}; // Dient fuer die Anzeige auf dem LED-Display

unsigned long startMillis,currentMillis;  // Varialblen, um Refresh-Times des LED-Display's zu setzen (Simuliert einen Timer)

/*------------------- Definition des Anfangszustands des EMG-Sensors -------------------*/
int max_analog_dta = 300; // max analog data
int min_analog_dta = 100; // min analog data
int static_analog_dta= 0; // static analog data

//------------------- EMG-Sensor Methoden -------------------
/* initialEMGSensor : Initialisert den EMG-Sensor bei Programmstart
*/
void initialEMGSensor()
{
    long sum = 0;
    for(int i=0; i<=10; i++)
    {
        for(int j=0; j<100; j++)
        {
            sum += getAnalog(A0);
            delay(1);
        }
    }
    sum /= 1100;
    static_analog_dta = sum;
}
/*
 *getAnalog : Auslesen des Sensors
 *param* pin : Uebergabe des PIN's, ueber welchen der EMG-Sensor angeschlossen ist
 *return* Aktuell anliegender Wert am EMG-Sensor
*/
int getAnalog(int pin)
{
    long sum = 0;
    for(int i=0; i<32; i++)
    {
        sum += analogRead(pin);
    };
    int dta = sum>>5;
    max_analog_dta = dta>max_analog_dta ? dta : max_analog_dta;
    min_analog_dta = min_analog_dta>dta ? dta : min_analog_dta;

    return sum>>5;
}
/*touchInterrupt: Interrupt-Funktion fuer den Touch-Sensor.
 * Bei Eventeintritt wird currentFinger hochgezaehlt (ein Finger weitergesprungen), sodass eine weitete Messung erfolgen kann
*/
void touchInterrupt()
{
  noInterrupts();   // Keine Interrupts zulassen
  digitalWrite(ledPin, LOW);  // LED wird ausgeschaltet
  interrupts();     // Interrupts zulassen
  delay(2000);      // Verzögerung 2 sek
  if(currentFinger != thumb)  currentFinger ++; // ist der currentFinger != Daumen --> wird hochgezaehlt
  else currentFinger = littleFinger;            // anderenfalls wird currentFinger = kleiner Finger gesetzt
  noInterrupts();   // Keine Interrupts zulassen
  digitalWrite(ledPin, HIGH); // LED wird eingeschaltet
  interrupts();     // Interrupts zulassen
}

/*setup-Methode: Wird beim Starten des Arduinos aufgerufen, um Parameter zu initialsieren
 * Defintion der Baudrate (Kommuniaktionsgeschwindigkeit) zwischen Arduino und PC
 * Initialisierung des LED-Display's
 * Initialisierung der PIN's
 * Initialisierung des EMG-Sensors
*/
void setup() {
  Serial.begin(115200);
  lcd.begin(16,2);
  lcd.setRGB(255,255,255);
  lcd.print("Initial!");
  initialEMGSensor();

  pinMode(ledPin, OUTPUT);  // Festegung des LED-PIN's als Output-PIN
  attachInterrupt(digitalPinToInterrupt(interruptPin), touchInterrupt, FALLING); // Definition des Interrupt-PIN's. Event wird bei fallender Flanke ausgelöst

  currentFinger = littleFinger;
  delay(2000);
  Serial.print("Wert\tAktuellerFinger\n");
  startMillis = millis();
  digitalWrite(ledPin, HIGH);
}

/* loop-Methode: Methode, die zur Laufzeit in einer Endlosschleife vom Arduino ausgefuehrt wird.
*/

void loop() {
  currentMillis = millis();
  int emgSignal = getAnalog(A0) -  static_analog_dta;
  Serial.print(emgSignal);
  Serial.print("\t");
  Serial.print((int)currentFinger);
  Serial.println();
  if(currentMillis - startMillis > 1000 || startMillis == 0)
  {
    startMillis = currentMillis;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(fingerTypes[currentFinger]);
    lcd.setCursor(0, 1);
    lcd.print(emgSignal);
  }
  delay(1);
}
