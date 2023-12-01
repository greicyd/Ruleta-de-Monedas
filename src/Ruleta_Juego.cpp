#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Print.h>
#include <MFRC522Extended.h>
#include <deprecated.h>
#include <require_cpp11.h>
#include <Adafruit_MCP23X08.h>
#include <LiquidCrystal_I2C.h>
#include <FastLED.h>


//**LCD**//
//Defino el número de columnas y filas de mi LCD
#define COLUMS 20
#define ROWS   4
#define PAGE   ((COLUMS) * (ROWS))

//**JOYSTICK//
//Pines a definir en el Joystick
const int joystickButtonPin = 14;
const int joyStickXPin = 34;
const int joyStickYPin = 35;
//variables para controlar el joystick
int aState;
//Menu
int Estado = 1;
int sig_estado = 1;
int estadoMenu = 0; //Muestra el menú o pantalla principal

//**LEDS**//
#define LED_PIN     32  // Pin de datos de la tira de LED WS2812B
#define NUM_LEDS    30 // Número de LEDs en la tira
#define BRIGHTNESS  100 // Brillo de los LEDs (0-255)

CRGB leds[NUM_LEDS]; // Array para almacenar el estado de cada LED

LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);

void menuPrincipal();


void setup()
{
  Serial.begin(115200);

  while (lcd.begin(COLUMS, ROWS) != 1) //colums - 20, rows - 4
  {
    Serial.println(F("PCF8574 is not connected or lcd pins declaration is wrong. Only pins numbers: 4,5,6,16,11,12,13,14 are legal."));
    delay(5000);   
  }
  pinMode(joyStickXPin,INPUT);
  pinMode(joyStickYPin,INPUT);
  pinMode(joystickButtonPin, INPUT_PULLUP);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);


  lcd.setCursor(1,0);	//columna - fila
  lcd.print("JUEGO DE LA RULETA");
  lcd.setCursor(4,1);
  lcd.print("TRAGAMONEDAS");
  lcd.setCursor(3,3);
  lcd.print("BY GRACE-VICTOR");
  delay(5000);
  lcd.clear();
  
}

void loop() {
  menuPrincipal();
}

int funcionMenu(String *arrayMenu, int size){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(">");
  float opcion = 1;
  int extraOpcion = 0;

  for(int x = 0; x < size && x <= 2; x++){
    lcd.setCursor(1,x);
    lcd.print(arrayMenu[x]);
  }
  delay(500);

  while(digitalRead(joystickButtonPin) == HIGH){
    aState = analogRead(joyStickYPin);
    delay(100);

    if(aState > 3200 || aState < 1000){
      if(aState > 3200){
        Serial.println("Abajo");
        if(opcion < size){
          opcion += 1;
          Serial.println(opcion);
        }
      }
      if(aState < 1000){
        Serial.println("Arriba");
        if(opcion > 1){
          opcion -= 1;
          Serial.println(opcion);
        }
      }

        lcd.clear();

      for (int x = extraOpcion; x < size && x <= 3; x++){
        lcd.setCursor(1,x -extraOpcion);
        lcd.print(arrayMenu[x]);
      }
      lcd.setCursor(0, opcion -extraOpcion -1);
      lcd.print(">");
    }
  }
  return opcion;
}

void menuPrincipal(){

  Estado = sig_estado;

  if (Estado == 1){
    int menu;
    String arrayMenu[] = {"1. Jugar", "2. Consultar saldo", "3. Salir"};
    menu = funcionMenu(arrayMenu, 3);
    if (menu == -1)
      sig_estado = 1;
    else if (menu == 1)
      sig_estado = 2;
    else if (menu == 2)
      sig_estado = 3;
    else if (menu == 3)
      sig_estado = 4;

  }
  else if (Estado == 2){
    lcd.clear();
    lcd.print("A jugar! :)");
    delay(1000);

    lcd.clear();
    estadoMenu = 0;
    sig_estado = 1;
  }
  else if (Estado == 3){
    lcd.clear();
    lcd.print("Su saldo es : $100");
    delay(1000);
    
    lcd.clear();
    estadoMenu = 0;
    sig_estado = 1;
  }
  else if (Estado == 4){
    lcd.clear();
    lcd.print("Saliendo..");
    delay(1000);
    
    lcd.clear();
    estadoMenu = 0;
    sig_estado = 1;
  }
}