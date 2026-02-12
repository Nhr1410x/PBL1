#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <chrono>
#ifdef _WIN32
#include <windows.h>
#include <cwchar>
#endif
#include "../lib/graphics.h"
#include "../lib/Global.h"
#include "../lib/GUI.h"
#include "../lib/graph.h"
#include "../lib/Algorithms.h"
#include "../lib/Comparison.h"

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

static bool promptGraphDirected() {
    std::vector<std::string> options = {
        "[1]. Có hướng",
        "[2]. Vô hướng"
    };
    int choice = gui->promptChoice("LOẠI ĐỒ THỊ", options, "Nhập lựa chọn:", 1, 2);
    return choice == 1;
}

static void createGraphFromGui(bool isDirected) {
    graph.clear();

    int numVertices = 0;
    int numEdges = 0;
    std::vector<std::tuple<int, int, int>> edges;
    gui->promptGraphInput(isDirected, numVertices, numEdges, edges);
    for (int i = 0; i < numVertices; i++) {
        graph.addVertex(std::to_string(i + 1));
    }

    for (const auto& edge : edges) {
        int source = std::get<0>(edge);
        int dest = std::get<1>(edge);
        int weight = std::get<2>(edge);
        graph.addEdge(source, dest, weight);
    }

    if (!isDirected) {
        graph.makeUndirected();
    }
}

static void handleGraphInput() {
    std::vector<std::string> options = {
        "[1]. Tải từ file",
        "[2]. Tạo đồ thị mới (nhập trực tiếp)",
        "[3]. Quay lại"
    };
    int choice = gui->promptChoice("TẠO/TẢI ĐỒ THỊ", options, "Nhập lựa chọn:", 1, 3);

    if (choice == 1) {
        bool isDirected = promptGraphDirected();
        std::string filename = gui->promptLine("TẢI ĐỒ THỊ",
            "Nhập đường dẫn file (mặc định: " + DEFAULT_GRAPH_FILE + "): ",
            DEFAULT_GRAPH_FILE
        );
        bool needCreate = false;
        if (!graph.readFromFile(filename, needCreate)) {
            if (needCreate) {
                bool create = gui->promptYesNo("TẢI ĐỒ THỊ", "Không tìm thấy file. Tạo file mới?");
                if (create) {
                    createGraphFromGui(isDirected);
                    lastResult = PathResult();
                    if (graph.saveToFile(filename)) {
                        gui->showMessage("TẢI ĐỒ THỊ", {"Đã lưu đồ thị vào: " + filename});
                    } else {
                        gui->showMessage("TẢI ĐỒ THỊ", {"Lưu đồ thị thất bại."});
                    }
                }
            } else {
                gui->showMessage("TẢI ĐỒ THỊ", {"Tải file thất bại."});
            }
        } else {
            if (!isDirected) {
                graph.makeUndirected();
            }
            lastResult = PathResult();
            gui->showMessage("TẢI ĐỒ THỊ", {
                "Đã tải đồ thị: V = " + std::to_string(graph.getVertexCount()) +
                ", E = " + std::to_string(graph.getEdgeCount()) + "."
            });
        }
    } else if (choice == 2) {
        bool isDirected = promptGraphDirected();
        createGraphFromGui(isDirected);

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
        gui->showMessage("THÔNG BÁO", {"Đồ thị rỗng. Vui lòng tạo hoặc tải đồ thị trước."});
        return;
    }

    if (type == AlgorithmType::DIJKSTRA && graph.hasNegativeWeights()) {
        gui->showMessage("THÔNG BÁO", {"Dijkstra không chạy được với cạnh có trọng số âm."});
        return;
    }

    int V = graph.getVertexCount();
    int startInput = 1;
    int endInput = 1;
    gui->promptStartEnd("CHỌN ĐỈNH", 1, V, startInput, endInput);
    int start = startInput - 1;
    int end = endInput - 1;

    PathResult result;
    auto startTime = std::chrono::high_resolution_clock::now();
    if (type == AlgorithmType::DIJKSTRA) {
        result = algorithms->dijkstra(start, true);
    } else {
        result = algorithms->bellmanFord(start, true);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto execUs = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();

    if (!result.success) {
        std::vector<std::string> lines = {"Thuật toán thất bại."};
        if (result.hasNegativeCycle) {
            lines.push_back("Phát hiện chu trình âm.");
        }
        gui->showMessage("KẾT QUẢ", lines);
        return;
    }

    result.shortestPath = algorithms->getShortestPath(result, end);
    lastResult = result;
    std::string algoTitle = (type == AlgorithmType::DIJKSTRA)
        ? "QUÁ TRÌNH THỰC HIỆN THUẬT TOÁN DIJKSTRA"
        : "QUÁ TRÌNH THỰC HIỆN THUẬT TOÁN BELLMAN-FORD";

    std::vector<std::string> displayLogs;
    std::string startLabel = graph.getVertexLabel(start);
    std::string endLabel = graph.getVertexLabel(end);
    int dist = algorithms->getDistance(result, end);
    // if (dist >= 0 && dist != INT_MAX) {
    //     displayLogs.push_back("Đường đi ngắn nhất từ đỉnh " + startLabel + " đến đỉnh " + endLabel + " là " + std::to_string(dist));
    // } 
    // else {
    //     displayLogs.push_back("Không tồn tại đường đi từ " + startLabel + " đến " + endLabel);
    // }

    // if (result.shortestPath.empty()) {
    //     displayLogs.push_back("Không tồn tại đường đi từ " + startLabel + " đến " + endLabel);
    // } else {
    //     std::string pathLine = "Đường đi: ";
    //     for (size_t i = 0; i < result.shortestPath.size(); i++) {
    //         if (i > 0) pathLine += " -> ";
    //         pathLine += graph.getVertexLabel(result.shortestPath[i]);
    //     }
    //     displayLogs.push_back(pathLine);
    // }
    // displayLogs.push_back("Thời gian thực hiện: " + std::to_string(execUs) + " us");
    // displayLogs.push_back("");
    displayLogs.insert(displayLogs.end(), result.logs.begin(), result.logs.end());

    gui->showAlgorithmLogs(algoTitle, displayLogs);

    std::vector<std::string> lines;
    lines.push_back("Đường đi ngắn nhất từ " + graph.getVertexLabel(start) +
                    " đến " + graph.getVertexLabel(end) + ":");
    if (result.shortestPath.empty()) {
        lines.push_back("Không tìm thấy đường đi.");
    } 
    else {
        lines.push_back("");
        std::string pathLine;
        for (size_t i = 0; i < result.shortestPath.size(); i++) {
            if (!pathLine.empty()) pathLine += " -> ";
            pathLine += graph.getVertexLabel(result.shortestPath[i]);
        }
        lines.push_back(pathLine);
        int dist = algorithms->getDistance(result, end);
        lines.push_back("Khoảng cách = " + std::to_string(dist));
        lines.push_back("Thời gian thực hiện: " + std::to_string(execUs) + " us");
    }
    if(type == AlgorithmType::DIJKSTRA){
        gui->showMessage("KẾT QUẢ THUẬT TOÁN DIJKSTRA", lines);
    } 
    else {
        gui->showMessage("KẾT QUẢ THUẬT TOÁN BELLMAN-FORD", lines);
    }
    // gui->showMessage("KẾT QUẢ", lines);
}

static void compareAlgorithms() {
    if (!graph.isValid()) {
        gui->showMessage("THÔNG BÁO", {"Đồ thị rỗng. Vui lòng tạo hoặc tải đồ thị trước."});
        return;
    }

    int V = graph.getVertexCount();
    int startInput = gui->promptInt("CHỌN ĐỈNH",
        "Đỉnh bắt đầu (1.." + std::to_string(V) + "): ", 1, V);
    int start = startInput - 1;

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
    char bgiPath[] = "";
    initgraph(&gd, &gm, bgiPath);

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
