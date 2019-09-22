 #include <Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <PID_v1.h>

/*******Motores********/
#define BRAKE 0
#define CW    1
#define CCW   2
#define CS_THRESHOLD 15   // Definition of safety current (Check: "1.3 Monster Shield Example").

//MOTOR 1
#define MOTOR_A1_PIN 7
#define MOTOR_B1_PIN 8

//MOTOR 2
#define MOTOR_A2_PIN 4
#define MOTOR_B2_PIN 9

#define PWM_MOTOR_1 5
#define PWM_MOTOR_2 6

#define EN_PIN_1 A0
#define EN_PIN_2 A1

#define MOTOR_1 0
#define MOTOR_2 1

LiquidCrystal_I2C lcd(0x27,16,2);

short usSpeed = 25;
short usSpeed2 = 0;
unsigned short usMotor_Status = BRAKE;

/****************************************************************************************
********************************************PID******************************************
****************************************************************************************/
double ganancia=100;
double Setpoint, Input, Output;
PID myPID(&Input, &Output, &Setpoint,0.5,0.001,0, DIRECT);

/*******Motores********/

Servo servoMotor;

/**********MOTORREDUCTOR********/
int enReduct = 11;
int inReduct1 = 48;
int inReduct2 = 49;

char* efecto[3] = {"Flatspin","Backspin", "Topspin"};
int ef=0, t=0, n=1, Tiros=0;
int pos1, pos2, ps1, ps2, vel1, vel2;
int angulo1[6] = {70, 90, 110, 70, 90 , 110};
int angulo2[6] = {45, 45, 45, 30, 30 , 30};
int tiro[6];
int vel[6]={200,300,200,400,500,400};
int velz[6]={80,80,80,30,30,30};
int cuenta=0, ciclos;

int ESTADO;
const int INICIO = 0, EFECTO = 1, REPIT = 2, TIEMPO = 3, TIRO1 = 4, TIRO2 = 5, TIRO3 = 6, TIRO4 = 7, TIRO5 = 8, TIRO6 = 9, CONFIRM = 10, EJECUCION = 11;

int z;
int A = 1, B = 2, C = 3, D = 4, E = 5, F = 6;
int TA, TB, TC, TD, TE, TF;
const int a = 0, b = 1, c = 2, d = 3, e = 4, f = 5, BK = 6, DN = 7, UP = 8, OK = 9;
char* Entrada[] = {"a", "b", "c", "d", "e", "f", "BK", "DN", "UP", "OK"}; 
uint8_t button[11] = {22, 24, 26, 28, 30, 32, 43, 38, 41, 39};
uint8_t button_estate[11];

/***Encoder***/
unsigned long tiempoUP,tiempoDN,tiempoUP1,tiempoDN1;
float tiempo,tiempo1,frec,frec1;

/*****Step Motor******/
int fn=0,fn1=0,fna=0,fna1=0;
int dir,dir1;
int x=0,y=0;
const int stepPin = 25; 
const int dirPin = 27; 
const int stepPin1 = 29;
const int dirPin1 = 31;
/*****Step Motor******/

int sensor = 52;
boolean lecSensor;

void setup() {

    lcd.init();
    lcd.backlight();

    servoMotor.attach(10);
    servoMotor.write(82);

    /**Motorreductor**/
    pinMode(enReduct, OUTPUT);
    pinMode(inReduct1, OUTPUT);
    pinMode(inReduct2, OUTPUT);
    digitalWrite(inReduct2, LOW);
    digitalWrite(inReduct1, HIGH);
    pinMode(52,INPUT);

    /*******Motores********/
    pinMode(MOTOR_A1_PIN, OUTPUT);
    pinMode(MOTOR_B1_PIN, OUTPUT);
  
    pinMode(MOTOR_A2_PIN, OUTPUT);
    pinMode(MOTOR_B2_PIN, OUTPUT);
  
    pinMode(PWM_MOTOR_1, OUTPUT);
    pinMode(PWM_MOTOR_2, OUTPUT);

    pinMode(EN_PIN_1, OUTPUT);
    pinMode(EN_PIN_2, OUTPUT);

    digitalWrite(EN_PIN_1, HIGH);
    digitalWrite(EN_PIN_2, HIGH); 
    
    /*******Motores********/

    /*PID*/
    myPID.SetMode(AUTOMATIC);
    myPID.SetOutputLimits(0,50);
    
    for (int i=0; i<16; i++)pinMode(Entrada[i], INPUT);
    for (int i=0; i<16; i++)button_estate[0] = HIGH;

    pinMode(sensor,INPUT);
    
    /****Encoder****/
    pinMode(2,INPUT);
    pinMode(3,INPUT);
    /*****Step Motor******/
    pinMode(stepPin,OUTPUT); 
    pinMode(dirPin,OUTPUT);
    pinMode(stepPin1,OUTPUT); 
    pinMode(dirPin1,OUTPUT);
    /*****Step Motor******/

    Serial.begin(9600);

     lcd.print("INICIANDO...");
     delay(100);
     lcd.clear();
     lcd.print("Bienvenido");
     delay(100);
     lcd.clear();
     lcd.print("Presiona OK");
     lcd.setCursor(0, 1);
     lcd.print("Para configurar");
}

uint8_t flancoSubida(int btn) {
  uint8_t valor_nuevo = digitalRead(button[btn]);
  uint8_t result = button_estate[btn] != valor_nuevo && valor_nuevo == 1;
  button_estate[btn] = valor_nuevo;
  return result;
}

void loop() {
  TA = flancoSubida(a) * A;
  TB = flancoSubida(b) * B;
  TC = flancoSubida(c) * C;
  TD = flancoSubida(d) * D;
  TE = flancoSubida(e) * E;
  TF = flancoSubida(f) * F;

  z = (TA + TB + TC + TD + TE + TF);

  switch (ESTADO) {
    case INICIO:        
      INICIO_C();
      break;

    case EFECTO:     
      EFECTO_C();
      break;

    case REPIT:         
      REPIT_C();
      break;

    case TIEMPO:         
      TIEMPO_C();
      break;

    case TIRO1:         
      TIRO1_C();
      break;

    case TIRO2:
      TIRO2_C();
      break;

    case TIRO3:
      TIRO3_C();
      break;

    case TIRO4:
      TIRO4_C();
      break;

    case TIRO5:
      TIRO5_C();
      break;

    case TIRO6:
      TIRO6_C();
      break;

    case CONFIRM:
      CONFIRM_C();
      break;

    case EJECUCION:         
      EJECUCION_C();
      break;
  }

}

void INICIO_C() {
  if (flancoSubida(OK)) {
    ESTADO = 1;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Efecto: ");
    lcd.print(efecto[ef]);
  }
}
void EFECTO_C() {
  if (flancoSubida(OK)) {
    ESTADO = 2;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Repeticiones: ");
    lcd.print(n);
  }
  if (flancoSubida(BK)) {
    ESTADO = 0;
    lcd.clear();
    lcd.print("Presiona OK");
    lcd.setCursor(0, 1);
    lcd.print("Para configurar");
  }
  if (flancoSubida(UP) && (ef < 2)) {
    ef++;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Efecto: ");
    lcd.print(efecto[ef]);
  }
  if (flancoSubida(DN) && (ef > 0)) {
    ef--;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Efecto: ");
    lcd.print(efecto[ef]);
  }
}

void REPIT_C() {
  if (flancoSubida(OK)) {
    ESTADO = 3;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Intervalo: ");
    lcd.print(t);
  }
  if (flancoSubida(BK)) {
    ESTADO = 1;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Efecto: ");
    lcd.print(efecto[ef]);
  }
  if (flancoSubida(UP) && (n < 10)) {
    n+=1;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Repeticiones: ");
    lcd.print(n);
  }
  if (flancoSubida(DN) && (n > 1)) {
    n-=1;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Repeticiones: ");
    lcd.print(n);
  }
}

void TIEMPO_C() {
  if (flancoSubida(OK)) {
    ESTADO = 4;
    lcd.clear();
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 1: ");
    lcd.print(tiro[0]);
  }
  if (flancoSubida(BK)) {
    ESTADO = 2;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Repeticiones: ");
    lcd.print(n);
  }
  if (flancoSubida(UP) && (t < 10)) {
    t++;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Intervalo: ");
    lcd.print(t);
  }
  if (flancoSubida(DN) && (t > 0)) {
    t--;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Intervalo: ");
    lcd.print(t);
  }
}

void POSICION() {
  if (z != 0) {
    ESTADO = 5;
    tiro[0] = z;
    Tiros++;
    Serial.print(angulo1[tiro[cuenta]]);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 1: ");
    lcd.print(tiro[0]);
    delay(300);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 2: ");
    lcd.print(tiro[1]);
  }
  if (flancoSubida(BK)) {
    ESTADO = 3;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Intervalo: ");
    lcd.print(t);
  }
  if (flancoSubida(OK)&&Tiros>0) {
    ESTADO = 16;
    lcd.clear();
    lcd.print("PRESIONE START");
    lcd.setCursor(0, 1);
    lcd.print("PARA COMENZAR");
  }
}

void TIRO1_C() {
  if (z != 0) {
    ESTADO = 5;
    tiro[0] = z;
    Tiros++;
    Serial.print(angulo1[tiro[cuenta-1]]);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 1: ");
    lcd.print(tiro[0]);
    delay(300);
    lcd.clear();
    lcd.print("Selecciontia Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 2: ");
    lcd.print(tiro[1]);
  }
  if (flancoSubida(BK)) {
    ESTADO = 3;
    lcd.clear();
    lcd.print("Seleccionar");
    lcd.setCursor(0, 1);
    lcd.print("Intervalo: ");
    lcd.print(t);
  }
  if (flancoSubida(OK)&&Tiros>0) {
    ESTADO = 10;
    lcd.clear();
    lcd.print("PRESIONE START");
    lcd.setCursor(0, 1);
    lcd.print("PARA COMENZAR");
  }
}

void TIRO2_C() {
  if (z != 0) {
    ESTADO = 6;
    tiro[1] = z;
    Tiros++;
    Serial.print(angulo1[tiro[cuenta]]);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 2: ");
    lcd.print(tiro[1]);
    delay(300);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 3: ");
    lcd.print(tiro[2]);
  }
  if (flancoSubida(BK)) {
    ESTADO = 4;
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 1: ");
    lcd.print(tiro[0]);
  }
  if (flancoSubida(OK)&&Tiros>=1) {
    ESTADO = 10;
    lcd.clear();
    lcd.print("PRESIONE START");
    lcd.setCursor(0, 1);
    lcd.print("PARA COMENZAR");
  }
}

void TIRO3_C() {
  if (z != 0) {
    ESTADO = 7;
    tiro[2] = z;
    Tiros++;
    Serial.print(angulo1[tiro[cuenta+1]]);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 3: ");
    lcd.print(tiro[2]);
    delay(300);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 4: ");
    lcd.print(tiro[3]);
  }
  if (flancoSubida(BK)) {
    ESTADO = 5;
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 2: ");
    lcd.print(tiro[1]);
  }
  if (flancoSubida(OK)&&Tiros>=2) {
    ESTADO = 10;
    lcd.clear();
    lcd.print("PRESIONE START");
    lcd.setCursor(0, 1);
    lcd.print("PARA COMENZAR");
  }
}

void TIRO4_C() {
  if (z != 0) {
    ESTADO = 8;
    tiro[3] = z;
    Tiros++;
    Serial.print(angulo1[tiro[cuenta+2]]);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 4: ");
    lcd.print(tiro[3]);
    delay(300);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 5: ");
    lcd.print(tiro[4]);
  }
  if (flancoSubida(BK)) {
    ESTADO = 6;
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 3: ");
    lcd.print(tiro[2]);
  }
  if (flancoSubida(OK)&&Tiros>=2) {
    ESTADO = 10;
    lcd.clear();
    lcd.print("PRESIONE START");
    lcd.setCursor(0, 1);
    lcd.print("PARA COMENZAR");
  }
}

void TIRO5_C() {
  if (z != 0) {
    ESTADO = 9;
    tiro[4] = z;
    Tiros++;
    Serial.print(angulo1[tiro[cuenta+3]]);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 5: ");
    lcd.print(tiro[4]);
    delay(300);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 6: ");
    lcd.print(tiro[5]);
  }
  if (flancoSubida(BK)) {
    ESTADO = 7;
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 4: ");
    lcd.print(tiro[3]);
  }
  if (flancoSubida(OK)&&Tiros>=4) {
    ESTADO = 10;
    lcd.clear();
    lcd.print("PRESIONE START");
    lcd.setCursor(0, 1);
    lcd.print("PARA COMENZAR");
  }
}
void TIRO6_C() {
  if (z != 0) {
    ESTADO = 10;
    tiro[5] = z;
    Tiros++;
    Serial.print(angulo1[tiro[cuenta+4]]);
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 6: ");
    lcd.print(tiro[5]);
    delay(300);
    lcd.clear();
    lcd.print("PRESIONE START");
    lcd.setCursor(0, 1);
    lcd.print("PARA COMENZAR");
    lcd.print(tiro[6]);
  }
  if (flancoSubida(BK)) {
    ESTADO = 8;
    lcd.clear();
    lcd.print("Selecciona Pos");
    lcd.setCursor(0, 1);
    lcd.print("Tiro 5: ");
    lcd.print(tiro[4]);
  }
  if (flancoSubida(OK)&&Tiros>=5) {
    ESTADO = 10;
    lcd.clear();
    lcd.print("PRESIONE START");
    lcd.setCursor(0, 1);
    lcd.print("PARA COMENZAR");
  }
}

void CONFIRM_C() {
  if (flancoSubida(OK)) {
    ESTADO = 11;
    lcd.clear();
    lcd.print("EJECUTANDO");
    lcd.setCursor(0, 1);
    lcd.print("SECUENCIA...");
    delay(500);
  }
  if (flancoSubida(BK)) {
    ESTADO = (Tiros+3);
    lcd.clear();
    lcd.print("Selecciona Pos:");
    lcd.setCursor(0, 1);
    lcd.print("Tiro ");
    lcd.print(Tiros);
    lcd.print(":");
  }
}

void EJECUCION_C() {
  if (ESTADO == 11) {
    ESTADO = 0;
    lcd.clear();
    lcd.print("Efecto: ");
    lcd.print(efecto[ef]);
    delay(200);
    lcd.clear();
    lcd.print("No. Repeticiones: ");
    lcd.print(n);
    delay(200);
    lcd.clear();
    lcd.print("Intervalo: ");
    lcd.print(t);
    delay(200);;
    lcd.clear();
    lcd.print("Ejecutando");
    lcd.setCursor(0, 1);
    lcd.print("Secuencias...");

    analogWrite(enReduct, 150);
    usMotor_Status = CW;
    
   for(ciclos=0;ciclos<n;ciclos++){ 
      for(cuenta=0;cuenta<Tiros;cuenta++){
              lcd.clear();
              lcd.print(angulo1[tiro[cuenta-1]]);
              lcd.setCursor(0, 1);
              lcd.print(angulo2[tiro[cuenta-1]]);
              posiciones();
              lcd.clear();
              lcd.print(velz[tiro[cuenta-1]]);
              lcd.setCursor(0, 1);
              lcd.print(cuenta);
              lcd.print("  ");
              lcd.print(Tiros);
              motores();
              lecSensor = digitalRead(sensor);
              Serial.println(lecSensor);
              while(lecSensor==0)lecSensor = digitalRead(sensor);
              while(lecSensor==1)lecSensor = digitalRead(sensor);
              delay(500);
              analogWrite(enReduct,20);
              delay(200);
              servoMotor.write(140);
              delay(500);
              servoMotor.write(82);
              delay(200);
              analogWrite(enReduct, 150);
              delay(t*1000+1000);
          
          
          if(digitalRead(43)==1)posicionf();  
      }
    }
    posicionf();
    }
}

void motores(){
     usMotor_Status = CW;

    if(ef==0){
      motorGo(MOTOR_1, usMotor_Status, velz[tiro[cuenta-1]]);
      motorGo(MOTOR_2, usMotor_Status, velz[tiro[cuenta-1]]);
      }
    if(ef==1){
      motorGo(MOTOR_1, usMotor_Status, velz[tiro[cuenta-1]]+5);
      motorGo(MOTOR_2, usMotor_Status, velz[tiro[cuenta-1]]);
      }
    if(ef==2){
      motorGo(MOTOR_1, usMotor_Status, velz[tiro[cuenta-1]]);
      motorGo(MOTOR_2, usMotor_Status, velz[tiro[cuenta-1]]+5);
      }
      if(digitalRead(43)==1)posicionf();  
    
    delay(1000);
  }

void posiciones(){
  
  while(fn1==0||fn==0){
      x = analogRead(A3);
      y = analogRead(A2); 
      x = (map(x,370,610,60,120))+5;
      y = map(y,382,612,60,0);
/*
          Serial.print("Pos. X =  ");
          Serial.print(x);
          Serial.print("    ");
          Serial.print("SP X =  ");
          Serial.print(angulo1[tiro[cuenta-1]]);
          Serial.print("    ");
          Serial.print("Pos. Y =  ");
          Serial.print(y);
          Serial.print("    ");
          Serial.print("SP Y =  ");
          Serial.println(angulo2[tiro[cuenta-1]]);
      */
      
      if(x<angulo1[tiro[cuenta-1]])dir1=LOW;
      if(x>angulo1[tiro[cuenta-1]])dir1=HIGH;
      if(fn1==0){
        //Serial.print(angulo1[tiro[cuenta-1]-1]);
        //Serial.println(angulo2[tiro[cuenta-1]-1]);
          digitalWrite(dirPin1,dir1);
          digitalWrite(stepPin1,HIGH);
          delayMicroseconds(500); 
          digitalWrite(stepPin1,LOW); 
          delayMicroseconds(500);
      }
      
      if(y<angulo2[tiro[cuenta-1]])dir=HIGH;
      if(y>angulo2[tiro[cuenta-1]])dir=LOW;
      if(fn==0){
          digitalWrite(dirPin,dir);
          digitalWrite(stepPin,HIGH);
          delayMicroseconds(500); 
          digitalWrite(stepPin,LOW); 
          delayMicroseconds(500);
      }
      
     
      if(x==angulo1[tiro[cuenta-1]])fn1=1;
      if(y==angulo2[tiro[cuenta-1]])fn=1;
      
      if(digitalRead(43)==1)posicionf();
  }
  Serial.print("SEC=");
  Serial.println(tiro[cuenta-1]);
  fn=0;
  fn1=0;
}

void posicionf(){

  motorGo(MOTOR_1, usMotor_Status, 0);
  motorGo(MOTOR_2, usMotor_Status, 0);
  analogWrite(enReduct, 0);
    
  while(fna1==0||fna==0){
    
      x = analogRead(A3);
      y = analogRead(A2); 
      x = (map(x,370,610,60,120))+5;
      y = map(y,382,612,60,0);
      
      if(x<90&&fna1==0)dir1=LOW;
      if(x>90&&fna1==0)dir1=HIGH;
      if(fna1==0){
        digitalWrite(dirPin1,dir1);
        digitalWrite(stepPin1,HIGH);
        delayMicroseconds(500); 
        digitalWrite(stepPin1,LOW); 
        delayMicroseconds(500);
      }
      if(x==90)fna1=1;

      
      if(y<5&&fna==0)dir=HIGH;
      if(y>5&&fna==0)dir=LOW;
      if(fna==0){
        digitalWrite(dirPin,dir);
        digitalWrite(stepPin,HIGH);
        delayMicroseconds(500); 
        digitalWrite(stepPin,LOW); 
        delayMicroseconds(500);
      }
      if(y==5)fna=1;
      
  }
  fna=0;
  fna1=0;
      
      lcd.clear();
      lcd.print("Pausado,");
      lcd.setCursor(0, 1);
      lcd.print("Reinicie");
  while(true){}
}

void lectura1(){
      tiempoUP = pulseIn(44,HIGH);
      tiempoDN = pulseIn(44,LOW);
      tiempo = (tiempoUP/1000000.0)+(tiempoDN/1000000.0);
      frec = (1.0/tiempo/16)*60;
  }
void lectura2(){
      tiempoUP1 = pulseIn(46,HIGH);
      tiempoDN1 = pulseIn(46,LOW);
      tiempo1 = (tiempoUP1/1000000.0)+(tiempoDN1/1000000.0);
      frec1 = (1.0/tiempo1/16)*60;
  }
void IncreaseSpeed()
{
  usSpeed += 1;
  if(usSpeed > 100)usSpeed = 100;
  
  //Serial.print("Speed +: ");
  //Serial.println(usSpeed);

  motorGo(MOTOR_1, usMotor_Status, usSpeed);
  motorGo(MOTOR_2, usMotor_Status, usSpeed);  
}

void DecreaseSpeed()
{
  usSpeed -= 1;
  if(usSpeed < 0)usSpeed = 0;
  
  //Serial.print("Speed -: ");
  //Serial.println(usSpeed);

  motorGo(MOTOR_1, usMotor_Status, usSpeed);
  motorGo(MOTOR_2, usMotor_Status, usSpeed);  
}
void motorGo(uint8_t motor, uint8_t direct, uint8_t pwm)         //Function that controls the variables: motor(0 ou 1), direction (cw ou ccw) e pwm (entra 0 e 255);
{
  if(motor == MOTOR_1)
  {
    if(direct == CW)
    {
      digitalWrite(MOTOR_A1_PIN, LOW); 
      digitalWrite(MOTOR_B1_PIN, HIGH);
    }
    else if(direct == CCW)
    {
      digitalWrite(MOTOR_A1_PIN, HIGH);
      digitalWrite(MOTOR_B1_PIN, LOW);      
    }
    else
    {
      digitalWrite(MOTOR_A1_PIN, LOW);
      digitalWrite(MOTOR_B1_PIN, LOW);            
    }
    
    analogWrite(PWM_MOTOR_1, pwm); 
  }
  else if(motor == MOTOR_2)
  {
    if(direct == CW)
    {
      digitalWrite(MOTOR_A2_PIN, LOW);
      digitalWrite(MOTOR_B2_PIN, HIGH);
    }
    else if(direct == CCW)
    {
      digitalWrite(MOTOR_A2_PIN, HIGH);
      digitalWrite(MOTOR_B2_PIN, LOW);      
    }
    else
    {
      digitalWrite(MOTOR_A2_PIN, LOW);
      digitalWrite(MOTOR_B2_PIN, LOW);            
    }
    
    analogWrite(PWM_MOTOR_2, pwm);
  }
}
