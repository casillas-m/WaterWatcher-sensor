#include "NXTIoT_dev.h"

NXTIoT_dev  mysigfox;

const int boton=6;

volatile int NumPulsos; //variable para la cantidad de pulsos recibidos
int PinSensor = 14;    
float factor_conversion=6.8; //para convertir de frecuencia a caudal
float volumen=0;
long dt=0; //variación de tiempo por cada bucle
long t0=0; //millis() del bucle anterior
long t1=0;

void ContarPulsos ()  
{ 
  NumPulsos++;  //incrementamos la variable de pulsos
} 

int ObtenerFrecuecia() 
{
  int frecuencia;
  NumPulsos = 0;   //Ponemos a 0 el número de pulsos
  //interrupts();    //Habilitamos las interrupciones
  attachInterrupt(digitalPinToInterrupt(2),ContarPulsos,RISING);
  delay(1000);   //muestra de 1 segundo
  //noInterrupts(); //Deshabilitamos  las interrupciones
  detachInterrupt(digitalPinToInterrupt(2));
  frecuencia=NumPulsos; //Hz(pulsos por segundo)
  return frecuencia;
}

void enviar(float flow, float volume)
{
  Serial.print ("Caudal: "); 
  Serial.print (flow,3); 
  Serial.print ("L/min\tVolumen: "); 
  Serial.print (volume,3); 
  Serial.println (" L");
  
  mysigfox.initpayload();
  mysigfox.addfloat(flow);
  mysigfox.addfloat(volume);
  mysigfox.sendmessage();
}

void setup() 
{
  Serial.begin(9600);
  pinMode(boton, INPUT);
  pinMode(PinSensor, INPUT);
  attachInterrupt(digitalPinToInterrupt(2),ContarPulsos,RISING);
  Serial.println ("Iniciando"); 
  t0=millis();
}

void loop() 
{
  float frecuencia=ObtenerFrecuecia(); //obtenemos la frecuencia de los pulsos en Hz
  float caudal_L_m=frecuencia/factor_conversion; //calculamos el caudal en L/m
  dt=millis()-t0; //calculamos la variación de tiempo
  t0=millis();
  volumen=volumen+(caudal_L_m/60)*(dt/1000); // volumen(L)=caudal(L/s)*tiempo(s)
  
  if (digitalRead(boton)==LOW)
  {
    enviar(caudal_L_m,volumen);
    delay(1000);
  }

  if (caudal_L_m > 0 && t1==0){
    //Contar un minuto para enviar mensaje
    Serial.println ("Inicio minuto");
    t1=millis();
  } 
  if (t1>0 &&(t0-t1)>=60000){
    //Ya pasó el minuto
    Serial.println ("Pasó minuto");
    enviar(caudal_L_m,volumen);
    t1=0;
    volumen = 0;
    delay(1000);
  }
}
