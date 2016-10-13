/******************************************************************
*Este programa é propriedade da Equipe de Barcos Solares VENTO 
*SUL, é proibida sua reprodução e apresentação total ou parcial 
*sem o conhecimento prévio do Capitão ou membro responsável. 
*
*Descrição: Header file do codigo da metria, contem as variaveis.
*******************************************************************/

//Atualizar o campo para mostrar a versão ao usuário
const String Version = ("Version 1.10 - 19/09/2016"); 

//variavel LCD
//LiquidCrystal lcd(rs_lcd,enable_lcd,d4_lcd,d5_lcd,d6_lcd,d7_lcd);

//Variaveis relacionadas ao BMV
//Indica se ocorre um erro de checksum ou timeout da serial
//boolean SYSTEM_ERROR = false;
// Diz se a string está ou não completa
boolean stringComplete = false;
//variavel que armanzena a quatidade de dados colhidos
int readByteNumber = 0;
//varialve usada no checksum, vide BMVReadSerialData ()
int inputnumber = 0;
char serialstring[140]=""; 
const int BMValSIZE = 22;
float BMValues[BMValSIZE];
String BMVlabel[BMValSIZE]={"V","VS","I","CE","SOC","TTG","H1","H2","H3","H4","H5","H6","H7","H8","H9","H10","H11","H12","H13","H14","H15","H16"};

//Variáveis relacionadas ao BMS Vento Sul
int readByteNumberBMS1 = 0;
char serialstringBMS1[140]="";
int readByteNumberBMS2 = 0;
char serialstringBMS2[140]=""; 

//Variáveis para definir intervalos de tempo
//variavel de tempo para armanzenar ultimo recebimento bem sucessedido pelo BMV
unsigned long TimeLastBMV;
//último salvamento de dados no cartão SD
unsigned long TimeLastSaveData;
//último envio de dados pela serial
unsigned long TimeLastPrint;
//último envio de dados por telemetria
unsigned long TimeLastRadio;
//último recebimento de dados pelo GPS
unsigned long TimeLastGPS; 

//Variáveis relacionadas a medição de temperatura
float Temp[NumberTempSensors]={};//Temperaturas dos termistore (ºC)
String TempLabel[5]={"T1","T2","T3","T4","T5"}; 

//Variáveis relacionados com o GPS
float kmph = 0; //armazena o velocidade em km/h
float knot = 0; //armazena o velocidade em nos
boolean newDataGPS; //indica um novo dado presente no GPS

//Variáveis relacionados com a medição de rotação
volatile byte half_revolutions;
unsigned int rpm=0;
unsigned long timeold=0;

//Variaveis DEBUG
boolean DEBUG_BMV = false;
boolean DEBUG_GPS = false;
boolean DEBUG_ROTATION = false;
boolean DEBUG_SD = false;

//Declaracoes
//Declarações relacionadas ao RTC
RTC_DS1307 RTC;
//Declarações relacionadas ao GPS
//SoftwareSerial(rxPin, txPin)
SoftwareSerial serial_gps(gps_TX_Pin, gps_RX_Pin); 

//Variáveis relacionadas ao BMV
//Associa a variavel a sua sequecia de escrita no protocolo do BMV600.
static const int V = 0;          
static const int VS = 1;
static const int I = 2;
static const int CE = 3;
static const int SOC = 4;
static const int TTG = 5;
static const int H1 = 6;
static const int H2 = 7;
static const int H3 = 8;
static const int H4 = 9;
static const int H5 = 10;
static const int H6 = 11;
static const int H7 = 12;
static const int H8 = 13;
static const int H9 = 14;
static const int H10 = 15;
static const int H11 = 16;
static const int H12 = 17;
static const int H13 = 18;
static const int H14 = 19;
static const int H15 = 20;
static const int H16 = 21;
