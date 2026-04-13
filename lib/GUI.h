#ifndef GUI_H
#define GUI_H

#include "graphics.h"
#include <vector>
#include <string>
#include <tuple>
#include "Global.h"
#include "Colors.h"

class GUI {
private:
    int screenWidth;
    int screenHeight;

public:
    GUI();
    ~GUI();
    void drawMenu();
    void drawComparisonScreen(const std::vector<std::string>& logs);
    void clearScreen();
    int promptMenuChoice();
    int promptChoice(const std::string& title, const std::vector<std::string>& options, const std::string& prompt, int minValue, int maxValue);
    int promptInt(const std::string& title, const std::string& prompt, int minValue, int maxValue);
    std::string promptLine(const std::string& title, const std::string& prompt, const std::string& defaultValue = "");
    bool promptYesNo(const std::string& title, const std::string& prompt);
    void promptGraphInput(bool isDirected, int& numVertices, int& numEdges, std::vector<std::tuple<int, int, int>>& edges);
    void promptStartEnd(const std::string& title, int minValue, int maxValue, int& startValue, int& endValue);
    void showAlgorithmLogs(const std::string& title, const std::vector<std::string>& logs);
    void showMessage(const std::string& title, const std::vector<std::string>& lines);
    void waitForKey();
};

#endif
