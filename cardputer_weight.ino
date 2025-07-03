// ===============================
// Проект: Умные весы на Cardputer
// Автор: @ilalex
// Используем библиотеки:
//   - M5Cardputer (для дисплея, клавиатуры и звука)
//   - HX711 (весовой модуль от Bogdan https://github.com/bogde/HX711)
//   - ArduinoJson (для хранения настроек в JSON)
// ===============================

#include <M5Cardputer.h>
#include <HX711.h>
#include <SD.h>
#include <ArduinoJson.h>

// Подключение пинов к весовому модулю HX711
#define DOUT 33
#define CLK  32

HX711 scale;

// Глобальные переменные
float displayedWeight = 0;   // сглаженное отображение веса
float actualWeight = 0;      // "сырое" значение
float smoothing = 0.1f;      // коэффициент сглаживания

// Единицы измерения
String unit = "g";
float scaleFactor = 1.0f;

// Заставка
bool showSplash = true;
unsigned long splashStart = 0;

// Меню выбора единиц
const String options[] = {"g — grams", "oz — ounces", "lb — pounds"};
const int optionsCount = 3;
int selectedOption = 0;
bool inMenu = false;

// Флаг наличия SD-карты
bool sdReady = false;

// Флаг отображения ошибки отсутствия весов
bool showScaleError = false;
bool scaleErrorBlink = false;
unsigned long lastBlinkTime = 0;
const int blinkInterval = 500;

// Флаг и состояние предупреждения об SD
bool showSDWarning = false;

// Функция: звуковой сигнал ошибки (двойной бип)
void playErrorBeepOnce() {
  for (int i = 0; i < 2; i++) {
    M5Cardputer.Speaker.tone(1000, 100);
    delay(150);
  }
}

// Функция: отрисовка заставки
void drawSplash() {
  M5Cardputer.Display.fillScreen(BLACK);

  const char* line1 = "SCALES";
  M5Cardputer.Display.setTextSize(4);
  int w1 = M5Cardputer.Display.textWidth(line1);
  int h1 = M5Cardputer.Display.fontHeight();

  const char* line2 = "by ILALEX";
  M5Cardputer.Display.setTextSize(2);
  int w2 = M5Cardputer.Display.textWidth(line2);
  int h2 = M5Cardputer.Display.fontHeight();

  int totalHeight = h1 + h2 + 6;
  int centerX = M5Cardputer.Display.width() / 2;
  int centerY = M5Cardputer.Display.height() / 2;

  int x1 = centerX - w1 / 2;
  int y1 = centerY - totalHeight / 2;
  int x2 = centerX - w2 / 2;
  int y2 = y1 + h1 + 6;

  M5Cardputer.Display.setTextSize(4);
  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setCursor(x1, y1);
  M5Cardputer.Display.print(line1);

  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setCursor(x2, y2);
  M5Cardputer.Display.print(line2);

  splashStart = millis();
}

// Функция: отрисовка ошибки весов
void drawScaleErrorOverlay() {
  if (scaleErrorBlink) {
    M5Cardputer.Display.fillRect(0, 40, M5Cardputer.Display.width(), 30, RED);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setTextColor(WHITE);
    M5Cardputer.Display.setCursor(10, 48);
    M5Cardputer.Display.print("Scales is not ready!");
  }
}

// Функция: отрисовка веса
void drawWeight() {
  M5Cardputer.Display.fillScreen(BLACK);

  M5Cardputer.Display.setTextSize(5);
  String weightStr = String(displayedWeight, 1);
  int textWidth = M5Cardputer.Display.textWidth(weightStr.c_str());
  int x = (M5Cardputer.Display.width() - textWidth) / 2;

  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setCursor(x, 60);
  M5Cardputer.Display.print(weightStr);

  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setCursor(200, 120);
  M5Cardputer.Display.printf("%s", unit.c_str());

  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(0x8410);
  M5Cardputer.Display.setCursor(10, M5Cardputer.Display.height() - 12);
  M5Cardputer.Display.print("R - tare");

  M5Cardputer.Display.setCursor(100, M5Cardputer.Display.height() - 12);
  M5Cardputer.Display.print("S - settings");

  drawScaleErrorOverlay(); // рисуем поверх, если ошибка
}

// Функция: сглаживание
void updateDisplayWeight() {
  displayedWeight = displayedWeight * (1 - smoothing) + actualWeight * smoothing;
}

// Функция: отрисовка меню выбора единиц
void drawMenu() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextSize(2);

  for (int i = 0; i < optionsCount; ++i) {
    M5Cardputer.Display.setTextColor(WHITE);
    if (i == selectedOption) {
      M5Cardputer.Display.setCursor(20, 40 + i * 30);
      M5Cardputer.Display.print(">");
    }
    M5Cardputer.Display.setCursor(40, 40 + i * 30);
    M5Cardputer.Display.println(options[i]);
  }
}

// Функция: отрисовка предупреждения об SD
void drawSDWarning() {
  M5Cardputer.Display.fillScreen(BLACK);
  M5Cardputer.Display.setTextSize(1);
  M5Cardputer.Display.setTextColor(RED);

  M5Cardputer.Display.setCursor(10, 30);
  M5Cardputer.Display.println("SD is not ready. Settings will");
  M5Cardputer.Display.setCursor(10, 45);
  M5Cardputer.Display.println("be reset after restart.");

  M5Cardputer.Display.setTextColor(WHITE);
  M5Cardputer.Display.setCursor(100, 90);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.print("OK");
}

// Функция: загрузка конфигурации
void loadConfig() {
  File file = SD.open("/ScalesByILALEX/config.json");
  if (!file) return;

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, file);
  if (err) return;

  String savedUnit = doc["unit"] | "g";
  if (savedUnit == "g")       { unit = "g";  scaleFactor = 1.0f; }
  else if (savedUnit == "oz") { unit = "oz"; scaleFactor = 0.035274f; }
  else if (savedUnit == "lb") { unit = "lb"; scaleFactor = 0.00220462f; }

  file.close();
}

// Функция: сохранение конфигурации
void saveConfig() {
  if (!sdReady) return;

  SD.mkdir("/ScalesByILALEX");
  File file = SD.open("/ScalesByILALEX/config.json", FILE_WRITE);
  if (!file) return;

  StaticJsonDocument<128> doc;
  doc["unit"] = unit;
  serializeJson(doc, file);
  file.close();
}

// Стартовая инициализация
void setup() {
  M5Cardputer.begin();
  M5Cardputer.Display.setRotation(1);

  M5Cardputer.Speaker.begin();
  M5Cardputer.Speaker.setVolume(128);

  // Инициализация SD
  if (SD.begin()) {
    sdReady = true;
    loadConfig();
  }

  scale.begin(DOUT, CLK);
  scale.set_scale(2280.f);
  scale.tare();

  drawSplash();
}

// Основной цикл
void loop() {
  M5Cardputer.update();

  // Мигаем ошибкой, если весы не готовы
  if (!scale.is_ready()) {
    if (!showScaleError) {
      playErrorBeepOnce();
      showScaleError = true;
    }
    if (millis() - lastBlinkTime > blinkInterval) {
      scaleErrorBlink = !scaleErrorBlink;
      drawWeight();
      lastBlinkTime = millis();
    }
    delay(50);
    return;
  } else {
    showScaleError = false;
  }

  if (showSplash) {
    if (millis() - splashStart > 2000) {
      showSplash = false;
      drawWeight();
    }
    return;
  }

  if (showSDWarning) {
    if (M5Cardputer.Keyboard.isPressed() && M5Cardputer.Keyboard.keysState().enter) {
      showSDWarning = false;
      inMenu = true;
      drawMenu();
    }
    return;
  }

  if (!inMenu) {
    if (scale.is_ready()) {
      long raw = scale.get_units();
      actualWeight = raw * scaleFactor;
      updateDisplayWeight();
      drawWeight();
    }

    if (M5Cardputer.Keyboard.isPressed()) {
      auto ks = M5Cardputer.Keyboard.keysState();
      if (ks.enter) {
        if (!sdReady) {
          showSDWarning = true;
          drawSDWarning();
        } else {
          inMenu = true;
          selectedOption = 0;
          drawMenu();
        }
      } else {
        for (char c : ks.word) {
          if (c == 'r') {
            scale.tare();
            actualWeight = displayedWeight = 0;
          }
        }
      }
    }
  } else {
    if (M5Cardputer.Keyboard.isPressed()) {
      auto ks = M5Cardputer.Keyboard.keysState();
      for (char c : ks.word) {
        if (c == '\xB1') { // стрелка вверх
          selectedOption = (selectedOption - 1 + optionsCount) % optionsCount;
          drawMenu();
        } else if (c == '\xB2') { // стрелка вниз
          selectedOption = (selectedOption + 1) % optionsCount;
          drawMenu();
        }
      }
      if (ks.enter) {
        switch (selectedOption) {
          case 0: unit = "g";  scaleFactor = 1.0f; break;
          case 1: unit = "oz"; scaleFactor = 0.035274f; break;
          case 2: unit = "lb"; scaleFactor = 0.00220462f; break;
        }
        saveConfig();
        inMenu = false;
        drawWeight();
      }
    }
  }

  delay(50);
}