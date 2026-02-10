#include <iostream>
#include <limits>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#include <cwchar>
#endif
#include "graphics.h"
#include "Global.h"
#include "GUI.h"
#include "Graph.h"
#include "Algorithms.h"
#include "Comparison.h"

// ==================== GLOBAL VARIABLES ====================
Graph graph;
Algorithms* algorithms = nullptr;
Comparison* comparison = nullptr;
GUI* gui = nullptr;

PathResult lastResult;

#ifdef _WIN32
static void initConsoleUtf8() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        CONSOLE_FONT_INFOEX cfi;
        cfi.cbSize = sizeof(cfi);
        if (GetCurrentConsoleFontEx(hOut, FALSE, &cfi)) {
            cfi.FontFamily = FF_DONTCARE;
            cfi.FontWeight = FW_NORMAL;
            std::wcsncpy(cfi.FaceName, L"Consolas", LF_FACESIZE - 1);
            cfi.FaceName[LF_FACESIZE - 1] = L'\0';
            SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
        }
    }
}
#endif

// ==================== HELPER FUNCTIONS ====================
static void createGraphFromGui() {
    graph.clear();

    int numVertices = gui->promptInt("TẠO ĐỒ THỊ", "Số đỉnh: ", 1, 500);
    for (int i = 0; i < numVertices; i++) {
        std::string label = gui->promptLine(
            "TẠO ĐỒ THỊ",
            "Nhãn đỉnh " + std::to_string(i) + " (mặc định: V" + std::to_string(i) + "): ",
            "V" + std::to_string(i)
        );
        if (label.empty()) {
            label = "V" + std::to_string(i);
        }
        graph.addVertex(label);
    }

    int maxEdges = numVertices * numVertices;
    int numEdges = gui->promptInt("TẠO ĐỒ THỊ", "Số cạnh: ", 0, maxEdges);
    for (int i = 0; i < numEdges; i++) {
        int source = gui->promptInt(
            "TẠO ĐỒ THỊ",
            "Cạnh " + std::to_string(i) + " - đỉnh nguồn (0.." + std::to_string(numVertices - 1) + "): ",
            0, numVertices - 1
        );
        int dest = gui->promptInt(
            "TẠO ĐỒ THỊ",
            "Cạnh " + std::to_string(i) + " - đỉnh đích (0.." + std::to_string(numVertices - 1) + "): ",
            0, numVertices - 1
        );
        int weight = gui->promptInt(
            "TẠO ĐỒ THỊ",
            "Cạnh " + std::to_string(i) + " - trọng số (âm/dương): ",
            -1000000, 1000000
        );
        graph.addEdge(source, dest, weight);
    }
}

static void handleGraphInput() {
    std::vector<std::string> options = {
        "                                  [1]. Tải từ file",
        "                                  [2]. Tạo đồ thị mới (nhập trực tiếp)",
        "                                  [3]. Quay lại"
    };
    int choice = gui->promptChoice("TẠO/TẢI ĐỒ THỊ", options, "                   Nhập lựa chọn:", 1, 3);

    if (choice == 1) {
        std::string filename = gui->promptLine("TẢI ĐỒ THỊ",
            "       Nhập đường dẫn file (mặc định: " + DEFAULT_GRAPH_FILE + "): ",
            DEFAULT_GRAPH_FILE
        );
        bool needCreate = false;
        if (!graph.readFromFile(filename, needCreate)) {
            if (needCreate) {
                bool create = gui->promptYesNo("TẢI ĐỒ THỊ", "          Không tìm thấy file. Tạo file mới?");
                if (create) {
                    createGraphFromGui();
                    lastResult = PathResult();
                    if (graph.saveToFile(filename)) {
                        gui->showMessage("TẢI ĐỒ THỊ", {"       Đã lưu đồ thị vào: " + filename});
                    } else {
                        gui->showMessage("TẢI ĐỒ THỊ", {"       Lưu đồ thị thất bại."});
                    }
                }
            } else {
                gui->showMessage("TẢI ĐỒ THỊ", {"             Tải file thất bại."});
            }
        } else {
            lastResult = PathResult();
            gui->showMessage("TẢI ĐỒ THỊ", {
                "                           Đã tải đồ thị: V=" + std::to_string(graph.getVertexCount()) +
                ", E=" + std::to_string(graph.getEdgeCount())
            });
        }
    } else if (choice == 2) {
        createGraphFromGui();

        lastResult = PathResult();
        bool save = gui->promptYesNo("TẠO ĐỒ THỊ", "Lưu đồ thị ra file?");
        if (save) {
            std::string filename = gui->promptLine(
                "TẠO ĐỒ THỊ",
                "Đường dẫn file (mặc định: " + DEFAULT_GRAPH_FILE + "): ",
                DEFAULT_GRAPH_FILE
            );
            if (graph.saveToFile(filename)) {
                gui->showMessage("TẠO ĐỒ THỊ", {"Đã lưu đồ thị vào " + filename});
            } else {
                gui->showMessage("TẠO ĐỒ THỊ", {"Lưu đồ thị thất bại."});
            }
        }
    }
}

static void runAlgorithm(AlgorithmType type) {
    if (!graph.isValid()) {
        gui->showMessage("THÔNG BÁO", {"                   Đồ thị rỗng. Vui lòng tạo hoặc tải đồ thị trước."});
        return;
    }

    if (type == AlgorithmType::DIJKSTRA && graph.hasNegativeWeights()) {
        gui->showMessage("THÔNG BÁO", {"                   Dijkstra không chạy được với cạnh có trọng số âm."});
        return;
    }

    int V = graph.getVertexCount();
    int start = gui->promptInt("CHỌN ĐỈNH",
        "Đỉnh bắt đầu (0.." + std::to_string(V - 1) + "): ",
        0, V - 1);
    int end = gui->promptInt("CHỌN ĐỈNH",
        "Đỉnh kết thúc (0.." + std::to_string(V - 1) + "): ",
        0, V - 1);

    PathResult result;
    if (type == AlgorithmType::DIJKSTRA) {
        result = algorithms->dijkstra(start, true);
    } else {
        result = algorithms->bellmanFord(start, true);
    }

    if (!result.success) {
        std::vector<std::string> lines = {"                   Thuật toán thất bại."};
        if (result.hasNegativeCycle) {
            lines.push_back("Phát hiện chu trình âm.");
        }
        gui->showMessage("KẾT QUẢ", lines);
        return;
    }

    result.shortestPath = algorithms->getShortestPath(result, end);
    lastResult = result;
    gui->drawRunningScreen(result.logs);
    gui->waitForKey();

    std::vector<std::string> lines;
    lines.push_back("Đường đi ngắn nhất từ " + graph.getVertexLabel(start) +
                    " đến " + graph.getVertexLabel(end) + ":");
    if (result.shortestPath.empty()) {
        lines.push_back("Không tìm thấy đường đi.");
    } else {
        std::string pathLine;
        for (size_t i = 0; i < result.shortestPath.size(); i++) {
            if (!pathLine.empty()) pathLine += " -> ";
            pathLine += graph.getVertexLabel(result.shortestPath[i]);
        }
        lines.push_back(pathLine);
        int dist = algorithms->getDistance(result, end);
        lines.push_back("Khoảng cách = " + std::to_string(dist));
    }
    gui->showMessage("KẾT QUẢ", lines);
}

static void compareAlgorithms() {
    if (!graph.isValid()) {
        gui->showMessage("THÔNG BÁO", {"Đồ thị rỗng. Vui lòng tạo hoặc tải đồ thị trước."});
        return;
    }

    int V = graph.getVertexCount();
    int start = gui->promptInt("CHỌN ĐỈNH",
        "Đỉnh bắt đầu (0.." + std::to_string(V - 1) + "): ", 0, V - 1);

    auto report = comparison->comparePerformance(start, AlgorithmType::BOTH);
    gui->drawComparisonScreen(report.logs);
    gui->waitForKey();
}

static void exportToPython() {
    if (!graph.isValid()) {
        gui->showMessage("THÔNG BÁO", {"Đồ thị rỗng. Vui lòng tạo hoặc tải đồ thị trước."});
        return;
    }

    bool ok = false;
    if (!lastResult.shortestPath.empty()) {
        ok = graph.exportForPython(lastResult.shortestPath);
    } else {
        ok = graph.exportForPython(TEMP_EXPORT_FILE);
    }

    if (ok) {
        gui->showMessage("XUẤT PYTHON", {
            "Đã xuất ra " + TEMP_EXPORT_FILE,
            "Chạy: python visualizer.py"
        });
    } else {
        gui->showMessage("XUẤT PYTHON", {"Xuất thất bại."});
    }
}

// ==================== MAIN FUNCTION ====================
int main() {
#ifdef _WIN32
    initConsoleUtf8();
#endif
    int gd = DETECT, gm;
    initgraph(&gd, &gm, "");

    if (graphresult() != grOk) {
        std::cerr << "Khởi tạo đồ họa thất bại. Chuyển sang chế độ console.\n";
    }

    algorithms = new Algorithms(graph);
    comparison = new Comparison(graph);
    gui = new GUI();

    bool running = true;
    while (running) {
        int choice = gui->promptMenuChoice();

        switch (choice) {
            case 1:
                handleGraphInput();
                break;
            case 2:
                runAlgorithm(AlgorithmType::DIJKSTRA);
                break;
            case 3:
                runAlgorithm(AlgorithmType::BELLMAN_FORD);
                break;
            case 4:
                compareAlgorithms();
                break;
            case 5:
                exportToPython();
                break;
            case 6:
                running = false;
                break;
            default:
                break;
        }
    }

    closegraph();
    delete algorithms;
    delete comparison;
    delete gui;

    return 0;
}
