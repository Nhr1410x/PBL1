#!/bin/bash
g++ -std=c++17 src/main.cpp src/MainWindow.cpp src/GraphCanvas.cpp src/AdvancedAlgorithms.cpp src/algorithms.cpp src/graph.cpp src/comparison.cpp -o app_gui.exe $(pkg-config --cflags --libs gtkmm-3.0)
