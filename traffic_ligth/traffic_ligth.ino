#include <LiquidCrystal.h>

// Configuración de los pines para el LCD
const byte RS = 2; 
const byte E = 3;  
const byte D4 = 8;
const byte D5 = 9;
const byte D6 = 10;
const byte D7 = 11;

//Byte para ahorrar memoria de la micro (arduino)
const byte pinLeds[] = {5, 6, 7};           // 0: Rojo, 1: Verde, 2: Amarillo
const int timeSeconds[] = {15, 16, 5};  // Tiempos reales en SEGUNDOS (la micro se ejecuta en milisegundos :)

LiquidCrystal lcd(RS, E, D4, D5, D6, D7);

void setup() {
  //Inicialización del LCD
  lcd.begin(16, 2);
  lcd.print("Semaforo Listo");
  delay(1000);

  // Inicialización de los pines de los LEDs
  for(byte i = 0; i < 3; i++) {
    pinMode(pinLeds[i], OUTPUT);
  }
}

void loop() {
  for(byte i = 0; i < 3; i++) {
    // Apaga el LED de la vuelta anterior
    digitalWrite(pinLeds[(i + 2) % 3], LOW);
    
    // Enciende el LED actual
    digitalWrite(pinLeds[i], HIGH);
    
    // Mostramos en el LCD el nombre del LED activo en la primera fila
    lcd.clear();
    if(i == 0) lcd.print("Luz: ROJO");
    if(i == 1) lcd.print("Luz: VERDE");
    if(i == 2) lcd.print("Luz: AMARILLO");
    
    // Ejecuta la cuenta regresiva pasando el tiempo asignado a este LED
    backCount(timeSeconds[i]);
  }
}

void backCount(int segundos) {
  while(segundos >= 0) {
    // Nos movemos a la segunda fila (columna 0, fila 1) para no borrar el nombre del LED
    lcd.setCursor(0, 1); 
    
    lcd.print("Tiempo: " + String(segundos) + "s   "); // Los espacios al final limpian residuos de texto anteriores
    
    segundos--;  // Restamos 1 al contador
    delay(1000); // Esperamos 1 segundo real
  }
}