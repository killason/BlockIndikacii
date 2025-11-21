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
    MonitorModbusScreen,
    MasterScreen,
    SlaveScreen,
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

// Определение кнопок
Button ArchBtn = {10, 245, 220, 30, "Archive Data"};
Button nextBtn = {10, 285, 50, 30, ">"};
Button offBtn = {120, 285, 50, 30, "OFF"};
Button onBtn = {70, 285, 50, 30, "ON"};
Button backBtn = {180, 285, 50, 30, "<", MAIN_MENU};
Button btn1 = {10, 10, 220, 40, "PVIT/ABB", PVIT_SCREEN};
Button btn2 = {10, 60, 220, 40, "AZUR4MK", AZUR_SCREEN};
Button btn3 = {10, 110, 220, 40, "BZU1200", BZU_SCREEN};
Button btn4 = {10, 160, 220, 40, "Monitor Modbus", MonitorModbusScreen};
Button btn5 = {10, 210, 220, 40, "Option Five", MAIN_MENU};
Button btn6 = {10, 260, 220, 40, "Option Six", MAIN_MENU};

// Состояние касания
bool touchDown = false;
uint16_t lastX = 0, lastY = 0;

// Функция для отрисовки кнопки
void drawButton(Button btn, uint16_t border, uint16_t fill, uint16_t textSize = 2) {
    tft.drawRect(btn.x, btn.y, btn.w, btn.h, border);
    tft.fillRect(btn.x + 1, btn.y + 1, btn.w - 2, btn.h - 2, fill);
    tft.setCursor(btn.x + 10, btn.y + (btn.h/2));
    tft.setTextSize(textSize);
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

    // кнопки
    tft.fillScreen(TFT_BLACK);
    drawButton(ArchBtn, TFT_WHITE, TFT_BLUE, 1.75);
    drawButton(nextBtn, TFT_WHITE, TFT_BLUE, 1.75);
    drawButton(offBtn, TFT_WHITE, TFT_BLUE, 1.75);
    drawButton(onBtn, TFT_WHITE, TFT_BLUE, 1.75);
    drawButton(backBtn, TFT_WHITE, TFT_BLUE, 1.75);

    tft.drawLine(10, 235, 230, 235, TFT_WHITE);
    
    // строка состояния
    tft.setCursor(20, 220);
    tft.setTextSize(1.75);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print("Inicializaja BI ..."); // может менять значения

    // рамка
    tft.drawRect(10, 10, 220, 195, TFT_WHITE);
    tft.drawLine(120, 10, 120, 49, TFT_WHITE);
    tft.drawLine(120, 88, 120, 205, TFT_WHITE);
    tft.drawLine(10, 49, 230, 49, TFT_WHITE);
    tft.drawLine(10, 88, 230, 88, TFT_WHITE);
    tft.drawLine(10, 127, 230, 127, TFT_WHITE);
    tft.drawLine(10, 166, 230, 166, TFT_WHITE);

    // подписи параметров
    tft.setTextSize(2);
    tft.setCursor(20, 29.5);
    tft.print("Un");
    // в этом же месте будет обновляемое значение напряжения  V
    tft.setCursor(105, 29.5);
    tft.print("V");
    tft.setCursor(20, 68.5);
    tft.print("Rizo");
    // здесь будет значение сопротивления изоляции
    tft.setCursor(180, 68.5);
    tft.print("kOhm");
    tft.setCursor(130, 29.5);
    tft.print("In");
    // здесь будет значение тока A
    tft.setCursor(215, 29.5);
    tft.print("A");
    tft.setCursor(20, 107.5);
    // здесь будет "K1_ON" или "K1_OFF" или "K2_ON" или "K2_OFF"
    tft.setCursor(130, 107.5);
    // здесь статусы блока бтз (сработал "MTZ" или "TZP" или "OK")
    tft.setCursor(20, 146.5);
    // здесь будут статусы блока бки ("Work" или "Fail")
    tft.setCursor(130, 146.5);
    tft.print("Umz");
    // здесь будет значение уставки A
    tft.setCursor(215, 146.5);
    tft.print("A");
    tft.setCursor(20, 185.5);
    tft.print("Uzp");
    // здесь будет значение уставки A
    tft.setCursor(105, 185.5);
    tft.print("A");
    tft.setCursor(130, 185.5);
    tft.print("Ut");
    // здесь будет значение уставки t, ms
    tft.setCursor(205, 185.5);
    tft.print("ms");

}

void showAZURScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(60, 80);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("AZUR4MK Screen");
    drawButton(backBtn, TFT_WHITE, TFT_BLUE, 1.75);
}

void showBZUScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(60, 80);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("BZU1200 Screen");
    drawButton(backBtn, TFT_WHITE, TFT_BLUE, 1.75);
}

void showMonitorModbusScreen() {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(40, 80);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("Monitor Modbus Screen");
    drawButton(backBtn, TFT_WHITE, TFT_BLUE, 1.75);
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

        tft.fillCircle(x, y, 3, TFT_RED);
        Serial.printf("Touch X: %d, Y: %d\n", x, y);

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
