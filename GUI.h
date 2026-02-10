#ifndef GUI_H
#define GUI_H

#include "graphics.h"
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include "Global.h"
#include "Colors.h"

// ==================== BUTTON CLASS ====================
class Button {
private:
    int x, y, width, height;
    std::string label;
    bool isHovered;
    std::function<void()> callback;

public:
    Button(int x, int y, int width, int height, const std::string& label);
    ~Button();

    void draw();
    bool isClicked(int mouseX, int mouseY);
    bool handleMouseMove(int mouseX, int mouseY);
    void setCallback(std::function<void()> cb);
    void getRect(int& rx, int& ry, int& rw, int& rh) const;
};

// ==================== TEXT BOX CLASS ====================
class TextBox {
private:
    int x, y, width, height;
    std::string content;
    bool isActive;

public:
    TextBox(int x, int y, int width, int height);
    ~TextBox();
    
    void draw();
    void setText(const std::string& text);
    std::string getText() const;
    void clear();
    void setActive(bool active);
};

// ==================== GUI CLASS ====================
class GUI {
private:
    int screenWidth;
    int screenHeight;
    std::vector<std::shared_ptr<Button>> buttons;
    std::vector<std::shared_ptr<TextBox>> textBoxes;

public:
    GUI();
    ~GUI();

    void drawMenu();
    void drawInputMenu();
    void drawRunningScreen(const std::vector<std::string>& logs);
    void drawComparisonScreen(const std::vector<std::string>& logs);
    void handleEvent();
    void clearScreen();

    // GUI-based input/output
    int promptMenuChoice();
    int promptChoice(const std::string& title, const std::vector<std::string>& options,
                     const std::string& prompt, int minValue, int maxValue);
    int promptInt(const std::string& title, const std::string& prompt, int minValue, int maxValue);
    std::string promptLine(const std::string& title, const std::string& prompt,
                           const std::string& defaultValue = "");
    bool promptYesNo(const std::string& title, const std::string& prompt);
    void showMessage(const std::string& title, const std::vector<std::string>& lines);
    void waitForKey();
};

#endif
