#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// Pines de la ESP32-CAM
#define PWDN_GPIO_NUM    32
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

// Configura tu red WiFi
const char* ssid = "2.4G papu";
const char* password = "ma47aa46e63e1";

// Página HTML con JavaScript para actualizar la imagen
const char* html_page = R"rawliteral(
<html>
  <body>
    <h1>ESP32-CAM</h1>
    <img id="camera" src="/capture" width="640" height="480"/>
    <script>
      function updateImage() {
        var img = document.getElementById('camera');
        img.src = '/capture?' + new Date().getTime();
      }
      setInterval(updateImage, 33); // Intenta actualizar cada 33ms para alcanzar 30 FPS
    </script>
  </body>
</html>
)rawliteral";

WebServer server(80);

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  // Conectar a la red WiFi
  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Configuración de la cámara
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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;  // 20 MHz para intentar alcanzar 30 FPS
  config.pixel_format = PIXFORMAT_JPEG;
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_VGA;  // 640x480 para mejor calidad
    config.jpeg_quality = 10;  // Calidad JPEG media-alta
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;  // 320x240 para mayor fluidez si no hay PSRAM
    config.jpeg_quality = 12; 
    config.fb_count = 1;
  }

  // Inicializar la cámara
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Otros ajustes de la cámara (opcional)
  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 1);     // -2 to 2
  s->set_contrast(s, 1);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  
  // Iniciar el servidor de la cámara
  startCameraServer();
}

void loop() {
  server.handleClient();
  delay(1);
}

void handleRoot() {
  server.send(200, "text/html", html_page);
}

void handleCapture() {
  camera_fb_t * fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    server.send(500, "text/plain", "Camera capture failed");
    return;
  }
  
  WiFiClient client = server.client();
  client.write("HTTP/1.1 200 OK\r\n");
  client.write("Content-Type: image/jpeg\r\n");
  client.printf("Content-Length: %u\r\n", fb->len);
  client.write("Connection: close\r\n\r\n");
  client.write((const char *)fb->buf, fb->len);
  esp_camera_fb_return(fb);
}

void startCameraServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/capture", HTTP_GET, handleCapture);
  server.begin();
  Serial.println("HTTP server started");
}
