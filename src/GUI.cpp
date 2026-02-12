#include "../lib/GUI.h"
#include <iostream>
#include <cctype>
#include <sstream>

namespace {
const int OUTER_MARGIN = 30;
const int HEADER_GAP = 20;
const int FRAME_PADDING = 24;

const int HEADER_LINES = 8;
const int MENU_LINES = 8;
const int CHOICE_LINES = 2;
const int LEFT_INDENT_SPACES = 30;

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

std::vector<std::string> wrapLineToWidth(const std::string& line, int maxWidth) {
    std::vector<std::string> out;
    if (maxWidth <= 0) {
        out.push_back(line);
        return out;
    }
    if (line.empty()) {
        out.push_back("");
        return out;
    }

    size_t pos = line.find_first_not_of(' ');
    if (pos == std::string::npos) {
        out.push_back(line);
        return out;
    }

    std::string indent = line.substr(0, pos);
    std::string content = line.substr(pos);
    std::istringstream iss(content);
    std::string word;
    std::string current = indent;
    bool hasWord = false;

    while (iss >> word) {
        std::string candidate = hasWord ? (current + " " + word) : (current + word);
        if (approxTextWidth(candidate) <= maxWidth || !hasWord) {
            current = candidate;
            hasWord = true;
        } else {
            out.push_back(current);
            current = indent + word;
            hasWord = true;
        }
    }

    if (hasWord) {
        out.push_back(current);
    } else {
        out.push_back(line);
    }
    return out;
}

std::vector<std::string> wrapLinesToWidth(const std::vector<std::string>& lines, int maxWidth) {
    std::vector<std::string> out;
    for (const auto& line : lines) {
        auto parts = wrapLineToWidth(line, maxWidth);
        out.insert(out.end(), parts.begin(), parts.end());
    }
    return out;
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

int bottomIndentX(int frameLeft) {
    return leftIndentX(frameLeft);
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

bool tryParseEdgeLine(const std::string& text, int& u, int& v, int& w) {
    std::istringstream iss(text);
    if (!(iss >> u >> v >> w)) {
        return false;
    }
    std::string extra;
    if (iss >> extra) {
        return false;
    }
    return true;
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

void clearTextLine(int x, int y, int maxLen) {
    if (maxLen < 1) maxLen = 1;
    std::string blank(static_cast<size_t>(maxLen), ' ');
    outtextxy(x, y, (char*)blank.c_str());
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

    innerLeft = leftIndentX(left);
}

void drawHeaderFrame(int screenW) {
    int extra = 2 * approxCharWidth(); 
    int left = OUTER_MARGIN - extra;
    int right = screenW - OUTER_MARGIN + extra;
    if (left < 0) left = 0;
    if (right > screenW) right = screenW;
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
    y += lineH + 14;
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
    int extra = 2 * approxCharWidth(); 
    left = OUTER_MARGIN - extra;
    right = screenW - OUTER_MARGIN + extra;
    if (left < 0) left = 0;
    if (right > screenW) right = screenW;
    top = OUTER_MARGIN + headerHeight() + HEADER_GAP;
    bottom = screenH - OUTER_MARGIN;
    drawFrameBox(title, left, top, right, bottom, centerX, innerLeft, innerTop);
}

// void drawContentFrameCustom(const std::string& title, int screenW, int screenH, int outerMargin, int headerGap,
//                             int& left, int& top, int& right, int& bottom, int& centerX, int& innerLeft, int& innerTop) {
//     left = outerMargin;
//     right = screenW - outerMargin;
//     top = outerMargin + headerHeight() + headerGap;
//     bottom = screenH - outerMargin;
//     drawFrameBox(title, left, top, right, bottom, centerX, innerLeft, innerTop);
// }

void drawLogFrame(const std::string& title, int left, int top, int right, int bottom,
                  int& centerX, int& innerLeft, int& innerTop) {
    setcolor(COLOR_BUTTON);
    rectangle(left, top, right, bottom);

    centerX = (left + right) / 2;
    int lineH = approxLineHeight();
    if (!title.empty()) {
        setcolor(COLOR_TEXT);
        int titleY = top + 2;
        drawCenteredText(centerX, titleY, title);
    }

    innerLeft = left + 2;
    innerTop = top + 2 * lineH;
}
} 

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
    int menuTextX = leftIndentX(left);
    drawLeftAlignedText(menuTextX, y, "[1]. Khởi tạo/Nạp đồ thị (từ file)"); y += lineH;
    drawLeftAlignedText(menuTextX, y, "[2]. Chạy thuật toán Dijkstra"); y += lineH;
    drawLeftAlignedText(menuTextX, y, "[3]. Chạy thuật toán Bellman-Ford"); y += lineH;
    drawLeftAlignedText(menuTextX, y, "[4]. So sánh hiệu năng"); y += lineH;
    drawLeftAlignedText(menuTextX, y, "[5]. Trực quan hóa (Python)"); y += lineH;
    drawLeftAlignedText(menuTextX, y, "[6]. Thoát"); y += lineH;
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


void GUI::drawComparisonScreen(const std::vector<std::string>& logs) {
    size_t index = 0;
    int lineH = approxLineHeight();
    int maxLinesPerPage = 0;
    std::vector<std::string> wrappedLogs = logs;
    const int safetyLines = 3;

    while (true) {
        clearScreen();
        setbkcolor(COLOR_BACKGROUND);
        
        drawHeaderFrame(WINDOW_WIDTH);

        int left, top, right, bottom, centerX, innerLeft, innerTop;
        drawContentFrame("SO SÁNH THUẬT TOÁN", WINDOW_WIDTH, WINDOW_HEIGHT,
                         left, top, right, bottom, centerX, innerLeft, innerTop);

        if (wrappedLogs.empty()) {
            wrappedLogs.push_back("");
        }

        if (maxLinesPerPage == 0) {
            int yProbe = innerTop + 6;
            while (yProbe < bottom - lineH * 2) {
                maxLinesPerPage++;
                yProbe += lineH + 4;
            }
            if (maxLinesPerPage < 1) maxLinesPerPage = 1;
            if (maxLinesPerPage > safetyLines) {
                maxLinesPerPage -= safetyLines;
            }
        }

        setcolor(LIGHTGREEN);
        int yPos = innerTop + 6;
        int count = 0;
        while (index < wrappedLogs.size() && count < maxLinesPerPage) {
            outtextxy(innerLeft, yPos, (char*)wrappedLogs[index].c_str());
            yPos += lineH + 4;
            index++;
            count++;
        }

        bool hasMore = index < wrappedLogs.size();
        setcolor(LIGHTCYAN);
        if (hasMore) {
            drawCenteredText(centerX, bottom - lineH - 4, "Nhấn phím bất kỳ để xem tiếp...");
            getch();
            continue;
        }

        drawCenteredText(centerX, bottom - lineH - 4, "Nhấn phím bất kỳ để quay lại menu...");
        break;
    }
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
        int inputX = leftIndentX(left) + approxTextWidth(MENU_PROMPT) + approxCharWidth();

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

void GUI::promptStartEnd(const std::string& title, int minValue, int maxValue,
                         int& startValue, int& endValue) {
    clearScreen();
    setbkcolor(COLOR_BACKGROUND);
    drawHeaderFrame(WINDOW_WIDTH);

    int left, top, right, bottom, centerX, innerLeft, innerTop;
    drawContentFrame(title, WINDOW_WIDTH, WINDOW_HEIGHT,
                     left, top, right, bottom, centerX, innerLeft, innerTop);

    int lineH = approxLineHeight();
    int y = innerTop + 6;

    setcolor(COLOR_TEXT);
    std::string promptStart = "Đỉnh bắt đầu (" + std::to_string(minValue) + ".." + std::to_string(maxValue) + "): ";
    std::string promptEnd = "Đỉnh kết thúc (" + std::to_string(minValue) + ".." + std::to_string(maxValue) + "): ";

    int charW = approxCharWidth();
    if (charW < 1) charW = 1;
    int errorX = innerLeft;
    int errorY = bottom - lineH - 6;
    int errorMaxChars = (right - innerLeft) / charW;

    auto clearError = [&]() {
        setcolor(COLOR_TEXT);
        clearTextLine(errorX, errorY, errorMaxChars);
    };
    auto showError = [&](const std::string& msg) {
        clearError();
        setcolor(LIGHTRED);
        drawLeftAlignedText(errorX, errorY, msg);
        setcolor(COLOR_TEXT);
    };

    drawLeftAlignedText(innerLeft, y, promptStart);
    int inputXStart = innerLeft + approxTextWidth(promptStart) + approxCharWidth();
    while (true) {
        std::string input = readLineAt(inputXStart, y, 8, InputFilter::Integer);
        int value = 0;
        if (tryParseInt(input, value) && value >= minValue && value <= maxValue) {
            startValue = value;
            clearError();
            break;
        }
        showError("Giá trị đỉnh bắt đầu không hợp lệ.");
    }

    y += lineH;
    drawLeftAlignedText(innerLeft, y, promptEnd);
    int inputXEnd = innerLeft + approxTextWidth(promptEnd) + approxCharWidth();
    while (true) {
        std::string input = readLineAt(inputXEnd, y, 8, InputFilter::Integer);
        int value = 0;
        if (tryParseInt(input, value) && value >= minValue && value <= maxValue) {
            endValue = value;
            clearError();
            break;
        }
        showError("Giá trị đỉnh kết thúc không hợp lệ.");
    }
}

void GUI::promptGraphInput(bool isDirected, int& numVertices, int& numEdges,
                           std::vector<std::tuple<int, int, int>>& edges) {
    edges.clear();
    numVertices = 0;
    numEdges = 0;

    clearScreen();
    setbkcolor(COLOR_BACKGROUND);
    drawHeaderFrame(WINDOW_WIDTH);

    int left, top, right, bottom, centerX, innerLeft, innerTop;
    drawContentFrame("TẠO ĐỒ THỊ", WINDOW_WIDTH, WINDOW_HEIGHT,
                     left, top, right, bottom, centerX, innerLeft, innerTop);

    int lineH = approxLineHeight();
    int y = innerTop + 6;

    setcolor(YELLOW);
    drawLeftAlignedText(innerLeft, y, "Nhập thông tin đồ thị:");
    y += lineH + 4;

    setcolor(COLOR_TEXT);
    int charW = approxCharWidth();
    if (charW < 1) charW = 1;
    int errorX = innerLeft;
    int errorY = bottom - lineH - 6;
    int errorMaxChars = (right - innerLeft) / charW;

    auto clearError = [&]() {
        setcolor(COLOR_TEXT);
        clearTextLine(errorX, errorY, errorMaxChars);
    };
    auto showError = [&](const std::string& msg) {
        clearError();
        setcolor(LIGHTRED);
        drawLeftAlignedText(errorX, errorY, msg);
        setcolor(COLOR_TEXT);
    };

    std::string promptV = "Số đỉnh (1-100): ";
    drawLeftAlignedText(innerLeft, y, promptV);
    int inputXV = innerLeft + approxTextWidth(promptV) + approxCharWidth();
    while (true) {
        std::string input = readLineAt(inputXV, y, 6, InputFilter::Integer);
        int value = 0;
        if (tryParseInt(input, value) && value >= 1 && value <= 100) {
            numVertices = value;
            clearError();
            break;
        }
        showError("Số đỉnh không hợp lệ (1-100).");
    }

    y += lineH;

    int maxEdges = numVertices * numVertices;
    std::string promptE = "Số cạnh (0-" + std::to_string(maxEdges) + "): ";
    drawLeftAlignedText(innerLeft, y, promptE);
    int inputXE = innerLeft + approxTextWidth(promptE) + approxCharWidth();

    while (true) {
        std::string input = readLineAt(inputXE, y, 8, InputFilter::Integer);
        int value = 0;
        if (tryParseInt(input, value) && value >= 0 && value <= maxEdges) {
            int yEdgesStart = y + lineH + 4;
            if (!isDirected) {
                yEdgesStart += lineH + 4;
            }
            int availableLines = (errorY - lineH) - yEdgesStart;
            int maxEdgeLines = INT_MAX;
            if (maxEdgeLines < 1) maxEdgeLines = 1;
            if (value > maxEdgeLines) {
                showError("Số cạnh quá nhiều để nhập trong 1 khung. Tối đa " + std::to_string(maxEdgeLines) + ".");
                continue;
            }
            numEdges = value;
            clearError();
            break;
        }
        showError("Số cạnh không hợp lệ.");
    }

    y += lineH + 4;

    if (!isDirected) {
        setcolor(LIGHTRED);
        drawLeftAlignedText(innerLeft, y, "Vô hướng: nhập mỗi cạnh 1 lần (u v w).");
        y += lineH + 4;
        setcolor(COLOR_TEXT);
    }

    for (int i = 0; i < numEdges; i++) {
        std::string prompt = "Cạnh " + std::to_string(i + 1) + " (u v w): ";
        drawLeftAlignedText(innerLeft, y, prompt);
        int inputX = innerLeft + approxTextWidth(prompt) + approxCharWidth();
        while (true) {
            std::string input = readLineAt(inputX, y, 24, InputFilter::Any);
            int u = 0, v = 0, w = 0;
            if (tryParseEdgeLine(input, u, v, w) &&
                u >= 1 && u <= numVertices &&
                v >= 1 && v <= numVertices &&
                w >= -1000000 && w <= 1000000) {
                edges.emplace_back(u - 1, v - 1, w);
                clearError();
                break;
            }
            showError("Cạnh không hợp lệ. Định dạng: u v w, u/v trong [1.." + std::to_string(numVertices) + "].");
        }
        y += lineH;
    }
}

void GUI::showAlgorithmLogs(const std::string& title, const std::vector<std::string>& logs) {
    size_t index = 0;
    int lineH = approxLineHeight();
    int lineStep = lineH;
    std::vector<std::string> wrappedLogs;
    const int margin = 1;
    const int left = margin;
    const int right = WINDOW_WIDTH - margin;
    const int top = margin;
    const int bottom = WINDOW_HEIGHT - margin;
    const int indentSpaces = 25;
    int indent = indentSpaces * approxCharWidth();
    int maxAllowedIndent = (right - left) / 2;
    if (maxAllowedIndent < 0) maxAllowedIndent = 0;
    if (indent > maxAllowedIndent) indent = maxAllowedIndent;

    int innerLeftBase = left + 2 + indent;
    int innerTopBase = top + 2 * lineH;

    int maxWidth = right - innerLeftBase - 2;
    wrappedLogs = wrapLinesToWidth(logs, maxWidth);
    if (wrappedLogs.empty()) {
        wrappedLogs.push_back("");
    }

    int usableBottom = bottom - lineH;
    int maxLinesPerPage = (usableBottom - innerTopBase) / lineStep + 1;
    if (maxLinesPerPage < 1) maxLinesPerPage = 1;
    const int safetyLines = 15;
    if (maxLinesPerPage > safetyLines) {
        maxLinesPerPage -= safetyLines;
    }
    int totalPages = static_cast<int>((wrappedLogs.size() + maxLinesPerPage - 1) / maxLinesPerPage);
    if (totalPages < 1) totalPages = 1;

    int page = 0;
    while (index < wrappedLogs.size()) {
        page++;
        clearScreen();
        setbkcolor(COLOR_BACKGROUND);

        std::string pageTitle = title;
        if (totalPages > 1) {
            pageTitle += " (Trang " + std::to_string(page) + "/" + std::to_string(totalPages) + ")";
        }

        int centerX = 0;
        int innerLeft = 0;
        int innerTop = 0;
        drawLogFrame(pageTitle, left, top, right, bottom, centerX, innerLeft, innerTop);
        innerLeft = innerLeftBase;
        innerTop = innerTopBase;

        setcolor(LIGHTGREEN);
        int yPos = innerTop;
        int count = 0;
        while (index < wrappedLogs.size() && count < maxLinesPerPage) {
            outtextxy(innerLeft, yPos, (char*)wrappedLogs[index].c_str());
            yPos += lineStep;
            index++;
            count++;
        }

        getch();
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
