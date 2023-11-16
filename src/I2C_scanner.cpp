#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>

// Dirección I2C del LCD
#define I2C_ADDR    0x3f  // Cambia esta dirección si es diferente

// Dimensiones del LCD (20 columnas x 4 filas)
#define LCD_COLUMNS 20
#define LCD_ROWS    4

// Inicialización del objeto LCD
LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLUMNS, LCD_ROWS);

void setup() {
  // Inicialización del LCD
  lcd.init();
  // Enciende la retroiluminación
  lcd.backlight();

  // Mensaje de inicio
  lcd.setCursor(0, 0);
  lcd.print("ESP32 con LCD");
  lcd.setCursor(0, 1);
  lcd.print("LCD 20x4");
  lcd.setCursor(0, 2);
  lcd.print("¡Hola, mundo!");
}

void loop() {
  // Tu código aquí
}