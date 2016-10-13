/*******************************************************************************************************************************************************************
*Este programa é propriedade da Equipe de Barcos Solares VENTO
*SUL, é proibida sua reprodução e apresentação total ou parcial
*sem o conhecimento prévio do Capitão ou membro responsável.
*
*Descrição: Programa que controla a metria do barco.
*
*Contém partes do programa desenvolvido por "Pezibaer" em Photovoltaik Forum, 23/03/2014.
*- Discussão em: http://www.photovoltaikforum.com/pv-inselanlagen-f57/victron-bmv-600-inselsteuern-gt-arduino-code-t98910.html
*- Código em: http://www.polz.info/download/BMV600serial-V1.ino
*- Código em: http://www.polz.info/BMV700.ino
*
*Outras referências:
*- BMV Text Protocol disponível em: http://www.plaisance-pratique.com/IMG/pdf/BMV_Text_Protocol_1_.pdf
*- BMV manual disponível em: http://www.georgekniest.nl/pdf/manuals/bmv602.pdf
*- Informações sobre o GPS Garmin: https://www.syz.com/gps/gpsinfo.pdf
*                                  http://www.gpsinformation.org/dale/interface.htm
*- Informação sobre medição de rotação em: http://playground.arduino.cc/Main/ReadingRPM
*
*O código a seguir foi escrito originalmente por Claudio Abilio da Silveira
*(claudio_ards@hotmail.com), membro da equipe VENTO SUL
*Contribuições: Abner do Canto Pereira (abnercpereira@hotmail.com)
*               Clarice Scheibe Ribeiro (scheibeclarice@gmail)
*
*************************************Informaço de Versao***********************************************************************************************************
*Version 1.01 - 13/04/2016 - Criação do código. Gravação de dados em cartão SD com a data fornecida por Real Time Clock.
*  Version 1.02 - 14/04/2016 - Inclusão da leitura de dados seriais do BMV600/602 por meio da serial1.
*  Version 1.03 - 17/04/2016 - Mudança de momeclatura de "chipSelect" para "chipSelectSD", visando expansão de uso da SPI;
*                              Inclusão de "String BMVlabel[]" para incluir o nome do parametro ao valor salvo no cartão SD.
*  Version 1.04 - 18/04/2016 - Correção de erro no código original do BMV, de "static const int H5 = 11;" para "static const int H5 = 10;" e sucessivamente;
*                              Mudança de "dtostrf(value,1,1,temp)" para "dtostrf(value,1,0,temp)", excluindo casa após a virgula no dados salvos.
*  Version 1.05 - 10/07/2016 - Inclusão da função "SerialOutput ()" para enviar os dados via serial para o PC.
*                              Inclusão da medição de temperatura utilizando termistor NTC ligado as entradas analógicas.
*                              Inclusão de "#define DEBUG" para permitir ao usuário optar por visualizar erros do sitema;
*                              Exclusão dos caracteres ":" em "BMVlabel[BMValSIZE]";
*                              Divisão de "String ReturnDateNow ()" em "String ReturnDateNow ()" e "String ReturnHourNow ()";
*                              Comunicação com o BMS Vento Sul por meio da serial2 e serial3;
*                              Leitura do transdutor de rotação com saída analógica proporcional a rotação.
*  Version 1.06 - 13/07/2016 - Retirada da verificação "&&SYSTEM_ERROR" nas funções "SerialOutput()" e "SaveDataInSD()";
*                              Transferência da função "ReadTemps()" do loop para dentro das funções "SerialOutput()" e "SaveDataInSD()".
*  Version 1.07 - 24/08/2016 - Inclusão da medição de velocidade utilizando o módulo GPS Ublox NEO6MV2.
*  Version 1.08 - 19/09/2016 - Substituição da medição de velocidade utilizando o módulo GPS Ublox NEO6MV2 pela utilização do GPS Garmin GPSmap78S
*                              Substituição da medição de rotação de analógica para leitura digital direta do sensor de rotação.
*  Version 1.09 - 20/09/2016 - Insercao de sistema para debugar o cdigo em LCD
*  Version 1.10 - 03/10/2016 - troca de pino da ingterrupço para pino digital 18, lcd comentado
*  Version 1.11 - 04/10/2016 - Separacao do codigo em Metria_VS.ino / defines.h / variaveis.c
********************************************************************************************************************************************************************/
#include <string.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include "defines.h"
#include "variaveis.h"

void setup(){

  /* lcd.begin(16,2);
  lcd.print("system started");
  pinMode(v0_lcd, INPUT);
  digitalWrite(v0_lcd, lcdLight);*/

  Serial.begin(9600);
  Serial1.begin (19200);
  Serial1.setTimeout(10);
  Serial2.begin (9600);
  Serial2.setTimeout(10);
  Serial3.begin (9600);
  Serial3.setTimeout(10);
  serial_gps.begin(9600);
  serial_gps.setTimeout(10);

  pinMode(SS, OUTPUT);

  RTC.begin();

  attachInterrupt(digitalPinToInterrupt(3), rpm_fun, CHANGE);

  for(int i=0;i<BMValSIZE;i++)
  BMValues[i]=0;

  //Mensagem de informacao do firmware
  Serial.print (F("FIRMWARE VERSION: "));
  Serial.println (Version);
  Serial.println (F("STARTING THE SYSTEM..."));

  //Teste de comunicacao com SD
  if (!SD.begin(chipSelectSD))
  Serial.println (F(" X Failure on the card or not present"));
  else
  Serial.println (F(" - Card starting OK!"));

  //Teste de comunicacao com RTC
  if (!RTC.isrunning()) {
    Serial.println(F(" X RTC not starting"));
    RTC.adjust(DateTime(16,8,26,15,12,0));
  }
  else
  Serial.println(F(" - RTC starting OK!"));
} //Fim de setup

void loop() {
  BMVReadSerialData();
  GPS_Data();
  SaveDataInSD();
  SerialOutput();
  //if(LCD_DEBUG)
  //  lcdDebug();
} //Fim de loop

/**********************Declaraçao das funcoes**********************************/
/*Funções serialEvent - Realiza a leitura das seriais a cada loop**************/
/*(Não precisam ser chamadas)**************************************************/

void serialEvent1() {
  while(Serial1.available()){
    readByteNumber = Serial1.readBytes(serialstring,140);
    stringComplete = true;
  }
}

void serialEvent2() {
  while(Serial2.available()){
    readByteNumberBMS1 = Serial2.readBytes(serialstringBMS1,140);
  }
  Serial.println(serialstringBMS1);
}

void serialEvent3() {
  while(Serial3.available()){
    readByteNumberBMS2 = Serial3.readBytes(serialstringBMS2,140);
  }
  Serial.println(serialstringBMS2);
}

/*Funções de leitura do BMV600*************************************************/

//início do processo de leitura
void BMVReadSerialData (){
  if(stringComplete)
  {
    inputnumber = 0;
    for(int i = 0 ; i < readByteNumber ; i++)
    inputnumber += (int)serialstring[i];
    //(nota: a soma dos bytes enviados pelo BMV é um multiplo de 256,
    //é o checksum)
    if((inputnumber % 256)==0)
    {
      convertTo(serialstring);
      stringComplete = false;
      DEBUG_BMV = false;
      //SYSTEM_ERROR = false;
      TimeLastBMV = millis();
    }
    else if (!DEBUG_BMV)
    {
      if (DEBUG) Serial.println(F(" X Checksum erro"));
      DEBUG_BMV = true;
    }
  }
  //se não houver erro antes e o timeout estourar
  if(!DEBUG_BMV && (millis() - TimeLastBMV) > 2000)
  {
    if (DEBUG) {
      //indica que houve perda de comunicação com o BMV600
      Serial.println(F(" X NOT SYNC"));
      DEBUG_BMV = true;
    }
  }
}

//dá inicio ao tratamento dos dados
void convertTo(String s){
  //envia pela serial os dados recebidos do BMV
  //Serial.println(s);
  int startindex=2;
  int endindex=3;
  String line = "";
  while(startindex < readByteNumber)
  {
    endindex = s.indexOf("\n",startindex);
    if(endindex < startindex || endindex > readByteNumber)
    {
      endindex = readByteNumber;
    }
    line = s.substring(startindex,endindex);
    getValues(line);
    startindex = endindex+1;
  }
}

//indentifica o rótulo e o valor de cada variavel dentro do dados recebidos
void getValues(String s){
  int slength = s.length();
  int startindex=0;
  int endindex=0;
  endindex = s.indexOf("\t",startindex);
  String name = s.substring(startindex,endindex);
  String value = s.substring(endindex+1,slength);
  //chama a função que armazena e converte os dados
  setValues(name, charToFloat(value));
}

//função que armazena os dados recebidos no formato float na string "BMValues"
//em sua respectiva ordem
void setValues(String name,float value){
  if(name == "V")        BMValues[V]   = value;
  else if(name == "VS")  BMValues[VS]  = value;
  else if(name == "I")   BMValues[I]   = value;
  else if(name == "CE")  BMValues[CE]  = value;
  else if(name == "SOC") BMValues[SOC] = value;
  else if(name == "TTG") BMValues[TTG] = value;
  else if(name == "H1")  BMValues[H1]  = value;
  else if(name == "H2")  BMValues[H2]  = value;
  else if(name == "H3")  BMValues[H3]  = value;
  else if(name == "H4")  BMValues[H4]  = value;
  else if(name == "H5")  BMValues[H5]  = value;
  else if(name == "H6")  BMValues[H6]  = value;
  else if(name == "H7")  BMValues[H7]  = value;
  else if(name == "H8")  BMValues[H8]  = value;
  else if(name == "H9")  BMValues[H9]  = value;
  else if(name == "H10") BMValues[H10] = value;
  else if(name == "H11") BMValues[H11] = value;
  else if(name == "H12") BMValues[H12] = value;
  else if(name == "H13") BMValues[H13] = value;
  else if(name == "H14") BMValues[H14] = value;
  else if(name == "H15") BMValues[H15] = value;
  else if(name == "H16") BMValues[H16] = value;
}

//função que converte a entrada "value" de char para float.
float charToFloat(String value){
  char tmp[value.length()];
  //joga "value" dentro de um buffer "tmp"
  value.toCharArray(tmp,value.length());
  //converte o buffer "tmp" de string para float e retona ele
  return atof(tmp);
}

/*funções do RTC***************************************************************/

String ReturnDateNow (){
  String dateNow = "";
  DateTime now = RTC.now();
  //Monta uma string com dia/mes/ano hora:minuto:segundo
  dateNow += (now.day());
  dateNow += "/";
  dateNow += (now.month());
  dateNow += "/";
  dateNow += (now.year());
  return (dateNow);
}

String ReturnHourNow (){
  String dateNow = "";
  //Recuperando a data e hora atual
  DateTime now = RTC.now();
  dateNow += (now.hour());
  dateNow += ":";
  dateNow += (now.minute());
  dateNow += ":";
  dateNow += (now.second());
  return (dateNow);
}

/*Funções de leitura de tempertura com termistor NTC***************************/

void ReadTemps(){
  //Tensão na entrada analógica, oriunda do divisor de tensão
  float VoDiv;
  //Resistor do divisor de tensão.
  float RD=5600.0;
  //RT=Resistência do termistor a 25ºC, T1=25º em K, bT = beta
  float RT=10000.0, T1T=298.15, bT=4250.0;
  //T2T=Temperatura do termistor (K), RTmeasured=resistência medida do termistor
  float T2T, RTmeasured;

  for (int x=0; x < NumberTempSensors; x++){
    VoDiv = (analogRead(x))*(5.00/1024.00);
    RTmeasured = ((-VoDiv*RD)/(VoDiv-5.00));
    //Equação b do termistor
    T2T=1/((1/T1T)+(1/bT)*log(RTmeasured/RT));
    Temp[x] = (T2T-kelvin);
  }
}

/*Função de leitura do transdutor de rotação***********************************/

int Rotation (){
  int amostras=4;
  int timeRot = millis();
  while(millis()-timeRot < TempLeitRot){
    if (rpm >= 120 && rpm < 480)
    amostras = 4;
    else if (rpm >= 480 && rpm < 840)
    amostras = 16;
    else if (rpm >= 840 && rpm < 1200)
    amostras = 28;
    else if (rpm >= 1200 && rpm < 1560)
    amostras = 40;
    else if (rpm >= 1560 && rpm < 1920)
    amostras = 52;
    else if (rpm >= 1920 && rpm < 2280)
    amostras = 64;
    else if (rpm >= 2280)
    amostras = 76;
    else if (rpm < 120)
    rpm=0;
    else if (DEBUG){
      Serial.print(" X ERROR IN ROTATION");
      DEBUG_ROTATION = true;
    }

    //realiza o calculo e imprime na serial
    if (half_revolutions >= amostras){
      rpm = 30*1000/(millis() - timeold)*half_revolutions;
      timeold = millis();
      half_revolutions = 0;
      //Serial.println(rpm,DEC);
      if (DEBUG)
      DEBUG_ROTATION = false;
    }
    else if ((millis()-timeold) > 1000){
      rpm = 0;
      //Serial.println(" zero");
      if (DEBUG) {
        Serial.println(" X No rotation motor detected");
        DEBUG_ROTATION = false;
      }
    }
  }
  return (rpm);
}

void rpm_fun(){
  //A cada rotação esta interrupção é chamada duas vezes
  half_revolutions++;
}

/*Função de leitura módulo GPS*************************************************/

void GPS_Data(){
  //Cria uma variavel para armazenar a os dados recevidos do GPS
  char serialstringGPS[60]="";
  while (serial_gps.available()){
    serial_gps.readBytes(serialstringGPS,60);
    newDataGPS = true;
  }

  if (newDataGPS == true){
    //Serial.print(serialstringGPS);
    TimeLastGPS = millis();
    //Chama o processamento do valor coletado na serial, exemplo de pacote "@000607204655N6012249E01107556S015+00130E0021N0018U0000\r\n";
    processGPSdata(serialstringGPS);
    //garante que esse if seja executado somente quando ocorre serial_gps.available
    newDataGPS = false;
    DEBUG_GPS = false;
  }
  if (DEBUG) if ((millis()- TimeLastGPS) > 5000) {
    //Caso nenhum dado seja recebido depois de determinado tempo
    Serial.println(" X No characters received from GPS: check wiring");
    DEBUG_GPS = true;
  }
}

//dá inicio ao tratamento dos dados do GPS
void processGPSdata(String s){
  //se o pacote possui possui tamanho igual a 57 (tamanho que deveria ter se correto)
  if((s.length())== 57){
    //cria variável para armazenar a velocidade do sentido LESTE-OESTE
    String VeloLO = "";
    //indem para a velocidade do sentido NORTE-SUL
    String VeloNS = "";
    //armazenará o cálculo da velocidade resultante, em m/s
    float mps;

    //busca o valor da velocidade LESTE-OESTE na sua posição dentro do pacote
    VeloLO = s.substring(41,46);
    //idem para NORTE-SUL
    VeloNS = s.substring(46,51);

    float a = charToFloat(VeloLO)/10;
    float b = charToFloat(VeloNS)/10;

    //calcula velocidade resultante dos vetores de velocidade
    mps = sqrt((a*a)+(b*b));
    //converte de m/s para km/h
    kmph = mps*3.6;
    //converte de m/s para nos
    knot = mps*1.943844;

    //Envia os dados intermediários e finais (opcional)
    /*Serial.print(" "); Serial.print(VeloLO);
    Serial.print(" "); Serial.print(VeloNS);
    Serial.print(" "); Serial.print(a);
    Serial.print(" "); Serial.print(b);
    Serial.print(" "); Serial.print(mps);
    Serial.print(" "); Serial.print(kmph);
    Serial.print(" "); Serial.println(knot); */
    if (DEBUG)
    DEBUG_GPS = false;
  }
  else if (DEBUG) {
    //se o pacote recebido for maior/menor que os esperado
    Serial.println(" X Invalid characters received from GPS: check parameters");
    DEBUG_GPS = true;
  }
}

/*Função de DEBUG do lcd*******************************************************/

/*void lcdDebug() {
lcd.clear();
lcd.print("BMV:");
lcd.print(DEBUG_BMV,BIN);
lcd.setCursor(6,1);
lcd.print("GPS:");
lcd.print(DEBUG_GPS,BIN);
lcd.setCursor(1,2);
lcd.print("ROT:");
lcd.print(DEBUG_ROTATION,BIN);
lcd.setCursor(6, 2);
lcd.print("SD:");
lcd.print(DEBUG_SD,BIN);
}*/

/*Funções do cartão SD*********************************************************/

//salva os dados no cartão SD
void SaveDataInSD(){
  //se tempo desde a ultima gravação é superior ao definido em "TimeSaveData"
  if (((millis() - TimeLastSaveData) >=  TimeSaveData)){
    //marca o instante de tempo em que os dados foram salvos
    TimeLastSaveData = millis();

    //faz a leitura dos termistores
    ReadTemps();

    String LabelDate = "DATE";
    //variavel que armazena a data completa [dia/mes/ano]
    String CompleteDate = "";
    //chama função que recupera a data atual no RCT
    CompleteDate = ReturnDateNow();

    String LabelHour = "HOUR";
    //variavel que armazena a hora completa [hora:minuto:segundo]
    String CompleteHour = "";
    //chama função que recupera a data atual no RCT
    CompleteHour = ReturnHourNow();

    //Bloco para gravação de dados no cartao SD
    //Abre o arquivo para gravação
    File dataFile = SD.open("METRIA.txt", FILE_WRITE);
    //se o arquivo estiver disponível
    if (dataFile){
      digitalWrite(13,HIGH);

      //escreve a data
      dataFile.print (CompleteDate);
      //acrescenta caracter
      dataFile.print (" ");

      //escreve a hora completa
      dataFile.print (CompleteHour);
      //acrescenta caracter
      dataFile.print (" ");

      //armazena um a um os valores recuperados do BMV
      for(int i = 0;i < BMValSIZE; i++) {
        //coloca o nome da variável
        dataFile.print (BMVlabel[i]);
        dataFile.print (":");
        //processa cada valor a serem gravados no SD, transformando float em char
        dataFile.print (generateDATA(i));
      }

      //armazena os valores de temperatura
      for(int i = 0;i < NumberTempSensors; i++) {
        //coloca o nome da variável
        dataFile.print (TempLabel[i]);
        dataFile.print (":");
        dataFile.print (floatToChar(Temp[i]));
        dataFile.print (" ");
      }

      dataFile.print ("S1:");
      dataFile.print (kmph);
      dataFile.print (" ");
      dataFile.print ("S2:");
      dataFile.print (knot);
      dataFile.print (" ");

      dataFile.print ("R1:");
      dataFile.print (Rotation ());

      //salva os dados coletados do BMV Vento Sul
      //dataFile.print (serialstringBMS1);
      //dataFile.print (serialstringBMS2);

      dataFile.println ("");
      dataFile.close();
      if (DEBUG) {
        Serial.println (F(" - Data saved in SD"));
        DEBUG_SD = false;
      }
      digitalWrite(13,LOW);
    }
    else if (DEBUG) {
      Serial.println (F(" X Error opening .txt in SD"));
      DEBUG_SD = true;
    }
  } //Fim if millis()
}

//converte valores de float para char, especificamente valores do BMV
String generateDATA(int nr){
  String dataString = floatToChar(BMValues[nr]);
  dataString += (" ");
  return dataString;
}

char* floatToChar(float value){
  static char temp[30];
  dtostrf(value,1,0,temp);
  return (temp);
}

/*Funções de envio via serial**************************************************/

void SerialOutput (){
  if (((millis() - TimeLastPrint) >=  TimeToPrint)){
    TimeLastPrint = millis();

    //faz a leitura dos termistores
    ReadTemps();

    String LabelDate = "DATE";
    String CompleteDate = "";
    CompleteDate = ReturnDateNow();

    String LabelHour = "HOUR";
    String CompleteHour = "";
    CompleteHour = ReturnHourNow();

    String LabelID = "ID";
    String ID = "Telemetria Guara VS";

    Serial.print (LabelID);
    Serial.print("\t");
    Serial.print (ID);
    Serial.print("\r");
    Serial.print("\n");

    Serial.print (LabelDate);
    Serial.print("\t");
    Serial.print (CompleteDate);
    Serial.print("\r");
    Serial.print("\n");

    Serial.print (LabelHour);
    Serial.print("\t");
    Serial.print (CompleteHour);

    for(int i = 0;i < BMValSIZE; i++) {
      Serial.print("\r");
      Serial.print("\n");
      Serial.print (BMVlabel[i]);
      Serial.print("\t");
      Serial.print (generateDATA(i));
    }

    for(int i = 0;i < NumberTempSensors; i++) {
      Serial.print("\r");
      Serial.print("\n");
      Serial.print (TempLabel[i]);
      Serial.print("\t");
      Serial.print (floatToChar(Temp[i]));
    }

    Serial.print("\r");
    Serial.print("\n");
    Serial.print ("S1:");
    Serial.print("\t");
    Serial.print (kmph);

    Serial.print("\r");
    Serial.print("\n");
    Serial.print ("S2:");
    Serial.print("\t");
    Serial.print (knot);

    Serial.print("\r");
    Serial.print("\n");
    Serial.print ("R1:");
    Serial.print("\t");
    Serial.print (Rotation ());

    //    Serial.print("\r");
    //    Serial.print("\n");
    //salva os dados coletados do BMV Vento Sul
    //    Serial.print (serialstringBMS1);
    //    Serial.print("\r");
    //    Serial.print("\n");
    //    Serial.print (serialstringBMS2);

    Serial.println ("\n");
  }
}
