#include "GUI.h"
#include <iostream>
#include <cstring>
#include <cctype>

namespace {
const int OUTER_MARGIN = 30;
const int HEADER_GAP = 20;
const int FRAME_PADDING = 24;

const int HEADER_LINES = 8;
const int MENU_LINES = 8;
const int CHOICE_LINES = 2;
const int LEFT_INDENT_SPACES = 34;

const char* MENU_PROMPT = "Nhập lựa chọn (1-6): ";
const char* PRESS_ANY_KEY = "Nhấn phím bất kỳ để tiếp tục...";

int approxCharWidth() {
    return WINDOW_WIDTH / 120;
}

int approxLineHeight() {
    return WINDOW_HEIGHT / 40;
}

int headerHeight() {
    return approxLineHeight() * HEADER_LINES;
}

int menuHeight() {
    return approxLineHeight() * MENU_LINES;
}

int choiceHeight() {
    return approxLineHeight() * CHOICE_LINES;
}

int utf8CharCount(const std::string& text) {
    int count = 0;
    for (unsigned char c : text) {
        if ((c & 0xC0) != 0x80) {
            count++;
        }
    }
    return count;
}

int approxTextWidth(const std::string& text) {
    return utf8CharCount(text) * approxCharWidth();
}

void drawCenteredText(int centerX, int y, const std::string& text) {
    int x = centerX - (approxTextWidth(text) / 2);
    outtextxy(x, y, (char*)text.c_str());
}

void drawLeftAlignedText(int leftX, int y, const std::string& text) {
    outtextxy(leftX, y, (char*)text.c_str());
}

int leftIndentX(int frameLeft) {
    return frameLeft + 4 + LEFT_INDENT_SPACES * approxCharWidth();
}

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(' ');
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(' ');
    return s.substr(start, end - start + 1);
}

bool tryParseInt(const std::string& text, int& value) {
    std::string s = trim(text);
    if (s.empty()) return false;
    try {
        size_t idx = 0;
        int v = std::stoi(s, &idx);
        if (idx != s.size()) return false;
        value = v;
        return true;
    } catch (...) {
        return false;
    }
}

enum class InputFilter { Any, Integer };

void drawInputField(int x, int y, int maxLen, const std::string& value) {
    if (maxLen < 1) maxLen = 1;
    std::string blank(static_cast<size_t>(maxLen), ' ');
    outtextxy(x, y, (char*)blank.c_str());
    std::string clipped = value;
    if ((int)clipped.size() > maxLen) {
        clipped = clipped.substr(0, static_cast<size_t>(maxLen));
    }
    outtextxy(x, y, (char*)clipped.c_str());
}

std::string readLineAt(int x, int y, int maxLen, InputFilter filter) {
    std::string value;
    setcolor(COLOR_TEXT);
    drawInputField(x, y, maxLen, value);
    while (true) {
        int c = getch();
        if (c == '\r' || c == '\n') {
            break;
        }
        if (c == 8 || c == 127) {
            if (!value.empty()) {
                value.pop_back();
            }
        } else if (c >= 32 && c <= 126) {
            char ch = static_cast<char>(c);
            bool accept = true;
            if (filter == InputFilter::Integer) {
                if (std::isdigit(static_cast<unsigned char>(ch))) {
                    accept = true;
                } else if ((ch == '-' || ch == '+') && value.empty()) {
                    accept = true;
                } else {
                    accept = false;
                }
            }
            if (accept && (int)value.size() < maxLen) {
                value.push_back(ch);
            }
        }
        drawInputField(x, y, maxLen, value);
    }
    return value;
}

void drawHorizontalRule(int leftX, int rightX, int y) {
    int charW = approxCharWidth();
    int count = (rightX - leftX) / (charW > 0 ? charW : 1);
    if (count < 2) count = 2;
    std::string rule;
    rule.reserve(static_cast<size_t>(count));
    rule.push_back('+');
    if (count > 2) {
        rule.append(static_cast<size_t>(count - 2), '-');
    }
    rule.push_back('+');
    outtextxy(leftX, y, (char*)rule.c_str());
}

void drawFrameBox(const std::string& title, int left, int top, int right, int bottom,
                  int& centerX, int& innerLeft, int& innerTop) {
    setcolor(COLOR_BUTTON);
    rectangle(left, top, right, bottom);
    rectangle(left + 4, top + 4, right - 4, bottom - 4);

    centerX = (left + right) / 2;
    int lineH = approxLineHeight();
    int titleY = top + 8;

    if (!title.empty()) {
        setcolor(COLOR_TEXT);
        drawCenteredText(centerX, titleY, title);
        innerTop = titleY + lineH;
    } else {
        innerTop = top + FRAME_PADDING;
    }

    innerLeft = left + FRAME_PADDING;
}

void drawHeaderFrame(int screenW) {
    int left = OUTER_MARGIN;
    int right = screenW - OUTER_MARGIN;
    int top = OUTER_MARGIN;
    int bottom = top + headerHeight();

    int centerX = 0;
    int innerLeft = 0;
    int innerTop = 0;
    drawFrameBox("", left, top, right, bottom, centerX, innerLeft, innerTop);

    int lineH = approxLineHeight();
    int y = top + 12;

    setcolor(COLOR_TEXT);
    y += lineH;
    drawCenteredText(centerX, y, "TRƯỜNG ĐẠI HỌC BÁCH KHOA - ĐẠI HỌC ĐÀ NẴNG");
    y += lineH + 4;
    // drawCenteredText(centerX, y, "|");
    // y += lineH + 2;

    setcolor(YELLOW);
    drawCenteredText(centerX, y, "PBL1 : ĐỒ ÁN LẬP TRÌNH TÍNH TOÁN");
    y += lineH + 8;
    const int paddingX = 40;

    setcolor(COLOR_TEXT);
    const std::string leftText = "Tên SV: Nguyễn Hữu Rin";
    const std::string rightText = "GVHD: Nguyễn Văn Hiệu";
    outtextxy(390, y, (char*)leftText.c_str());
    outtextxy(right - 7*paddingX - approxTextWidth(rightText), y, (char*)rightText.c_str());
    y += lineH + 4;
    drawCenteredText(centerX-25, y, "Huỳnh Nguyễn Hồng Nhi");
}

void drawContentFrame(const std::string& title, int screenW, int screenH,
                      int& left, int& top, int& right, int& bottom, int& centerX, int& innerLeft, int& innerTop) {
    left = OUTER_MARGIN;
    right = screenW - OUTER_MARGIN;
    top = OUTER_MARGIN + headerHeight() + HEADER_GAP;
    bottom = screenH - OUTER_MARGIN;
    drawFrameBox(title, left, top, right, bottom, centerX, innerLeft, innerTop);
}
} // namespace

// ==================== BUTTON CLASS IMPLEMENTATION ====================
Button::Button(int x, int y, int width, int height, const std::string& label)
    : x(x), y(y), width(width), height(height), label(label), isHovered(false) {
}

Button::~Button() {}

void Button::draw() {
    int color = isHovered ? COLOR_BUTTON_HOVER : COLOR_BUTTON;
    setcolor(color);
    rectangle(x, y, x + width, y + height);
    
    // Draw label
    setcolor(COLOR_TEXT);
    outtextxy(x + 5, y + 5, (char*)label.c_str());
}

bool Button::isClicked(int mouseX, int mouseY) {
    return (mouseX >= x && mouseX <= x + width &&
            mouseY >= y && mouseY <= y + height);
}

bool Button::handleMouseMove(int mouseX, int mouseY) {
    bool wasHovered = isHovered;
    isHovered = isClicked(mouseX, mouseY);
    return isHovered != wasHovered;
}

void Button::setCallback(std::function<void()> cb) {
    callback = cb;
}

void Button::getRect(int& rx, int& ry, int& rw, int& rh) const {
    rx = x;
    ry = y;
    rw = width;
    rh = height;
}

// ==================== TEXT BOX CLASS IMPLEMENTATION ====================
TextBox::TextBox(int x, int y, int width, int height)
    : x(x), y(y), width(width), height(height), isActive(false) {
}

TextBox::~TextBox() {}

void TextBox::draw() {
    int color = isActive ? YELLOW : WHITE;
    setcolor(color);
    rectangle(x, y, x + width, y + height);
    
    // Draw content
    setcolor(COLOR_TEXT);
    if (!content.empty()) {
        outtextxy(x + 5, y + 5, (char*)content.c_str());
    }
}

void TextBox::setText(const std::string& text) {
    content = text;
}

std::string TextBox::getText() const {
    return content;
}

void TextBox::clear() {
    content.clear();
}

void TextBox::setActive(bool active) {
    isActive = active;
}

// ==================== GUI CLASS IMPLEMENTATION ====================
GUI::GUI() : screenWidth(WINDOW_WIDTH), screenHeight(WINDOW_HEIGHT) {
}

GUI::~GUI() {}

void GUI::drawMenu() {
    clearScreen();
    setbkcolor(COLOR_BACKGROUND);

    drawHeaderFrame(WINDOW_WIDTH);

    int left = OUTER_MARGIN;
    int right = WINDOW_WIDTH - OUTER_MARGIN;
    int menuTop = OUTER_MARGIN + headerHeight() + HEADER_GAP;
    int menuBottom = menuTop + menuHeight();

    int centerX = 0;
    int innerLeft = 0;
    int innerTop = 0;
    drawFrameBox("CHƯƠNG TRÌNH TÌM ĐƯỜNG ĐI NGẮN NHẤT", left, menuTop, right, menuBottom,
                 centerX, innerLeft, innerTop);

    int lineH = approxLineHeight();
    int y = innerTop;

    setcolor(YELLOW);
    std :: cout << '\n';
    drawLeftAlignedText(450, y, "[1]. Khởi tạo/Nạp đồ thị (từ file)"); y += lineH;
    drawLeftAlignedText(450, y, "[2]. Chạy thuật toán Dijkstra"); y += lineH;
    drawLeftAlignedText(450, y, "[3]. Chạy thuật toán Bellman-Ford"); y += lineH;
    drawLeftAlignedText(450, y, "[4]. So sánh hiệu năng"); y += lineH;
    drawLeftAlignedText(450, y, "[5]. Trực quan hóa (Python)"); y += lineH;
    drawLeftAlignedText(450, y, "[6]. Thoát"); y += lineH;
    std :: cout << '\n';

    setcolor(COLOR_BUTTON);
    drawHorizontalRule(left + 4, right - 4, y);

    int choiceTop = menuBottom + HEADER_GAP;
    int choiceBottom = choiceTop + choiceHeight();
    int choiceCenterX = 0;
    int choiceInnerLeft = 0;
    int choiceInnerTop = 0;
    drawFrameBox("", left, choiceTop, right, choiceBottom,
                 choiceCenterX, choiceInnerLeft, choiceInnerTop);

    int choiceTextY = choiceTop + (choiceHeight() - lineH) / 2;
    setcolor(COLOR_TEXT);
    drawLeftAlignedText(choiceInnerLeft, choiceTextY, MENU_PROMPT);
}

void GUI::drawInputMenu() {
    clearScreen();
    setbkcolor(COLOR_BACKGROUND);
    
    drawHeaderFrame(WINDOW_WIDTH);

    int left, top, right, bottom, centerX, innerLeft, innerTop;
    drawContentFrame("TẠO ĐỒ THỊ/ NHẬP ĐỒ THỊ TỪ FILE", WINDOW_WIDTH, WINDOW_HEIGHT,
                     left, top, right, bottom, centerX, innerLeft, innerTop);

    int lineH = approxLineHeight();
    int y = innerTop + 10;

    setcolor(YELLOW);
    drawCenteredText(centerX, y, "Nhập thông tin đồ thị trong console."); y += lineH + 6;
    drawCenteredText(centerX, y, "Số đỉnh:"); y += lineH + 6;
    drawCenteredText(centerX, y, "Các cạnh (u v w lần lược là đỉnh nguồn, đỉnh đích, trọng số):");
}

void GUI::drawRunningScreen(const std::vector<std::string>& logs) {
    clearScreen();
    setbkcolor(COLOR_BACKGROUND);
    
    drawHeaderFrame(WINDOW_WIDTH);

    int left, top, right, bottom, centerX, innerLeft, innerTop;
    drawContentFrame("ĐANG CHẠY THUẬT TOÁN", WINDOW_WIDTH, WINDOW_HEIGHT,
                     left, top, right, bottom, centerX, innerLeft, innerTop);

    setcolor(LIGHTGREEN);
    int lineH = approxLineHeight();
    int yPos = innerTop + 6;
    for (const auto& log : logs) {
        if (yPos < bottom - lineH * 2) {
            outtextxy(innerLeft, yPos, (char*)log.c_str());
            yPos += lineH + 4;
        }
    }
    
    setcolor(LIGHTCYAN);
    drawCenteredText(centerX, yPos + 8, "Nhấn phím bất kỳ để quay lại menu...");
}

void GUI::drawComparisonScreen(const std::vector<std::string>& logs) {
    clearScreen();
    setbkcolor(COLOR_BACKGROUND);
    
    drawHeaderFrame(WINDOW_WIDTH);

    int left, top, right, bottom, centerX, innerLeft, innerTop;
    drawContentFrame("SO SÁNH THUẬT TOÁN", WINDOW_WIDTH, WINDOW_HEIGHT,
                     left, top, right, bottom, centerX, innerLeft, innerTop);

    setcolor(LIGHTGREEN);
    int lineH = approxLineHeight();
    int yPos = innerTop + 6;
    for (const auto& log : logs) {
        if (yPos < bottom - lineH * 2) {
            outtextxy(innerLeft, yPos, (char*)log.c_str());
            yPos += lineH + 4;
        }
    }
    
    setcolor(LIGHTCYAN);
    drawCenteredText(centerX, yPos + 8, "Nhấn phím bất kỳ để quay lại menu...");
}

void GUI::handleEvent() {
    // Event handling can be implemented here if needed
}

void GUI::clearScreen() {
    cleardevice();
}

int GUI::promptMenuChoice() {
    while (true) {
        drawMenu();
        int lineH = approxLineHeight();

        int left = OUTER_MARGIN;
        int menuTop = OUTER_MARGIN + headerHeight() + HEADER_GAP;
        int menuBottom = menuTop + menuHeight();
        int choiceTop = menuBottom + HEADER_GAP;

        int promptY = choiceTop + (choiceHeight() - lineH) / 2;
        int inputX = left + FRAME_PADDING + approxTextWidth(MENU_PROMPT) + approxCharWidth();

        std::string input = readLineAt(inputX, promptY, 3, InputFilter::Integer);
        int value = 0;
        if (tryParseInt(input, value) && value >= 1 && value <= 6) {
            return value;
        }

        setcolor(LIGHTRED);
        drawLeftAlignedText(left + FRAME_PADDING, promptY + lineH, "Dữ liệu không hợp lệ. Vui lòng thử lại.");
        waitForKey();
    }
}

int GUI::promptChoice(const std::string& title, const std::vector<std::string>& options,
                      const std::string& prompt, int minValue, int maxValue) {
    std::string error;
    while (true) {
        clearScreen();
        setbkcolor(COLOR_BACKGROUND);
        drawHeaderFrame(WINDOW_WIDTH);

        int left, top, right, bottom, centerX, innerLeft, innerTop;
        drawContentFrame(title, WINDOW_WIDTH, WINDOW_HEIGHT,
                         left, top, right, bottom, centerX, innerLeft, innerTop);

        int lineH = approxLineHeight();
        int y = innerTop + 4;

        setcolor(YELLOW);
        for (const auto& opt : options) {
            drawLeftAlignedText(innerLeft, y, opt);
            y += lineH;
        }

        y += lineH / 2;
        setcolor(COLOR_TEXT);
        drawLeftAlignedText(innerLeft, y, prompt);

        int inputX = innerLeft + approxTextWidth(prompt) + approxCharWidth();
        int inputY = y;

        if (!error.empty()) {
        setcolor(LIGHTRED);
        drawLeftAlignedText(bottomIndentX(left), y + lineH, error);
        }

        std::string input = readLineAt(inputX, inputY, 6, InputFilter::Integer);
        int value = 0;
        if (tryParseInt(input, value) && value >= minValue && value <= maxValue) {
            return value;
        }

        error = "Dữ liệu không hợp lệ. Vui lòng thử lại.";
    }
}

int GUI::promptInt(const std::string& title, const std::string& prompt, int minValue, int maxValue) {
    std::string error;
    while (true) {
        clearScreen();
        setbkcolor(COLOR_BACKGROUND);
        drawHeaderFrame(WINDOW_WIDTH);

        int left, top, right, bottom, centerX, innerLeft, innerTop;
        drawContentFrame(title, WINDOW_WIDTH, WINDOW_HEIGHT,
                         left, top, right, bottom, centerX, innerLeft, innerTop);

        int lineH = approxLineHeight();
        int y = innerTop + 6;

        setcolor(COLOR_TEXT);
        drawLeftAlignedText(innerLeft, y, prompt);

        int inputX = innerLeft + approxTextWidth(prompt) + approxCharWidth();
        int inputY = y;

        if (!error.empty()) {
        setcolor(LIGHTRED);
        drawLeftAlignedText(bottomIndentX(left), y + lineH, error);
        }

        std::string input = readLineAt(inputX, inputY, 8, InputFilter::Integer);
        int value = 0;
        if (tryParseInt(input, value) && value >= minValue && value <= maxValue) {
            return value;
        }

        error = "Dữ liệu không hợp lệ. Vui lòng thử lại.";
    }
}

std::string GUI::promptLine(const std::string& title, const std::string& prompt,
                            const std::string& defaultValue) {
    clearScreen();
    setbkcolor(COLOR_BACKGROUND);
    drawHeaderFrame(WINDOW_WIDTH);

    int left, top, right, bottom, centerX, innerLeft, innerTop;
    drawContentFrame(title, WINDOW_WIDTH, WINDOW_HEIGHT,
                     left, top, right, bottom, centerX, innerLeft, innerTop);

    int y = innerTop + 6;
    setcolor(COLOR_TEXT);
    drawLeftAlignedText(innerLeft, y, prompt);

    int inputX = innerLeft + approxTextWidth(prompt) + approxCharWidth();
    int inputY = y;
    int maxLen = (right - innerLeft) / (approxCharWidth() > 0 ? approxCharWidth() : 1) - 2;
    if (maxLen < 8) maxLen = 8;

    std::string input = readLineAt(inputX, inputY, maxLen, InputFilter::Any);
    if (input.empty()) {
        return defaultValue;
    }
    return input;
}

bool GUI::promptYesNo(const std::string& title, const std::string& prompt) {
    while (true) {
        std::string input = promptLine(title, prompt + " (y/n): ");
        if (input.empty()) {
            continue;
        }
        char c = static_cast<char>(std::tolower(static_cast<unsigned char>(input[0])));
        if (c == 'y') return true;
        if (c == 'n') return false;
        showMessage(title, {"Vui lòng nhập y hoặc n."});
    }
}

void GUI::showMessage(const std::string& title, const std::vector<std::string>& lines) {
    clearScreen();
    setbkcolor(COLOR_BACKGROUND);
    drawHeaderFrame(WINDOW_WIDTH);

    int left, top, right, bottom, centerX, innerLeft, innerTop;
    drawContentFrame(title, WINDOW_WIDTH, WINDOW_HEIGHT,
                     left, top, right, bottom, centerX, innerLeft, innerTop);

    int lineH = approxLineHeight();
    int y = innerTop + 4;
    int messageX = bottomIndentX(left);
    setcolor(COLOR_TEXT);
    for (const auto& line : lines) {
        if (y > bottom - lineH * 2) break;
        drawLeftAlignedText(messageX, y, line);
        y += lineH;
    }

    setcolor(LIGHTCYAN);
    drawCenteredText(centerX, bottom - lineH - 4, PRESS_ANY_KEY);
    waitForKey();
}

void GUI::waitForKey() {
    getch();
}
