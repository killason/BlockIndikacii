#include <TFT_eSPI.h>
#include <SPI.h>
#include <ModbusMaster.h>

// ================= Modbus ===================
#define RS485_TX 17
#define RS485_RX 16
#define DE_RE_PIN 26
#define SLAVE_ID 3
#define BAUDRATE 300

ModbusMaster node;

// ================= Функция для переключения направления RS485 =================
void preTransmission() {
  digitalWrite(DE_RE_PIN, HIGH); // Передача
}

void postTransmission() {
  digitalWrite(DE_RE_PIN, LOW); // Прием
}

// ================ переменные Modbus PVIT ===============
int pv_un = 0.0f;
int pv_in = 0.0f;
int pv_riso = 0.0f;
String pv_kstate = "K1_OFF";
String pv_btz = "OK";
String pv_bki = "Work";
int pv_umz = 0.0f;
int pv_uzp = 0.0f;
int pv_ut = 0.0f;
int pv_errorCode = 0;

// ================= Функция чтения регистров =================
bool readPVITRegisters() {
  uint8_t result = node.readHoldingRegisters(0x0001, 10); // читаем 10 регистров начиная с 0x0001
  if (result == node.ku8MBSuccess) {
    pv_un   = node.getResponseBuffer(0x00); // 0001 Un
    pv_in   = node.getResponseBuffer(0x01); // 0002 In
    pv_riso = node.getResponseBuffer(0x02); // 0003 Riso
    uint16_t k_flags = node.getResponseBuffer(0x03); // 0004 K1/K2 status
    pv_kstate = (k_flags == 0x02) ? "K1_ON" : (k_flags == 0x01) ? "K2_ON" : "K1_OFF";
    uint16_t btz_flags = node.getResponseBuffer(0x04); // 0005 BTZ status
    pv_btz = String(btz_flags, BIN);
    uint16_t bki_flags = node.getResponseBuffer(0x05); // 0006 BKI status
    pv_bki = (bki_flags == 0x00) ? "Work" : "Fail";
    pv_umz = node.getResponseBuffer(0x06);  // 0007 Umz
    pv_uzp = node.getResponseBuffer(0x07);  // 0008 Uzp
    pv_ut  = node.getResponseBuffer(0x08);  // 0009 Ut
    // 000A в btz_flags уже учтён
    pv_errorCode = 0; // ошибок нет
    return true;
  } else {
    pv_errorCode = result; // сюда заносим код ошибки Modbus
    return false;
  }
}

TFT_eSPI tft = TFT_eSPI();

// ================ СПИСОК ЭКРАНОВ =======================
enum ScreenID {
    MAIN_MENU,
    PVIT_SCREEN,
    AZUR_SCREEN,
    BZU_SCREEN,
    MonitorModbusScreen,
    MasterScreen,
    SlaveScreen,
};

// ============== Список действия для кнопок =================
enum ButtonAction {
    ACTION_NONE,
    ACTION_GOTO_SCREEN,
    ACTION_NEXT_PAGE,
    ACTION_PREV_PAGE,
    ACTION_CUSTOM  // можно назначить callback позже
};

// ================== Структура для кнопки ====================
struct Button {
    int x, y, w, h;
    const char* label;
    ButtonAction action;
    ScreenID target;
};


// =================== Кнопки MAIN MENU =======================
Button btn1 = {10, 10, 220, 40, "PVIT/ABB", ACTION_GOTO_SCREEN, PVIT_SCREEN};
Button btn2 = {10, 60, 220, 40, "AZUR4MK", ACTION_GOTO_SCREEN, AZUR_SCREEN};
Button btn3 = {10, 110, 220, 40, "BZU1200", ACTION_GOTO_SCREEN, BZU_SCREEN};
Button btn4 = {10, 160, 220, 40, "Monitor Modbus", ACTION_GOTO_SCREEN, MonitorModbusScreen};
Button btn5 = {10, 210, 220, 40, "MasterScreen", ACTION_NONE, MAIN_MENU};
Button btn6 = {10, 260, 220, 40, "SlaveScreen", ACTION_NONE, MAIN_MENU};

// ================= КСТОМНЫЕ КНОПКИ ==============================
Button ArchBtn = {10, 245, 220, 30, "Archive Data", ACTION_NONE, MAIN_MENU};
Button nextBtn = {10, 285, 50, 30, ">", ACTION_NEXT_PAGE, PVIT_SCREEN};
Button offBtn  = {120, 285, 50, 30, "OFF", ACTION_CUSTOM, MAIN_MENU};
Button onBtn   = {70, 285, 50, 30, "ON", ACTION_CUSTOM, MAIN_MENU};
Button backBtn = {180, 285, 50, 30, "<", ACTION_PREV_PAGE, MAIN_MENU};

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ =======================
ScreenID currentScreen = MAIN_MENU;
int pageIndex_PVIT = 0;   // пока 0, т.к. одна страница
int pageIndex_Archive = 0;
bool buttonWasPressed = false;
unsigned long lastModbusTime = 0;
const unsigned long modbusInterval = 200; // ms

// ======================== статус-бар =========================
String statusText = "Inicializaja BI ...";

// Функция для отрисовки кнопки
void drawButton(Button btn, uint16_t border, uint16_t fill, uint16_t textSize = 2) {
    tft.drawRect(btn.x, btn.y, btn.w, btn.h, border);
    tft.fillRect(btn.x + 1, btn.y + 1, btn.w - 2, btn.h - 2, fill);
    tft.setCursor(btn.x + 10, btn.y + (btn.h/2));
    tft.setTextSize(textSize);
    tft.setTextColor(TFT_BLACK, fill);
    tft.print(btn.label);
}

// ---------- Проверка попадания в кнопку ----------
bool touchIn(Button btn, uint16_t x, uint16_t y) {
    return (x >= btn.x && x <= btn.x + btn.w &&
            y >= btn.y && y <= btn.y + btn.h);
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

// ======= ФУНКЦИЯ ОБНОВЛЕНИЯ ЗНАЧЕНИЙ PVIT НА ЭКРАНЕ ======
void updatePVITValues() {

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);

    // Un
    tft.fillRect(55, 20, 50, 25, TFT_BLACK);
    tft.setCursor(55, 29.5);
    tft.print(pv_un);

    // In
    tft.fillRect(165, 20, 50, 25, TFT_BLACK);
    tft.setCursor(165, 29.5);
    tft.print(pv_in);

    // Riso
    tft.fillRect(80, 59, 85, 25, TFT_BLACK);
    tft.setCursor(80, 68.5);
    tft.print(pv_riso);

    // K-state
    tft.fillRect(20, 97, 85, 25, TFT_BLACK);
    tft.setCursor(20, 107.5);
    tft.print(pv_kstate);

    // BTZ
    tft.fillRect(130, 97, 90, 25, TFT_BLACK);
    tft.setCursor(130, 107.5);
    tft.print(pv_btz);

    // BKI
    tft.fillRect(175, 136, 30, 25, TFT_BLACK);
    tft.setCursor(175, 146);
    tft.print(pv_umz);

    // Umz
    tft.fillRect(20, 136, 40, 25, TFT_BLACK);
    tft.setCursor(20, 146);
    tft.print(pv_bki);

    // Uzp
    tft.fillRect(65, 175, 30, 25, TFT_BLACK);
    tft.setCursor(65, 185.5);
    tft.print(pv_uzp);

    // Ut
    tft.fillRect(165, 175, 40, 25, TFT_BLACK);
    tft.setCursor(165, 185.5);
    tft.print(pv_ut);

    // Error code (в статус бар)
    statusText = "Err:" + String(pv_errorCode);
}

// ======================= PVIT SCREEN =====================
void showPVITScreen() {

    tft.fillScreen(TFT_BLACK);

    drawButton(ArchBtn, TFT_WHITE, TFT_BLUE, 1.75);
    drawButton(nextBtn, TFT_WHITE, TFT_BLUE, 1.75);
    drawButton(onBtn,   TFT_WHITE, TFT_BLUE, 1.75);
    drawButton(offBtn,  TFT_WHITE, TFT_BLUE, 1.75);
    drawButton(backBtn, TFT_WHITE, TFT_BLUE, 1.75);

    // Верхняя рамка
    tft.drawRect(10, 10, 220, 195, TFT_WHITE);
    tft.drawLine(120, 10, 120, 49, TFT_WHITE);
    tft.drawLine(120, 88, 120, 205, TFT_WHITE);
    tft.drawLine(10, 49, 230, 49, TFT_WHITE);
    tft.drawLine(10, 88, 230, 88, TFT_WHITE);
    tft.drawLine(10, 127, 230, 127, TFT_WHITE);
    tft.drawLine(10, 166, 230, 166, TFT_WHITE);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);

    tft.setCursor(20, 29);  tft.print("Un");  tft.setCursor(105, 29); tft.print("V");
    tft.setCursor(20, 68.5);  tft.print("Rizo"); tft.setCursor(180, 68.5); tft.print("kOhm");
    tft.setCursor(130, 29); tft.print("In");  tft.setCursor(215, 29); tft.print("A");

    // tft.setCursor(130, 107.5); tft.print("BTZ");
    // tft.setCursor(130, 146); tft.print("BKI");

    tft.setCursor(130, 146.5); tft.print("Umz"); tft.setCursor(215, 146.5); tft.print("A");
    tft.setCursor(20, 185.5); tft.print("Uzp"); tft.setCursor(105, 185.5); tft.print("A");
    tft.setCursor(130, 185.5); tft.print("Ut"); tft.setCursor(205, 185.5); tft.print("ms");


    // Статус-бар
    tft.drawLine(10, 235, 230, 235, TFT_WHITE);
    tft.setCursor(20, 220);
    tft.setTextSize(1);
    tft.print(statusText);

    // ==== нарисуем значения ====
    updatePVITValues();
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



// ================== ОБРАБОТКА ТАЧА ======================
bool touchDown = false;
uint16_t lastX, lastY;

void performButtonAction(Button btn) {

    switch (btn.action) {

        case ACTION_GOTO_SCREEN:
            currentScreen = btn.target;
            break;

        case ACTION_NEXT_PAGE:
            // if (currentScreen == PVIT_SCREEN)
            //     pageIndex_PVIT++;
            break;

        case ACTION_PREV_PAGE:
            if (currentScreen == PVIT_SCREEN)
                // pageIndex_PVIT = max(0, pageIndex_PVIT - 1);
                currentScreen = MAIN_MENU;
            else
                currentScreen = MAIN_MENU;
            break;

        case ACTION_CUSTOM:
            // Позже сюда добавим Modbus-команды (ON/OFF)
            break;

        case ACTION_NONE:
            break;
    }
}

void onTouchDown(uint16_t x, uint16_t y) {
    buttonWasPressed = false;   // каждый новый тап начинаем с false
    Button* b = nullptr;

    switch (currentScreen) {

        case MAIN_MENU:
            if      (touchIn(btn1, x, y)) b = &btn1;
            else if (touchIn(btn2, x, y)) b = &btn2;
            else if (touchIn(btn3, x, y)) b = &btn3;
            else if (touchIn(btn4, x, y)) b = &btn4;
            else if (touchIn(btn5, x, y)) b = &btn5;
            else if (touchIn(btn6, x, y)) b = &btn6;
            break;

        case PVIT_SCREEN:
            if      (touchIn(ArchBtn, x, y)) b = &ArchBtn;
            // else if (touchIn(nextBtn, x, y)) b = &nextBtn;
            else if (touchIn(onBtn,   x, y)) b = &onBtn;
            else if (touchIn(offBtn,  x, y)) b = &offBtn;
            else if (touchIn(backBtn, x, y)) b = &backBtn;
            break;

        default:
            break;
    }

    if (b != nullptr) {
        buttonWasPressed = true;
        drawButton(*b, TFT_WHITE, TFT_WHITE);
        performButtonAction(*b);
    }
}

void drawCurrentScreen() {
    switch (currentScreen) {
        case MAIN_MENU: showMainMenu(); break;
        case PVIT_SCREEN: showPVITScreen(); break;
        case AZUR_SCREEN: showAZURScreen(); break;
        case BZU_SCREEN:  showBZUScreen(); break;
        case MonitorModbusScreen: showMonitorModbusScreen(); break;
        case MasterScreen: showMainMenu(); break;
        case SlaveScreen: showMainMenu(); break;
    }
}

// ================= ОБРАБОТКА ОТПУСКАНИЯ =================
void onTouchUp() {
    // Если НАЖАТА БЫЛА КНОПКА → да, надо перерисовать
    if (buttonWasPressed) {
        drawCurrentScreen();
    }

    // Если нет — ничего не делаем
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

  // RS485 Modbus
    pinMode(DE_RE_PIN, OUTPUT);
    digitalWrite(DE_RE_PIN, LOW);
    Serial2.begin(300, SERIAL_8N1, 16, 17);
    node.begin(SLAVE_ID, Serial2);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);

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

    if (currentScreen == PVIT_SCREEN) {
        if (millis() - lastModbusTime > modbusInterval) {
            lastModbusTime = millis();
            readPVITRegisters();
            updatePVITValues();
        }
    }

}

