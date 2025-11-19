// сделать полноценный double-buffer на TFT_eSprite


#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// Машина состояний экранов
enum Screen {
    MAIN_MENU,
    PVIT_SCREEN,
    AZUR_SCREEN,
    BZU_SCREEN,
    MonitorModbusScreen
};

Screen currentScreen = MAIN_MENU;

// Структура для кнопки
struct Button {
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
    const char* label;
    Screen gotoScreen;  // какой экран открывать
};

Button backBtn = {170, 260, 60, 50, "<"};
Button btn1 = {10, 10, 220, 40, "PVIT/ABB"};
Button btn2 = {10, 60, 220, 40, "AZUR4MK"};
Button btn3 = {10, 110, 220, 40, "BZU1200"};
Button btn4 = {10, 160, 220, 40, "Monitor Modbus"};
Button btn5 = {10, 210, 220, 40, "Option Five"};
Button btn6 = {10, 260, 220, 40, "Option Six"};

// Состояние касания
bool touchDown = false;
uint16_t lastX = 0, lastY = 0;

// Функция для отрисовки кнопки
void drawButton(Button btn, uint16_t border, uint16_t fill) {
    tft.drawRect(btn.x, btn.y, btn.w, btn.h, border);
    tft.fillRect(btn.x + 1, btn.y + 1, btn.w - 2, btn.h - 2, fill);
    tft.setCursor(btn.x + 10, btn.y + (btn.h/2));
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK, fill);
    tft.print(btn.label);
}
// Функция для отображения главного меню
void showMainMenu() {
    tft.fillScreen(TFT_BLACK);
    
    // Рисуем кнопки
    drawButton(btn1, TFT_WHITE, TFT_YELLOW);
    drawButton(btn2, TFT_WHITE, TFT_YELLOW);
    drawButton(btn3, TFT_WHITE, TFT_YELLOW);
    drawButton(btn4, TFT_WHITE, TFT_YELLOW);
    drawButton(btn5, TFT_WHITE, TFT_YELLOW);
    drawButton(btn6, TFT_WHITE, TFT_YELLOW);
}

void showPVITScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(60, 80);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("PVIT/ABB Screen");
    drawButton(backBtn, TFT_WHITE, TFT_BLUE);
}

void showAZURScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(60, 80);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("AZUR4MK Screen");
    drawButton(backBtn, TFT_WHITE, TFT_BLUE);
}

void showBZUScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(60, 80);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("BZU1200 Screen");
    drawButton(backBtn, TFT_WHITE, TFT_BLUE);
}

void showMonitorModbusScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(40, 80);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("Monitor Modbus Screen");
    drawButton(backBtn, TFT_WHITE, TFT_BLUE);
}

// ---------- Проверка попадания в кнопку ----------
bool touchIn(Button btn, uint16_t x, uint16_t y) {
    return (x >= btn.x && x <= btn.x + btn.w &&
            y >= btn.y && y <= btn.y + btn.h);
}

// ================= ОБРАБОТКА НАЖАТИЯ ===================
void onTouchDown(uint16_t x, uint16_t y) {
    lastX = x;
    lastY = y;

    if (currentScreen == MAIN_MENU) {
        if (touchIn(btn1, x, y)) { drawButton(btn1, TFT_WHITE, TFT_WHITE); currentScreen = btn1.gotoScreen; return; }
        if (touchIn(btn2, x, y)) { drawButton(btn2, TFT_WHITE, TFT_WHITE); currentScreen = btn2.gotoScreen; return; }
        if (touchIn(btn3, x, y)) { drawButton(btn3, TFT_WHITE, TFT_WHITE); currentScreen = btn3.gotoScreen; return; }
        if (touchIn(btn4, x, y)) { drawButton(btn4, TFT_WHITE, TFT_WHITE); currentScreen = btn4.gotoScreen; return; }
        if (touchIn(btn5, x, y)) { drawButton(btn5, TFT_WHITE, TFT_WHITE); currentScreen = btn5.gotoScreen; return; }
        if (touchIn(btn6, x, y)) { drawButton(btn6, TFT_WHITE, TFT_WHITE); currentScreen = btn6.gotoScreen; return; }
    }
    else {
        if (touchIn(backBtn, x, y)) {
            drawButton(backBtn, TFT_WHITE, TFT_WHITE);
            currentScreen = MAIN_MENU;
            return;
        }
    }
}

// ================= ОБРАБОТКА ОТПУСКАНИЯ =================
void onTouchUp() {
    switch (currentScreen) {
        case MAIN_MENU:   showMainMenu(); break;
        case PVIT_SCREEN: showPVITScreen(); break;
        case AZUR_SCREEN: showAZURScreen(); break;
        case BZU_SCREEN:  showBZUScreen(); break;
        case MonitorModbusScreen: showMonitorModbusScreen(); break;
    }
}

// =================== Touch loop ===================
void processTouch() {
    uint16_t x, y;
    bool pressed = tft.getTouch(&x, &y);

    if (pressed && !touchDown) {
        touchDown = true;
        onTouchDown(x, y);
    }
    else if (!pressed && touchDown) {
        touchDown = false;
        onTouchUp();
    }
}

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(2);
  tft.fillScreen(TFT_BLACK);

  uint16_t calData[5] = { 350, 3600, 350, 3600, 0 };
  tft.setTouch(calData);

  tft.setCursor(60, 80);
  tft.setTextSize(2.5);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("BLOK");
  tft.setCursor(60, 140);
  tft.println("INDIKACII");
  delay(2000);

  Serial.println("Modbus RS485 Ready");

  showMainMenu();

}

void loop() {

    processTouch();
    
}
