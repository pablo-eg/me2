/**************** LIBRERIAS ****************/
//#include <Arduino.h>
#include <EtherCard.h>
#include <IPAddress.h>
#include <LiquidCrystal.h>
#include <Wire.h>

#include "timer.h"

/***************** DEFINES *****************/
#define BUFFER 20 // tamaño del buffer donde se van a guardar las muestras
#define LED 14
int8_t voltageSensor = A0;
int8_t currentSensor = A9;

/**************** VARIABLES ****************/
static byte mymac[] = { 0x1A, 0x2B, 0x3C, 0x4D, 0x5E, 0x6F };
static byte myip[] = { 192, 168, 0, 99 };
static byte gwip[] = { 192, 168, 0, 1 };
static byte dnsip[] = { 8,8,8,8 };
static byte mask[] = { 255,255,255,0 };
const int16_t dstPort = 12345;
const int16_t srcPort = 8888;
static byte dstIp[] = {192, 168, 0, 7};
static byte dstIp2[] = {192, 168, 1, 30};
byte Ethernet::buffer[700];

int16_t samplesV[BUFFER];
int16_t samplesI[BUFFER];
uint8_t s = 0;
float Vef = 0.0;
char Vef_c[8];
float Ief = 0.0;
char Ief_c[8];
const float offset_v = 505; // 2.466 V
const float offset_i = 505; // 2.466 V

char payload[13];

unsigned long	lastTime = millis();
unsigned long	lastTime2 = millis();
unsigned long	lastTime3 = millis();

int8_t rele1 = A8;
bool rele1State = 1; // 1 => apagado, 0 => prendido
bool rele1LastState = 0;
char power[6] = "000.0";
char voltage[6] = "000.0";

// se crea el objeto LCD (rs, en, d4, d5, d6, d7)
LiquidCrystal lcd(5, 4, 7, 3, 6, 2);

/********* FUNCIONES: DECLARACIONES ********/
void rms(void);
void armaPayload(void);
void udpSerialPrint(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len);
void refreshDisplay(void);

/****************** SETUP ******************/
void setup()
{
	// se deshabilitan las interrupciones
	cli();

  // configuracion de pines
	pinMode(LED, OUTPUT);
	pinMode(rele1, OUTPUT); // rele
	digitalWrite(rele1, rele1State);

	// inicializo el timer
  timer_init();

	// enciendo el timer
	timer_on();

  _delay_ms(1000);

  // inicializo el puerto serie
	Serial.begin(9600);

  // configuración de ethernet
	if(!ether.begin(sizeof Ethernet::buffer, mymac, 53))
  {
    Serial.println("No se ha podido acceder a la controlador Ethernet");
  }
  else
  {
    Serial.println("Controlador Ethernet inicializado" );
  }

	ether.dhcpSetup();	// agregar alerta para cuando no pueda conectarse
	// ether.staticSetup(myip, gwip, dnsip, mask); // Cuando seteo una dirección estática, no puedo enviar paquetes por UDP
	 ether.printIp("IP:  ", ether.myip);
	 ether.printIp("GW:  ", ether.gwip);
	// ether.printIp("DNS: ", ether.dnsip);

	// se elige el puerto 1337 para que reciba la trama
	ether.udpServerListenOnPort(&udpSerialPrint, 1337);

	// se inicializa el LCD con los numeros de columnas y filas
	lcd.begin(16, 2);

	lcd.print("192.168.1.33");

  // se habilitan las interrupciones
  sei();
}

/****************** LOOP *******************/
void loop()
{

		if( millis() - lastTime >= 1500)
		{
			digitalWrite(LED, !digitalRead(LED)); // cambio el estado del pin 14
			lastTime = millis();
		}

		if (ether.packetLoop(ether.packetReceive()))
    {

    }

		if(s >= BUFFER && millis() - lastTime2 >= 1000)
		{
			lastTime2 = millis();
			timer_off();
      rms();
			armaPayload();
			ether.sendUdp(payload, sizeof(payload), srcPort, dstIp, dstPort);
			ether.sendUdp(payload, sizeof(payload), srcPort, dstIp2, dstPort);
			timer_on();
		}

		// se prende o apaga el rele
		if(rele1State != rele1LastState)
		{
			digitalWrite(rele1, rele1State);
			rele1LastState = rele1State;
		}

		if(millis() - lastTime3 >= 1000)
		{
			lastTime3 = millis();
			refreshDisplay();
		}

}

/********* FUNCIONES: DEFINICIONES *********/
 ISR(TIMER5_COMPA_vect)
{
  /*
  Rutina de servición de atencion de la interrupción del timer 5.
  Si BUFFER no esta lleno, toma una muestra de tensión y corriente.
  */

	if( s < BUFFER )
	{
		samplesV[s] = analogRead(voltageSensor);
		samplesI[s] = analogRead(currentSensor);
		s++;
	}
}

void rms(void)
{
  /*
  Calcula del valor RMS de la tensión y la corriente
  */

  uint8_t i;
  unsigned long acumuladorV = 0;
  unsigned long acumuladorI = 0;

  for(i=0; i<BUFFER; i++)
  {
		samplesV[i] = samplesV[i] - offset_v;
		samplesI[i] = samplesI[i] - offset_i;
    acumuladorV = acumuladorV + (unsigned long) samplesV[i] * samplesV[i];
    acumuladorI = acumuladorI + (unsigned long) samplesI[i] * samplesI[i];
  }

  Vef = sqrt( (float) acumuladorV / BUFFER ) * 5/1024;
  Ief = sqrt( (float) acumuladorI / BUFFER ) * 5/1024;

  acumuladorV = 0;
  acumuladorI = 0;

	Serial.print(" Vef: "); // sacar
  Serial.print(Vef);
	/*
	Serial.print(" Ief: ");
  Serial.println(Ief);
	*/

  s = 0;

  // timer_on();
}

void armaPayload(void)
{
	/*
	Arma el paquete que va a ser enviado por UDP
	Suponiendo que:
	Vef = 3.45
	Ief = 0.234
	El string enviado debe ser: "V3.450I0.234"
	Entonces cada paquete tendra un payload de 13 caracteres.
	*/

	// convierto la tensión (float) a string
	dtostrf(Vef, 5, 3, Vef_c);

	// convierto la corriente (float) a string
	dtostrf(Ief, 5, 3, Ief_c);

	// se arma el payload
	sprintf(payload, "V%sI%s",  Vef_c,  Ief_c);
}

void udpSerialPrint(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *data, uint16_t len)
{
	/*
	Se ejecuta cada vez que se recibe una trama UDP

	La trama que recibe es: "Tn_e_p"
	Donde:
	n: es el numero de toma para el cual va a el mensaje, en nuestro caso siempre sera "1"
	e: estado del toma: 1 significa activar, 0 significa desactivar
	p: consumo de potencia del toma n
	*/

	int i = 0;

	if(data[0] == 'T' && data[1] == '1' && data[2] == '_' && data[4] == '_') // mensaje para toma 1
	{
		if(data[3] == '1') rele1State = 0; // si se recibe un 1, se activa el rele
		if(data[3] == '0') rele1State = 1; // si se recibe un 1, se desactiva el rele

		for(i=0; i<5; i++)
		{
			power[i] = data[5+i];
			voltage[i] = data[11+i];
		}
	}
}

//0123456789012345
//000.0W    000.0V
void refreshDisplay(void)
{
	/*
	Actualiza la pantalla
	*/

	lcd.setCursor(0, 1);
	lcd.print("               ");
	lcd.setCursor(0, 1);
	lcd.print(power);
	lcd.setCursor(5, 1);
	lcd.print("W    ");
	lcd.print(voltage);
	lcd.setCursor(15, 1);
	lcd.print("V");
}
