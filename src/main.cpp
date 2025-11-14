// сделать полноценный double-buffer на TFT_eSprite


#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

enum Screen {
    MAIN_MENU,
    PVIT_SCREEN,
    AZUR_SCREEN,
    BZU_SCREEN
};

Screen currentScreen = MAIN_MENU;

// Структура для кнопки
struct Button {
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
    const char* label;
};

Button backBtn = {170, 260, 60, 50, "<"};
Button btn1 = {10, 10, 220, 40, "PVIT/ABB"};
Button btn2 = {10, 60, 220, 40, "AZUR4MK"};
Button btn3 = {10, 110, 220, 40, "BZU1200"};
Button btn4 = {10, 160, 220, 40, "Option Four"};
Button btn5 = {10, 210, 220, 40, "Option Five"};
Button btn6 = {10, 260, 220, 40, "Option Six"};

// Функция для отрисовки кнопки
void drawButton(Button btn, uint16_t color1, uint16_t color2) {
    // Рамка кнопки
    tft.drawRect(btn.x, btn.y, btn.w, btn.h, color1);
    // Заполнение кнопки
    tft.fillRect(btn.x + 1, btn.y + 1, btn.w - 2, btn.h - 2, color2);
    
    // Текст кнопки
    tft.setCursor(btn.x + 10, btn.y + (btn.h/2));
    tft.setTextSize(2.5);
    tft.setTextColor(TFT_BLACK, color2);
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
// Функция ожидания отпускания кнопки
void waitForRelease() {
    uint16_t rx, ry;
    while (tft.getTouch(&rx, &ry)) {
        delay(10);
    }
}
// Обработка нажатий в подменю
void handleSubScreenTouch(uint16_t x, uint16_t y) {
    // проверяем только кнопку назад
        if (x >= backBtn.x && x <= (backBtn.x + backBtn.w) &&
            y >= backBtn.y && y <= (backBtn.y + backBtn.h)) {
            drawButton(backBtn, TFT_WHITE, TFT_WHITE);
            waitForRelease();
            delay(300);
            showMainMenu();
            currentScreen = MAIN_MENU;
        }
}


// Функция для для меню ПВИТ\АВВ
void showPVITScreen(uint16_t x, uint16_t y) {
    handleSubScreenTouch(x, y);
    if (currentScreen != PVIT_SCREEN) return;
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(60, 80);
    tft.setTextSize(2.5);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("PVIT/ABB Screen");
    drawButton(backBtn, TFT_WHITE, TFT_BLUE);
}
//  Функция для для меню AZUR4MK
void showAZURScreen(uint16_t x, uint16_t y) {
    handleSubScreenTouch(x, y);
    if (currentScreen != AZUR_SCREEN) return;
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(60, 80);
    tft.setTextSize(2.5);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("AZUR4MK Screen");
    drawButton(backBtn, TFT_WHITE, TFT_BLUE);
}
// Функция для для меню BZU1200
void showBZUScreen(uint16_t x, uint16_t y) {
    handleSubScreenTouch(x, y);
    if (currentScreen != BZU_SCREEN) return;
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(60, 80);
    tft.setTextSize(2.5);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.println("BZU1200 Screen");
    drawButton(backBtn, TFT_WHITE, TFT_BLUE);
}
// Обработка нажатий в главном меню
void handleMainMenuTouch(uint16_t x, uint16_t y) {
    // проверяем зоны и показываем эффект нажатия + ждём отпускания
    if (x >= 10 && x <= 230) {
        // PVIT
        if (y >= 10 && y <= 50) {
            Serial.println("PVIT/ABB button pressed");
            drawButton(btn1, TFT_WHITE, TFT_WHITE);
            // ждём отпускания
            waitForRelease();
            delay(300);
            currentScreen = PVIT_SCREEN;
            return;
        }
        // AZUR
        if (y >= 60 && y <= 100) {
            Serial.println("AZUR4MK button pressed");
            drawButton(btn2, TFT_WHITE, TFT_WHITE);
            waitForRelease();
            delay(300);
            currentScreen = AZUR_SCREEN;
            return;
        }
        // BZU
        if (y >= 110 && y <= 150) {
            Serial.println("BZU1200 button pressed");
            drawButton(btn3, TFT_WHITE, TFT_WHITE);
            waitForRelease();
            delay(300);
            currentScreen = BZU_SCREEN;
            return;
        }
        // Option 4
        if (y >= 160 && y <= 200) {
            Serial.println("Option Four button pressed");
            Button b = {10,160,220,40,"Option 4"};
            drawButton(btn4, TFT_WHITE, TFT_WHITE);
            waitForRelease();
            delay(300);
            // пока просто вернёмся в меню (здесь можно вызвать showOption4)
            showMainMenu();
            currentScreen = MAIN_MENU;
            return;
        }
        // Option 5
        if (y >= 210 && y <= 250) {
            Serial.println("Option Five button pressed");
            drawButton(btn5, TFT_WHITE, TFT_WHITE);
            waitForRelease();
            delay(300);
             // пока просто вернёмся в меню (здесь можно вызвать showOption5)
            showMainMenu();
            currentScreen = MAIN_MENU;
            return;
        }
        // Option 6
        if (y >= 260 && y <= 300) {
            Serial.println("Option Six button pressed");
            drawButton(btn6, TFT_WHITE, TFT_WHITE);
            waitForRelease();
            delay(300);
             // пока просто вернёмся в меню (здесь можно вызвать showOption6)
            showMainMenu();
            currentScreen = MAIN_MENU;
            return;
        }
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

  showMainMenu();

}

void loop() {
    uint16_t x, y;
    if (tft.getTouch(&x, &y)) {    
        tft.fillCircle(x, y, 3, TFT_RED);
        Serial.printf("Touch X: %d, Y: %d\n", x, y);

        if (currentScreen == MAIN_MENU) {
            handleMainMenuTouch(x, y);
        } 
        if (currentScreen == PVIT_SCREEN) {
            showPVITScreen(x, y);
        } 
        if (currentScreen == AZUR_SCREEN) {
            showAZURScreen(x, y);
        }
        if (currentScreen == BZU_SCREEN) {
            showBZUScreen(x, y);
        }
    }
}
