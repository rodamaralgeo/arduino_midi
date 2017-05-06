// Axe-FX midi.controller 03-Oct-2016 V4.1
// Edited for use with Guitix by Rodrigo Amaral
// Originnal post: http://forum.fractalaudio.com/threads/diy-axe-fx-foot-controller-for-50-open-project.119882/
// Inclue CC 32 For Change Preset
// Remove AxeFx notes

#include <MIDI.h>
#include <LiquidCrystal.h>

MIDI_CREATE_DEFAULT_INSTANCE();

// define os pinos digitais das chaves

#define SWITCH1 23
#define SWITCH2 25
#define SWITCH3 27
#define SWITCH4 29
#define SWITCH5 31
#define SWITCH6 33
#define SWITCH7 35
#define SWITCH8 37
#define SWITCH9 39
#define SWITCH10 41
#define SWITCH11 43
#define SWITCH12 45
#define SWITCH13 14
#define SWITCH14 16
#define SWITCH15 18
#define SWITCH16 20


// define os pinos digitais dos leds
#define LED1 22
#define LED2 24
#define LED3 26
#define LED4 28
#define LED5 30
#define LED6 32
#define LED7 34
#define LED8 36
#define LED9 38
#define LED10 40
#define LED11 42
#define LED12 44
#define LED13 15
#define LED14 17
#define LED15 19
#define LED16 21


#define BankCC 32 // CC for change Bank
#define BOUNCEDELAY 200

#define NUM_AI 16
#define USED_AI 16 //Define quantas portas analógicas serão lidas. 
#define MIDICHAN_POT 15 //Define qual canal MIDI será utilizado pelos Poteciometro
#define MIDICHAN 16 //Define qual canal MIDI será utilizado pelos Botões
#define MIDI_CC 0
#define FILTER_AMOUNT 3
#define ANALOGUE_INPUT_CHANGE_TIMEOUT 250000 // Timeout is in microseconds

//LIGAÇÃO DO LCD
//PIN 01 - VSS - GROUND
//PIN 02 - VDD - 5V
//PIN 03 - VE - CONTRAST VOLTAGE (TRIMPOT OUTPUT)
//PIN 04 - RS - REGISTER SELECT - PIN D2
//PIN 05 - RW - READ/WRITE - GND
//PIN 06 - ENABLE - PIN D3
//PIN 07 - NOTHING
//PIN 08 - NOTHING
//PIN 09 - NOTHING
//PIN 10 - NOTHING
//PIN 11 - DATA 4 - PIN D4
//PIN 12 - DATA 5 - PIN D5
//PIN 13 - DATA 6 - PIN D6
//PIN 14 - DATA 7 - PIN D7
//PIN 15 - BACKLIGTH ANODE - 5V
//PIN 16 - BACKLIGTH CATODE - GROUND


LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

// Variables:

bool fxactive[16] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
int switches[16] = {SWITCH1, SWITCH2, SWITCH3, SWITCH4, SWITCH5, SWITCH6, SWITCH7, SWITCH8, SWITCH9, SWITCH10, SWITCH11, SWITCH12, SWITCH13, SWITCH14, SWITCH15, SWITCH16};
int switchState[16] = {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH, HIGH};
// Initial state of switch is high due to internal pullup
int leds[16] = {LED1, LED2, LED3, LED4, LED5, LED6, LED7, LED8, LED9, LED10, LED11, LED12, LED13, LED14, LED15, LED16};

int currentSwitch = 0;
int pedalActiveFlash = 50; // Delay for flash when pedal is pressed
int Preset = 0;
int PresetMax = 10;
int Bank = 0;
int BankMax = 10;

int type;

byte analogueInputMapping[NUM_AI] = {A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15};
int analogueInputNote[NUM_AI] = {50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65};
int analogueValueMax[NUM_AI] = {1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023, 1023};
int analogueValueMin[NUM_AI] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int analogueInputs[NUM_AI]; // Variable to hold temporary analogue values, used for analogue filtering logic.
byte tempAnalogueInput; // Preallocate the for loop index so we don't keep reallocating it for every program iteration.
byte i = 0;
byte analogueDiff = 0; // This is used as a flag to indicate that an analogue input is changing.
boolean analogueInputChanging[NUM_AI]; // Time the analogue input was last moved
unsigned long analogueInputTimer[NUM_AI];


int updatefx()
{
  if (fxactive[currentSwitch] == false)
  {
    MIDI.sendControlChange(currentSwitch - 5, 127, MIDICHAN);
    digitalWrite( leds[currentSwitch], HIGH);
    lcd.setCursor(9, 0);
    lcd.print("FX");
    lcd.print(currentSwitch - 4);
    lcd.print(" ON");
    fxactive[currentSwitch] = true;
  }
  else
  {
    MIDI.sendControlChange(currentSwitch - 5, 0, MIDICHAN);
    digitalWrite( leds[currentSwitch] , LOW);
    lcd.setCursor(9, 0);
    lcd.print("FX");
    lcd.print(currentSwitch - 4);
    //lcd.setCursor(13, 0);
    //lcd.print("OFF");
    //delay(2000);
    lcd.setCursor(9, 0);
    lcd.print("       ");
    fxactive[currentSwitch] = false;
  }
}

int updatelcd()
{
// LEITURA MIDI (EM IMPLEMENTAÇÃO)
//  if (MIDI.read())
//  {
//    type = MIDI.getType();
//    if (type == midi::ControlChange)
//    {
//      Bank = MIDI.getData2();
//    }
//    else if (type == midi::ProgramChange)
//    {
//      Preset = MIDI.getData1();
//    }
//  }

  //LIMPA A PARTE DE PRESET E BANCO DO LCD
  lcd.setCursor(0, 0);
  lcd.print("     ");
  lcd.setCursor(0, 1);
  lcd.print("     ");

  //LIMPA O TEXTO DOS EFEITOS DO LCD
  lcd.setCursor(9, 0);
  lcd.print("       ");

  //EXIBE O BANCO E PRESET ATUAL NO LCD
  lcd.setCursor(0, 0);
  lcd.print("B ");
  lcd.print(Bank);
  lcd.setCursor(0, 1);
  lcd.print("P ");
  lcd.print(Preset);

  fxactive[5] = false;
  fxactive[6] = false;
  fxactive[7] = false;
  fxactive[8] = false;
  fxactive[9] = false;

  digitalWrite( leds[5] , LOW);
  digitalWrite( leds[6] , LOW);
  digitalWrite( leds[7] , LOW);
  digitalWrite( leds[8] , LOW);
  digitalWrite( leds[9] , LOW);


  if (Preset == 0) {
    digitalWrite( LED1, HIGH);
    digitalWrite( LED2, LOW);
    digitalWrite( LED3, LOW);
    digitalWrite( LED4, LOW);
    digitalWrite( LED5, LOW);
  }
  else if (Preset == 1) {
    digitalWrite( LED1, LOW);
    digitalWrite( LED2, HIGH);
    digitalWrite( LED3, LOW);
    digitalWrite( LED4, LOW);
    digitalWrite( LED5, LOW);
  }
  else if (Preset == 2) {
    digitalWrite( LED1, LOW);
    digitalWrite( LED2, LOW);
    digitalWrite( LED3, HIGH);
    digitalWrite( LED4, LOW);
    digitalWrite( LED5, LOW);
  }
  else if (Preset == 3) {
    digitalWrite( LED1, LOW);
    digitalWrite( LED2, LOW);
    digitalWrite( LED3, LOW);
    digitalWrite( LED4, HIGH);
    digitalWrite( LED5, LOW);
  }
  else if (Preset == 4) {
    digitalWrite( LED1, LOW);
    digitalWrite( LED2, LOW);
    digitalWrite( LED3, LOW);
    digitalWrite( LED4, LOW);
    digitalWrite( LED5, HIGH);
  }
  else {
    digitalWrite( LED1, LOW);
    digitalWrite( LED2, LOW);
    digitalWrite( LED3, LOW);
    digitalWrite( LED4, LOW);
    digitalWrite( LED5, LOW);
  }
}

void setup() {

  //  LCD Initial Message
  lcd.clear();
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Guitarix ");
  lcd.setCursor(0, 1);
  lcd.print("Controller v0.0");
  delay(2000); // 2 Segundos

  lcd.clear();
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Waiting for");
  lcd.setCursor(0, 1);
  lcd.print("Guitarix...");
  delay(4000); // 4 Segundos
  lcd.clear();

  //Set MIDI baud rate:
  //Serial.begin(115200);
  //Serial.begin(31250);
  //Serial.begin(9600);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();

  //   Setup Switches and activation LEDs
  for ( currentSwitch = 0; currentSwitch < 16; currentSwitch++ ) {
    pinMode( switches[currentSwitch], INPUT_PULLUP);          // Set pin for switch
    digitalWrite( switches[currentSwitch], HIGH );      // Turn on internal pullup
    pinMode( leds[currentSwitch], OUTPUT );             // Set pin for LED
    fxactive[currentSwitch] = false;
    flashPin( leds[currentSwitch], 100 ); // Flash LED
  }
  // Setup AnalogPins an Filt
  for (i = 0; i < NUM_AI; i++)
  {
    // Set the pin direction to input.
    pinMode(analogueInputMapping[i], INPUT);
    // Initialise the analogue value with a read to the input pin.
    analogueInputs[i] = analogRead(analogueInputMapping[i]);
    // Assume no analogue inputs are active
    analogueInputChanging[i] = false;
    analogueInputTimer[i] = 0;
  }
  
//  MIDI.sendControlChange(127, 0, MIDICHAN); // Inicializa o MIDI do Guitarix
//  MIDI.sendControlChange(BankCC, 0, MIDICHAN);
//  MIDI.sendProgramChange(0, MIDICHAN);

  //Atualiza o LCD Com Bank 0 e Preset 0
  updatelcd();

}



void loop() {

  // FAZ A LEITURA DAS CHAVES E ATIVAÇÃO DOS LEDS
  for ( currentSwitch = 0; currentSwitch < 16; currentSwitch++ ) {
    if ((digitalRead(switches[currentSwitch]) != switchState[currentSwitch] ) && (switchState[currentSwitch] == HIGH)) {

      switch ( currentSwitch ) {
        // Switch 1.                            Select preset on switch 1
        case 0:
          Preset = 0;
          MIDI.sendProgramChange(0, MIDICHAN);
          updatelcd();
          break;
        // Switch 2.                            Select preset on switch 2
        case 1:
          Preset = 1;
          MIDI.sendProgramChange(1, MIDICHAN);
          updatelcd();
          break;
        // Switch 3.                            Select preset on switch 3
        case 2:
          Preset = 2;
          MIDI.sendProgramChange(2, MIDICHAN);
          updatelcd();
          break;
        // Switch 4.                            Select preset on switch 4
        case 3:
          Preset = 3;
          MIDI.sendProgramChange(3, MIDICHAN);
          updatelcd();
          break;
        // Switch 5.                            Select preset on switch 5
        case 4:
          Preset = 4;
          MIDI.sendProgramChange(4, MIDICHAN);
          updatelcd();
          break;
        // Switch 6.                            Fx 1
        case 5:
          updatefx();
          break;
        // Switch 7.                            Fx 2
        case 6:
          updatefx();
          break;
        // Switch 8.                            Fx 3
        case 7:
          updatefx();
          break;
        // Switch 9.                            Fx 4
        case 8:
          updatefx();
          break;
        // Switch 10.                          Fx 5
        case 9:
          updatefx();
          break;
        // Switch 11.                          Fx 6
        case 10:
          if (fxactive[currentSwitch] == false)
          {
            MIDI.sendControlChange(5, 127, MIDICHAN);
            digitalWrite(leds[currentSwitch], HIGH);
            lcd.setCursor(11, 1);
            lcd.print("TUNER");
            fxactive[currentSwitch] = true;
          }
          else
          {
            MIDI.sendControlChange(5, 0, MIDICHAN);
            digitalWrite(leds[currentSwitch], LOW);
            lcd.setCursor(11, 1);
            lcd.print("     ");
            fxactive[currentSwitch] = false;
          }
          break;
        // Switch 12.                          Fx 7
        case 11:
          if (fxactive[currentSwitch] == false)
          {
            MIDI.sendControlChange(6, 127, MIDICHAN);
            digitalWrite(leds[currentSwitch], HIGH);
            lcd.setCursor(11, 1);
            lcd.print(" MUTE ");
            fxactive[currentSwitch] = true;
          }
          else
          {
            MIDI.sendControlChange(6, 0, MIDICHAN);
            digitalWrite(leds[currentSwitch], LOW);
            lcd.setCursor(11, 1);
            lcd.print("     ");
            fxactive[currentSwitch] = false;
          }
          break;
        // Switch 13.                         Bank Up
        case 12:
          if (Bank < BankMax)
          {
            Bank = Bank + 1;
          }
          else
          {
            Bank = 0;
          }
          Preset = 0;
          MIDI.sendControlChange(BankCC, Bank, MIDICHAN);
          MIDI.sendProgramChange(0, MIDICHAN);
          updatelcd();
          break;

        // Switch 14.                         Bank Down
        case 13:
          if (Bank > 0)
          {
            Bank = Bank - 1;
          }
          else
          {
            Bank = BankMax;
          }
          Preset = 0;
          MIDI.sendControlChange(BankCC, Bank, MIDICHAN);
          MIDI.sendProgramChange(0, MIDICHAN);
          updatelcd();


          break;

        // Switch 15.                        Preset Down
        case 14:
          if (Preset > 0)
          {
            Preset = Preset - 1;
          }
          else
          {
            Preset = PresetMax;
          }

          flashPin( leds[currentSwitch], pedalActiveFlash );
          MIDI.sendProgramChange(Preset, MIDICHAN);
          updatelcd();
          break;

        // Switch 15.                        Preset UP
        case 15:
          if (Preset < PresetMax)
          {
            Preset = Preset + 1;
          }
          else
          {
            Preset = 0;
          }
          flashPin( leds[currentSwitch], pedalActiveFlash );
          MIDI.sendProgramChange(Preset, MIDICHAN);
          updatelcd();
          break;
      }
      delay( BOUNCEDELAY );
    }
    switchState[currentSwitch] = digitalRead( switches[currentSwitch] );
  }


  // FAZ A LEITURA DOS POTENCIOMETROS =========================================================================================================================================
  for (i = 0; i < USED_AI; i++) //
  {
    // Read the analogue input pin, dividing it by 8 so the 10-bit ADC value (0-1023) is converted to a 7-bit MIDI value (0-127).
    tempAnalogueInput = analogRead(analogueInputMapping[i]) / 8;
    //tempAnalogueInput = map(analogRead(analogueInputMapping[i]), analogueValueMin[NUM_AI], analogueValueMax[NUM_AI], 0, 127);
    analogueDiff = abs(tempAnalogueInput - analogueInputs[i]);
    // Only continue if the threshold was exceeded, or the input was already changing
    if ((analogueDiff > 0 && analogueInputChanging[i] == true) || analogueDiff >= FILTER_AMOUNT)
    {
      // Only restart the timer if we're sure the input isn't 'between' a value
      // ie. It's moved more than FILTER_AMOUNT
      if (analogueInputChanging[i] == false || analogueDiff >= FILTER_AMOUNT)
      {
        // Reset the last time the input was moved
        analogueInputTimer[i] = micros();
        // The analogue input is moving
        analogueInputChanging[i] = true;
      }
      else if (micros() - analogueInputTimer[i] > ANALOGUE_INPUT_CHANGE_TIMEOUT)
      {
        analogueInputChanging[i] = false;
      }
      // Only send data if we know the analogue input is moving
      if (analogueInputChanging[i] == true)
      {
        // Record the new analogue value
        analogueInputs[i] = tempAnalogueInput;
        // Send the analogue value out on the general MIDI CC (see definitions at beginning of this file)
        //controlChange(MIDICHAN, analogueInputNote[i], analogueInputs[i]);
        MIDI.sendControlChange(analogueInputNote[i], analogueInputs[i], MIDICHAN_POT);
      }
    }
  }


}
//====== FUNÇAO QUE FAZ O LED PISCAR =============================================================================================================
void flashPin( int ledPin, int flashDelay ) {
  digitalWrite( ledPin, HIGH );
  delay( flashDelay );
  digitalWrite( ledPin, LOW );
}

