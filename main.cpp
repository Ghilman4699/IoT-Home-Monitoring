#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "parse_merge.h"

#define SS_PIN 10
#define RST_PIN 9
#define USER_DATA_TOTAL  2
#define USER_DATA_NAME(x)  user_data[x][0]
#define USER_DATA_ID(x)  user_data[x][1]
#define TIMER_OVF_MILLIS(x) ((millis() - elapsed_time) >= x)

uint16_t elapsed_time = 0;

const String user_data[USER_DATA_TOTAL][2] = {
  {"Ghilman", "18589242176"},
  {"Istianah", "2641751"}
};
 
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 

static byte nuidPICC[4];
static String card = "";
static String kunci = "";
static String nilai = "";

void setup() { 
  Serial.begin(9600);
  pinMode(A0, INPUT);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 
  elapsed_time = millis();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}
 
void kondisi_kunci(){
  int lock;
  String kunci = "Pintu dikunci";
  lock = digitalRead(8);
  if (lock==HIGH){
    kunci = "Pintu tidak dikunci";
  }else{
    kunci = "Pintu dikunci";
  }
  Serial.println(kunci);
  kunci = "";
}

void gas(){
  // String nilai = "";
  int sensorValue = analogRead(A0);
  int outputValue = map(sensorValue, 0, 1023, 0, 4);

  switch (outputValue)
  {
  case 0:
  nilai = "Kondisi Udara Aman";
    break;
  case 1:
  nilai = "Terjadi Kebocoran Gas";
    break;
  case 2:
  nilai = "Kebocoran Gas Cukup Parah";
    break;
  case 3:
  nilai = "Kondisi Udara Bahaya";
    break;
  default:
    break;
  }
}

void loop() {
  if(Serial.available() > 0){
    data = Serial.readString();
    parse_string();
  
    Serial.println("total data [" + String(get_data_total()) + "] :");
    for(uint8_t x=0; x<get_data_total(); x++)
      Serial.println("[" + String(x) + "] " + DATA_STR(x));
  
    clear_str();
  }
 
  if ( !rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("[rfid] Card is not supported!"));
    
    return;
  }

  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
    card += String(nuidPICC[i]);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
   
  Serial.println("[rfid] scanned id " + card);
  for(uint8_t x=0; x<USER_DATA_TOTAL; x++){
    if(card.equals(USER_DATA_ID(x))){
      Serial.println(
        "ID\t: " + USER_DATA_ID(x) + "\n" +
        "NAME\t: " + USER_DATA_NAME(x)
      );
      digitalWrite(8, HIGH);
      delay(5000);
      digitalWrite(8, LOW);
      break;
    }
  }
  
  kondisi_kunci();
  gas();
  card = "";
  if(TIMER_OVF_MILLIS(1000)){
    Serial.println(String(card)); 
    Serial.println(String(kunci)); 
    Serial.println(String(nilai));
  }
}