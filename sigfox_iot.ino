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
long t2= 0;
unsigned int continuityLow = 1;
unsigned int continuityHigh = 0;

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

void enviar(float flow, float volume, unsigned int continuityL, unsigned int continuityH)
{
  Serial.print ("Continuity number: "); 
  Serial.print (continuity); 
  Serial.print ("Caudal: "); 
  Serial.print (flow,3); 
  Serial.print ("L/min\tVolumen: "); 
  Serial.print (volume,3); 
  Serial.println (" L");
  
  mysigfox.initpayload();
  mysigfox.addfloat(flow);
  mysigfox.addfloat(volume);
  mysigfox.addint(continuityL); 
  mysigfox.addint(continuityH); 
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
    enviar(caudal_L_m,volumen, continuityLow);
    delay(1000);
  }

  if (caudal_L_m > 0 && t1==0){
    //Contar un minuto para enviar mensaje
    Serial.println ("Inicio minuto");
    t1=millis();
  } 

  if ((t0-t2)>=60000 && volumen <= 0) {
    continuityLow = 1;
  }

  if (t1>0 &&(t0-t1)>=60000){
    t2 = millis();
    //Ya pasó el minuto
    Serial.println ("Pasó minuto");
    if(continuityLow = 255) {
      continuityHigh++;
      continuityLow = 0;
    }
    enviar(caudal_L_m,volumen, continuityLow++, continuityHigh);
    t1=0;
    volumen = 0;
    delay(1000);
  }

}
