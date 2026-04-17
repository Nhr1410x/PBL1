#include <iostream>
#include <gtkmm.h>
#include "../lib/MainWindow.h"
#include "../lib/graph.h"
#include "../lib/Algorithms.h"
#include "../lib/AdvancedAlgorithms.h"
#include "../lib/Comparison.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    // Khởi tạo ứng dụng GTK
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.shortestpath");

    // Khởi tạo các Core object
    Graph graph;
    Algorithms algorithms(graph);
    AdvancedAlgorithms advancedAlgorithms(graph);
    Comparison comparison(graph);

    // Khởi tạo giao diện Window
    MainWindow window(graph, &algorithms, &advancedAlgorithms, &comparison);

    // Bắt đầu vòng lặp sự kiện của GTK
    return app->run(window);
}
