//Codigo Original: https://gitlab.com/open-led-race/olr-arduino-basic

//Este Codigo:
//Codigo Unit para ESP32 y placa UNIT Race Led Kit
//Codigo modificado para 2 jugadores

char const softwareId[] = "A2P0"; 
char const version[] = "1.0.0";

#include "Arduino.h"
#include <Adafruit_NeoPixel.h>

#define MAXLED         600 // Maximo numero de Leds en la tira

#define PIN_LED        22 
#define PIN_P1         17 //Jugador 1
#define PIN_P2         18 //Jugador 2


#define NoVueltas 2 // Definimos la cantidad de vueltas de la carrera

#define INI_RAMP 80
#define MED_RAMP 90
#define END_RAMP 100
#define HIGH_RAMP 16
bool ENABLE_RAMP = 0;
bool VIEW_RAMP = 0;

int NPIXELS = MAXLED; // Numero de LEDs en la pista
int cont_print = 0;

#define COLOR1   Color(0,251,255)
#define COLOR2   Color(0,255,0)

#define COLOR1_tail   Color(0,i*3,i*3)
#define COLOR2_tail   Color(0,i*3,0)

// Serial Communications
#define EOL                 '\n' // Carácter de fin de comando utilizado en el protocolo

#define REC_COMMAND_BUFLEN  32
char cmd[REC_COMMAND_BUFLEN];    // Almacena el comando recibido por ReadSerialComand()

#define TX_COMMAND_BUFLEN  64
char txbuff[TX_COMMAND_BUFLEN];  // para preparar cadenas de comando para enviar



int win_music[] = {
  2637, 2637, 0, 2637,
  0, 2093, 2637, 0,
  3136
};

byte  gravity_map[MAXLED];

int TBEEP = 0;
int FBEEP = 0;
byte SMOTOR = 0;
float speed1 = 0;
float speed2 = 0;
float dist1 = 0;
float dist2 = 0;

byte loop1 = 0;
byte loop2 = 0;

byte leader = 0;
byte loop_max = NoVueltas;


float ACEL = 0.2;
float kf = 0.02; //constante de fricción 0.015
float kg = 0.003; //constante de gravedad

byte flag_sw1 = 0;
byte flag_sw2 = 0;
byte draworder = 0;

unsigned long timestamp = 0;

Adafruit_NeoPixel track = Adafruit_NeoPixel(MAXLED, PIN_LED, NEO_GRB + NEO_KHZ800);

int tdelay = 5;

void set_ramp(byte H, byte a, byte b, byte c)
{ for (int i = 0; i < (b - a); i++) {
    gravity_map[a + i] = 127 - i * ((float)H / (b - a));
  };
  gravity_map[b] = 127;
  for (int i = 0; i < (c - b); i++) {
    gravity_map[b + i + 1] = 127 + H - i * ((float)H / (c - b));
  };
}

void set_loop(byte H, byte a, byte b, byte c)
{ for (int i = 0; i < (b - a); i++) {
    gravity_map[a + i] = 127 - i * ((float)H / (b - a));
  };
  gravity_map[b] = 255;
  for (int i = 0; i < (c - b); i++) {
    gravity_map[b + i + 1] = 127 + H - i * ((float)H / (c - b));
  };
}


void setup() {
  Serial.begin(115200);
  for (int i = 0; i < NPIXELS; i++) {
    gravity_map[i] = 127;
  };
  track.begin();
  pinMode(PIN_P1, INPUT_PULLUP);
  pinMode(PIN_P2, INPUT_PULLUP);

  if ((digitalRead(PIN_P1) == 0)) //Presione el Jugador 1 en el reinicio para activar la física
  { ENABLE_RAMP = 1;
    set_ramp(HIGH_RAMP, INI_RAMP, MED_RAMP, END_RAMP);
    for (int i = 0; i < (MED_RAMP - INI_RAMP); i++) {
      track.setPixelColor(INI_RAMP + i, track.Color(24 + i * 4, 0, 24 + i * 4) );
    };
    for (int i = 0; i < (END_RAMP - MED_RAMP); i++) {
      track.setPixelColor(END_RAMP - i, track.Color(24 + i * 4, 0, 24 + i * 4) );
    };
    track.show();
    delay(1000);
    if ((digitalRead(PIN_P1) == 0)) {
      VIEW_RAMP = 1; //Si mantiene presionado el Jugador 1, configure la visualizacion de la rampa
    }
    else {
      for (int i = 0; i < NPIXELS; i++) {
        track.setPixelColor(i, track.Color(0, 0, 0));
      };
      track.show();
      VIEW_RAMP = 0;
    };

  };

  if ((digitalRead(PIN_P2) == 0)) {
    delay(1500);
    if ((digitalRead(PIN_P2) == 1)) SMOTOR = 1;
  }

  start_race();
}

void start_race() {
  send_race_phase(4); //Fase 4 de la carrera: Cuenta atrás
  for (int i = 0; i < NPIXELS; i++) {
    track.setPixelColor(i, track.Color(0, 0, 0));
  };
  track.show();
  delay(2000);
  track.setPixelColor(12, track.Color(255, 0, 0));
  track.setPixelColor(11, track.Color(255, 0, 0));
  track.show();
  delay(2000);
  track.setPixelColor(12, track.Color(0, 0, 0));
  track.setPixelColor(11, track.Color(0, 0, 0));
  track.setPixelColor(10, track.Color(255, 255, 0));
  track.setPixelColor(9, track.Color(255, 255, 0));
  track.show();
  delay(2000);
  track.setPixelColor(9, track.Color(0, 0, 0));
  track.setPixelColor(10, track.Color(0, 0, 0));
  track.setPixelColor(8, track.Color(0, 255, 0));
  track.setPixelColor(7, track.Color(0, 255, 0));
  track.show();
  delay(2000);
  timestamp = 0;
  send_race_phase(5); // Fase de carrera 4: carrera iniciada
};

void winner_fx(byte w) {
  int msize = sizeof(win_music) / sizeof(int);
  for (int note = 0; note < msize; note++) {
    if (SMOTOR == 1) {
    } else {
    };
    delay(230);
  }
};


int get_relative_position1( void ) {
  enum {
    MIN_RPOS = 0,
    MAX_RPOS = 99,
  };
  int trackdist = 0;
  int pos = 0;
  trackdist = (int)dist1 % NPIXELS;
  pos = map(trackdist, 0, NPIXELS - 1, MIN_RPOS, MAX_RPOS);
  return pos;
}

int get_relative_position2( void ) {
  enum {
    MIN_RPOS = 0,
    MAX_RPOS = 99,
  };
  int trackdist = 0;
  int pos = 0;
  trackdist = (int)dist2 % NPIXELS;
  pos = map(trackdist, 0, NPIXELS - 1, MIN_RPOS, MAX_RPOS);
  return pos;
}

void print_cars_position( void ) {
  int  rpos = get_relative_position1();
  sprintf( txbuff, "p%d%d%d,%d%c", 1, 1, loop1, rpos, EOL );
  sendSerialCommand(txbuff);

  rpos = get_relative_position2();
  sprintf( txbuff, "p%d%d%d,%d%c", 2, 1, loop2, rpos, EOL );
  sendSerialCommand(txbuff);

}


void burning1() {
  //que hacer
}

void burning2() {
  //que hacer
}

void track_rain_fx() {
  //que hacer
}

void track_oil_fx() {
  //que hacer
}

void track_snow_fx() {
  //que hacer
}


void fuel_empty() {
  //que hacer
}

void fill_fuel_fx() {
  //que hacer
}

void in_track_boxs_fx() {
  //que hacer
}

void pause_track_boxs_fx() {
  //que hacer
}

void flag_boxs_stop() {
  //que hacer
}

void flag_boxs_ready() {
  //que hacer
}

void draw_safety_car() {
  //que hacer
}

void telemetry_rx() {
  //que hacer
}

void telemetry_tx() {
  //que hacer
}

void telemetry_lap_time_car1() {
  //que hacer
}

void telemetry_lap_time_car2() {
  //que hacer
}

void telemetry_record_lap() {
  //que hacer
}

void telemetry_total_time() {
  //que hacer
}

int read_sensor(byte player) {
  return (0); //que hacer
}

int calibration_sensor(byte player) {
  return (0); //que hacer
}

int display_lcd_laps() {
  return (0); //que hacer
}

int display_lcd_time() {
  return (0); //que hacer
}

void draw_car1(void) {
  for (int i = 0; i <= loop1; i++) {
    track.setPixelColor(((word)dist1 % NPIXELS) + i, track.COLOR1);
  };
}

void draw_car2(void) {
  for (int i = 0; i <= loop2; i++) {
    track.setPixelColor(((word)dist2 % NPIXELS) + i, track.COLOR2);
  };
}

void loop() {

  // buscar comandos recibidos en serie
  checkSerialCommand();


  for (int i = 0; i < NPIXELS; i++) {
    track.setPixelColor(i, track.Color(0, 0, 0));
  };
  if ((ENABLE_RAMP == 1) && ( VIEW_RAMP == 1)) {
    for (int i = 0; i < (MED_RAMP - INI_RAMP); i++) {
      track.setPixelColor(INI_RAMP + i, track.Color(24 + i * 4, 0, 24 + i * 4) );
    };
    for (int i = 0; i < (END_RAMP - MED_RAMP); i++) {
      track.setPixelColor(END_RAMP - i, track.Color(24 + i * 4, 0, 24 + i * 4) );
    };
  };

  if ( (flag_sw1 == 1) && (digitalRead(PIN_P1) == 0) ) {
    flag_sw1 = 0;
    speed1 += ACEL;
  };
  if ( (flag_sw1 == 0) && (digitalRead(PIN_P1) == 1) ) {
    flag_sw1 = 1;
  };

  if ((gravity_map[(word)dist1 % NPIXELS]) < 127) speed1 -= kg * (127 - (gravity_map[(word)dist1 % NPIXELS]));
  if ((gravity_map[(word)dist1 % NPIXELS]) > 127) speed1 += kg * ((gravity_map[(word)dist1 % NPIXELS]) - 127);

  speed1 -= speed1 * kf;

  if ( (flag_sw2 == 1) && (digitalRead(PIN_P2) == 0) ) {
    flag_sw2 = 0;
    speed2 += ACEL;
  };
  if ( (flag_sw2 == 0) && (digitalRead(PIN_P2) == 1) ) {
    flag_sw2 = 1;
  };

  if ((gravity_map[(word)dist2 % NPIXELS]) < 127) speed2 -= kg * (127 - (gravity_map[(word)dist2 % NPIXELS]));
  if ((gravity_map[(word)dist2 % NPIXELS]) > 127) speed2 += kg * ((gravity_map[(word)dist2 % NPIXELS]) - 127);

  speed2 -= speed2 * kf;

  dist1 += speed1;
  dist2 += speed2;

  if (dist1 > dist2) {
    if (leader == 2) {
      FBEEP = 440;
      TBEEP = 10;
    }
    leader = 1;
  }
  if (dist2 > dist1) {
    if (leader == 1) {
      FBEEP = 440 * 2;
      TBEEP = 10;
    }
    leader = 2;
  };



  if (dist1 > NPIXELS * loop1) {
    loop1++;
    TBEEP = 10;
    FBEEP = 440;
  };
  if (dist2 > NPIXELS * loop2) {
    loop2++;
    TBEEP = 10;
    FBEEP = 440 * 2;
  };

  if (loop1 > loop_max) {
    sprintf( txbuff, "w1%c", EOL );
    sendSerialCommand(txbuff); //Enviar ganador = 1er Jugador
    for (int i = 0; i < NPIXELS / 10; i++) {
      track.setPixelColor(i, track.COLOR1_tail);
    }; track.show();
    winner_fx(1); loop1 = 0; loop2 = 0; dist1 = 0; dist2 = 0; speed1 = 0; speed2 = 0; timestamp = 0;
    start_race();
  }
  if (loop2 > loop_max) {
    sprintf( txbuff, "w2%c", EOL );
    sendSerialCommand(txbuff); // Enviar ganador = 2do Jugador
    for (int i = 0; i < NPIXELS / 10; i++) {
      track.setPixelColor(i, track.COLOR2_tail);
    }; track.show();
    winner_fx(2); loop1 = 0; loop2 = 0; dist1 = 0; dist2 = 0; speed1 = 0; speed2 = 0; timestamp = 0;
    start_race();
  }

  if ((millis() & 512) == (512 * draworder)) {
    if (draworder == 0) {
      draworder = 1;
    }
    else {
      draworder = 0;
    }
  };
  if (abs(round(speed1 * 100)) > abs(round(speed2 * 100))) {
    draworder = 1;
  };
  if (abs(round(speed2 * 100)) > abs(round(speed1 * 100))) {
    draworder = 0;
  };

  if ( draworder == 0 ) {
    draw_car1();
    draw_car2();
  }
  else {
    draw_car2();
    draw_car1();
  }

  track.show();
  if (SMOTOR == 1)
    delay(tdelay);
  if (TBEEP > 0) {
    TBEEP--;
  } else {
    FBEEP = 0;
  };
  cont_print++;
  if (cont_print > 100) {
    print_cars_position();
    cont_print = 0;
  }

}


void checkSerialCommand() {

  int clen = checkSerial(cmd);
  if (clen == 0) return ;      // No se recibieron comandos
  if (clen  < 0) {             // Error al recibir el comando
    sprintf( txbuff, "!1Error reading serial command:[%d]", clen); // Enviar una advertencia al anfitrión
    sendSerialCommand(txbuff);
    return;
  }
  // clen > 0 ---> Command with length=clen ready in  cmd[]

  switch (cmd[0]) {
    case '#':                         // Apretón de manos -> enviar de vuelta
      {
        sprintf( txbuff, "#%c", EOL );
        sendSerialCommand(txbuff);
      }
      return;

    case  '@' :                         // Ingrese al "Modo de configuración"
      {
        // enviar de vuelta @OK
        // No hay modo cfg real aquí, pero envíe @OK para que la aplicación de escritorio (Cargar, configurar)
        //puede enviar un comando GET SOFTWARE Type/Ver e identificar este software
        sprintf( txbuff, "@OK%c", EOL );
        sendSerialCommand(txbuff);
      }
      return;

    case '?' :                          // Obtener ID de software
      {
        sprintf( txbuff, "%s%s%c", "?", softwareId, EOL );
        sendSerialCommand(txbuff);
      }
      return;

    case '%' :                          // Obtener versión de software
      {
        sprintf( txbuff, "%s%s%c", "%", version, EOL );
        sendSerialCommand(txbuff);
      }
      return;
  }

  // si llegamos aquí, el comando no es administrado por este software -> Responder <CommandId>NOK
  sprintf(txbuff, "%cNOK%c", cmd[0], EOL );
  sendSerialCommand(txbuff);

  return;
}


void send_race_phase( int phase ) {
  sprintf(txbuff, "R%d%c", phase, EOL);
  sendSerialCommand(txbuff);
}


// Funciones básicas de envío/recepción de comandos
///////////////////////////////////////

// vars usados ​​solo en estas funciones
Stream*  _stream = &Serial;
int      _bufIdx;

int checkSerial(char *   buf) {
  while (_stream->available()) {
    if (_bufIdx < REC_COMMAND_BUFLEN - 2) {
      char data = _stream->read();
      if (data == EOL) {
        int cmsSize = _bufIdx;
        buf[_bufIdx++] = '\0';
        _bufIdx = 0;
        return (cmsSize);
      } else {
        buf[_bufIdx++] = data;
      }
    } else {
      // Buffer lleno
      // Reinicio y error de retorno
      buf[_bufIdx++] = '\0';
      _bufIdx = 0;
      return (-2);
    }
  }
  return (0);
}


void sendSerialCommand(char* str) {
  // obtener la longitud del comando
  int dlen = 0;
  for (; dlen < TX_COMMAND_BUFLEN; dlen++ ) {
    if (*(str + dlen) == EOL ) {
      dlen++; // enviar EOC
      break;
    }
  }
  _stream->write(str, dlen);
  return;
}
