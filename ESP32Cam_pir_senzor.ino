/*********

DOLEZITE!!!
    - Vyberte dosku "AI Thinker ESP32-CAM"
    - Pri nahravani kodu musí byť GPIO 0 pripojeny k GND
    - Po nahrati kodu GPIO 0 odpojite od GND porom stlacte tlacidlo RESET na doske ESP32-CAM, cím prepnete dosku do rezimu blikania
 

*********/
 //kniznice
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD karta ESP32
#include "SD_MMC.h"            // SD karta ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // cítat a zapisovat z flash pamäte
// definije pocet bajtov s ktorymi chce pracovat
#define EEPROM_SIZE 1
 
RTC_DATA_ATTR int bootCount = 0;

// Definicie pinov pre CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
 
int pictureNumber = 0;
  
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
 
  Serial.setDebugOutput(true);
  
 // nastavenie kamery
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  pinMode(4, INPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_dis(GPIO_NUM_4);
 //Pre fotoaparat s PSRAM pouzite nasledujuce nastavenia
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
 //Ak doska nema PSRAM nastavte ju takto
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
 
  // Spustenie fotoaparata
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Inicializacia fotoaparatu zlyhala s chybou 0x%x", err);
    return;
  }
 //Nastavenie SD karty
  Serial.println("Spustenie SD karty");
 
  delay(500);
  if(!SD_MMC.begin()){
    Serial.println("Pripojenie SD karty zlyhalo");
    //return;
  }
 
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("Nie je pripojena SD karta");
    return;
  }
   
  camera_fb_t * fb = NULL;
 
  // Urobenie fotky fotoaparatom
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Snimanie fotoaparatom zlyhalo");
    return;
  }
  
  EEPROM.begin(EEPROM_SIZE);// inicializuje EEPROM s preddefinovanou velkostou
  pictureNumber = EEPROM.read(0) + 1; //cislo fotky sa zvacsi o +1
 
  //fotografiu ulozime do adresara na SD karte a nazov suboru bude obrazok1.jpg, obrazok2.jpg atd
  String path = "/obrazok" + String(pictureNumber) +".jpg";
 // ulozenie fotky na SD kartu
  fs::FS &fs = SD_MMC;
  Serial.printf("Nazov suboru obrazka: %s\n", path.c_str());
 
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Subor sa nepodarilo otvorit v rezime zapisu");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Subor bol ulozeny na: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber); //Po ulození fotografie ulozíme císlo aktualnej snimky do flash pamate aby sme mali prehlad o pocte zhotovenych fotografii.
    EEPROM.commit();
  }
  file.close();
  esp_camera_fb_return(fb);
  
  delay(1000);
  
  // Vypnutie led diody ESP32-Cam pripojenej k GPIO 4
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  rtc_gpio_hold_en(GPIO_NUM_4);

//ESP32-Cam sa externe prebudi cez GPIO 13
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 0);
//ESP32-Cam sa ulozi do rezimu spanok 
  Serial.println("Ulozenie k spanku");
  delay(1000);
  esp_deep_sleep_start();
  Serial.println("Toto sa nikdy nevytlaci");
} 
 
void loop() {
 
}
