#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "esp_camera.h"

//  network credentials
const char* ssid = "Staff";
const char* password = "Staff@dcb7";
const char* serverURL = "http://10.10.15.58:5000/upload";

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Push button pin
#define BUTTON_PIN 13

// Camera Pins for AI Thinker ESP32-CAM
#define PWDN_GPIO_NUM    32
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27
#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

void startCamera() {
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

  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QQVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.begin(115200);
 // Initialize Wire with custom I2C pins
  Wire.begin(14, 15); // SDA=14, SCL=15

  // OLED init
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Connecting WiFi...");
  display.display();

  // Connect WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected");
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("WiFi Connected");
  display.display();

  startCamera();

  // Setup button pin
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  // Check button press (active LOW)
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button Pressed - Capturing Image...");

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Capturing...");
    display.display();

    camera_fb_t * fb = esp_camera_fb_get();
    if(!fb) {
      Serial.println("Camera capture failed");
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Capture failed");
      display.display();
      delay(1000);
      return;
    }

    Serial.println("Sending Image to Server...");
    if(WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverURL);
      http.addHeader("Content-Type", "image/jpeg");
      int httpResponseCode = http.POST(fb->buf, fb->len);
      if(httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(response);
        
        // Show on OLED
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Number Plate:");
        display.setTextSize(2);
        display.println(response);
        display.display();
      } else {
        Serial.printf("HTTP POST failed: %d\n", httpResponseCode);
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("POST failed");
        display.display();
      }
      http.end();
    }

    esp_camera_fb_return(fb);

    delay(1000); // Debounce delay to avoid multiple captures per press
  }
}
