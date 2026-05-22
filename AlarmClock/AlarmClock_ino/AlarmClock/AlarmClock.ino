
#include <LiquidCrystal.h>

// ============================================
// ESTRUCTURA PARA ALMACENAR TIEMPO
// ============================================
// Define una estructura que guarda las horas, minutos y segundos
struct time_t
{
  short hours;      // Horas (0-23)
  short minutes;    // Minutos (0-59)
  short seconds;    // Segundos (0-59)

  // Operador de comparación personalizado
  // Permite comparar si dos tiempos son iguales (para detectar cuando suena la alarma)
  bool operator==(time_t current)
  {
    if ((current.hours == hours) && (current.minutes == minutes) && (current.seconds == seconds))
      return true;  // Los tiempos son iguales
      else
        return false; // Los tiempos son diferentes
  }
} alarm; // Variable global que almacena la hora de la alarma

// ============================================
// CONFIGURACIÓN DE PINES DEL LCD
// ============================================
// Estos pines del Arduino controlan las líneas del LCD
const int rs = 12;  // RS (Register Select) - selecciona comando o dato
const int en = 11;  // EN (Enable) - activa la lectura del LCD
const int d4 = 5;   // D4 (Data 4) - línea de datos
const int d5 = 4;   // D5 (Data 5) - línea de datos
const int d6 = 3;   // D6 (Data 6) - línea de datos
const int d7 = 2;   // D7 (Data 7) - línea de datos

// ============================================
// CONFIGURACIÓN DE OTROS PINES
// ============================================
const int ledPin = LED_BUILTIN;  // LED incorporado del Arduino (Pin 13) - se activa cuando suena la alarma

// ============================================
// DEFINICIÓN DE ESTADOS (MODOS)
// ============================================
// Enum que define los 5 estados posibles del reloj
enum state {
  TIME,                 // Estado 0: Mostrar hora actual
  SET_ALARM,            // Estado 1: Encender/apagar alarma
  SET_ALARM_HOURS,      // Estado 2: Configurar horas de alarma
  SET_ALARM_MINUTES,    // Estado 3: Configurar minutos de alarma
  SET_ALARM_SECONDS     // Estado 4: Configurar segundos de alarma
};

// ============================================
// CONFIGURACIÓN DE BOTONES
// ============================================
const int buttonMode = 6;  // Botón MODE en pin 6 - cambia entre modos
const int buttonSet = 7;   // Botón SET en pin 7 - confirma o modifica valores

// ============================================
// VARIABLES GLOBALES
// ============================================
enum state mode = TIME;     // Estado inicial: mostrar hora
bool alarmOn = false;       // La alarma comienza desactivada
int ledState = LOW;         // El LED comienza apagado

// Variables para evitar múltiples lecturas del botón (anti-rebote)
unsigned long lastDebounceTime = 0;  // Última vez que se detectó un cambio
unsigned long debounceDelay = 50;    // Espera 50ms para confirmar el cambio

// ============================================
// INICIALIZACIÓN DEL LCD
// ============================================
// Crea un objeto LCD con los pines especificados
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// ============================================
// FUNCIÓN SETUP (Se ejecuta una sola vez)
// ============================================
void setup() {
  // Configura el LED como salida (para encenderlo/apagarlo)
  pinMode(ledPin, OUTPUT);

  // Configura los botones como entradas (para leer su estado)
  pinMode(buttonMode, INPUT);
  pinMode(buttonSet, INPUT);

  // Inicializa el LCD: 16 columnas x 2 filas
  lcd.begin(16, 2);

  // Imprime un mensaje inicial en el LCD
  lcd.print("Time");

  // Configura la hora de la alarma inicial (0 horas, 0 minutos, 7 segundos)
  alarm = {.hours = 0, .minutes = 0, .seconds = 7};
}

// ============================================
// FUNCIÓN PARA IMPRIMIR LA HORA EN FORMATO HH:MM:SS
// ============================================
// Parámetro: t - estructura time_t con la hora a mostrar
void printTime(struct time_t t)
{
  // Imprime las HORAS con un cero al inicio si es menor a 10
  if (t.hours < 10)
    lcd.print("0");  // Ejemplo: 9 se muestra como 09
    lcd.print(t.hours);

  // Imprime dos puntos como separador
  lcd.print(":");

  // Imprime los MINUTOS con un cero al inicio si es menor a 10
  if (t.minutes < 10)
    lcd.print("0");  // Ejemplo: 5 se muestra como 05
    lcd.print(t.minutes);

  // Imprime dos puntos como separador
  lcd.print(":");

  // Imprime los SEGUNDOS con un cero al inicio si es menor a 10
  if (t.seconds < 10)
  {
    lcd.print("0");  // Ejemplo: 3 se muestra como 03
  }
  lcd.print(t.seconds);
}

// ============================================
// FUNCIÓN PRINCIPAL (Loop - se ejecuta continuamente)
// ============================================
void loop() {

  // ╔═══════════════════════════════════════════════════════╗
  // ║ LECTURA DEL BOTÓN MODE (navegar entre modos)         ║
  // ╚═══════════════════════════════════════════════════════╝
  if (digitalRead(buttonMode))
  {
    // Si estamos en SET_ALARM y la alarma está apagada, vuelve a TIME
    // (para no quedarse atrapado en la configuración)
    if ((mode == SET_ALARM) && (!alarmOn))
      mode = TIME;
    else
      // Avanza al siguiente modo (0→1→2→3→4→0)
      // % 5 hace que después del modo 4 vuelva al modo 0
      mode = static_cast<state>((mode + 1) % 5);

    delay(100);  // Espera 100ms para evitar rebotes
  }

  // ═════════════════════════════════════════════════════════
  // MODO 0: TIME - Mostrar la hora actual
  // ═════════════════════════════════════════════════════════
  if (mode == TIME)
  {
    // Posiciona el cursor en la primera línea, primera columna
    lcd.setCursor(0, 0);
    lcd.print("Time ");

    // Posiciona el cursor en la segunda línea, primera columna
    lcd.setCursor(0, 1);

    // Calcula la hora actual basada en milisegundos desde que se encendió el Arduino
    // millis() devuelve los milisegundos desde el inicio
    time_t t = {
      .hours = static_cast<short>(millis()/1000/60/60 % 24),    // Convierte a horas (0-23)
      .minutes = static_cast<short>(millis()/1000/60 % 60),     // Convierte a minutos (0-59)
      .seconds = static_cast<short>(millis()/1000 % 60)         // Convierte a segundos (0-59)
    };

    // Imprime la hora en formato HH:MM:SS
    printTime(t);

    // Comprueba si la alarma está activada Y si la hora actual coincide con la alarma
    if (alarmOn && (t == alarm))
      digitalWrite(ledPin, HIGH);  // Enciende el LED (suena la alarma)
  }

  // ═════════════════════════════════════════════════════════
  // MODO 1: SET_ALARM - Activar/desactivar la alarma
  // ═════════════════════════════════════════════════════════
  else if (mode == SET_ALARM)
  {
    // Lectura del botón SET (activar/desactivar alarma)
    if (digitalRead(buttonSet) == HIGH)
    {
      alarmOn = !alarmOn;  // Cambia el estado (ON↔OFF)
      delay(100);          // Espera para evitar rebotes
    }

    // Imprime "Alarm" en la primera línea
    lcd.setCursor(0, 0);
    lcd.print("Alarm");

    // Posiciona el cursor en la segunda línea
    lcd.setCursor(0, 1);

    // Efecto de parpadeo: cambia cada 250ms
    // millis()%500 devuelve un número entre 0-499
    // Si es menor a 250, muestra ON/OFF; si no, muestra espacios en blanco
    if (millis()%500 < 250)
    {
      if (alarmOn)
        lcd.print("ON      ");   // Muestra ON si está activada
        else
          lcd.print("OFF     ");   // Muestra OFF si está desactivada
    }
    else
    {
      // Limpia la pantalla (parpadea)
      lcd.setCursor(0, 1);
      lcd.print("   ");
    }
  }

  // ═════════════════════════════════════════════════════════
  // MODO 2: SET_ALARM_HOURS - Configurar horas de la alarma
  // ═════════════════════════════════════════════════════════
  else if (mode == SET_ALARM_HOURS)
  {
    // Lectura del botón SET (incrementar horas)
    if (digitalRead(buttonSet) == HIGH)
    {
      alarm.hours = (alarm.hours + 1) % 24;  // Suma 1 a las horas (0-23)
      delay(200);  // Espera más tiempo para evitar cambios muy rápidos
    }

    // Posiciona el cursor en la segunda línea
    lcd.setCursor(0, 1);

    // Efecto de parpadeo: muestra y oculta las horas alternadamente
    if (millis()%500 < 250)
    {
      printTime(alarm);  // Muestra la hora completa
    }
    else
    {
      // Borra las dos primeras posiciones (las horas parpadean)
      lcd.print("  ");
    }
  }

  // ═════════════════════════════════════════════════════════
  // MODO 3: SET_ALARM_MINUTES - Configurar minutos de la alarma
  // ═════════════════════════════════════════════════════════
  else if (mode == SET_ALARM_MINUTES)
  {
    // Lectura del botón SET (incrementar minutos)
    if (digitalRead(buttonSet) == HIGH)
    {
      alarm.minutes = (alarm.minutes + 1) % 60;  // Suma 1 a los minutos (0-59)
      delay(200);  // Espera más tiempo para evitar cambios muy rápidos
    }

    // Posiciona el cursor en la segunda línea
    lcd.setCursor(0, 1);

    // Efecto de parpadeo: muestra y oculta los minutos alternadamente
    if (millis()%500 < 250)
    {
      printTime(alarm);  // Muestra la hora completa
    }
    else
    {
      // Posiciona el cursor en los minutos (columna 3: HH:MM:SS)
      lcd.setCursor(3, 1);
      // Borra los dos dígitos de los minutos (parpadean)
      lcd.print("  ");
    }
  }

  // MODO 4: SET_ALARM_SECONDS - Configurar segundos de la alarma
  else if (mode == SET_ALARM_SECONDS)
  {
    // Lectura del botón SET (incrementar segundos)
    if (digitalRead(buttonSet) == HIGH)
    {
      alarm.seconds = (alarm.seconds + 1) % 60;  // Suma 1 a los segundos (0-59)
      delay(200);  // Espera más tiempo para evitar cambios muy rápidos
    }

    // Posiciona el cursor en la segunda línea
    lcd.setCursor(0, 1);

    // Efecto de parpadeo: muestra y oculta los segundos alternadamente
    if (millis()%500 < 250)
    {
      printTime(alarm);  // Muestra la hora completa
    }
    else
    {
      // Posiciona el cursor en los segundos (columna 6: HH:MM:SS)
      lcd.setCursor(6, 1);
      // Borra los dos dígitos de los segundos (parpadean)
      lcd.print("  ");
    }
  }
}
