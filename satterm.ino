#include <LiquidCrystal.h>
#include <Servo.h>

// THERMALSAT - Sistema autonomo de controle termico e energetico


const int PIN_TEMP = A0;
const int PIN_BATERIA = A1;

const int LED_VERDE = 2;
const int LED_AMARELO = 3;
const int LED_VERMELHO = 4;

const int BUZZER = 5;

const int AQUECEDOR = 6;
const int RESFRIADOR = 7;
const int CARGA_NAO_CRITICA = 8;

const int PIN_SERVO = 9;




LiquidCrystal lcd(10, 11, 12, 13, A2, A3);
Servo servoAleta;


// ===================== ESTADOS =====================

const int ESTADO_NORMAL = 1;
const int ESTADO_ATENCAO = 2;
const int ESTADO_TEMP_ALTA = 3;
const int ESTADO_TEMP_BAIXA = 4;
const int ESTADO_BATERIA_CRITICA = 5;
const int ESTADO_BATERIA_NULA = 6;
const int ESTADO_FALHA_SENSOR = 7;

int estadoAtual = ESTADO_NORMAL;
int estadoAnterior = ESTADO_NORMAL;

const float TEMP_MIN_PLAUSIVEL = -40.0;
const float TEMP_MAX_PLAUSIVEL = 90.0;

const float TEMP_CRITICA_BAIXA = 0.0;
const float TEMP_ATENCAO_BAIXA = 10.0;
const float TEMP_ATENCAO_ALTA = 35.0;
const float TEMP_CRITICA_ALTA = 45.0;

const int BAT_NULA = 0;
const int BAT_CRITICA = 5;
const int BAT_ATENCAO = 50;


//TEMPORIZACAO COM MILLIS

unsigned long tempoSensores = 0;
unsigned long tempoLCD = 0;
unsigned long tempoSerial = 0;
unsigned long tempoBuzzer = 0;

const unsigned long INTERVALO_SENSORES = 200;
const unsigned long INTERVALO_LCD = 500;
const unsigned long INTERVALO_SERIAL = 1000;
const unsigned long INTERVALO_BUZZER = 300;


//VARIAVEIS DO SISTEMA 

float temperatura = 25.0;
float ultimaTemperaturaValida = 25.0;

int bateria = 100;
int ultimaBateriaValida = 100;

bool erroTempAtual = false;
bool erroBatAtual = false;

int contadorErroTemp = 0;
int contadorErroBat = 0;

bool buzzerAtivo = false;

String ultimaLinha0 = "";
String ultimaLinha1 = "";

// SETUP


void setup() {
  Serial.begin(9600);

  configurarPinos();

  servoAleta.attach(PIN_SERVO);

  lcd.begin(16, 2);
  lcd.clear();

  desligarTudo();

  imprimirDiagnosticoInicial();

  escreverLCD("SatTherm", "Sistema OK");
}


// LOOP PRINCIPAL

void loop() {
  unsigned long agora = millis();

  if (agora - tempoSensores >= INTERVALO_SENSORES) {
    tempoSensores = agora;

    lerSensores();
    definirEstado();
    executarAtuadores();
  }

  if (agora - tempoLCD >= INTERVALO_LCD) {
    tempoLCD = agora;
    atualizarLCD();
  }

  if (agora - tempoSerial >= INTERVALO_SERIAL) {
    tempoSerial = agora;
    imprimirSerial();
  }

  if (agora - tempoBuzzer >= INTERVALO_BUZZER) {
    tempoBuzzer = agora;
    controlarBuzzer();
  }
}


// CONFIGURACAO DOS PINOS

void configurarPinos() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  pinMode(AQUECEDOR, OUTPUT);
  pinMode(RESFRIADOR, OUTPUT);
  pinMode(CARGA_NAO_CRITICA, OUTPUT);
}


// =======================================================
// LEITURA DOS SENSORES


void lerSensores() {
  lerTemperatura();
  lerBateria();
}

void lerTemperatura() {
  int leituraTemp = analogRead(PIN_TEMP);
  float tensaoTemp = leituraTemp * (5.0 / 1023.0);

  // Formula para sensor TMP36:
  // Temperatura C = (tensao - 0.5) * 100
  float tempCalculada = (tensaoTemp - 0.5) * 100.0;

  if (validarTemperatura(tempCalculada)) {
    temperatura = tempCalculada;
    ultimaTemperaturaValida = tempCalculada;
    erroTempAtual = false;
  } else {
    temperatura = ultimaTemperaturaValida;
    erroTempAtual = true;
    contadorErroTemp++;
  }
}

void lerBateria() {
  int leituraBat = analogRead(PIN_BATERIA);
  int batCalculada = map(leituraBat, 0, 1023, 0, 100);

  if (validarBateria(batCalculada)) {
    bateria = batCalculada;
    ultimaBateriaValida = batCalculada;
    erroBatAtual = false;
  } else {
    bateria = ultimaBateriaValida;
    erroBatAtual = true;
    contadorErroBat++;
  }
}

// VALIDACAO DAS LEITURAS

bool validarTemperatura(float temp) {
  if (temp < TEMP_MIN_PLAUSIVEL) {
    return false;
  }

  if (temp > TEMP_MAX_PLAUSIVEL) {
    return false;
  }

  return true;
}

bool validarBateria(int bat) {
  if (bat < 0) {
    return false;
  }

  if (bat > 100) {
    return false;
  }

  return true;
}


// MAQUINA DE ESTADOS


void definirEstado() {
  estadoAnterior = estadoAtual;

  if (erroTempAtual || erroBatAtual) {
    estadoAtual = ESTADO_FALHA_SENSOR;
  }
  else if (temperatura > TEMP_CRITICA_ALTA) {
    estadoAtual = ESTADO_TEMP_ALTA;
  }
  else if (temperatura < TEMP_CRITICA_BAIXA) {
    estadoAtual = ESTADO_TEMP_BAIXA;
  }
  else if (bateria <= BAT_NULA) {
    estadoAtual = ESTADO_BATERIA_NULA;
  }
  else if (bateria <= BAT_CRITICA) {
    estadoAtual = ESTADO_BATERIA_CRITICA;
  }
  else if ((temperatura >= TEMP_ATENCAO_ALTA && temperatura <= TEMP_CRITICA_ALTA) ||
           (temperatura >= TEMP_CRITICA_BAIXA && temperatura < TEMP_ATENCAO_BAIXA) ||
           (bateria > BAT_CRITICA && bateria <= BAT_ATENCAO)) {
    estadoAtual = ESTADO_ATENCAO;
  }
  else {
    estadoAtual = ESTADO_NORMAL;
  }

  if (estadoAtual != estadoAnterior) {
    imprimirMudancaEstado();
  }
}


// ATUADORES

void executarAtuadores() {
  desligarAtuadores();

  switch (estadoAtual) {
    case ESTADO_NORMAL:
      digitalWrite(LED_VERDE, HIGH);
      digitalWrite(CARGA_NAO_CRITICA, HIGH);
      servoAleta.write(0);
      break;

    case ESTADO_ATENCAO:
      digitalWrite(LED_AMARELO, HIGH);
      digitalWrite(CARGA_NAO_CRITICA, HIGH);
      servoAleta.write(0);
      break;

    case ESTADO_TEMP_ALTA:
      digitalWrite(LED_VERMELHO, HIGH);
      digitalWrite(RESFRIADOR, HIGH);
      digitalWrite(CARGA_NAO_CRITICA, HIGH);
      servoAleta.write(90);
      break;

    case ESTADO_TEMP_BAIXA:
      digitalWrite(LED_VERMELHO, HIGH);
      digitalWrite(AQUECEDOR, HIGH);
      digitalWrite(CARGA_NAO_CRITICA, HIGH);
      servoAleta.write(0);
      break;

    case ESTADO_BATERIA_CRITICA:
      digitalWrite(LED_AMARELO, HIGH);
      digitalWrite(CARGA_NAO_CRITICA, LOW);
      servoAleta.write(0);
      break;

    case ESTADO_BATERIA_NULA:
      digitalWrite(LED_VERMELHO, HIGH);
      digitalWrite(CARGA_NAO_CRITICA, LOW);
      servoAleta.write(0);
      break;

    case ESTADO_FALHA_SENSOR:
      digitalWrite(LED_VERMELHO, HIGH);
      digitalWrite(CARGA_NAO_CRITICA, LOW);
      servoAleta.write(0);
      break;
  }
}

void desligarAtuadores() {
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERMELHO, LOW);

  digitalWrite(AQUECEDOR, LOW);
  digitalWrite(RESFRIADOR, LOW);
  digitalWrite(CARGA_NAO_CRITICA, LOW);
}

void desligarTudo() {
  desligarAtuadores();
  noTone(BUZZER);
  servoAleta.write(0);
}


// BUZZER NAO BLOQUEANTE

void controlarBuzzer() {
  bool deveApitar = false;
  int frequencia = 0;

  if (estadoAtual == ESTADO_TEMP_ALTA || estadoAtual == ESTADO_TEMP_BAIXA) {
    deveApitar = true;
    frequencia = 1200;
  }
  else if (estadoAtual == ESTADO_BATERIA_NULA) {
    deveApitar = true;
    frequencia = 1000;
  }
  else if (estadoAtual == ESTADO_FALHA_SENSOR) {
    deveApitar = true;
    frequencia = 1500;
  }

  if (!deveApitar) {
    noTone(BUZZER);
    buzzerAtivo = false;
    return;
  }

  buzzerAtivo = !buzzerAtivo;

  if (buzzerAtivo) {
    tone(BUZZER, frequencia);
  } else {
    noTone(BUZZER);
  }
}

// LCD


void atualizarLCD() {
  String linha0 = "";
  String linha1 = "";

  if (estadoAtual == ESTADO_FALHA_SENSOR) {
    linha0 = "FALHA SENSOR";

    if (erroTempAtual) {
      linha1 = "TEMP INVALIDA";
    }
    else if (erroBatAtual) {
      linha1 = "BAT INVALIDA";
    }
    else {
      linha1 = "VERIFICAR";
    }

    escreverLCD(linha0, linha1);
    return;
  }

  linha0 = "T:";
  linha0 += String(temperatura, 1);
  linha0 += "C B:";
  linha0 += String(bateria);
  linha0 += "%";

  linha1 = nomeEstadoCurto(estadoAtual);

  escreverLCD(linha0, linha1);
}

void escreverLCD(String linha0, String linha1) {
  linha0 = limitar16(linha0);
  linha1 = limitar16(linha1);

  if (linha0 != ultimaLinha0) {
    lcd.setCursor(0, 0);
    lcd.print("                ");
    lcd.setCursor(0, 0);
    lcd.print(linha0);
    ultimaLinha0 = linha0;
  }

  if (linha1 != ultimaLinha1) {
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    lcd.print(linha1);
    ultimaLinha1 = linha1;
  }
}

String limitar16(String texto) {
  if (texto.length() > 16) {
    texto = texto.substring(0, 16);
  }

  while (texto.length() < 16) {
    texto += " ";
  }

  return texto;
}


// SERIAL

void imprimirDiagnosticoInicial() {
  Serial.println("================================");
  Serial.println(" SATTHERM GUARDIAN");
  Serial.println(" DIAGNOSTICO INICIAL");
  Serial.println("================================");
  Serial.println("Versao estavel para Tinkercad");
  Serial.println("Temporizacao: millis()");
  Serial.println("delay(): nao utilizado no loop");
  Serial.println("EEPROM: simulada por contadores RAM");
  Serial.println("Sistema autonomo iniciado.");
  Serial.println("Bateria: 0%=NULA | 1-5%=CRITICA | 6-50%=ATENCAO");
  Serial.println("================================");
}

void imprimirSerial() {
  Serial.println("----------------------------");

  Serial.print("Temperatura: ");
  Serial.print(temperatura, 1);
  Serial.println(" C");

  Serial.print("Bateria: ");
  Serial.print(bateria);
  Serial.println(" %");

  Serial.print("Estado: ");
  Serial.println(nomeEstado(estadoAtual));

  Serial.print("Erro temperatura atual: ");
  Serial.println(erroTempAtual ? "SIM" : "NAO");

  Serial.print("Erro bateria atual: ");
  Serial.println(erroBatAtual ? "SIM" : "NAO");

  Serial.print("Contador erro temperatura: ");
  Serial.println(contadorErroTemp);

  Serial.print("Contador erro bateria: ");
  Serial.println(contadorErroBat);

  Serial.print("Aquecedor D6: ");
  Serial.println(digitalRead(AQUECEDOR) == HIGH ? "ON" : "OFF");

  Serial.print("Resfriador D7: ");
  Serial.println(digitalRead(RESFRIADOR) == HIGH ? "ON" : "OFF");

  Serial.print("Carga D8: ");
  Serial.println(digitalRead(CARGA_NAO_CRITICA) == HIGH ? "ON" : "OFF");

  Serial.print("Servo: ");
  if (estadoAtual == ESTADO_TEMP_ALTA) {
    Serial.println("ABERTO");
  } else {
    Serial.println("FECHADO");
  }
}

void imprimirMudancaEstado() {
  Serial.print("[MUDANCA DE ESTADO] ");
  Serial.print(nomeEstado(estadoAnterior));
  Serial.print(" -> ");
  Serial.println(nomeEstado(estadoAtual));
}

String nomeEstado(int estado) {
  switch (estado) {
    case ESTADO_NORMAL:
      return "NORMAL";

    case ESTADO_ATENCAO:
      return "ATENCAO";

    case ESTADO_TEMP_ALTA:
      return "TEMPERATURA ALTA";

    case ESTADO_TEMP_BAIXA:
      return "TEMPERATURA BAIXA";

    case ESTADO_BATERIA_CRITICA:
      return "BATERIA CRITICA";

    case ESTADO_BATERIA_NULA:
      return "BATERIA NULA";

    case ESTADO_FALHA_SENSOR:
      return "FALHA DE SENSOR";
  }

  return "DESCONHECIDO";
}

String nomeEstadoCurto(int estado) {
  switch (estado) {
    case ESTADO_NORMAL:
      return "STATUS:NORMAL";

    case ESTADO_ATENCAO:
      return "STATUS:ATENCAO";

    case ESTADO_TEMP_ALTA:
      return "TEMP ALTA!";

    case ESTADO_TEMP_BAIXA:
      return "TEMP BAIXA!";

    case ESTADO_BATERIA_CRITICA:
      return "BAT CRITICA!";

    case ESTADO_BATERIA_NULA:
      return "BAT NULA!";

    case ESTADO_FALHA_SENSOR:
      return "FALHA SENSOR";
  }

  return "DESCONHECIDO";
}