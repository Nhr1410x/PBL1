cmake_minimum_required(VERSION 3.10)
project(ShortestPathGUI)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Giữ Console mở để dễ debug trên Windows
# (nếu muốn ẩn console dùng WIN32 trong add_executable)

# Sử dụng module pkg-config để tìm thông tin bộ thư viện gtkmm
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

# Danh sách mã nguồn
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/GraphCanvas.cpp
    src/AdvancedAlgorithms.cpp
    src/algorithms.cpp
    src/comparison.cpp
    src/graph.cpp
)

add_executable(app_gui ${SOURCES})

# Cấu hình include và thư viện cho target
target_include_directories(app_gui PUBLIC ${GTKMM_INCLUDE_DIRS})
target_link_directories(app_gui PUBLIC ${GTKMM_LIBRARY_DIRS})
target_compile_options(app_gui PUBLIC ${GTKMM_CFLAGS_OTHER})
target_link_libraries(app_gui PUBLIC ${GTKMM_LIBRARIES})
target_link_options(app_gui PUBLIC ${GTKMM_LDFLAGS_OTHER})
