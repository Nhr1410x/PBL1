import sys

with open("GUI.cpp", "r", encoding="utf-8") as f:
    text = f.read()

# Replace showAlgorithmLogs signature
text = text.replace('void GUI::showAlgorithmLogs(const std::string& title, const std::vector<std::string>& logs) {',
                    'void GUI::showAlgorithmLogs(const std::string& title, const std::vector<std::pair<int, std::string>>& logs) {')

# inside showAlgorithmLogs
text = text.replace('std::vector<std::string> wrappedLogs;',
                    'std::vector<std::pair<int, std::string>> wrappedLogs;')

wrap_old = '''    wrappedLogs = wrapLinesToWidth(logs, maxWidth);
    if (wrappedLogs.empty()) {
        wrappedLogs.push_back("");
    }'''

wrap_new = '''    for (const auto& log : logs) {
        auto parts = wrapLineToWidth(log.second, maxWidth);
        for (const auto& p : parts) {
            wrappedLogs.push_back({log.first, p});
        }
    }
    if (wrappedLogs.empty()) {
        wrappedLogs.push_back({COLOR_TEXT, ""});
    }'''
text = text.replace(wrap_old, wrap_new)

print_old = '''        setcolor(COLOR_TEXT);
        int yPos = innerTop;
        int count = 0;
        while (index < wrappedLogs.size() && count < maxLinesPerPage) {
            outtextxy(innerLeft, yPos, (char*)wrappedLogs[index].c_str());
            yPos += lineStep;
            index++;
            count++;
        }'''

print_new = '''        int yPos = innerTop;
        int count = 0;
        while (index < wrappedLogs.size() && count < maxLinesPerPage) {
            setcolor(wrappedLogs[index].first);
            outtextxy(innerLeft, yPos, (char*)wrappedLogs[index].second.c_str());
            yPos += lineStep;
            index++;
            count++;
        }'''
text = text.replace(print_old, print_new)

with open("GUI.cpp", "w", encoding="utf-8") as f:
    f.write(text)
