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
#include <WiFi.h>
#include <LittleFS.h>
#include <SD.h>
#include <Update.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#include <set>






/* 2. Define the API Key */
#define API_KEY "AIzaSyD8NhHq3P2a7fsUHKO03iIrqnhhli6aELo"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://ruleta-53e4c-default-rtdb.firebaseio.com/"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;


unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;




//*RFID*//
#define SS_PIN 5  // Pin del lector RFID
#define RST_PIN 4   // Pin de reset del lector RFID
MFRC522 rfid(SS_PIN, RST_PIN);

//*WIFI*//
const char *ssid = "Galaxy A24 2EAB";
const char *password = "vicflara09";


//*LCD*//
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

//*LEDS*//
#define LED_PIN    32  // Pin de datos del LED
#define NUM_LEDS   30 // Número total de LEDs
#define MAX_SELECTED_LED 4

CRGB leds[NUM_LEDS];

//*VARIABLES DE LA FUNCIÓN JUEGO RULETA*

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
bool ledsAciertos[NUM_LEDS] = {false};  // Variable para rastrear los LEDs acertados
bool ledsCompletos = false;// Indicador de si todos los LEDs han sido encendidos
bool ledsImprimidos[NUM_LEDS] = {false};// Bandera para controlar los LEDs ya impresos
bool tarjetaReconocida = false;
bool mensajeMostrado = false;  // Variable para rastrear si el mensaje ha sido mostrado
int totalAciertosGlobal = 0;
bool ledsAciertosUnicos[NUM_LEDS];
bool tarjetaLectura = false;



LiquidCrystal_I2C lcd(PCF8574_ADDR_A21_A11_A01, 4, 5, 6, 16, 11, 12, 13, 14, POSITIVE);

void menuPrincipal();
void setupLEDs();
void leerDatosDesdeFirebase(const char *ruta);
String obtenerIDTarjetaRFID();





void reiniciarJuego() {
    // Mostrar mensaje en LCD indicando que se debe pasar la tarjeta
    lcd.clear();
    lcd.print("Pase la tarjeta");
    delay(2000);

    // Verificar si la tarjeta RFID está reconocida
    if (tarjetaReconocida) {
        // Reinicia las variables del juego
        //ledsAcertados = 0; // Reinicia el contador de LEDs acertados
        intentos = 3; // Reinicia el número de intentos disponibles
        ledsCompletos = false; // Reinicia el indicador de si todos los LEDs han sido encendidos
        ledsSeleccionadosCount = 0; // Reinicia el contador de LEDs seleccionados
        currentPosition = 0; // Reinicia la posición actual del joystick
        memset(ledsImprimidos, 0, sizeof(ledsImprimidos)); // Reinicia la bandera de LEDs ya impresos
        memset(ledsSeleccionados, 0, sizeof(ledsSeleccionados)); // Reinicia la matriz de LEDs seleccionados
        memset(coincidencias, 0, sizeof(coincidencias)); // Reinicia la matriz de coincidencias
        memset(ledsAciertos, 0, sizeof(ledsAciertos));// Reinicia el contador de LEDs acertados

        // Mostrar mensaje en LCD indicando que el juego ha sido reiniciado
        lcd.clear();
        lcd.print("Juego reiniciado");
        delay(2000);

        // Establecer que la tarjeta aún no ha sido reconocida
        tarjetaReconocida = false;
    }
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
  SPI.begin();
  rfid.PCD_Init();


  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.signer.tokens.legacy_token = API_KEY;
  config.database_url = DATABASE_URL;

  
  // Comment or pass false value when WiFi reconnection will be controlled by your code or third party library e.g., WiFiManager
  Firebase.reconnectNetwork(true);

  // Set SSL buffer size
  fbdo.setBSSLBufferSize(4096, 1024); // Adjust buffer size as needed

  
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  config.max_token_generation_retry = 5;

  // Begin Firebase connection
  Firebase.begin(&config, &auth);

  Serial.println("Firebase connection established!");
  



  lcd.clear();
  lcd.setCursor(1,0);	//columna - fila
  lcd.print("JUEGO DE LA RULETA");
  lcd.setCursor(4,1);
  lcd.print("TRAGAMONEDAS");
  lcd.setCursor(3,3);
  lcd.print("BY GRACE-VICTOR");
  delay(7000);
  lcd.clear();

  
  
}


void loop() {
  menuPrincipal();
  

}





const char* USUARIO_FIELD = "usuario";
const char* SALDO_FIELD = "Saldo";


String obtenerDatoDesdeDatos(const String& datos, const char* campo) {
    FirebaseJson json;
    FirebaseJsonData result;

    // Deserializar los datos en un objeto JSON
    json.setJsonData(datos);

    // Obtener el campo deseado
    json.get(result, campo);

    if (result.success && result.type == "string") {
        return result.to<String>();
    } else {
        return "";  // Retornar cadena vacía si no se puede obtener el campo
    }
}

float obtenerSaldoDesdeDatos(const String& datos) {
    FirebaseJson json;
    FirebaseJsonData result;

    // Deserializar los datos en un objeto JSON
    json.setJsonData(datos);

    // Obtener el saldo anidado dentro de "Saldo"
    json.get(result, "Saldo/Saldo");

    if (result.success && (result.type == "int" || result.type == "float")) {
        return result.to<float>();
    } else {
        Serial.println("Error al obtener el saldo desde los datos.");
        return 0.0;  // Retornar 0.0 si no se puede obtener el saldo
    }
}

void mostrarEnLCD(const String& usuario, float saldo) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Usuario: ");

    if (usuario != "") {
        lcd.setCursor(0, 1);
        lcd.print(usuario);
        lcd.setCursor(0, 2);
        lcd.print("Saldo: $");
        lcd.print(saldo, 2); // Mostrar el saldo con dos decimales
        delay(2000);
    } else {
        lcd.setCursor(0, 1);
        lcd.print("No encontrado");
        delay(2000);
    }
}


void aumentarSaldoRFID(float incrementoSaldo) {
    // Obtener el ID de la tarjeta RFID
    String uid = obtenerIDTarjetaRFID();

    // Verificar si se obtuvo un ID válido
    if (!uid.isEmpty()) {
        // Construir la ruta en base al UID leído
        String rutaUsuario = "/Usuarios/" + uid;

        // Obtener los datos del usuario desde Firebase
        if (Firebase.get(fbdo, rutaUsuario)) {
            String datosUsuario = fbdo.stringData();

            // Obtener el saldo actual del usuario
            float saldoActual = obtenerSaldoDesdeDatos(datosUsuario.c_str());

            // Verificar si se obtuvo correctamente el saldo actual
            if (saldoActual != 0.0) {
                // Aumentar el saldo con el valor proporcionado
                float nuevoSaldo = saldoActual + incrementoSaldo;

                // Actualizar solo el saldo en Firebase
                FirebaseJson json;
                json.set(SALDO_FIELD, nuevoSaldo);

                if (Firebase.set(fbdo, rutaUsuario + "/" + SALDO_FIELD, json)) {
                    Serial.println("Saldo aumentado correctamente. Nuevo saldo: " + String(nuevoSaldo));
                    // Puedes mostrar en LCD si lo deseas
                    mostrarEnLCD(obtenerDatoDesdeDatos(datosUsuario.c_str(), USUARIO_FIELD), nuevoSaldo);
                } else {
                    Serial.println("Error al actualizar el saldo en Firebase: " + fbdo.errorReason());
                }
            } else {
                Serial.println("Error al obtener el saldo actual desde Firebase.");
            }
        } else {
            Serial.println("Error al obtener datos desde Firebase: " + fbdo.errorReason());
        }
    } else {
        // Manejar el caso cuando no se obtiene un ID válido
        Serial.println("Error al obtener el ID de la tarjeta RFID.");
    }
}







void disminuirSaldoRFID(float decrementoSaldo) {
    // Obtener el ID de la tarjeta RFID
    String uid = obtenerIDTarjetaRFID();

    // Verificar si se obtuvo un ID válido
    if (!uid.isEmpty()) {
        // Construir la ruta en base al UID leído
        String rutaUsuario = "/Usuarios/" + uid;

        // Obtener los datos del usuario desde Firebase
        if (Firebase.get(fbdo, rutaUsuario)) {
            String datosUsuario = fbdo.stringData();

            // Obtener el saldo actual del usuario
            float saldoActual = obtenerSaldoDesdeDatos(datosUsuario.c_str());

            // Verificar si se obtuvo correctamente el saldo actual
            if (saldoActual != 0.0) {
                // Verificar que haya suficiente saldo para el decremento
                if (saldoActual >= decrementoSaldo) {
                    // Disminuir el saldo con el valor proporcionado
                    float nuevoSaldo = saldoActual - decrementoSaldo;

                    // Actualizar solo el saldo en Firebase
                    FirebaseJson json;
                    json.set(SALDO_FIELD, nuevoSaldo);

                    if (Firebase.set(fbdo, rutaUsuario + "/" + SALDO_FIELD, json)) {
                        Serial.println("Saldo disminuido correctamente. Nuevo saldo: " + String(nuevoSaldo));
                        // Puedes mostrar en LCD si lo deseas
                        mostrarEnLCD(obtenerDatoDesdeDatos(datosUsuario.c_str(), USUARIO_FIELD), nuevoSaldo);
                    } else {
                        Serial.println("Error al actualizar el saldo en Firebase: " + fbdo.errorReason());
                    }
                } else {
                    Serial.println("Saldo insuficiente para el decremento");
                }
            } else {
                Serial.println("Error al obtener el saldo actual desde Firebase.");
            }
        } else {
            Serial.println("Error al obtener datos desde Firebase: " + fbdo.errorReason());
        }
    } else {
        // Manejar el caso cuando no se obtiene un ID válido
        Serial.println("Error al obtener el ID de la tarjeta RFID.");
    }
}


void otorgarPremio(int opcionesAcertadas) {
    float montoGanado = 0.0;

    // Determinar el monto ganado según las opciones acertadas
    switch (opcionesAcertadas) {
        case 4:
            montoGanado = 2.0;
            break;
        case 3:
            montoGanado = 1.5;
            break;
        case 2:
        case 1:
            montoGanado = 0.5;
            break;
        default:
            montoGanado = 0.0;
            break;
    }

    // Aumentar el saldo en la cuenta del usuario
    aumentarSaldoRFID(montoGanado);
}






void leerDatosDesdeFirebase(const char* ruta) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Leyendo desde Firebase...");

    Serial.print("Leyendo datos desde la ruta: ");
    Serial.println(ruta);

    if (Firebase.get(fbdo, ruta)) {
        Serial.println("Datos obtenidos:");

        // Obtener los datos directamente
        String datos = fbdo.stringData();
        Serial.println("Datos: " + datos);

        // Procesar los datos
        String usuario = obtenerDatoDesdeDatos(datos, USUARIO_FIELD);
        float saldo = obtenerSaldoDesdeDatos(datos);

        mostrarEnLCD(usuario, saldo);
        
    } else {
        Serial.println("Error al obtener datos desde Firebase: " + fbdo.errorReason());
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Error Firebase");
    }
}




void mostrarMensajeEnLCD(const String& mensaje) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(mensaje);
    delay(2000);  // Espera 2 segundos para que el mensaje sea visible
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
          if (!ledsAciertosUnicos[ledCoincidente]) {
                  // Si el LED aún no ha sido contabilizado, hazlo ahora
                  ledsAciertos[ledCoincidente] = true;  // Marca el LED como acertado
                  ledsAciertosUnicos[ledCoincidente] = true;  // Marca el LED como único
                  totalAciertosGlobal++;  // Incrementa el contador global de aciertos
              }

        } else {
          // No hubo coincidencia, muestra por Serial
          lcd.println("No coincidió  LED");
          delay(1000);
           // Resta un intento solo si no hubo coincidencia y el LED actual no está seleccionado
          if (!ledsSeleccionados[currentPosition]) {
              intentos--;
          }
                // Apagar el LED naranja solo si no coincide con ningún LED seleccionado
          if (!ledsSeleccionados[currentPosition]) {
              leds[currentPosition] = CRGB::Black;
          }
          ledsSeleccionados[currentPosition] = false;  // Apagar el LED seleccionado
      }


        FastLED.show(); // Muestra los LEDs actualizados
        delay(500); // Pequeño retardo al final


        bool ledsAciertosCoinciden = true;
        for (int i = 0; i < NUM_LEDS; ++i) {
            if (ledsSeleccionados[i] && !ledsAciertos[i]) {
                ledsAciertosCoinciden = false;
                break;
            }
        }
        if (intentos <= 0 || ledsAciertosCoinciden ) {
          // Si se agotan los intentos o el jugador decide salir, regresa al menú principal
          lcd.clear();
          lcd.print("Fin del juego");
          lcd.setCursor(0, 1);
          lcd.print("Total aciertos: ");
          lcd.print(totalAciertosGlobal);
          otorgarPremio(totalAciertosGlobal);
          delay(2000); // Espera 2 segundos para que el jugador pueda leer el mensaje
          estadoMenu = 0;
          sig_estado = 1;
          reiniciarJuego(); // Llama a la función para reiniciar el juego
        
        } else {
          // Si aún hay intentos y no se ha completado el juego, continúa jugando
        lcd.clear();
        lcd.print("Intentos: ");
        lcd.print(intentos);
        lcd.setCursor(0, 1);
        lcd.print("Aciertos: ");

        for (int i = 0; i < NUM_LEDS; ++i) {
    if (ledsSeleccionados[i]) {
        if (ledsAciertos[i]) {
            lcd.print(i); // Muestra la posición del LED seleccionado en que acertó
        } else {
            lcd.print("X"); // Muestra 'X' si no acertó en esa posición
        }
    }
}

        delay(1000); // Pequeño retardo al final
        }
        
    }
   
   else {
    leds[currentPosition] = CRGB::Green; // Mantiene encendido el LED en la posición actual mientras se seleccionan los LEDs
    FastLED.show(); // Muestra los LEDs actualizados
  }
 FastLED.clear();
 // Mostrar la cantidad total de LEDs acertados sin repetirse
  
}




String obtenerIDTarjetaRFID() {
    String idTarjeta = "";
    for (byte i = 0; i < rfid.uid.size; ++i) {
        // Agregar espaciado y convertir a mayúsculas
        idTarjeta += String(rfid.uid.uidByte[i], HEX);
        if (i < rfid.uid.size - 1) {
            idTarjeta += " ";
        }
    }
    idTarjeta.toUpperCase();  // Convertir a mayúsculas
    return idTarjeta;
}



void verificarSaldoRFID() {
    String uid = obtenerIDTarjetaRFID();
    String ruta = "/Usuarios/" + uid;  // Construir la ruta en base al UID leído
    leerDatosDesdeFirebase(ruta.c_str());
    // Establecer la tarjeta como reconocida
    //tarjetaReconocida = true;
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
    String arrayMenu[] = {"1. Jugar", "2. Consultar saldo"};
    menu = funcionMenu(arrayMenu, 2);
    if (menu == -1)
      sig_estado = 1;
    else if (menu == 1)
      sig_estado = 2;
    else if (menu == 2)
      sig_estado = 3;
    

  }
  else if (Estado == 2){
    // Verificar si ya se ha reconocido la tarjeta
        if (!tarjetaReconocida) {
            // Mostrar el mensaje solo si no se ha mostrado previamente
            if (!mensajeMostrado) {
                mostrarMensajeEnLCD("Pase tarjeta");
                mensajeMostrado = true;  // Marcar el mensaje como mostrado
            }

            // Esperar hasta que se detecte la tarjeta
            if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
                verificarSaldoRFID();
                disminuirSaldoRFID(0.5);
                
                

                // Mostrar el saldo después de verificar la tarjeta
                lcd.clear();
                delay(2000);  // Espera 2 segundos para que el usuario pueda ver el saldo

                // Marcar la tarjeta como no reconocida
                tarjetaReconocida = true;
                lcd.clear();
            }
        } else {
            // Ejecutar el juego solo si la tarjeta ya ha sido reconocida
            lcd.clear();
            delay(150);
            juegoRuleta();
            Estado = 0;  // Restablecer el estado después de ejecutar el juego
            tarjetaReconocida = true;  // Reiniciar la bandera de reconocimiento de tarjeta
            mensajeMostrado = true;  // Reiniciar la bandera de mensaje mostrado
        }
    }

  
    


  else if (Estado == 3){
    lcd.clear();
    lcd.print("Acerque tarjeta");
    //delay(1000);
      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        verificarSaldoRFID();
        delay(1000);  // Agrega un pequeño retardo para evitar lecturas repetidas
        estadoMenu = 0;
        sig_estado = 1;
    }
    
  
  }
 
}

