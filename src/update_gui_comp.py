import sys

with open("GUI.cpp", "r", encoding="utf-8") as f:
    text = f.read()

# Replace drawComparisonScreen signature
text = text.replace('void GUI::drawComparisonScreen(const std::vector<std::string>& logs) {',
                    'void GUI::drawComparisonScreen(const std::vector<std::pair<int, std::string>>& logs) {')

# inside drawComparisonScreen
text = text.replace('std::vector<std::string> wrappedLogs = logs;',
                    'std::vector<std::pair<int, std::string>> wrappedLogs = logs;')

text = text.replace('wrappedLogs.push_back("");',
                    'wrappedLogs.push_back({15, ""});')

print_old = '''        setcolor(COLOR_TEXT);
        int yPos = innerTop + 6;
        int count = 0;
        while (index < wrappedLogs.size() && count < maxLinesPerPage) {
            outtextxy(innerLeft, yPos, (char*)wrappedLogs[index].c_str());
            yPos += lineH + 4;
            index++;
            count++;
        }'''

print_new = '''        int yPos = innerTop + 6;
        int count = 0;
        while (index < wrappedLogs.size() && count < maxLinesPerPage) {
            setcolor(wrappedLogs[index].first);
            outtextxy(innerLeft, yPos, (char*)wrappedLogs[index].second.c_str());
            yPos += lineH + 4;
            index++;
            count++;
        }'''
text = text.replace(print_old, print_new)

with open("GUI.cpp", "w", encoding="utf-8") as f:
    f.write(text)
