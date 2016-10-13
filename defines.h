/******************************************************************
*Este programa é propriedade da Equipe de Barcos Solares VENTO
*SUL, é proibida sua reprodução e apresentação total ou parcial
*sem o conhecimento prévio do Capitão ou membro responsável.
*
*Descrição: Header file do codigo da metria, contem as definicoes.
*******************************************************************/

//Definições escolhidas pelo usuário:
//Define tempo entre envios via nF24L01
#define TimeToRadio 5000
//Define tempo entre a gravação de amostras
#define TimeSaveData 2000
//Define tempo para envio via serial
#define TimeToPrint 5000
//Numero de sensores de temperatura, conectados a partir de A0
#define NumberTempSensors 2
//Se true envia as mensagens de erro geradas no programa
#define DEBUG true
//Se true ativa o debug do lcd
#define LCD_DEBUG true
//Define se a backlight do lcd vai estar ligado
#define lcdLight HIGH
//Define tempo de leitura da rotação (em ms)
#define TempLeitRot 5000

//Definições em função do hardware:
//Define o pino CS ligado ao cartão SD
#define chipSelectSD 53
#define chipEnableRF24 48
#define chipSelectRF24 49
#define gps_TX_Pin 10
#define gps_RX_Pin 11
/*definicao pinos lcd
#define v0_lcd 6
#define rs_lcd 5
#define enable_lcd 4
#define d4_lcd 3
#define d5_lcd 2
#define d6_lcd 1
#define d7_lcd 0*/

//Definições de uso em software:
//Consntante para conversão kelvin - Celcius
#define kelvin 273.15
