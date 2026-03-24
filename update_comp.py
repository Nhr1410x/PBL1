import sys

with open("comparison.cpp", "r", encoding="utf-8") as f:
    text = f.read()

# Replace push_back string with push_back colored pair
text = text.replace('report.logs.push_back("        ========================================");', 
                    'report.logs.push_back({14, "        ========================================"});')
text = text.replace('report.logs.push_back("                BÁO CÁO SO SÁNH HIỆU NĂNG");',
                    'report.logs.push_back({14, "                BÁO CÁO SO SÁNH HIỆU NĂNG"});')
text = text.replace('report.logs.push_back("Đỉnh bắt đầu: "',
                    'report.logs.push_back({11, "Đỉnh bắt đầu: "')

text = text.replace('report.logs.push_back(border);', 'report.logs.push_back({15, border});')
text = text.replace('report.logs.push_back(row("", "DIJKSTRA", "BELLMAN-FORD"));', 'report.logs.push_back({11, row("", "DIJKSTRA", "BELLMAN-FORD")});')

text = text.replace('report.logs.push_back(row("Thời gian', 'report.logs.push_back({15, row("Thời gian')
text = text.replace('report.logs.push_back(row("Bộ nhớ', 'report.logs.push_back({15, row("Bộ nhớ')
text = text.replace('report.logs.push_back(row("Độ', 'report.logs.push_back({15, row("Độ')
text = text.replace('report.logs.push_back(row("Trạng', 'report.logs.push_back({15, row("Trạng')

text = text.replace('report.logs.push_back("                        --- SO SÁNH ---");',
                    'report.logs.push_back({14, "                        --- SO SÁNH ---"});')

text = text.replace('report.logs.push_back("Bellman-Ford chậm hơn Dijkstra "',
                    'report.logs.push_back({10, "Bellman-Ford chậm hơn Dijkstra "')
text = text.replace('report.logs.push_back("Phát hiện trọng số âm, không thể thực hiện thuật toán Dijkstra. KHÔNG THỂ SO SÁNH.");',
                    'report.logs.push_back({12, "Phát hiện trọng số âm, không thể thực hiện thuật toán Dijkstra. KHÔNG THỂ SO SÁNH."});')

text = text.replace('report.logs.push_back("");', 'report.logs.push_back({15, ""});')

with open("comparison.cpp", "w", encoding="utf-8") as f:
    f.write(text)
