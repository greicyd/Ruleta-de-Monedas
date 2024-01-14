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
#include <LittleFS.h>
#include <WiFi.h>
#include <SD.h>
#include <Update.h>
//#include <FirebaseESP32.h>


const char *ssid = "Celerity_lara";
const char *password = "vicflara09";



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
#define LED_PIN    32  // Pin de datos del LED
#define NUM_LEDS   30 // Número total de LEDs
#define MAX_SELECTED_LED 4

CRGB leds[NUM_LEDS];

//**VARIABLES DE LA FUNCIÓN JUEGO RULETA**

const int threshold = 200;// Umbral para considerar movimiento en el joystick
int currentPosition = 0;// Posición actual del joystick
int ledsEncendidos = 0;// Número actual de LEDs encendidos
const int maxLedsEncendidos = 4;// Número máximo de LEDs que se pueden encender simultáneamente
bool joystickPressed = false;// Variable para verificar si el joystick ha sido presionado
bool ledsSeleccionados[NUM_LEDS] = {false};// Array para registrar los LEDs seleccionados
int ledsSeleccionadosCount = 0;// Contador de LEDs seleccionados
int siguientePosicion = 0;// Próxima posición a la que se moverá el programa
bool coincidencias[NUM_LEDS];// Array para almacenar información sobre coincidencias
int intentos = 3;// Número de intentos disponibles
int ledsAcertados = 0;// Número de LEDs acertados
bool ledsCompletos = false;// Indicador de si todos los LEDs han sido encendidos
bool ledsImprimidos[NUM_LEDS] = {false};// Bandera para controlar los LEDs ya impresos



LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);

void menuPrincipal();
void setupLEDs();


void reiniciarJuego() {
  ledsAcertados = 0; // Reinicia el contador de LEDs acertados
  intentos = 3; // Reinicia el número de intentos disponibles
  ledsCompletos = false; // Reinicia el indicador de si todos los LEDs han sido encendidos
  ledsSeleccionadosCount = 0; // Reinicia el contador de LEDs seleccionados
  currentPosition = 0; // Reinicia la posición actual del joystick
  memset(ledsImprimidos, 0, sizeof(ledsImprimidos)); // Reinicia la bandera de LEDs ya impresos
  memset(ledsSeleccionados, 0, sizeof(ledsSeleccionados)); // Reinicia la matriz de LEDs seleccionados
  memset(coincidencias, 0, sizeof(coincidencias)); // Reinicia la matriz de coincidencias
}



void setup()
{
  Serial.begin(115200);
  
    // Conectar a la red WiFi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }

  Serial.println("Conectado a la red WiFi");


  while (lcd.begin(COLUMS, ROWS) != 1) //colums - 20, rows - 4
  {
    Serial.println(F("PCF8574 is not connected or lcd pins declaration is wrong. Only pins numbers: 4,5,6,16,11,12,13,14 are legal."));
    delay(5000);   
  }
  pinMode(joyStickXPin,INPUT);
  pinMode(joyStickYPin,INPUT);
  pinMode(joystickButtonPin, INPUT_PULLUP);

  setupLEDs();
  

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







void juegoRuleta() {
  FastLED.clear(); // Apaga todos los LEDs

  int xVal = analogRead(joyStickXPin);
  int yVal = analogRead(joyStickYPin);

  int xDirection = 0;
  int yDirection = 0;

  if (xVal < threshold) {
    xDirection = -1; // Mover a la izquierda
  } else if (xVal > 4295 - threshold) {
    xDirection = 1; // Mover a la derecha
  }

  if (yVal < threshold) {
    yDirection = -1; // Mover hacia arriba
  } else if (yVal > 3325 - threshold) {
    yDirection = 1; // Mover hacia abajo
  }

  currentPosition += xDirection + yDirection;

  if (currentPosition >= NUM_LEDS) {
    currentPosition = 0; // Reiniciar al llegar al final
  } else if (currentPosition < 0) {
    currentPosition = NUM_LEDS - 1; // Reiniciar al llegar al inicio
  }
  
  // Enciende el LED en la posición actual con un color diferente
  leds[currentPosition] = CRGB::Green;
  FastLED.show();

  
    bool joystickActive = digitalRead(joystickButtonPin) == LOW;

  if (joystickActive && !joystickPressed) {
    joystickPressed = true;

    if (!ledsSeleccionados[currentPosition] && ledsSeleccionadosCount < maxLedsEncendidos) {
      ledsSeleccionados[currentPosition] = true; // Marca el LED como seleccionado
      leds[currentPosition] = CRGB::Blue; // Enciende el LED en la posición actual
      ledsSeleccionadosCount++; // Incrementa el contador de LEDs seleccionados
    }
      siguientePosicion = (currentPosition + 1) % NUM_LEDS; // Siguiente posición circular
      //leds[siguientePosicion] = CRGB::Green; // Enciende el siguiente LED con el color original (verde)
      currentPosition = siguientePosicion; // Actualiza la posición actual
    }
     // Verifica si se han seleccionado los 4 LEDs
      
 

  if (!joystickActive) {
    joystickPressed = false;
  }
  
 
 // Lógica para encender o apagar el LED en la posición actual
    if (ledsSeleccionadosCount == 4) {
      lcd.clear(); // Limpiar la pantalla LCD
      lcd.setCursor(0, 0); // Establecer cursor en la primera línea

        for (int i = 0; i < NUM_LEDS; ++i) {
            if (ledsSeleccionados[i]) {
                leds[i] = CRGB::Blue; // Enciende los LEDs seleccionados en azul
                
                // Imprime solo si el LED no se ha impreso previamente
                if (!ledsImprimidos[i]) {
                    lcd.print("LED ");
                    lcd.print(i);
                    lcd.println(" seleccionado");
                    ledsImprimidos[i] = true; // Marca el LED como ya impreso
                    delay(1000);
                    
                }
            } else {
                leds[i] = CRGB::Black; // Apaga los LEDs no seleccionados
            }
            
        }
       


        
        leds[currentPosition] = CRGB::Orange;
        FastLED.show(); // Muestra los LEDs actualizados

        unsigned long tiempoInicio = millis(); // Guarda el tiempo actual
        unsigned long tiempoActual = millis(); // Variable para rastrear el tiempo actual
        bool coincidencia = false; // Variable para rastrear si hay coincidencia
        int ledCoincidente = -1; // Variable para guardar el LED coincidente

        // Mover el LED naranja aleatoriamente por 3 segundos
        while (tiempoActual - tiempoInicio < 3000) {
          leds[currentPosition] = CRGB::Black; // Apaga el LED actual
          currentPosition = random(NUM_LEDS); // Mueve aleatoriamente el LED
          leds[currentPosition] = CRGB::Orange; // Enciende el siguiente LED
          FastLED.show(); // Muestra los LEDs actualizados
          delay(200); // Retardo para la animación del movimiento
          tiempoActual = millis(); // Actualiza el tiempo actual

          // Verifica si el LED naranja coincide con uno de los LEDs azules
          if (ledsSeleccionados[currentPosition]) {
            coincidencia = true;
            ledCoincidente = currentPosition; // Guarda la posición del LED coincidente
            break; // Sale del bucle si hay coincidencia
          }
        }


        // Si hubo coincidencia justo después de los 3 segundos, apaga el LED coincidente y muestra por Serial
        if (coincidencia) {
          lcd.clear(); // Limpiar la pantalla LCD
          lcd.setCursor(0, 0); // Establecer cursor en la primera línea
          leds[ledCoincidente] = CRGB::Black; // Apaga el LED coincidente
          FastLED.show(); // Muestra los LEDs actualizados
          lcd.setCursor(0,0);
          lcd.print("Coincidio con LED, ");
          lcd.setCursor(0,1);
          lcd.print( "posicion:");
          lcd.println(ledCoincidente);
          delay(1000);
          ledsAcertados++;  // Incrementa la cantidad de LEDs acertados

        } else {
          // No hubo coincidencia, muestra por Serial
          lcd.println("No coincidió  LED");
          delay(1000);
           // Resta un intento si no hubo coincidencia
           intentos--;
        }

        FastLED.show(); // Muestra los LEDs actualizados
        delay(500); // Pequeño retardo al final
       if (intentos <= 0 ) {
        // Si se agotan los intentos o el jugador decide salir, regresa al menú principal
        lcd.clear();
        lcd.print("Fin del juego");
        delay(2000); // Espera 2 segundos para que el jugador pueda leer el mensaje
        estadoMenu = 0;
        sig_estado = 1;
        reiniciarJuego(); // Llama a la función para reiniciar el juego
      
      } else {
        // Si aún hay intentos, continúa jugando
        lcd.clear();
        lcd.print("Intentos: ");
        lcd.print(intentos);
        lcd.setCursor(0, 1);
        lcd.print("Acertados: ");
        lcd.print(ledsAcertados);
        delay(1000); // Pequeño retardo al final
      }
     
   }else {
    leds[currentPosition] = CRGB::Green; // Mantiene encendido el LED en la posición actual mientras se seleccionan los LEDs
    FastLED.show(); // Muestra los LEDs actualizados
  }
 FastLED.clear();
 
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


void setupLEDs() {
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(20);  // Puedes ajustar el brillo si es necesario
  
  // Enciende todos los LEDs al inicio
  //fill_solid(leds, NUM_LEDS, CRGB::MediumAquamarine); // Puedes cambiar el color inicial si prefieres
  FastLED.clear();
  FastLED.show();
}


void menuPrincipal(){

  Estado = sig_estado;
  bool mensajeAJugarMostrado = false;
  unsigned long tiempoMostrarMensaje = 0;

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
    delay(150);
    juegoRuleta(); 
    
    
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


