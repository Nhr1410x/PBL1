import sys
import re

with open("algorithms.cpp", "r", encoding="utf-8") as f:
    text = f.read()

# Replace logStep signature
text = text.replace('void Algorithms::logStep(std::vector<std::string>& logs, const std::string& message) {',
                    'void Algorithms::logStep(std::vector<std::pair<int, std::string>>& logs, int color, const std::string& message) {')
text = text.replace('logs.push_back(message);', 'logs.push_back({color, message});')

# Replace formatDistanceTable to return pairs
text = text.replace('std::vector<std::string> formatDistanceTable(',
                    'std::vector<std::pair<int, std::string>> formatDistanceTable(')
text = text.replace('std::vector<std::string> lines;',
                    'std::vector<std::pair<int, std::string>> lines;')
text = text.replace('lines.push_back(header.str());\n    lines.push_back(values.str());',
                    'lines.push_back({15, header.str()});\n    lines.push_back({14, values.str()});')

# Replace specific logStep calls using regex or string replace
# Title
text = text.replace('logStep(result.logs, "              ======= THUẬT TOÁN DIJKSTRA =======");',
                    'logStep(result.logs, 14, "              ======= THUẬT TOÁN DIJKSTRA =======");')
text = text.replace('logStep(result.logs, "           ======= THUẬT TOÁN BELLMAN-FORD =======");',
                    'logStep(result.logs, 14, "           ======= THUẬT TOÁN BELLMAN-FORD =======");')
text = text.replace('logStep(result.logs, "Đỉnh bắt đầu:', 'logStep(result.logs, 11, "Đỉnh bắt đầu:')
text = text.replace('logStep(result.logs, "Khởi tạo khoảng cách:', 'logStep(result.logs, 7, "Khởi tạo khoảng cách:')

# For loop step
text = text.replace('logStep(result.logs, "");', 'logStep(result.logs, 15, "");')
text = text.replace('logStep(result.logs, "[Lần lặp "', 'logStep(result.logs, 11, "[Lần lặp "')
text = text.replace('logStep(result.logs, "[Lượt "', 'logStep(result.logs, 11, "[Lượt "')
text = text.replace('logStep(result.logs, "Xử lý đỉnh: "', 'logStep(result.logs, 10, "Xử lý đỉnh: "')
text = text.replace('logStep(result.logs, "  Cập nhật: "', 'logStep(result.logs, 13, "  Cập nhật: "')
text = text.replace('logStep(result.logs, "Khoảng cách sau lần lặp "', 'logStep(result.logs, 15, "Khoảng cách sau lần lặp "')
text = text.replace('logStep(result.logs, "Bảng khoảng cách sau lượt "', 'logStep(result.logs, 15, "Bảng khoảng cách sau lượt "')
text = text.replace('logStep(result.logs, line);', 'logStep(result.logs, line.first, line.second);')

text = text.replace('logStep(result.logs, "                  === KHOẢNG CÁCH CUỐI ===");',
                    'logStep(result.logs, 14, "                  === KHOẢNG CÁCH CUỐI ===");')
text = text.replace('logStep(result.logs, "=== KHOẢNG CÁCH CUỐI ===");',
                    'logStep(result.logs, 14, "=== KHOẢNG CÁCH CUỐI ===");')
text = text.replace('logStep(result.logs,"dist["', 'logStep(result.logs, 11, "dist["')
text = text.replace('logStep(result.logs, graph.getVertexLabel(i) + " = INF', 'logStep(result.logs, 11, graph.getVertexLabel(i) + " = INF')
text = text.replace('logStep(result.logs, graph.getVertexLabel(i) + " = "', 'logStep(result.logs, 11, graph.getVertexLabel(i) + " = "')

text = text.replace('logStep(result.logs, "  (Không có cập nhật ở lượt này - có thể dừng sớm)");',
                    'logStep(result.logs, 7, "  (Không có cập nhật ở lượt này - có thể dừng sớm)");')
text = text.replace('logStep(result.logs, "=== KIỂM TRA CHU TRÌNH ÂM ===");',
                    'logStep(result.logs, 14, "=== KIỂM TRA CHU TRÌNH ÂM ===");')
text = text.replace('logStep(result.logs, "PHÁT HIỆN CHU TRÌNH ÂM!");',
                    'logStep(result.logs, 12, "PHÁT HIỆN CHU TRÌNH ÂM!");')
text = text.replace('logStep(result.logs, "Cạnh: "', 'logStep(result.logs, 12, "Cạnh: "')
text = text.replace('logStep(result.logs, "Không phát hiện chu trình âm.");',
                    'logStep(result.logs, 10, "Không phát hiện chu trình âm.");')

with open("algorithms.cpp", "w", encoding="utf-8") as f:
    f.write(text)
