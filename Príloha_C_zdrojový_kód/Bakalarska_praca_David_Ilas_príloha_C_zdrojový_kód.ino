#include "DHT.h"
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
#define VENTILATOR 13   // Ventilatr na pine 13
#define BUZZER 10    // Alarm na pine 10
#define DHTPIN 2    // Modul pre meranie teploty DHT11 na pine 2
#define DHTTYPE DHT11   // DHT 11

/* --- BUTTONS_VARIABLES --- */
int page_counter = 1; 
int up = 6;               
int down = 5;
boolean current_up = LOW;          
boolean last_up = LOW;            
boolean last_down = LOW;
boolean current_down = LOW;

/* --- DELAY PRE VYPISOVANIE DAT NA DISPLEJ --- */
int period = 1000;
unsigned long time_now = 0;

/* --- MQ3_VARIABLES --- */
int MQ3_sensorValue;
float MQ3_sensor_volt;
float MQ3_RS_gas; 
float MQ3_R0;
float MQ3_ratio;
int MQ3_RL = 200;
float MQ3_a = -0.699; //Slope 
float MQ3_b = -0.301; //Y-Intercept
int MQ3_function_running = 0;

double ppm_MQ3;

/* --- MQ6_VARIABLES --- */
int MQ6_sensorValue;
float MQ6_sensor_volt;
float MQ6_RS_gas; 
float MQ6_R0;
float MQ6_ratio;
int MQ6_RL = 20;
float MQ6_a = -0.430; //Slope 
float MQ6_b = 1.289; //Y-Intercept
int MQ6_function_running = 0;
double ppm_MQ6;

/* --- MQ7_VARIABLES --- */
int MQ7_sensorValue;
float MQ7_sensor_volt;
float MQ7_RS_gas; 
float MQ7_R0;
float MQ7_ratio;
int MQ7_RL = 10;
float MQ7_a = -1.321; //Slope 
float MQ7_b = 3.712; //Y-Intercept
int MQ7_function_running = 0;
int MQ7_AirConstant = 27;
double ppm_MQ7;

/* --- MQ135_VARIABLES --- */
int MQ135_sensorValue;
float MQ135_sensor_volt;
float MQ135_RS_gas; 
float MQ135_R0;
float MQ135_ratio;
int MQ135_RL = 20;
float MQ135_a = -0.366; //Slope 
float MQ135_b = 0.762; //Y-Intercept
int MQ135_function_running = 0;
int MQ135_AirConstant = 3.6;
double ppm_MQ135;


DHT dht(DHTPIN, DHTTYPE);

/* --- INITIALIZING FUNCTIONS --- */
//MQ3
int MQ3_calib();
double MQ3_ppm();
//MQ6
int MQ6_calib();
double MQ6_ppm();
//MQ7
int MQ7_calib();
double MQ7_ppm();
//MQ135
int MQ135_calib();
double MQ135_ppm();

void setup() {
  Serial.begin(9600);
  Serial.println(F("DHT11 test!"));
  dht.begin();
  pinMode(VENTILATOR, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(2,0);
  lcd.print("Zohrievam!!!");
  lcd.setCursor(0,1);
  lcd.print("Prosim pockajte!");
  delay(5000);
  lcd.clear();
  }

boolean press_btn (boolean last, int pin){
    boolean current = digitalRead(pin);
      if (last != current){
        delay(5);
        current = digitalRead(pin);
      }
    return current;
  }

void loop() {
/* DHT11_MAIN */
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println(F("Chyba pri nacitani DHT11 senzora!"));
    return;
  }
  
/* --- PREPINANIE DISPLEJA --- */
  current_up = press_btn(last_up, up);         //Stlačenie tlačidla pre pohyb doprava
  current_down = press_btn(last_down, down);   //Stlačenie tlačidla pre pohyb dolava

  //tlačidolo doprava
  if (last_up== LOW && current_up == HIGH){  //ak je tlačidlo stlačene
    lcd.clear();                     //vyčisti sa displej po prejdení na novu stranku 
    if(page_counter < 5){              //počitadlo pre stranku 
      page_counter = page_counter +1;   //po stalčni sa pripočita +1
    }else{
      page_counter= 5 ;  
    }
  }  
  last_up = current_up;

  //tlačidolo dolava
   if (last_down== LOW && current_down == HIGH){ //ak je tlačidlo stlačene
     lcd.clear();                      //vyčisti sa displej po prejdení na predchadzajucu stranku 
     if(page_counter > 1){              //počitadlo pre stranku 
       page_counter= page_counter - 1;    //po stalčni sa odpočitá - 1
     }else{
       page_counter = 1;  
     }
   }
   last_down = current_down;

   /* --- PREPINANIE DISPLEJA --- */
   switch (page_counter){
    /* --- DHT11 --- */  // Prva stranka displeja, zobria sa informacie o telpte a vlhkosti
    case 1:{
      lcd.setCursor(0, 0);
      lcd.print("Vlhkost:");
      lcd.print(humidity);
      lcd.print("%");
      lcd.setCursor(0, 1);
      lcd.print("Teplota:");
      lcd.print(temperature);
      lcd.print("C");
      
    }
    break;
    /* --- MQ3_MAIN --- */  // druha stranka displeja, zobria sa informacie o koncentracii alkoholu 
    case 2: {  
      ppm_MQ3 = MQ3_ppm();
        if(millis() > time_now + period){
          time_now = millis();
          lcd.setCursor(0, 0);
          lcd.print("Alkohol:");
          lcd.print(ppm_MQ3);  
          lcd.print("mg/L"); 
        }
    }
    break;
    
    /* --- MQ6_MAIN --- */  // Tretia stranka displeja, zobria sa informacie o koncentracii LPG
    case 3: { 
      ppm_MQ6 = MQ6_ppm();
        if(millis() > time_now + period){
          time_now = millis();
          lcd.setCursor(0, 0);
          lcd.print("LPG:");
          lcd.print(ppm_MQ6);  
          lcd.print("ppm");  
        }
    }
    break;
    /* --- MQ7_MAIN --- */  // Stvrta stranka displeja, zobria sa informacie o koncentracii CO 
    case 4: { 
      ppm_MQ7 = MQ7_ppm();
        if(millis() > time_now + period){
          time_now = millis();
          lcd.setCursor(0, 0);
          lcd.print("CO:");
          lcd.print(ppm_MQ7);  
          lcd.print("ppm");   
        }
    }
    break;
    /* --- MQ135_MAIN --- */  // Piata stranka displeja, zobria sa informacie o koncentracii CO2 
    case 5: { 
      ppm_MQ135 = MQ135_ppm();
        if(millis() > time_now + period){
          time_now = millis();
          lcd.setCursor(0, 0);
          lcd.print("CO2:");
          lcd.print(ppm_MQ135);  
          lcd.print("ppm");    
        }
    }
    break;
 }
  
 
/* --- PODMIENKY PRE ZAPNUTIE VENTILATORA/REKUPERACIE --- */
  if(temperature>=27 || humidity<30 || humidity>80 || ppm_MQ3>=0.01 || ppm_MQ6>=10 || ppm_MQ7>=100 || ppm_MQ135>=1000){
    digitalWrite(VENTILATOR, HIGH);
  }else{
    digitalWrite(VENTILATOR, LOW);
  }
  if(ppm_MQ3>=0.01 || ppm_MQ6>=10 || ppm_MQ7>=100 || ppm_MQ135>=1000){
    digitalWrite(BUZZER, HIGH);
  }else{
    digitalWrite(BUZZER, LOW);
  }
}
/* --- MQ3_FUNCTIONS --- */
int MQ3_calib(){
    float RS_air; 
    float R0; 
    MQ3_sensorValue = analogRead(A0);   
    MQ3_sensor_volt = 0.3;
    RS_air = ((5.0*MQ3_RL)/MQ3_sensor_volt)-MQ3_RL;
    R0 = RS_air / 60;
    Serial.print("Kalibracia MQ3 senzora dokoncena!!!\n");
    return R0;
    
  }
double MQ3_ppm(){
    if(MQ3_function_running == 0){
      MQ3_R0 = MQ3_calib();
      MQ3_function_running = MQ3_function_running + 1;
    }
    
    MQ3_sensorValue = analogRead(A0);
    MQ3_sensor_volt = 0.3;
    MQ3_RS_gas = ((5.0*MQ3_RL)/MQ3_sensor_volt)-MQ3_RL;
    MQ3_ratio = MQ3_RS_gas/MQ3_R0;
    double MQ3_ppm_log = (log10(MQ3_ratio)-MQ3_b)/MQ3_a;
    double MQ3_ppm = pow(10, MQ3_ppm_log);
    return MQ3_ppm;
}

/* --- MQ6_FUNCTIONS --- */
int MQ6_calib(){
    float RS_air; 
    float R0; 
    MQ6_sensorValue = analogRead(A1);   
    MQ6_sensor_volt = (float)MQ6_sensorValue/1024*5.0;
    RS_air = ((5.0*MQ6_RL)/MQ6_sensor_volt)-MQ6_RL;
    R0 = RS_air/ 10;
    Serial.print("Kalibracia MQ6 senzora dokoncena!!!\n");
    return R0;
  }
double MQ6_ppm(){
    if(MQ6_function_running == 0){
      MQ6_R0 = MQ6_calib();
      MQ6_function_running = MQ6_function_running + 1;
    }
    MQ6_sensorValue = analogRead(A1);
    MQ6_sensor_volt = (float)MQ6_sensorValue/1024*5.0;
    MQ6_RS_gas = ((5.0*MQ6_RL)/MQ6_sensor_volt)-MQ6_RL;
    MQ6_ratio = MQ6_RS_gas/MQ6_R0;
    double MQ6_ppm_log = (log10(MQ6_ratio)-MQ6_b)/MQ6_a;
    double MQ6_ppm = pow(10, MQ6_ppm_log);
    //delay(1000);
    return MQ6_ppm;
}

/* --- MQ7_FUNCTIONS --- */
int MQ7_calib(){
    float RS_air; 
    float R0; 
    
    MQ7_sensorValue = analogRead(A2);   
    MQ7_sensor_volt = (float)MQ7_sensorValue/1024*5.0;
    RS_air = ((5.0*MQ7_RL)/MQ7_sensor_volt)-MQ7_RL;
    R0 = RS_air / MQ7_AirConstant;
    Serial.print("Kalibracia MQ7 senzora dokoncena!!!\n");
    //Serial.print(MQ7_sensor_volt);
    return R0;
}
double MQ7_ppm(){
    if(MQ7_function_running == 0){
        MQ7_R0 = MQ7_calib();
        MQ7_function_running = MQ7_function_running + 1;
    }
    MQ7_sensorValue = analogRead(A2);
    MQ7_sensor_volt = (float)MQ7_sensorValue/1024*5.0;
    MQ7_RS_gas = ((5.0*MQ7_RL)/MQ7_sensor_volt)-MQ7_RL;
    MQ7_ratio = MQ7_RS_gas/MQ7_R0;
    double MQ7_ppm_log = (log10(MQ7_ratio)-MQ7_b)/MQ7_a;
    double MQ7_ppm = pow(10, MQ7_ppm_log);
    return MQ7_ppm;
}

/* --- MQ135_FUNCTIONS --- */
int MQ135_calib(){
    float RS_air; 
    float R0; 
    MQ135_sensorValue = analogRead(A3);   
    MQ135_sensor_volt = (float)MQ135_sensorValue/1024*5.0;
    RS_air = ((5.0*MQ135_RL)/MQ135_sensor_volt)-MQ135_RL;
    R0 = RS_air / MQ135_AirConstant;
    Serial.print("Kalibracia MQ135 senzora dokoncena!!!\n");
    return R0;
  }
double MQ135_ppm(){
    if(MQ135_function_running == 0){
      MQ135_R0 = MQ135_calib();
      MQ135_function_running = MQ135_function_running + 1;
    }
    MQ135_sensorValue = analogRead(A3);
    MQ135_sensor_volt = (float)MQ135_sensorValue/1024*5.0;
    MQ135_RS_gas = ((5.0*MQ135_RL)/MQ135_sensor_volt)-MQ135_RL;
    MQ135_ratio = MQ135_RS_gas/MQ135_R0;  
    double MQ135_ppm_log = (log10(MQ135_ratio)-MQ135_b)/MQ135_a;
    double MQ135_ppm = pow(10, MQ135_ppm_log);
   return MQ135_ppm;
}
