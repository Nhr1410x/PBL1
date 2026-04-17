#include "../lib/MainWindow.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <limits>
#include <unordered_set>

#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>

#ifdef _WIN32
#include <windows.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  CSS: Light GTK/GNOME Style
// ─────────────────────────────────────────────────────────────────────────────
static const char* APP_CSS = R"css(
* {
    font-family: "Cantarell", "Noto Sans", "Segoe UI", sans-serif;
}

window {
    background-color: #F2F5F9;
}

/* ── Header ── */
#app-header {
    background-color: #FFFFFF;
    border-bottom: 1px solid #D7DEE7;
    padding: 10px 20px;
    min-height: 56px;
}

#app-header-title {
    color: #1E293B;
    font-size: 15px;
    font-weight: bold;
    letter-spacing: 0.4px;
}

/* ── Sidebar ── */
#sidebar {
    background-color: #FFFFFF;
    border-right: 1px solid #D7DEE7;
    padding: 16px 12px;
    min-width: 230px;
}

#sidebar-title {
    color: #334155;
    font-size: 11px;
    font-weight: bold;
    letter-spacing: 1px;
}

/* ── Action Buttons ── */
#btn-action {
    background: #2F6FEB;
    color: #ffffff;
    font-weight: bold;
    font-size: 12px;
    border: 1px solid #275CC2;
    border-radius: 8px;
    padding: 8px 10px;
    min-height: 38px;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-action:hover {
    background: #3B7BFA;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-action:active {
    background: #275CC2;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-action:focus {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

/* ── Visualize Button (highlight đặc biệt) ── */
#btn-visualize {
    background: #1F9D63;
    color: #FFFFFF;
    font-weight: bold;
    font-size: 12px;
    border: 1px solid #198552;
    border-radius: 8px;
    padding: 8px 10px;
    min-height: 38px;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-visualize:hover {
    background: #27B271;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-visualize:active {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-visualize:focus {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

/* ── Exit Button ── */
#btn-exit {
    background: #E45757;
    color: #FFFFFF;
    font-weight: bold;
    font-size: 12px;
    border: 1px solid #C53F3F;
    border-radius: 8px;
    padding: 8px 10px;
    min-height: 36px;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-exit:hover {
    background: #F26A6A;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-exit:active {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-exit:focus {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-action label, #btn-action image,
#btn-visualize label, #btn-visualize image,
#btn-exit label, #btn-exit image {
    color: #FFFFFF;
    text-shadow: none;
    -gtk-icon-shadow: none;
}

#btn-action:hover label, #btn-action:active label, #btn-action:focus label,
#btn-visualize:hover label, #btn-visualize:active label, #btn-visualize:focus label,
#btn-exit:hover label, #btn-exit:active label, #btn-exit:focus label {
    color: #FFFFFF;
}

/* ── Entry Inputs ── */
entry {
    background-color: #FFFFFF;
    color: #0F172A;
    border: 1px solid #C7D2DF;
    border-radius: 7px;
    padding: 4px 8px;
    font-size: 12px;
}

combobox {
    background-color: #FFFFFF;
    color: #0F172A;
    border: 1px solid #C7D2DF;
    border-radius: 7px;
}

entry:focus {
    border: 1px solid #2F6FEB;
}

/* ── Labels ── */
label {
    color: #334155;
    font-size: 12px;
}

#graph-info-label {
    color: #2F6FEB;
    font-weight: bold;
    font-size: 11px;
    letter-spacing: 0.4px;
}

#graph-stats-label {
    color: #0F172A;
    font-size: 11px;
    font-family: "JetBrains Mono", "Cascadia Mono", monospace;
}

/* ── Log TextView ── */
textview {
    background-color: #FFFFFF;
}
textview text {
    background-color: #FFFFFF;
    color: #111827;
    font-family: "JetBrains Mono", "Cascadia Mono", "Fira Code", monospace;
    font-size: 12px;
}

#log-scroll {
    border: 1px solid #D7DEE7;
    border-radius: 10px;
    background-color: #FFFFFF;
}

scrolledwindow {
    border-radius: 8px;
}

/* ── Welcome Page ── */
#welcome-page {
    background-image: none;
    background-color: #FFFFFF;
    border: 1px solid #D7DEE7;
    border-radius: 16px;
    box-shadow: none;
    padding: 72px 64px;
}

#wlc-university {
    color: #1F2937;
    font-size: 18px;
    font-weight: bold;
    letter-spacing: 1px;
}

#wlc-faculty {
    color: #4B5563;
    font-size: 13px;
    letter-spacing: 0.5px;
}

#wlc-project-label {
    color: #2F6FEB;
    font-size: 14px;
    font-weight: bold;
    letter-spacing: 1px;
}

#wlc-project-name {
    color: #1E40AF;
    font-size: 22px;
    font-weight: bold;
}

#wlc-sv-header {
    color: #1F2937;
    font-size: 12px;
    font-weight: bold;
}

#wlc-sv-name {
    color: #0044ff;
    font-size: 14px;
    font-weight: bold;
}

#wlc-gvhd-header {
    color: #1F2937;
    font-size: 12px;
    font-weight: bold;
}

#wlc-gvhd-name {
    color: #00ffaa;
    font-size: 14px;
    font-weight: bold;
}

#wlc-hint {
    color: #64748B;
    font-size: 11px;
    font-style: italic;
}

separator {
    background-color: #D8DEE7;
    min-height: 1px;
    margin: 8px 0;
}

/* ── Content area ── */
#content-area {
    background-color: #F2F5F9;
    padding: 16px;
}

/* ── Load Graph Page ── */
#load-panel {
    background-color: #FFFFFF;
    border: 1px solid #D7DEE7;
    border-radius: 14px;
    padding: 24px;
}

#load-title {
    color: #1F2937;
    font-size: 18px;
    font-weight: bold;
    letter-spacing: 0.4px;
}

#load-subtitle {
    color: #64748B;
    font-size: 12px;
}

#load-hint {
    color: #64748B;
    font-size: 11px;
}

#load-status {
    color: #D14343;
    font-size: 12px;
    font-weight: bold;
}

#inline-prompt {
    background-color: #FFFFFF;
    border: 1px solid #D7DEE7;
    border-radius: 10px;
    padding: 10px 14px;
}

#inline-prompt label {
    color: #0F172A;
    font-size: 12px;
    font-weight: bold;
}

/* ── Loaded Summary Page ── */
#loaded-summary-page {
    background: transparent;
}

#loaded-summary-card {
    background: #FFFFFF;
    border: 1px solid #D7DEE7;
    border-radius: 14px;
    padding: 16px 18px;
    box-shadow: none;
}

#loaded-summary-title {
    color: #1F2937;
    font-size: 22px;
    font-weight: 800;
}

#loaded-summary-status {
    color: #15803D;
    font-size: 13px;
    font-weight: 700;
}

#loaded-summary-source {
    color: #0F172A;
    font-size: 13px;
    font-weight: 600;
}

#loaded-summary-meta {
    color: #334155;
    font-size: 13px;
}

#loaded-summary-preview-box {
    background: #F8FAFC;
    border: 1px solid #DDE4EE;
    border-radius: 10px;
    padding: 10px 12px;
    box-shadow: none;
}

#loaded-summary-preview-title {
    color: #1D4ED8;
    font-size: 12px;
    font-weight: 700;
}

#loaded-summary-edge-preview {
    color: #1F2937;
    font-size: 12px;
    font-family: "JetBrains Mono", "Cascadia Mono", "Fira Code", monospace;
}

#btn-loaded-run {
    background: #1D4ED8;
    color: #FFFFFF;
    border: 1px solid #1E40AF;
    border-radius: 9px;
    font-weight: 700;
    padding: 8px 14px;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-loaded-run:hover {
    background: #2563EB;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-loaded-run:active {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-loaded-run:focus {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-loaded-visualize {
    background: #0F766E;
    color: #FFFFFF;
    border: 1px solid #115E59;
    border-radius: 9px;
    font-weight: 700;
    padding: 8px 14px;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-loaded-visualize:hover {
    background: #0D9488;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-loaded-visualize:active {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-loaded-visualize:focus {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-loaded-run label, #btn-loaded-run image,
#btn-loaded-visualize label, #btn-loaded-visualize image {
    color: #FFFFFF;
    text-shadow: none;
    -gtk-icon-shadow: none;
}

#btn-loaded-run:hover label, #btn-loaded-run:active label, #btn-loaded-run:focus label,
#btn-loaded-visualize:hover label, #btn-loaded-visualize:active label, #btn-loaded-visualize:focus label {
    color: #FFFFFF;
}

/* ── Visualizer Page ── */
#visualizer-page {
    background: #FFFFFF;
    padding: 2px;
}

frame#visualizer-canvas-frame {
    background: #FFFFFF;
    border: 1px solid #D7DEE7;
    border-radius: 12px;
    padding: 8px;
}

#visualizer-toolbar {
    background: #FFFFFF;
    border: 1px solid #D7DEE7;
    border-radius: 12px;
    padding: 10px 12px;
}

#btn-viz-play {
    background: #2563EB;
    color: #FFFFFF;
    border: 1px solid #1D4ED8;
    border-radius: 9px;
    font-weight: 700;
    padding: 8px 14px;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-viz-play:hover {
    background: #2F6FEB;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-viz-pause,
#btn-viz-reset {
    background: #334155;
    color: #FFFFFF;
    border: 1px solid #1F2937;
    border-radius: 9px;
    font-weight: 700;
    padding: 8px 14px;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-viz-pause:hover,
#btn-viz-reset:hover {
    background: #475569;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-viz-back {
    background: #E45757;
    color: #FFFFFF;
    border: 1px solid #C53F3F;
    border-radius: 9px;
    font-weight: 700;
    padding: 8px 14px;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-viz-back:hover {
    background: #F26A6A;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-viz-play:active, #btn-viz-play:focus,
#btn-viz-pause:active, #btn-viz-pause:focus,
#btn-viz-reset:active, #btn-viz-reset:focus,
#btn-viz-back:active, #btn-viz-back:focus {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-viz-play label, #btn-viz-play image,
#btn-viz-pause label, #btn-viz-pause image,
#btn-viz-reset label, #btn-viz-reset image,
#btn-viz-back label, #btn-viz-back image {
    color: #FFFFFF;
    text-shadow: none;
    -gtk-icon-shadow: none;
}

/* ── Simulation Module ── */
#sim-page {
    padding: 2px;
}

#sim-control-frame,
#sim-status-frame,
#sim-result-frame,
#sim-log-frame {
    background-color: #FFFFFF;
    border: 1px solid #D7DEE7;
    border-radius: 12px;
}

#sim-control-row {
    padding: 10px;
}

#sim-panel-title {
    color: #0F172A;
    font-size: 16px;
    font-weight: bold;  
}

#sim-result-title {
    color: #0B1F5E;
    font-size: 18px;
    font-weight: 700;
}

#btn-run-primary {
    background: #2563EB;
    color: #FFFFFF;
    font-weight: bold;
    border: 1px solid #1D4ED8;
    border-radius: 9px;
    padding: 8px 14px;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-run-primary:hover {
    background: #2F6FEB;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-run-primary:active,
#btn-run-primary:focus {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-run-step,
#btn-log-clear {
    background: #334155;
    color: #FFFFFF;
    border: 1px solid #1F2937;
    border-radius: 9px;
    padding: 8px 14px;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-run-step:hover,
#btn-log-clear:hover {
    background: #475569;
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-run-step:active,
#btn-log-clear:active,
#btn-run-step:focus,
#btn-log-clear:focus {
    box-shadow: none;
    text-shadow: none;
    -gtk-icon-shadow: none;
    outline: none;
}

#btn-run-primary label, #btn-run-primary image,
#btn-run-step label, #btn-run-step image,
#btn-log-clear label, #btn-log-clear image {
    color: #FFFFFF;
    text-shadow: none;
    -gtk-icon-shadow: none;
}

#btn-run-primary:hover label, #btn-run-primary:active label, #btn-run-primary:focus label,
#btn-run-step:hover label, #btn-run-step:active label, #btn-run-step:focus label,
#btn-log-clear:hover label, #btn-log-clear:active label, #btn-log-clear:focus label {
    color: #FFFFFF;
}

#btn-settings-icon {
    background: #FFFFFF;
    border: 1px solid #CBD5E1;
    border-radius: 9px;
    min-width: 36px;
}

#btn-settings-icon:hover {
    background: #F3F6FB;
}

treeview {
    background-color: #FFFFFF;
    color: #111827;
    border-radius: 8px;
}

treeview.view header button {
    background: #F4F7FC;
    color: #1F2937;
    border: 1px solid #E2E8F0;
    font-weight: 600;
}

treeview.view:selected,
treeview.view:selected:focus {
    background-color: #EEF4FF;
    color: #111827;
}

#result-key {
    color: #475569;
    font-size: 12px;
    font-weight: 600;
}

#result-value {
    color: #0F172A;
    font-size: 13px;
}

#result-value-strong {
    color: #0B1F5E;
    font-size: 13px;
    font-weight: 700;
}
)css";

namespace {
std::string trimCopy(const std::string& text) {
    const auto begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }
    const auto end = text.find_last_not_of(" \t\r\n");
    return text.substr(begin, end - begin + 1);
}

bool containsNoCase(const std::string& text, const std::string& needle) {
    if (needle.empty()) {
        return true;
    }

    std::string lhs = text;
    std::string rhs = needle;
    std::transform(lhs.begin(), lhs.end(), lhs.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    std::transform(rhs.begin(), rhs.end(), rhs.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });

    return lhs.find(rhs) != std::string::npos;
}

std::string statusMarkup(const std::string& status) {
    if (status == "Đã chốt") {
        return "<span foreground='#166534'><b>Đã chốt</b></span>";
    }
    if (status == "Chờ xử lý") {
        return "<span foreground='#C56A1A'><b>Chờ xử lý</b></span>";
    }
    if (status == "Đang xét") {
        return "<span foreground='#1D4ED8'><b>Đang xét</b></span>";
    }
    return "<span foreground='#d9d9d9'>Chưa tới</span>";
}
} // namespace

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(Graph& g, Algorithms* alg, AdvancedAlgorithms* adv, Comparison* comp)
    : graph(g), algorithms(alg), advancedAlgorithms(adv), comparison(comp),
      m_Root(Gtk::ORIENTATION_VERTICAL, 0),
      m_Header(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_Body(Gtk::ORIENTATION_HORIZONTAL, 0),
      m_Sidebar(Gtk::ORIENTATION_VERTICAL, 6),
      m_Input_Row(Gtk::ORIENTATION_HORIZONTAL, 6),
      m_GraphInfo_Box(Gtk::ORIENTATION_VERTICAL, 4),
      m_Button_LoadGraph(" Tải Đồ Thị"),
      m_Button_Dijkstra("▶  Chạy Dijkstra"),
      m_Button_Bellman("▶  Chạy Bellman-Ford"),
      m_Button_Advanced("▶  Chạy BMSSP 2025"),
      m_Button_Compare("  So Sánh Thuật Toán"),
      m_Button_Visualize("  Trực Quan Hóa"),
      m_Button_Exit("✕  Thoát"),
      m_Page_Welcome(Gtk::ORIENTATION_VERTICAL, 0),
      m_Page_LoadGraph(Gtk::ORIENTATION_VERTICAL, 0),
      m_Load_Panel(Gtk::ORIENTATION_VERTICAL, 14),
      m_Load_MethodRow(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_Load_Mode_File("Từ File"),
      m_Load_Mode_Direct("Nhập Trực Tiếp"),
      m_Load_FileForm(Gtk::ORIENTATION_VERTICAL, 10),
      m_Load_FileDirected("Đồ thị CÓ HƯỚNG"),
      m_Load_FilePathRow(Gtk::ORIENTATION_HORIZONTAL, 8),
      m_Load_FileBrowse("Chọn File"),
      m_Load_DirectForm(Gtk::ORIENTATION_VERTICAL, 10),
      m_Load_DirectDirected("Đồ thị CÓ HƯỚNG"),
      m_Load_VertexRow(Gtk::ORIENTATION_HORIZONTAL, 8),
      m_Load_VertexLabel("Số đỉnh (V):"),
      m_Load_ActionRow(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_Load_Confirm("Nạp Đồ Thị"),
      m_Load_Reset("Khôi Phục Mẫu"),
      m_Page_Main(Gtk::ORIENTATION_VERTICAL, 0),
      m_Main_InlinePrompt(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_Main_InlinePrompt_Visualize("Trực Quan Hóa Ngay"),
      m_Main_InlinePrompt_Dismiss("Để Sau"),
      m_Page_LoadedSummary(Gtk::ORIENTATION_VERTICAL, 0),
      m_LoadedSummaryCard(Gtk::ORIENTATION_VERTICAL, 10),
      m_LoadedSummaryPreviewBox(Gtk::ORIENTATION_VERTICAL, 6),
      m_LoadedSummaryActionRow(Gtk::ORIENTATION_HORIZONTAL, 10),
      m_LoadedSummaryRun("Chạy thuật toán"),
      m_LoadedSummaryVisualize("Trực quan hóa ngay"),
      m_Page_Simulation(Gtk::ORIENTATION_VERTICAL, 10),
      m_Sim_ControlRow(Gtk::ORIENTATION_HORIZONTAL, 8),
      m_Sim_AlgoLabel("Thuật toán"),
      m_Sim_RunButton("Chạy"),
      m_Sim_StepButton("Chạy từng bước"),
      m_Sim_ClearLogButton("Xóa log"),
      m_Sim_SettingsButton("⚙"),
      m_Sim_TopSplit(Gtk::ORIENTATION_HORIZONTAL),
      m_Sim_StatusBox(Gtk::ORIENTATION_VERTICAL, 8),
      m_Sim_StatusTitle("Trạng Thái Đỉnh"),
      m_Sim_ResultBox(Gtk::ORIENTATION_VERTICAL, 8),
      m_Sim_ResultTitle("Kết Quả Cuối Cùng"),
      m_Sim_LogBox(Gtk::ORIENTATION_VERTICAL, 8),
      m_Sim_LogTitle("Log Thuật Toán"),
      m_lastAlgoType(AlgorithmType::DIJKSTRA),
      m_lastEndVertex(-1),
      m_lastElapsedUs(0),
      m_lastRunStepMode(false),
      m_welcomeShown(true),
      m_isDirected(true)
{
    set_title("Đồ Án Tìm Đường Đi Ngắn Nhất – PBL1");
    set_default_size(1200, 780);
    set_border_width(0);

    setupCss();

    // ── Header Bar ──────────────────────────────────────────────────────────
    m_Header.set_name("app-header");
    m_Header_Title.set_markup(
        "<b>CHƯƠNG TRÌNH TÌM ĐƯỜNG ĐI NGẮN NHẤT</b>   "
        "<span foreground='#546E7A' size='small'>Dijkstra, Bellman-Ford &amp; BMSSP 2025</span>"
    );
    m_Header_Title.set_name("app-header-title");
    m_Header_Title.set_halign(Gtk::ALIGN_START);
    m_Header.pack_start(m_Header_Title, true, true, 0);
    m_Root.pack_start(m_Header, false, false, 0);

    // ── Sidebar ─────────────────────────────────────────────────────────────
    m_Sidebar.set_name("sidebar");
    m_Sidebar.set_border_width(0);

    m_Sidebar_Title.set_label("CHỨC NĂNG");
    m_Sidebar_Title.set_name("sidebar-title");
    m_Sidebar_Title.set_halign(Gtk::ALIGN_START);
    m_Sidebar.pack_start(m_Sidebar_Title, false, false, 4);

    m_Sep1.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    m_Sidebar.pack_start(m_Sep1, false, false, 2);

    // Buttons
    m_Button_LoadGraph.set_name("btn-action");
    m_Button_LoadGraph.set_halign(Gtk::ALIGN_FILL);
    m_Sidebar.pack_start(m_Button_LoadGraph, false, false, 2);

    // Input row: Start + End vertex
    m_Label_Start.set_label("Từ:");
    m_Label_End.set_label("Đến:");
    m_Entry_Start.set_text("1");
    m_Entry_Start.set_width_chars(4);
    m_Entry_Start.set_max_width_chars(5);
    m_Entry_End.set_text("1");
    m_Entry_End.set_width_chars(4);
    m_Entry_End.set_max_width_chars(5);
    m_Input_Row.pack_start(m_Label_Start, false, false, 0);
    m_Input_Row.pack_start(m_Entry_Start, true, true, 0);
    m_Input_Row.pack_start(m_Label_End, false, false, 0);
    m_Input_Row.pack_start(m_Entry_End, true, true, 0);
    m_Sidebar.pack_start(m_Input_Row, false, false, 2);

    m_Button_Dijkstra.set_name("btn-action");
    m_Sidebar.pack_start(m_Button_Dijkstra, false, false, 2);

    m_Button_Bellman.set_name("btn-action");
    m_Sidebar.pack_start(m_Button_Bellman, false, false, 2);

    m_Button_Advanced.set_name("btn-action");
    m_Sidebar.pack_start(m_Button_Advanced, false, false, 2);

    m_Button_Compare.set_name("btn-action");
    m_Sidebar.pack_start(m_Button_Compare, false, false, 2);

    m_Button_Visualize.set_name("btn-visualize");
    m_Sidebar.pack_start(m_Button_Visualize, false, false, 2);

    m_Sep2.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    m_Sidebar.pack_start(m_Sep2, false, false, 4);

    // Graph info box
    m_Label_GraphInfo.set_label("THÔNG TIN ĐỒ THỊ");
    m_Label_GraphInfo.set_name("graph-info-label");
    m_Label_GraphInfo.set_halign(Gtk::ALIGN_START);
    m_Label_GraphStats.set_label("Chưa tải đồ thị.");
    m_Label_GraphStats.set_name("graph-stats-label");
    m_Label_GraphStats.set_halign(Gtk::ALIGN_START);
    m_Label_GraphStats.set_line_wrap(true);
    m_GraphInfo_Box.pack_start(m_Label_GraphInfo, false, false, 0);
    m_GraphInfo_Box.pack_start(m_Label_GraphStats, false, false, 0);
    m_Sidebar.pack_start(m_GraphInfo_Box, false, false, 4);

    // Spacer
    Gtk::Box* spacer = Gtk::manage(new Gtk::Box());
    m_Sidebar.pack_start(*spacer, true, true, 0);

    m_Button_Exit.set_name("btn-exit");
    m_Sidebar.pack_end(m_Button_Exit, false, false, 2);

    // ── Stack: Welcome + Load + Main ────────────────────────────────────────
    buildWelcomePage();
    buildLoadGraphPage();
    buildMainPage();
    buildVisualizerPage();

    m_Stack.add(m_Page_Welcome, "welcome");
    m_Stack.add(m_Page_LoadGraph, "load_graph");
    m_Stack.add(m_Page_Main, "main");
    m_Stack.add(m_Page_Visualizer, "visualizer");
    m_Stack.set_visible_child("welcome");
    m_Stack.set_transition_type(Gtk::STACK_TRANSITION_TYPE_SLIDE_LEFT);
    m_Stack.set_transition_duration(350);

    // Content area wrapper
    Gtk::Box* content_wrap = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 0));
    content_wrap->set_name("content-area");
    content_wrap->pack_start(m_Stack, true, true, 0);

    // Body = Sidebar + Content
    m_Body.pack_start(m_Sidebar, false, false, 0);
    m_Body.pack_start(*content_wrap, true, true, 0);
    m_Root.pack_start(m_Body, true, true, 0);

    add(m_Root);

    // ── Signals ─────────────────────────────────────────────────────────────
    m_Button_LoadGraph.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_load_graph));
    m_Load_Mode_File.signal_toggled().connect(
        sigc::mem_fun(*this, &MainWindow::updateLoadGraphInputMode));
    m_Load_Mode_Direct.signal_toggled().connect(
        sigc::mem_fun(*this, &MainWindow::updateLoadGraphInputMode));
    m_Load_FileBrowse.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_browse_graph_file));
    m_Load_Confirm.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_submit_load_graph));
    m_Load_Reset.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_reset_load_graph_form));
    m_Button_Dijkstra.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_run_dijkstra));
    m_Button_Bellman.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_run_bellman));
    m_Button_Advanced.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_run_advanced));
    m_Button_Compare.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_compare));
    m_Button_Visualize.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_visualize));
    m_Main_InlinePrompt_Visualize.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_visualize_now_inline));
    m_Main_InlinePrompt_Dismiss.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_visualize_later_inline));
    m_LoadedSummaryRun.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_loaded_summary_run));
    m_LoadedSummaryVisualize.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_loaded_summary_visualize));
    m_Sim_RunButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_toolbar_run_clicked));
    m_Sim_StepButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_toolbar_step_clicked));
    m_Sim_ClearLogButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_toolbar_clear_log_clicked));
    m_Sim_LogLevelCombo.signal_changed().connect(
        sigc::mem_fun(*this, &MainWindow::on_toolbar_log_level_changed));
    m_Button_Exit.signal_clicked().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_exit));

    show_all_children();
}

MainWindow::~MainWindow() {}

// ─────────────────────────────────────────────────────────────────────────────
//  CSS Setup
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupCss() {
    m_CssProvider = Gtk::CssProvider::create();
    try {
        m_CssProvider->load_from_data(APP_CSS);
    } catch (const Glib::Error& ex) {
        std::cerr << "[CSS Error] " << ex.what() << std::endl;
    }
    auto screen = Gdk::Screen::get_default();
    Gtk::StyleContext::add_provider_for_screen(
        screen, m_CssProvider,
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

// ─────────────────────────────────────────────────────────────────────────────
//  Load Graph Page
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::buildLoadGraphPage() {
    m_Page_LoadGraph.set_border_width(0);
    m_Page_LoadGraph.set_halign(Gtk::ALIGN_FILL);
    m_Page_LoadGraph.set_valign(Gtk::ALIGN_FILL);

    m_Load_Panel.set_name("load-panel");
    m_Load_Panel.set_halign(Gtk::ALIGN_CENTER);
    m_Load_Panel.set_valign(Gtk::ALIGN_START);
    m_Load_Panel.set_size_request(760, -1);
    m_Load_Panel.set_margin_top(16);
    m_Load_Panel.set_margin_bottom(16);

    m_Load_Title.set_markup("<b>NẠP ĐỒ THỊ</b>");
    m_Load_Title.set_name("load-title");
    m_Load_Title.set_halign(Gtk::ALIGN_START);

    m_Load_Subtitle.set_markup(
        "<span>Chọn phương thức nhập liệu ngay trên panel này để tránh popup chồng lớp.</span>"
    );
    m_Load_Subtitle.set_name("load-subtitle");
    m_Load_Subtitle.set_halign(Gtk::ALIGN_START);
    m_Load_Subtitle.set_line_wrap(true);

    Gtk::RadioButton::Group modeGroup = m_Load_Mode_File.get_group();
    m_Load_Mode_Direct.set_group(modeGroup);
    m_Load_Mode_File.set_active(true);

    m_Load_MethodRow.pack_start(m_Load_Mode_File, false, false, 0);
    m_Load_MethodRow.pack_start(m_Load_Mode_Direct, false, false, 0);

    m_Load_FormStack.set_transition_type(Gtk::STACK_TRANSITION_TYPE_CROSSFADE);
    m_Load_FormStack.set_transition_duration(200);

    m_Load_FileDirected.set_active(true);

    m_Load_FileHint.set_markup(
        "<span>Nhập đường dẫn file `.txt` hoặc dùng nút chọn file.</span>"
    );
    m_Load_FileHint.set_name("load-hint");
    m_Load_FileHint.set_halign(Gtk::ALIGN_START);

    m_Load_FilePath.set_placeholder_text("Ví dụ: data\\G1.txt");
    m_Load_FilePath.set_hexpand(true);
    m_Load_FilePath.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_submit_load_graph)
    );

    m_Load_FileBrowse.set_name("btn-action");
    m_Load_FilePathRow.pack_start(m_Load_FilePath, true, true, 0);
    m_Load_FilePathRow.pack_start(m_Load_FileBrowse, false, false, 0);

    m_Load_FileForm.pack_start(m_Load_FileDirected, false, false, 0);
    m_Load_FileForm.pack_start(m_Load_FileHint, false, false, 0);
    m_Load_FileForm.pack_start(m_Load_FilePathRow, false, false, 0);

    m_Load_DirectDirected.set_active(true);

    m_Load_VertexEntry.set_placeholder_text("Ví dụ: 5");
    m_Load_VertexEntry.set_input_purpose(Gtk::INPUT_PURPOSE_DIGITS);
    m_Load_VertexEntry.set_activates_default(true);
    m_Load_VertexEntry.signal_activate().connect(
        sigc::mem_fun(*this, &MainWindow::on_button_submit_load_graph)
    );
    m_Load_VertexRow.pack_start(m_Load_VertexLabel, false, false, 0);
    m_Load_VertexRow.pack_start(m_Load_VertexEntry, true, true, 0);

    m_Load_EdgesHint.set_markup(
        "<span>Danh sách cạnh, mỗi dòng đúng 3 giá trị: <b>u v w</b>. Đỉnh bắt đầu từ 1.</span>"
    );
    m_Load_EdgesHint.set_name("load-hint");
    m_Load_EdgesHint.set_halign(Gtk::ALIGN_START);
    m_Load_EdgesHint.set_line_wrap(true);

    m_Load_EdgesBuffer = Gtk::TextBuffer::create();
    m_Load_EdgesTextView.set_buffer(m_Load_EdgesBuffer);
    m_Load_EdgesTextView.set_monospace(true);
    m_Load_EdgesTextView.set_wrap_mode(Gtk::WRAP_NONE);
    m_Load_EdgesScroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_Load_EdgesScroll.set_min_content_height(260);
    m_Load_EdgesScroll.set_name("log-scroll");
    m_Load_EdgesScroll.add(m_Load_EdgesTextView);

    m_Load_DirectForm.pack_start(m_Load_DirectDirected, false, false, 0);
    m_Load_DirectForm.pack_start(m_Load_VertexRow, false, false, 0);
    m_Load_DirectForm.pack_start(m_Load_EdgesHint, false, false, 0);
    m_Load_DirectForm.pack_start(m_Load_EdgesScroll, true, true, 0);

    m_Load_FormStack.add(m_Load_FileForm, "file");
    m_Load_FormStack.add(m_Load_DirectForm, "direct");

    m_Load_Status.set_name("load-status");
    m_Load_Status.set_halign(Gtk::ALIGN_START);
    m_Load_Status.set_line_wrap(true);
    m_Load_Status.set_xalign(0.0f);

    m_Load_Confirm.set_name("btn-action");
    m_Load_Confirm.set_can_default(true);
    m_Load_Reset.set_name("btn-action");
    m_Load_ActionRow.pack_start(m_Load_Confirm, false, false, 0);
    m_Load_ActionRow.pack_start(m_Load_Reset, false, false, 0);

    m_Load_Panel.pack_start(m_Load_Title, false, false, 0);
    m_Load_Panel.pack_start(m_Load_Subtitle, false, false, 0);
    m_Load_Panel.pack_start(m_Load_MethodRow, false, false, 6);
    m_Load_Panel.pack_start(m_Load_FormStack, true, true, 0);
    m_Load_Panel.pack_start(m_Load_Status, false, false, 0);
    m_Load_Panel.pack_start(m_Load_ActionRow, false, false, 0);

    m_Page_LoadGraph.pack_start(m_Load_Panel, false, false, 0);

    resetLoadGraphForm();
    updateLoadGraphInputMode();
    clearLoadGraphStatus();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Welcome Page
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::buildWelcomePage() {
    m_Page_Welcome.set_name("welcome-page");
    m_Page_Welcome.set_halign(Gtk::ALIGN_FILL);
    m_Page_Welcome.set_valign(Gtk::ALIGN_FILL);
    m_Page_Welcome.set_hexpand(true);
    m_Page_Welcome.set_vexpand(true);
    m_Page_Welcome.set_spacing(20);
    m_Page_Welcome.set_margin_top(12);
    m_Page_Welcome.set_margin_bottom(12);
    m_Page_Welcome.set_margin_left(12);
    m_Page_Welcome.set_margin_right(12);

    m_Wlc_University.set_markup(
        "<b>TRƯỜNG ĐẠI HỌC BÁCH KHOA – ĐẠI HỌC ĐÀ NẴNG</b>"
    );
    m_Wlc_University.set_name("wlc-university");
    m_Wlc_University.set_justify(Gtk::JUSTIFY_CENTER);
    m_Page_Welcome.pack_start(m_Wlc_University, false, false, 0);

    m_Wlc_Faculty.set_label("KHOA CÔNG NGHỆ THÔNG TIN");
    m_Wlc_Faculty.set_name("wlc-faculty");
    m_Wlc_Faculty.set_justify(Gtk::JUSTIFY_CENTER);
    m_Page_Welcome.pack_start(m_Wlc_Faculty, false, false, 0);

    m_Wlc_Sep.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    m_Page_Welcome.pack_start(m_Wlc_Sep, false, false, 8);

    m_Wlc_ProjectLabel.set_markup(
        "<span foreground='#e8b200' font='12' weight='bold' letter_spacing='2000'>"
        "PBL1 : ĐỒ ÁN LẬP TRÌNH TÍNH TOÁN</span>"
    );
    m_Wlc_ProjectLabel.set_justify(Gtk::JUSTIFY_CENTER);
    m_Page_Welcome.pack_start(m_Wlc_ProjectLabel, false, false, 0);

    m_Wlc_ProjectName.set_markup(
        "<span foreground='#141414' font='20' weight='bold'>"
        "THUẬT TOÁN TÌM ĐƯỜNG ĐI NGẮN NHẤT</span>"
    );
    m_Wlc_ProjectName.set_justify(Gtk::JUSTIFY_CENTER);
    m_Page_Welcome.pack_start(m_Wlc_ProjectName, false, false, 0);

    m_Wlc_Sep2.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    m_Page_Welcome.pack_start(m_Wlc_Sep2, false, false, 8);

    m_Wlc_InfoGrid.set_row_spacing(8);
    m_Wlc_InfoGrid.set_column_spacing(40);
    m_Wlc_InfoGrid.set_halign(Gtk::ALIGN_CENTER);

    m_Wlc_SV_Label.set_markup("<b>Tên Sinh Viên</b>");
    m_Wlc_SV_Label.set_name("wlc-sv-header");
    m_Wlc_SV_Label.set_halign(Gtk::ALIGN_START);

    m_Wlc_GVHD_Label.set_markup("<b>Giảng Viên Hướng Dẫn</b>");
    m_Wlc_GVHD_Label.set_name("wlc-gvhd-header");
    m_Wlc_GVHD_Label.set_halign(Gtk::ALIGN_START);

    m_Wlc_SV_Name1.set_markup(
        "<span foreground='#0000ad' font='13' weight='bold'>Nguyễn Hữu Rin</span>"
    );
    m_Wlc_SV_Name1.set_halign(Gtk::ALIGN_START);

    m_Wlc_SV_Name2.set_markup(
        "<span foreground='#0000ad' font='13' weight='bold'>Huỳnh Nguyễn Hồng Nhi</span>"
    );
    m_Wlc_SV_Name2.set_halign(Gtk::ALIGN_START);

    m_Wlc_GVHD_Name.set_markup(
        "<span foreground='#1e890d' font='13' weight='bold'>Nguyễn Văn Hiệu</span>"
    );
    m_Wlc_GVHD_Name.set_halign(Gtk::ALIGN_START);
    m_Wlc_GVHD_Name.set_valign(Gtk::ALIGN_CENTER);

    m_Wlc_InfoGrid.attach(m_Wlc_SV_Label,   0, 0, 1, 1);
    m_Wlc_InfoGrid.attach(m_Wlc_GVHD_Label, 1, 0, 1, 1);
    m_Wlc_InfoGrid.attach(m_Wlc_SV_Name1,   0, 1, 1, 1);
    m_Wlc_InfoGrid.attach(m_Wlc_GVHD_Name,  1, 1, 1, 2);
    m_Wlc_InfoGrid.attach(m_Wlc_SV_Name2,   0, 2, 1, 1);

    m_Page_Welcome.pack_start(m_Wlc_InfoGrid, false, false, 0);

    m_Wlc_Hint.set_markup(
        "<i><span foreground='#777777' font='10'>"
        "Chọn chức năng bên trái để bắt đầu...</span></i>"
    );
    m_Wlc_Hint.set_justify(Gtk::JUSTIFY_CENTER);
    m_Page_Welcome.pack_start(m_Wlc_Hint, false, false, 20);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Main (Log) Page
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::buildMainPage() {
    m_Page_Main.set_border_width(0);
    m_Page_Main.set_spacing(10);

    m_Main_InlinePrompt_Revealer.set_transition_type(Gtk::REVEALER_TRANSITION_TYPE_SLIDE_DOWN);
    m_Main_InlinePrompt_Revealer.set_transition_duration(220);
    m_Main_InlinePrompt_Revealer.set_reveal_child(false);

    m_Main_InlinePrompt.set_name("inline-prompt");
    m_Main_InlinePrompt_Label.set_markup(
        "<b>Nạp đồ thị thành công.</b> Bạn có muốn trực quan hóa ngay bây giờ không?"
    );
    m_Main_InlinePrompt_Label.set_halign(Gtk::ALIGN_START);
    m_Main_InlinePrompt_Label.set_line_wrap(true);
    m_Main_InlinePrompt_Label.set_xalign(0.0f);

    m_Main_InlinePrompt_Visualize.set_name("btn-visualize");
    m_Main_InlinePrompt_Dismiss.set_name("btn-action");

    Gtk::Box* promptSpacer = Gtk::manage(new Gtk::Box());
    m_Main_InlinePrompt.pack_start(m_Main_InlinePrompt_Label, true, true, 0);
    m_Main_InlinePrompt.pack_start(*promptSpacer, true, true, 0);
    m_Main_InlinePrompt.pack_start(m_Main_InlinePrompt_Visualize, false, false, 0);
    m_Main_InlinePrompt.pack_start(m_Main_InlinePrompt_Dismiss, false, false, 0);
    m_Main_InlinePrompt_Revealer.add(m_Main_InlinePrompt);

    // ── Loaded summary page ──────────────────────────────────────────────────
    m_Page_LoadedSummary.set_name("loaded-summary-page");
    m_Page_LoadedSummary.set_spacing(10);
    m_Page_LoadedSummary.set_margin_top(16);
    m_Page_LoadedSummary.set_margin_bottom(16);
    m_Page_LoadedSummary.set_margin_left(16);
    m_Page_LoadedSummary.set_margin_right(16);
    m_Page_LoadedSummary.set_halign(Gtk::ALIGN_FILL);
    m_Page_LoadedSummary.set_valign(Gtk::ALIGN_FILL);

    m_LoadedSummaryCard.set_name("loaded-summary-card");
    m_LoadedSummaryCard.set_spacing(12);
    m_LoadedSummaryCard.set_halign(Gtk::ALIGN_FILL);
    m_LoadedSummaryCard.set_valign(Gtk::ALIGN_START);
    m_LoadedSummaryCard.set_size_request(820, -1);

    m_LoadedSummaryTitle.set_name("loaded-summary-title");
    m_LoadedSummaryTitle.set_text("Đã nạp đồ thị thành công");
    m_LoadedSummaryTitle.set_halign(Gtk::ALIGN_START);
    m_LoadedSummaryTitle.set_xalign(0.0f);
    m_LoadedSummaryTitle.set_line_wrap(true);

    m_LoadedSummaryStatus.set_name("loaded-summary-status");
    m_LoadedSummaryStatus.set_text("● Trạng thái: Thành công");
    m_LoadedSummaryStatus.set_halign(Gtk::ALIGN_START);
    m_LoadedSummaryStatus.set_xalign(0.0f);
    m_LoadedSummaryStatus.set_line_wrap(true);

    m_LoadedSummarySource.set_name("loaded-summary-source");
    m_LoadedSummarySource.set_halign(Gtk::ALIGN_START);
    m_LoadedSummarySource.set_xalign(0.0f);
    m_LoadedSummarySource.set_line_wrap(true);

    m_LoadedSummaryMeta.set_name("loaded-summary-meta");
    m_LoadedSummaryMeta.set_halign(Gtk::ALIGN_START);
    m_LoadedSummaryMeta.set_xalign(0.0f);
    m_LoadedSummaryMeta.set_line_wrap(true);

    m_LoadedSummaryPreviewBox.set_name("loaded-summary-preview-box");
    m_LoadedSummaryPreviewTitle.set_name("loaded-summary-preview-title");
    m_LoadedSummaryPreviewTitle.set_text("Xem nhanh cạnh đầu tiên");
    m_LoadedSummaryPreviewTitle.set_halign(Gtk::ALIGN_START);
    m_LoadedSummaryPreviewTitle.set_xalign(0.0f);

    m_LoadedSummaryEdgePreview.set_name("loaded-summary-edge-preview");
    m_LoadedSummaryEdgePreview.set_halign(Gtk::ALIGN_START);
    m_LoadedSummaryEdgePreview.set_xalign(0.0f);
    m_LoadedSummaryEdgePreview.set_line_wrap(true);

    m_LoadedSummaryRun.set_name("btn-loaded-run");
    m_LoadedSummaryVisualize.set_name("btn-loaded-visualize");
    m_LoadedSummaryActionRow.set_halign(Gtk::ALIGN_START);
    m_LoadedSummaryActionRow.pack_start(m_LoadedSummaryRun, false, false, 0);
    m_LoadedSummaryActionRow.pack_start(m_LoadedSummaryVisualize, false, false, 0);

    m_LoadedSummaryPreviewBox.pack_start(m_LoadedSummaryPreviewTitle, false, false, 0);
    m_LoadedSummaryPreviewBox.pack_start(m_LoadedSummaryEdgePreview, false, false, 0);

    m_LoadedSummaryCard.pack_start(m_LoadedSummaryTitle, false, false, 0);
    m_LoadedSummaryCard.pack_start(m_LoadedSummaryStatus, false, false, 0);
    m_LoadedSummaryCard.pack_start(m_LoadedSummarySource, false, false, 0);
    m_LoadedSummaryCard.pack_start(m_LoadedSummaryMeta, false, false, 0);
    m_LoadedSummaryCard.pack_start(m_LoadedSummaryPreviewBox, false, false, 0);
    m_LoadedSummaryCard.pack_start(m_LoadedSummaryActionRow, false, false, 0);

    Gtk::Box* summaryWrap = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 0));
    summaryWrap->set_halign(Gtk::ALIGN_CENTER);
    summaryWrap->set_valign(Gtk::ALIGN_START);
    summaryWrap->set_hexpand(true);
    summaryWrap->pack_start(m_LoadedSummaryCard, false, false, 0);

    Gtk::Box* summarySpacer = Gtk::manage(new Gtk::Box());
    m_Page_LoadedSummary.pack_start(*summaryWrap, false, false, 0);
    m_Page_LoadedSummary.pack_start(*summarySpacer, true, true, 0);

    // ── Simulation module page ───────────────────────────────────────────────
    m_Page_Simulation.set_name("sim-page");
    m_Page_Simulation.set_spacing(10);
    m_Page_Simulation.set_halign(Gtk::ALIGN_FILL);
    m_Page_Simulation.set_valign(Gtk::ALIGN_FILL);

    // Control bar
    m_Sim_ControlFrame.set_name("sim-control-frame");
    m_Sim_ControlRow.set_name("sim-control-row");
    m_Sim_ControlRow.set_halign(Gtk::ALIGN_FILL);
    m_Sim_ControlRow.set_valign(Gtk::ALIGN_CENTER);

    m_Sim_AlgoLabel.set_xalign(0.0f);
    m_Sim_AlgoCombo.append("Dijkstra");
    m_Sim_AlgoCombo.append("Bellman-Ford");
    m_Sim_AlgoCombo.append("BMSSP - 2025");
    m_Sim_AlgoCombo.set_active(0);

    m_Sim_RunButton.set_name("btn-run-primary");
    m_Sim_StepButton.set_name("btn-run-step");
    m_Sim_ClearLogButton.set_name("btn-log-clear");
    m_Sim_SettingsButton.set_name("btn-settings-icon");
    m_Sim_SettingsButton.set_tooltip_text("Thiết lập giao diện mô phỏng");

    m_Sim_LogLevelCombo.append("Ngắn");
    m_Sim_LogLevelCombo.append("Vừa");
    m_Sim_LogLevelCombo.append("Chi tiết");
    m_Sim_LogLevelCombo.set_active(1);

    Gtk::Label* logLevelLabel = Gtk::manage(new Gtk::Label("Mức log"));
    logLevelLabel->set_xalign(0.0f);

    Gtk::Box* controlSpacer = Gtk::manage(new Gtk::Box());
    m_Sim_ControlRow.pack_start(m_Sim_AlgoLabel, false, false, 0);
    m_Sim_ControlRow.pack_start(m_Sim_AlgoCombo, false, false, 0);
    m_Sim_ControlRow.pack_start(m_Sim_RunButton, false, false, 0);
    m_Sim_ControlRow.pack_start(m_Sim_StepButton, false, false, 0);
    m_Sim_ControlRow.pack_start(m_Sim_ClearLogButton, false, false, 0);
    m_Sim_ControlRow.pack_start(*logLevelLabel, false, false, 6);
    m_Sim_ControlRow.pack_start(m_Sim_LogLevelCombo, false, false, 0);
    m_Sim_ControlRow.pack_start(*controlSpacer, true, true, 0);
    m_Sim_ControlRow.pack_end(m_Sim_SettingsButton, false, false, 0);
    m_Sim_ControlFrame.add(m_Sim_ControlRow);

    // Top split: vertex state + final result
    m_Sim_TopSplit.set_wide_handle(true);
    m_Sim_TopSplit.set_position(670);
    m_Sim_TopSplit.set_hexpand(true);
    m_Sim_TopSplit.set_vexpand(true);

    m_Sim_StatusFrame.set_name("sim-status-frame");
    m_Sim_StatusBox.set_margin_top(10);
    m_Sim_StatusBox.set_margin_bottom(10);
    m_Sim_StatusBox.set_margin_left(10);
    m_Sim_StatusBox.set_margin_right(10);
    m_Sim_StatusTitle.set_name("sim-panel-title");
    m_Sim_StatusTitle.set_xalign(0.0f);

    m_Sim_StatusStore = Gtk::ListStore::create(m_Sim_StatusColumns);
    m_Sim_StatusView.set_model(m_Sim_StatusStore);
    m_Sim_StatusView.set_headers_visible(true);
    m_Sim_StatusView.set_enable_search(false);
    m_Sim_StatusView.set_rules_hint(true);
    m_Sim_StatusView.set_grid_lines(Gtk::TREE_VIEW_GRID_LINES_BOTH);
    if (auto selection = m_Sim_StatusView.get_selection()) {
        selection->set_mode(Gtk::SELECTION_NONE);
    }

    m_Sim_StatusView.append_column("Đỉnh", m_Sim_StatusColumns.colVertex);
    m_Sim_StatusView.append_column("Khoảng Cách", m_Sim_StatusColumns.colDistance);
    m_Sim_StatusView.append_column("Cha", m_Sim_StatusColumns.colParent);

    auto* statusColumn = Gtk::manage(new Gtk::TreeViewColumn("Trạng Thái"));
    auto* statusRenderer = Gtk::manage(new Gtk::CellRendererText());
    statusColumn->pack_start(*statusRenderer, true);
    statusColumn->add_attribute(statusRenderer->property_markup(), m_Sim_StatusColumns.colStatusMarkup);
    m_Sim_StatusView.append_column(*statusColumn);

    m_Sim_StatusScroll.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_Sim_StatusScroll.set_name("log-scroll");
    m_Sim_StatusScroll.set_min_content_height(250);
    m_Sim_StatusScroll.add(m_Sim_StatusView);

    m_Sim_StatusBox.pack_start(m_Sim_StatusTitle, false, false, 0);
    m_Sim_StatusBox.pack_start(m_Sim_StatusScroll, true, true, 0);
    m_Sim_StatusFrame.add(m_Sim_StatusBox);

    m_Sim_ResultFrame.set_name("sim-result-frame");
    m_Sim_ResultFrame.set_size_request(370, -1);
    m_Sim_ResultBox.set_margin_top(10);
    m_Sim_ResultBox.set_margin_bottom(10);
    m_Sim_ResultBox.set_margin_left(12);
    m_Sim_ResultBox.set_margin_right(12);
    m_Sim_ResultTitle.set_name("sim-result-title");
    m_Sim_ResultTitle.set_xalign(0.0f);

    auto configureKey = [](Gtk::Label& label, const std::string& text) {
        label.set_text(text);
        label.set_name("result-key");
        label.set_xalign(0.0f);
    };
    auto configureValue = [](Gtk::Label& label, bool strong = false) {
        label.set_name(strong ? "result-value-strong" : "result-value");
        label.set_use_markup(true);
        label.set_xalign(0.0f);
        label.set_line_wrap(true);
    };

    configureKey(m_Result_Label_Algorithm, "Thuật toán");
    configureKey(m_Result_Label_Source, "Đỉnh nguồn");
    configureKey(m_Result_Label_Target, "Đỉnh đích");
    configureKey(m_Result_Label_Path, "Đường đi ngắn nhất");
    configureKey(m_Result_Label_Cost, "Tổng chi phí");
    configureKey(m_Result_Label_Status, "Trạng thái");

    configureValue(m_Result_Value_Algorithm, true);
    configureValue(m_Result_Value_Source);
    configureValue(m_Result_Value_Target);
    configureValue(m_Result_Value_Path, true);
    configureValue(m_Result_Value_Cost, true);
    configureValue(m_Result_Value_Status, true);

    m_Sim_ResultGrid.set_row_spacing(8);
    m_Sim_ResultGrid.set_column_spacing(12);
    m_Sim_ResultGrid.attach(m_Result_Label_Algorithm, 0, 0, 1, 1);
    m_Sim_ResultGrid.attach(m_Result_Value_Algorithm, 1, 0, 1, 1);
    m_Sim_ResultGrid.attach(m_Result_Label_Source, 0, 1, 1, 1);
    m_Sim_ResultGrid.attach(m_Result_Value_Source, 1, 1, 1, 1);
    m_Sim_ResultGrid.attach(m_Result_Label_Target, 0, 2, 1, 1);
    m_Sim_ResultGrid.attach(m_Result_Value_Target, 1, 2, 1, 1);
    m_Sim_ResultGrid.attach(m_Result_Label_Path, 0, 3, 1, 1);
    m_Sim_ResultGrid.attach(m_Result_Value_Path, 1, 3, 1, 1);
    m_Sim_ResultGrid.attach(m_Result_Label_Cost, 0, 4, 1, 1);
    m_Sim_ResultGrid.attach(m_Result_Value_Cost, 1, 4, 1, 1);
    m_Sim_ResultGrid.attach(m_Result_Label_Status, 0, 5, 1, 1);
    m_Sim_ResultGrid.attach(m_Result_Value_Status, 1, 5, 1, 1);

    m_Sim_ResultBox.pack_start(m_Sim_ResultTitle, false, false, 0);
    m_Sim_ResultBox.pack_start(m_Sim_ResultGrid, false, false, 0);
    m_Sim_ResultFrame.add(m_Sim_ResultBox);

    m_Sim_TopSplit.pack1(m_Sim_StatusFrame, true, false);
    m_Sim_TopSplit.pack2(m_Sim_ResultFrame, false, false);

    // Log panel
    m_Sim_LogFrame.set_name("sim-log-frame");
    m_Sim_LogBox.set_margin_top(10);
    m_Sim_LogBox.set_margin_bottom(10);
    m_Sim_LogBox.set_margin_left(10);
    m_Sim_LogBox.set_margin_right(10);
    m_Sim_LogTitle.set_name("sim-panel-title");
    m_Sim_LogTitle.set_xalign(0.0f);

    m_ScrolledWindow_Log.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_ScrolledWindow_Log.set_border_width(0);
    m_ScrolledWindow_Log.set_name("log-scroll");
    m_ScrolledWindow_Log.set_min_content_height(250);

    m_TextView_Log.set_editable(false);
    m_TextView_Log.set_cursor_visible(false);
    m_TextView_Log.set_name("log-view");
    m_TextView_Log.set_wrap_mode(Gtk::WRAP_WORD_CHAR);
    m_TextView_Log.set_left_margin(14);
    m_TextView_Log.set_right_margin(12);
    m_TextView_Log.set_top_margin(12);
    m_TextView_Log.set_bottom_margin(10);
    m_TextView_Log.set_justification(Gtk::JUSTIFY_LEFT);
    m_TextBuffer_Log = m_TextView_Log.get_buffer();

    // ── Text color tags ──────────────────────────────────────────────────────
    m_TextBuffer_Log->create_tag("red")->property_foreground()    = "#B91C1C";
    m_TextBuffer_Log->create_tag("green")->property_foreground()  = "#166534";
    m_TextBuffer_Log->create_tag("blue")->property_foreground()   = "#1D4ED8";
    m_TextBuffer_Log->create_tag("yellow")->property_foreground() = "#B45309";
    m_TextBuffer_Log->create_tag("dim")->property_foreground()    = "#6B7280";
    m_TextBuffer_Log->create_tag("bold")->property_weight()       = Pango::WEIGHT_BOLD;

    auto tagCyan = m_TextBuffer_Log->create_tag("cyan");
    tagCyan->property_foreground() = "#1E3A8A";
    tagCyan->property_weight()     = Pango::WEIGHT_BOLD;

    auto tagCenter = m_TextBuffer_Log->create_tag("center");
    tagCenter->property_justification() = Gtk::JUSTIFY_CENTER;

    auto tagHeader = m_TextBuffer_Log->create_tag("header");
    tagHeader->property_justification() = Gtk::JUSTIFY_CENTER;
    tagHeader->property_foreground()    = "#1E3A8A";
    tagHeader->property_weight()        = Pango::WEIGHT_BOLD;

    auto tagInfo = m_TextBuffer_Log->create_tag("info");
    tagInfo->property_justification() = Gtk::JUSTIFY_CENTER;
    tagInfo->property_foreground()    = "#374151";

    auto tagInit = m_TextBuffer_Log->create_tag("log-init");
    tagInit->property_foreground() = "#A16207";
    tagInit->property_weight() = Pango::WEIGHT_BOLD;

    auto tagProcess = m_TextBuffer_Log->create_tag("log-process");
    tagProcess->property_foreground() = "#C2410C";
    tagProcess->property_weight() = Pango::WEIGHT_BOLD;

    auto tagUpdate = m_TextBuffer_Log->create_tag("log-update");
    tagUpdate->property_foreground() = "#15803D";
    tagUpdate->property_weight() = Pango::WEIGHT_BOLD;

    auto tagWarning = m_TextBuffer_Log->create_tag("log-warning");
    tagWarning->property_foreground() = "#B91C1C";
    tagWarning->property_weight() = Pango::WEIGHT_BOLD;

    auto tagResult = m_TextBuffer_Log->create_tag("log-result");
    tagResult->property_foreground() = "#1D4ED8";
    tagResult->property_weight() = Pango::WEIGHT_BOLD;

    m_ScrolledWindow_Log.add(m_TextView_Log);
    m_Sim_LogBox.pack_start(m_Sim_LogTitle, false, false, 0);
    m_Sim_LogBox.pack_start(m_ScrolledWindow_Log, true, true, 0);
    m_Sim_LogFrame.add(m_Sim_LogBox);

    m_Page_Simulation.pack_start(m_Sim_ControlFrame, false, false, 0);
    m_Page_Simulation.pack_start(m_Sim_TopSplit, true, true, 0);
    m_Page_Simulation.pack_start(m_Sim_LogFrame, true, true, 0);

    // Compare page (chỉ section So sánh thuật toán)
    m_Page_Compare.set_orientation(Gtk::ORIENTATION_VERTICAL);
    m_Page_Compare.set_spacing(14);
    m_Page_Compare.set_margin_top(18);
    m_Page_Compare.set_margin_bottom(18);
    m_Page_Compare.set_margin_left(18);
    m_Page_Compare.set_margin_right(18);
    m_Page_Compare.get_style_context()->add_class("compare-section");

    // CSS provider riêng cho section so sánh.
    static Glib::RefPtr<Gtk::CssProvider> compareCssProvider;
    if (!compareCssProvider) {
        compareCssProvider = Gtk::CssProvider::create();
        try {
            compareCssProvider->load_from_data(R"css(
.compare-section {
    background: transparent;
}

.compare-title {
    color: #1F2937;
    font-size: 20px;
    font-weight: 800;
}

.compare-subtitle {
    color: #64748B;
    font-size: 12px;
}

.compare-info {
    background: #F5F8FC;
    border: 1px solid #D9E1EC;
    border-radius: 10px;
    padding: 8px 12px;
}

.compare-info-text {
    color: #334155;
    font-size: 14px;
    font-weight: 600;
}

.compare-card {
    background: #FFFFFF;
    border: 1px solid #DDE4EE;
    border-radius: 12px;
}

.compare-card-title {
    color: #1F2937;
    font-size: 15px;
    font-weight: 800;
}

.compare-card-badge {
    font-size: 13px;
    font-weight: 700;
    border-radius: 8px;
    padding: 4px 9px;
}

.compare-card-time {
    font-size: 28px;
    font-weight: 800;
}

.compare-card-mem {
    color: #334155;
    font-size: 16px;
    font-weight: 700;
}

.card-dijkstra {
    border-top: 3px solid #2F8F67;
}

.card-bellman {
    border-top: 3px solid #D2922C;
}

.card-bmssp {
    border-top: 3px solid #D9534F;
}

.compare-tabs {
    background: #F7F9FC;
    border: 1px solid #DDE4EE;
    border-radius: 10px;
    padding: 4px;
}

.compare-tab-active {
    background: #FFFFFF;
    border: 1px solid #C9D5E5;
    border-radius: 8px;
    color: #2563EB;
    font-weight: 700;
}

.compare-tab {
    background: transparent;
    border: 1px solid transparent;
    border-radius: 8px;
    color: #64748B;
    font-weight: 600;
}

.compare-table-wrap {
    background: #FFFFFF;
    border: 1px solid #DDE4EE;
    border-radius: 10px;
    padding: 8px;
}

.compare-table {
    background: #DDE4EE;
    border: 1px solid #DDE4EE;
    border-radius: 8px;
}

.compare-cell {
    background: #FFFFFF;
    color: #111827;
    padding: 10px 11px;
    font-size: 14px;
}

.compare-header {
    background: #F3F6FB;
    color: #1F2937;
    padding: 12px 10px;
    font-size: 15px;
    font-weight: 700;
}

.compare-bar-green trough,
.compare-bar-orange trough,
.compare-bar-red trough {
    background: #E8EDF5;
    border-radius: 8px;
    min-height: 8px;
}

.compare-bar-green progress {
    background: #39B76B;
    border-radius: 8px;
    min-height: 8px;
}

.compare-bar-orange progress {
    background: #E6A23C;
    border-radius: 8px;
    min-height: 8px;
}

.compare-bar-red progress {
    background: #DE5A58;
    border-radius: 8px;
    min-height: 8px;
}

.compare-badge-best {
    background: #E7F7EC;
    color: #1D7D4F;
    border-radius: 7px;
    padding: 3px 8px;
    font-size: 12px;
    font-weight: 700;
}

.compare-badge-worst {
    background: #FDECEC;
    color: #C23B37;
    border-radius: 7px;
    padding: 3px 8px;
    font-size: 12px;
    font-weight: 700;
}

.compare-badge-low {
    background: #E8F6ED;
    color: #1D7D4F;
    border-radius: 7px;
    padding: 3px 8px;
    font-size: 12px;
    font-weight: 700;
}

.compare-status-ok {
    color: #1F9D63;
    font-size: 14px;
    font-weight: 700;
}

.compare-conclusion {
    background: #FFF8E8;
    border: 1px solid #F2E4BE;
    border-radius: 10px;
    padding: 10px 12px;
}

.compare-conclusion-title {
    color: #854D0E;
    font-size: 14px;
    font-weight: 800;
}

.compare-conclusion-text {
    color: #5B4631;
    font-size: 14px;
    font-weight: 500;
}
)css");
        } catch (const Glib::Error& ex) {
            std::cerr << "[Compare CSS Error] " << ex.what() << std::endl;
        }
        auto screen = Gdk::Screen::get_default();
        Gtk::StyleContext::add_provider_for_screen(
            screen,
            compareCssProvider,
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
    }

    // Cấu hình bảng chi tiết (dùng lại khi render section compare).
    m_Compare_Grid.set_row_spacing(1);
    m_Compare_Grid.set_column_spacing(1);
    m_Compare_Grid.set_column_homogeneous(true);
    m_Compare_Grid.get_style_context()->add_class("compare-table");
    m_Compare_Grid.set_halign(Gtk::ALIGN_FILL);
    m_Compare_Grid.set_valign(Gtk::ALIGN_FILL);

    // Stack inside Main: loaded_summary | log | compare
    m_Main_ContentStack.set_transition_type(Gtk::STACK_TRANSITION_TYPE_CROSSFADE);
    m_Main_ContentStack.set_transition_duration(160);
    m_Main_ContentStack.add(m_Page_LoadedSummary, "loaded_summary");
    m_Main_ContentStack.add(m_Page_Simulation, "log");
    m_Main_ContentStack.add(m_Page_Compare, "compare");
    m_Main_ContentStack.set_visible_child("log");

    m_Page_Main.pack_start(m_Main_InlinePrompt_Revealer, false, false, 0);
    m_Page_Main.pack_start(m_Main_ContentStack, true, true, 0);

    resetFinalResultPanel();
    populateSampleSimulationData();
}

AlgorithmType MainWindow::getSelectedAlgorithm() const {
    const int selected = m_Sim_AlgoCombo.get_active_row_number();
    if (selected == 1) {
        return AlgorithmType::BELLMAN_FORD;
    }
    if (selected == 2) {
        return AlgorithmType::ADVANCED_BMSSP;
    }
    return AlgorithmType::DIJKSTRA;
}

void MainWindow::syncAlgorithmSelection(AlgorithmType type) {
    if (type == AlgorithmType::BELLMAN_FORD) {
        m_Sim_AlgoCombo.set_active(1);
    } else if (type == AlgorithmType::ADVANCED_BMSSP) {
        m_Sim_AlgoCombo.set_active(2);
    } else {
        m_Sim_AlgoCombo.set_active(0);
    }
}

std::string MainWindow::getAlgorithmLabel(AlgorithmType type) const {
    if (type == AlgorithmType::DIJKSTRA) {
        return "Dijkstra";
    }
    if (type == AlgorithmType::BELLMAN_FORD) {
        return "Bellman-Ford";
    }
    return "BMSSP - 2025";
}

void MainWindow::appendStructuredLog(const std::string& level, const std::string& message) {
    std::string tagName;
    if (level == "Khởi tạo") {
        tagName = "log-init";
    } else if (level == "Xử lý") {
        tagName = "log-process";
    } else if (level == "Cập nhật") {
        tagName = "log-update";
    } else if (level == "Cảnh báo") {
        tagName = "log-warning";
    } else {
        tagName = "log-result";
    }

    auto iter = m_TextBuffer_Log->end();
    m_TextBuffer_Log->insert_with_tag(
        iter,
        "[" + level + "] " + message + "\n",
        tagName
    );
    auto scrollIter = m_TextBuffer_Log->end();
    m_TextView_Log.scroll_to(scrollIter);
}

void MainWindow::resetFinalResultPanel() {
    m_Result_Value_Algorithm.set_markup("<span>Chưa chạy</span>");
    m_Result_Value_Source.set_markup("<span>-</span>");
    m_Result_Value_Target.set_markup("<span>-</span>");
    m_Result_Value_Path.set_markup("<span>-</span>");
    m_Result_Value_Cost.set_markup("<span>-</span>");
    m_Result_Value_Status.set_markup("<span foreground='#64748B'><b>Sẵn sàng</b></span>");
}

void MainWindow::populateSampleSimulationData() {
    if (!m_Sim_StatusStore) {
        return;
    }

    m_Sim_StatusStore->clear();
    struct SampleRow {
        const char* vertex;
        const char* distance;
        const char* parent;
        const char* status;
    };

    const SampleRow rows[] = {
        {"0", "0", "-", "Đã chốt"},
        {"1", "4", "0", "Đã chốt"},
        {"2", "2", "0", "Chờ xử lý"},
        {"3", "9", "2", "Chờ xử lý"},
        {"4", "∞", "-", "Chưa tới"},
        {"5", "5", "1", "Đã chốt"},
        {"6", "7", "5", "Đã chốt"},
        {"7", "8", "6", "Chờ xử lý"},
    };

    for (const auto& rowData : rows) {
        auto row = *(m_Sim_StatusStore->append());
        row[m_Sim_StatusColumns.colVertex] = rowData.vertex;
        row[m_Sim_StatusColumns.colDistance] = rowData.distance;
        row[m_Sim_StatusColumns.colParent] = rowData.parent;
        row[m_Sim_StatusColumns.colStatus] = rowData.status;
        row[m_Sim_StatusColumns.colStatusMarkup] = statusMarkup(rowData.status);
    }

    m_Result_Value_Algorithm.set_markup("<b>Dijkstra</b>");
    m_Result_Value_Source.set_markup("<b>0</b>");
    m_Result_Value_Target.set_markup("<b>7</b>");
    m_Result_Value_Path.set_markup("<b>0 -> 1 -> 5 -> 6 -> 7</b>");
    m_Result_Value_Cost.set_markup("<b>8</b>");
    m_Result_Value_Status.set_markup("<span foreground='#15803D'><b>Thành công</b></span>");

    clearLog();
    appendStructuredLog("Khởi tạo", "Thuật toán Dijkstra, đỉnh nguồn: 0");
    appendStructuredLog("Xử lý", "Chốt đỉnh 0 với khoảng cách 0");
    appendStructuredLog("Cập nhật", "0 -> 1 (trọng số 4): dist[1] từ vô cùng thành 4, cha[1] = 0");
    appendStructuredLog("Cập nhật", "0 -> 2 (trọng số 2): dist[2] từ vô cùng thành 2, cha[2] = 0");
    appendStructuredLog("Xử lý", "Chốt đỉnh 1 với khoảng cách 4");
    appendStructuredLog("Cập nhật", "1 -> 5 (trọng số 1): dist[5] từ vô cùng thành 5, cha[5] = 1");
    appendStructuredLog("Xử lý", "Chốt đỉnh 5 với khoảng cách 5");
    appendStructuredLog("Cập nhật", "5 -> 6 (trọng số 2): dist[6] từ 10 thành 7, cha[6] = 5");
    appendStructuredLog("Kết quả", "Đường đi ngắn nhất từ 0 đến 7: 0 -> 1 -> 5 -> 6 -> 7");
}

void MainWindow::refreshVertexStatusTable(const PathResult& result, int startVertex, bool stepMode) {
    if (!m_Sim_StatusStore) {
        return;
    }

    m_Sim_StatusStore->clear();
    if (result.distances.empty()) {
        return;
    }

    const int INF = std::numeric_limits<int>::max();
    std::unordered_set<int> pathVertices(result.shortestPath.begin(), result.shortestPath.end());
    std::unordered_set<int> visitedVertices;
    int activeVertex = -1;

    for (const auto& step : result.traceSteps) {
        if (step.type == "visit" && step.node >= 0) {
            visitedVertices.insert(step.node);
            if (stepMode) {
                activeVertex = step.node;
            }
        }
    }

    for (int i = 0; i < static_cast<int>(result.distances.size()); ++i) {
        std::string status = "Chưa tới";
        if (result.distances[i] != INF) {
            if (stepMode && i == activeVertex) {
                status = "Đang xét";
            } else if (i == startVertex || pathVertices.count(i) > 0) {
                status = "Đã chốt";
            } else if (result.previousVertex[i] != -1 || visitedVertices.count(i) > 0) {
                status = "Chờ xử lý";
            } else {
                status = "Đang xét";
            }
        }

        const std::string distanceText = (result.distances[i] == INF)
            ? "∞"
            : std::to_string(result.distances[i]);
        const std::string parentText = (i < static_cast<int>(result.previousVertex.size())
            && result.previousVertex[i] >= 0)
            ? graph.getVertexLabel(result.previousVertex[i])
            : "-";

        auto row = *(m_Sim_StatusStore->append());
        row[m_Sim_StatusColumns.colVertex] = graph.getVertexLabel(i);
        row[m_Sim_StatusColumns.colDistance] = distanceText;
        row[m_Sim_StatusColumns.colParent] = parentText;
        row[m_Sim_StatusColumns.colStatus] = status;
        row[m_Sim_StatusColumns.colStatusMarkup] = statusMarkup(status);
    }
}

void MainWindow::updateFinalResultPanel(
    AlgorithmType type,
    int startVertex,
    int endVertex,
    const PathResult& result,
    long long elapsedUs
) {
    auto escape = [](const std::string& text) {
        return Glib::Markup::escape_text(text);
    };

    m_Result_Value_Algorithm.set_markup("<b>" + escape(getAlgorithmLabel(type)) + "</b>");
    m_Result_Value_Source.set_markup("<b>" + escape(graph.getVertexLabel(startVertex)) + "</b>");
    m_Result_Value_Target.set_markup("<b>" + escape(graph.getVertexLabel(endVertex)) + "</b>");

    if (!result.success) {
        m_Result_Value_Path.set_markup("<span foreground='#B91C1C'><b>Không khả dụng</b></span>");
        m_Result_Value_Cost.set_markup("<span foreground='#B91C1C'><b>-</b></span>");
        if (result.hasNegativeCycle) {
            m_Result_Value_Status.set_markup(
                "<span foreground='#B91C1C'><b>Có chu trình âm</b></span>"
            );
        } else {
            m_Result_Value_Status.set_markup(
                "<span foreground='#B91C1C'><b>Thất bại</b></span>"
            );
        }
        return;
    }

    if (result.shortestPath.empty()) {
        m_Result_Value_Path.set_markup("<span foreground='#B45309'><b>Không tồn tại đường đi</b></span>");
        m_Result_Value_Cost.set_markup("<span foreground='#B45309'><b>∞</b></span>");
        m_Result_Value_Status.set_markup(
            "<span foreground='#B45309'><b>Không thể tới đích</b></span>"
        );
        return;
    }

    std::string pathText;
    for (std::size_t i = 0; i < result.shortestPath.size(); ++i) {
        if (i > 0) {
            pathText += " -> ";
        }
        pathText += graph.getVertexLabel(result.shortestPath[i]);
    }
    m_Result_Value_Path.set_markup("<b>" + escape(pathText) + "</b>");

    const int distance = (endVertex >= 0 && endVertex < static_cast<int>(result.distances.size()))
        ? result.distances[endVertex]
        : std::numeric_limits<int>::max();
    if (distance == std::numeric_limits<int>::max()) {
        m_Result_Value_Cost.set_markup("<span foreground='#B45309'><b>∞</b></span>");
    } else {
        m_Result_Value_Cost.set_markup("<b>" + std::to_string(distance) + "</b>");
    }

    std::string successText = "Thành công";
    if (elapsedUs >= 0) {
        successText += " (" + std::to_string(elapsedUs) + " μs)";
    }
    m_Result_Value_Status.set_markup(
        "<span foreground='#15803D'><b>" + escape(successText) + "</b></span>"
    );
}

void MainWindow::rebuildAlgorithmLogView(
    const PathResult& result,
    AlgorithmType type,
    int startVertex,
    int endVertex,
    long long elapsedUs,
    bool stepMode
) {
    clearLog();
    if (startVertex < 0 || endVertex < 0) {
        return;
    }

    const int level = std::max(0, m_Sim_LogLevelCombo.get_active_row_number());
    int maxProcess = 6;
    int maxUpdate = 4;
    if (level == 1) {
        maxProcess = 14;
        maxUpdate = 12;
    } else if (level >= 2) {
        maxProcess = std::numeric_limits<int>::max();
        maxUpdate = std::numeric_limits<int>::max();
    }

    appendStructuredLog(
        "Khởi tạo",
        "Thuật toán " + getAlgorithmLabel(type) + ", đỉnh nguồn: " + graph.getVertexLabel(startVertex)
    );
    appendStructuredLog("Khởi tạo", "Đỉnh đích: " + graph.getVertexLabel(endVertex));
    if (stepMode) {
        appendStructuredLog("Khởi tạo", "Chế độ chạy từng bước đang bật.");
    }

    int shownProcess = 0;
    int shownUpdate = 0;
    int hiddenProcess = 0;
    int hiddenUpdate = 0;

    for (const auto& linePair : result.logs) {
        std::string line = trimCopy(linePair.second);
        if (line.empty()) {
            continue;
        }

        if (line.find("====") != std::string::npos) {
            continue;
        }

        std::string category = "Xử lý";
        if (containsNoCase(line, "cập nhật") || containsNoCase(line, "relax")) {
            category = "Cập nhật";
        } else if (containsNoCase(line, "chu trình âm") || containsNoCase(line, "thất bại")) {
            category = "Cảnh báo";
        } else if (containsNoCase(line, "khởi tạo")
                   || containsNoCase(line, "bắt đầu")
                   || containsNoCase(line, "tham số")
                   || containsNoCase(line, "lưu ý")) {
            category = "Khởi tạo";
        }

        if (category == "Xử lý" && shownProcess >= maxProcess) {
            ++hiddenProcess;
            continue;
        }
        if (category == "Cập nhật" && shownUpdate >= maxUpdate) {
            ++hiddenUpdate;
            continue;
        }

        if (category == "Xử lý") {
            ++shownProcess;
        } else if (category == "Cập nhật") {
            ++shownUpdate;
        }

        appendStructuredLog(category, line);
    }

    if (hiddenProcess > 0 || hiddenUpdate > 0) {
        std::ostringstream hidden;
        hidden << "[Xử lý] Đã lược bớt ";
        if (hiddenProcess > 0) {
            hidden << hiddenProcess << " dòng xử lý";
            if (hiddenUpdate > 0) {
                hidden << ", ";
            }
        }
        if (hiddenUpdate > 0) {
            hidden << hiddenUpdate << " dòng cập nhật";
        }
        hidden << " theo mức log đã chọn.";
        logMessageColored(hidden.str(), "dim");
    }

    if (!result.success) {
        if (result.hasNegativeCycle) {
            appendStructuredLog("Cảnh báo", "Phát hiện chu trình âm có thể đi tới từ đỉnh nguồn.");
        } else {
            appendStructuredLog("Cảnh báo", "Thuật toán không trả về kết quả hợp lệ.");
        }
        appendStructuredLog("Kết quả", "Không thể xác định đường đi ngắn nhất.");
        return;
    }

    if (result.shortestPath.empty()) {
        appendStructuredLog(
            "Kết quả",
            "Không có đường đi từ " + graph.getVertexLabel(startVertex)
                + " đến " + graph.getVertexLabel(endVertex) + "."
        );
    } else {
        std::string pathText;
        for (std::size_t i = 0; i < result.shortestPath.size(); ++i) {
            if (i > 0) {
                pathText += " -> ";
            }
            pathText += graph.getVertexLabel(result.shortestPath[i]);
        }
        appendStructuredLog(
            "Kết quả",
            "Đường đi ngắn nhất từ " + graph.getVertexLabel(startVertex)
                + " đến " + graph.getVertexLabel(endVertex) + ": " + pathText
        );

        if (endVertex >= 0 && endVertex < static_cast<int>(result.distances.size())) {
            if (result.distances[endVertex] == std::numeric_limits<int>::max()) {
                appendStructuredLog("Kết quả", "Tổng chi phí: ∞");
            } else {
                appendStructuredLog("Kết quả", "Tổng chi phí: " + std::to_string(result.distances[endVertex]));
            }
        }
    }

    appendStructuredLog("Kết quả", "Thời gian chạy: " + std::to_string(elapsedUs) + " μs");
}

void MainWindow::on_toolbar_run_clicked() {
    runAlgorithm(getSelectedAlgorithm(), false);
}

void MainWindow::on_toolbar_step_clicked() {
    runAlgorithm(getSelectedAlgorithm(), true);
}

void MainWindow::on_toolbar_clear_log_clicked() {
    clearLog();
}

void MainWindow::on_toolbar_log_level_changed() {
    if (m_lastResult.startVertex == -1 || m_lastEndVertex < 0) {
        return;
    }
    rebuildAlgorithmLogView(
        m_lastResult,
        m_lastAlgoType,
        m_lastResult.startVertex,
        m_lastEndVertex,
        m_lastElapsedUs,
        m_lastRunStepMode
    );
}

// ─────────────────────────────────────────────────────────────────────────────
//  Visualizer Page (Native C++)
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::buildVisualizerPage() {
    m_Page_Visualizer.set_name("visualizer-page");
    m_Page_Visualizer.set_orientation(Gtk::ORIENTATION_VERTICAL);
    m_Page_Visualizer.set_border_width(0);
    m_Page_Visualizer.set_spacing(10);

    Gtk::Frame* frame = Gtk::manage(new Gtk::Frame());
    frame->set_name("visualizer-canvas-frame");
    frame->set_shadow_type(Gtk::SHADOW_NONE);
    frame->set_hexpand(true);
    frame->set_vexpand(true);
    frame->add(m_GraphCanvas);
    m_Page_Visualizer.pack_start(*frame, true, true, 0);

    Gtk::Box* toolbar = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 8));
    toolbar->set_name("visualizer-toolbar");

    Gtk::Button* btnPlay  = Gtk::manage(new Gtk::Button("▶ Chạy"));
    Gtk::Button* btnPause = Gtk::manage(new Gtk::Button("⏸ Tạm dừng"));
    Gtk::Button* btnReset = Gtk::manage(new Gtk::Button("⟲ Chạy lại"));
    Gtk::Button* btnClose = Gtk::manage(new Gtk::Button("↩ Quay lại"));

    btnPlay->set_name("btn-viz-play");
    btnPause->set_name("btn-viz-pause");
    btnReset->set_name("btn-viz-reset");
    btnClose->set_name("btn-viz-back");

    btnPlay->signal_clicked().connect([this]()  { m_GraphCanvas.playAnimation(); });
    btnPause->signal_clicked().connect([this]() { m_GraphCanvas.pauseAnimation(); });
    btnReset->signal_clicked().connect([this]() {
        m_GraphCanvas.resetAnimation();
        m_GraphCanvas.playAnimation();
    });
    btnClose->signal_clicked().connect([this]() {
        m_GraphCanvas.pauseAnimation();
        m_Stack.set_visible_child("main");
    });

    toolbar->pack_start(*btnPlay,  false, false, 0);
    toolbar->pack_start(*btnPause, false, false, 0);
    toolbar->pack_start(*btnReset, false, false, 0);

    Gtk::Box* spacer = Gtk::manage(new Gtk::Box());
    toolbar->pack_start(*spacer, true, true, 0);
    toolbar->pack_end(*btnClose, false, false, 0);

    m_Page_Visualizer.pack_start(*toolbar, false, false, 0);
}

void MainWindow::showLoadGraphPage() {
    m_welcomeShown = false;
    clearLoadGraphStatus();
    m_Stack.set_visible_child("load_graph");
}

void MainWindow::updateLoadGraphInputMode() {
    clearLoadGraphStatus();
    if (m_Load_Mode_Direct.get_active()) {
        m_Load_FormStack.set_visible_child("direct");
        m_Load_Confirm.set_label("Nạp Từ Dữ Liệu Nhập Tay");
    } else {
        m_Load_FormStack.set_visible_child("file");
        m_Load_Confirm.set_label("Nạp Từ File");
    }
}

void MainWindow::resetLoadGraphForm() {
    m_Load_Mode_File.set_active(true);
    m_Load_FileDirected.set_active(true);
    m_Load_DirectDirected.set_active(true);
    m_Load_FilePath.set_text("");
    m_Load_VertexEntry.set_text("4");
    if (m_Load_EdgesBuffer) {
        m_Load_EdgesBuffer->set_text("1 2 4\n1 3 2\n2 4 5\n3 4 1\n");
    }
    updateLoadGraphInputMode();
    clearLoadGraphStatus();
}

void MainWindow::clearLoadGraphStatus() {
    m_Load_Status.set_text("");
}

void MainWindow::setLoadGraphStatus(const std::string& message, const std::string& colorHex) {
    if (message.empty()) {
        clearLoadGraphStatus();
        return;
    }

    m_Load_Status.set_markup(
        "<span foreground='" + colorHex + "' weight='bold'>"
        + Glib::Markup::escape_text(message) +
        "</span>"
    );
}

void MainWindow::hideInlineVisualizePrompt() {
    m_Main_InlinePrompt_Revealer.set_reveal_child(false);
}

void MainWindow::updateLoadedSummary(const std::string& sourceLabel, bool isDirected) {
    const int vertexCount = graph.getVertexCount();
    const int edgeCount = graph.getEdgeCount();
    const bool hasNegative = graph.hasNegativeWeights();

    m_LoadedSummaryTitle.set_text("Đã nạp đồ thị thành công");
    m_LoadedSummaryStatus.set_text("● Trạng thái: Thành công");
    m_LoadedSummarySource.set_text("Nguồn dữ liệu: " + sourceLabel);

    std::ostringstream meta;
    meta << "Loại đồ thị: " << (isDirected ? "Có hướng" : "Vô hướng")
         << "\nSố đỉnh (V): " << vertexCount
         << "\nSố cạnh (E): " << edgeCount
         << "\nCạnh âm: " << (hasNegative ? "Có" : "Không");
    m_LoadedSummaryMeta.set_text(meta.str());

    std::ostringstream preview;
    const auto& adjList = graph.getAdjacencyList();
    constexpr int kPreviewLimit = 5;
    int shown = 0;

    for (int u = 0; u < static_cast<int>(adjList.size()) && shown < kPreviewLimit; ++u) {
        for (const auto& edge : adjList[u]) {
            if (shown > 0) {
                preview << "\n";
            }
            preview << "• " << graph.getVertexLabel(u)
                    << " -> " << graph.getVertexLabel(edge.destination)
                    << " (w = " << edge.weight << ")";
            ++shown;
            if (shown >= kPreviewLimit) {
                break;
            }
        }
    }

    if (shown == 0) {
        preview << "Không có cạnh để hiển thị.";
    } else if (edgeCount > shown) {
        preview << "\n… và " << (edgeCount - shown) << " cạnh khác.";
    }
    m_LoadedSummaryEdgePreview.set_text(preview.str());
}

void MainWindow::finalizeGraphLoad(const std::string& sourceLabel, bool isDirected) {
    m_isDirected = isDirected;
    m_lastResult = PathResult();
    m_lastAlgoType = AlgorithmType::DIJKSTRA;
    m_lastEndVertex = -1;
    m_lastElapsedUs = 0;
    m_lastRunStepMode = false;

    updateGraphSummary();
    switchToMain();
    resetFinalResultPanel();
    if (m_Sim_StatusStore) {
        m_Sim_StatusStore->clear();
        for (int i = 0; i < graph.getVertexCount(); ++i) {
            auto row = *(m_Sim_StatusStore->append());
            row[m_Sim_StatusColumns.colVertex] = graph.getVertexLabel(i);
            row[m_Sim_StatusColumns.colDistance] = "∞";
            row[m_Sim_StatusColumns.colParent] = "-";
            row[m_Sim_StatusColumns.colStatus] = "Chưa tới";
            row[m_Sim_StatusColumns.colStatusMarkup] = statusMarkup("Chưa tới");
        }
    }
    updateLoadedSummary(sourceLabel, isDirected);
    m_Main_ContentStack.set_visible_child("loaded_summary");

    m_Main_InlinePrompt_Label.set_markup(
        "<b>Nạp đồ thị thành công.</b> Bạn muốn trực quan hóa ngay hay tiếp tục thao tác?"
    );
    m_Main_InlinePrompt_Revealer.set_reveal_child(true);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Switch to Main page
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::switchToMain() {
    m_Stack.set_visible_child("main");
    m_Main_ContentStack.set_visible_child("log");
    m_welcomeShown = false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Get directory of exe
// ─────────────────────────────────────────────────────────────────────────────
std::string MainWindow::getExeDir() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    std::filesystem::path exePath(buffer);
    return exePath.parent_path().string();
#else
    return std::filesystem::current_path().string();
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
//  Log helpers
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::clearLog() {
    m_TextBuffer_Log->set_text("");
}

void MainWindow::logMessage(const std::string& msg) {
    auto iter = m_TextBuffer_Log->end();
    m_TextBuffer_Log->insert(iter, msg + "\n");
    auto scroll_iter = m_TextBuffer_Log->end();
    m_TextView_Log.scroll_to(scroll_iter);
}

void MainWindow::logMessageColored(const std::string& msg, const std::string& colorTag) {
    auto iter = m_TextBuffer_Log->end();
    if (colorTag.empty()) {
        m_TextBuffer_Log->insert(iter, msg + "\n");
    } else {
        m_TextBuffer_Log->insert_with_tag(iter, msg + "\n", colorTag);
    }
    auto scroll_iter = m_TextBuffer_Log->end();
    m_TextView_Log.scroll_to(scroll_iter);
}

void MainWindow::logHeader(const std::string& msg) {
    auto startMark = m_TextBuffer_Log->create_mark(m_TextBuffer_Log->end());
    m_TextBuffer_Log->insert(m_TextBuffer_Log->end(), msg + "\n");
    auto startIter = m_TextBuffer_Log->get_iter_at_mark(startMark);
    auto endIter   = m_TextBuffer_Log->end();
    m_TextBuffer_Log->apply_tag_by_name("header", startIter, endIter);
    m_TextBuffer_Log->delete_mark(startMark);
    auto scroll_iter = m_TextBuffer_Log->end();
    m_TextView_Log.scroll_to(scroll_iter);
}

void MainWindow::logInfo(const std::string& msg) {
    auto startMark = m_TextBuffer_Log->create_mark(m_TextBuffer_Log->end());
    m_TextBuffer_Log->insert(m_TextBuffer_Log->end(), msg + "\n");
    auto startIter = m_TextBuffer_Log->get_iter_at_mark(startMark);
    auto endIter   = m_TextBuffer_Log->end();
    m_TextBuffer_Log->apply_tag_by_name("info", startIter, endIter);
    m_TextBuffer_Log->delete_mark(startMark);
    auto scroll_iter = m_TextBuffer_Log->end();
    m_TextView_Log.scroll_to(scroll_iter);
}

// ─────────────────────────────────────────────────────────────────────────────
//  updateGraphSummary (sidebar label)
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::updateGraphSummary() {
    if (!graph.isValid()) {
        m_Label_GraphStats.set_label("Chưa tải đồ thị.");
        return;
    }
    std::ostringstream oss;
    oss << "Đỉnh: " << graph.getVertexCount()
        << "\nCạnh: " << graph.getEdgeCount();
    m_Label_GraphStats.set_label(oss.str());
}

// ─────────────────────────────────────────────────────────────────────────────
//  Helper nội bộ: in tóm tắt đồ thị ra Right Panel
//  Dùng chung cho cả load-from-file và nhập trực tiếp.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::printGraphSummaryToLog(bool isDirected) {
    const auto& adjList = graph.getAdjacencyList();
    int V = graph.getVertexCount();
    int E = graph.getEdgeCount();

    logHeader("═══════════════  ĐỒ THỊ ĐÃ NẠP  ═══════════════");
    logMessage("");

    // Thông tin tổng quan
    logMessageColored("  Loại đồ thị  :  " + std::string(isDirected ? "CÓ HƯỚNG (Directed)" : "VÔ HƯỚNG (Undirected)"), "blue");
    logMessageColored("  Số đỉnh  (V) :  " + std::to_string(V), "blue");
    logMessageColored("  Số cạnh  (E) :  " + std::to_string(E), "blue");
    logMessage("");

    // Xây danh sách cạnh phẳng: (u, v, w)
    struct EdgeEntry { int u, v, w; };
    std::vector<EdgeEntry> edges;
    edges.reserve(E);
    for (int u = 0; u < V; ++u) {
        for (const auto& e : adjList[u]) {
            edges.push_back({u, e.destination, e.weight});
        }
    }

    logInfo("── Danh sách cạnh ──");

    auto printEdge = [&](const EdgeEntry& e) {
        std::ostringstream line;
        line << "    "
             << graph.getVertexLabel(e.u)
             << "  →  "
             << graph.getVertexLabel(e.v)
             << "   (w = " << e.w << ")";
        logMessage(line.str());
    };

    const int PREVIEW = 5; // số cạnh đầu / cuối khi lược bỏ

    if (E <= 10) {
        // In toàn bộ
        for (const auto& e : edges) printEdge(e);
    } else {
        // In 5 đầu
        for (int i = 0; i < PREVIEW; ++i) printEdge(edges[i]);

        // Dòng lược bỏ
        std::ostringstream ellipsis;
        ellipsis << "    ... (lược bỏ " << (E - 2 * PREVIEW) << " cạnh) ...";
        logMessageColored(ellipsis.str(), "dim");

        // In 5 cuối
        for (int i = E - PREVIEW; i < E; ++i) printEdge(edges[i]);
    }

    logMessage("");
    logMessageColored("✓ Nạp đồ thị thành công.", "green");
    logMessage("");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Load Graph: chọn file từ page inline
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::on_button_browse_graph_file() {
    Gtk::FileChooserDialog fileDialog(
        *this, "Chọn file đồ thị",
        Gtk::FILE_CHOOSER_ACTION_OPEN
    );
    fileDialog.add_button("_Hủy", Gtk::RESPONSE_CANCEL);
    fileDialog.add_button("_Mở",  Gtk::RESPONSE_OK);

    auto filterTxt = Gtk::FileFilter::create();
    filterTxt->set_name("Text files (*.txt)");
    filterTxt->add_pattern("*.txt");
    fileDialog.add_filter(filterTxt);

    auto filterAny = Gtk::FileFilter::create();
    filterAny->set_name("All files");
    filterAny->add_pattern("*");
    fileDialog.add_filter(filterAny);

    std::string currentPath = m_Load_FilePath.get_text();
    if (!currentPath.empty()) {
        std::filesystem::path current(currentPath);
        if (std::filesystem::exists(current.parent_path())) {
            fileDialog.set_current_folder(current.parent_path().string());
        }
    } else {
        std::string exeDir = getExeDir();
        std::filesystem::path dataDir =
            std::filesystem::path(exeDir).parent_path() / "data";
        if (std::filesystem::exists(dataDir)) {
            fileDialog.set_current_folder(dataDir.string());
        }
    }

    if (fileDialog.run() != Gtk::RESPONSE_OK) {
        setLoadGraphStatus("Chưa chọn file nào.", "#94A3B8");
        return;
    }

    m_Load_FilePath.set_text(fileDialog.get_filename());
    setLoadGraphStatus("Đã chọn file đồ thị. Nhấn 'Nạp' để tiếp tục.", "#93C5FD");
}

// ─────────────────────────────────────────────────────────────────────────────
//  Load Graph: submit từ page inline
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::on_button_submit_load_graph() {
    hideInlineVisualizePrompt();

    if (m_Load_Mode_File.get_active()) {
        std::string filename = m_Load_FilePath.get_text();
        const size_t begin = filename.find_first_not_of(" \t\r\n");
        if (begin == std::string::npos) {
            setLoadGraphStatus("Hãy nhập hoặc chọn đường dẫn file đồ thị.", "#FCA5A5");
            return;
        }
        const size_t end = filename.find_last_not_of(" \t\r\n");
        filename = filename.substr(begin, end - begin + 1);

        bool needCreate = false;
        if (!graph.readFromFile(filename, needCreate)) {
            if (needCreate) {
                setLoadGraphStatus("Không tìm thấy file đồ thị ở đường dẫn đã nhập.", "#FCA5A5");
            } else {
                setLoadGraphStatus("Không thể đọc file. Kiểm tra lại định dạng và dữ liệu trong file.", "#FCA5A5");
            }
            return;
        }

        const bool isDirected = m_Load_FileDirected.get_active();
        if (!isDirected) {
            graph.makeUndirected();
        }

        finalizeGraphLoad(filename, isDirected);
        return;
    }

    int numVertices = 0;
    try {
        numVertices = std::stoi(m_Load_VertexEntry.get_text());
    } catch (...) {
        numVertices = 0;
    }

    if (numVertices < 1 || numVertices > 1000) {
        setLoadGraphStatus("Số đỉnh phải là số nguyên trong khoảng từ 1 đến 1000.", "#FCA5A5");
        return;
    }

    const bool isDirected = m_Load_DirectDirected.get_active();
    std::string errorMessage;
    std::string edgeText;
    if (m_Load_EdgesBuffer) {
        edgeText = m_Load_EdgesBuffer->get_text();
    }
    if (!graph.loadFromEdgeListText(numVertices, edgeText, isDirected, errorMessage)) {
        setLoadGraphStatus(errorMessage, "#FCA5A5");
        return;
    }

    finalizeGraphLoad("Nhập trực tiếp trên giao diện", isDirected);
}

void MainWindow::on_button_reset_load_graph_form() {
    resetLoadGraphForm();
}

void MainWindow::on_button_visualize_now_inline() {
    hideInlineVisualizePrompt();
    on_button_visualize();
}

void MainWindow::on_button_visualize_later_inline() {
    hideInlineVisualizePrompt();
    logMessageColored("→ Bạn có thể nhấn [Trực Quan Hóa] bất kỳ lúc nào từ thanh chức năng bên trái.", "dim");
}

void MainWindow::on_button_loaded_summary_run() {
    hideInlineVisualizePrompt();
    m_Stack.set_visible_child("main");
    m_Main_ContentStack.set_visible_child("log");
    m_Sim_AlgoCombo.grab_focus();
}

void MainWindow::on_button_loaded_summary_visualize() {
    hideInlineVisualizePrompt();
    on_button_visualize();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Button: Load Graph → mở page inline
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::on_button_load_graph() {
    hideInlineVisualizePrompt();
    showLoadGraphPage();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Button: Run Dijkstra / Bellman-Ford
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::on_button_run_dijkstra() {
    syncAlgorithmSelection(AlgorithmType::DIJKSTRA);
    runAlgorithm(AlgorithmType::DIJKSTRA, false);
}

void MainWindow::on_button_run_bellman() {
    syncAlgorithmSelection(AlgorithmType::BELLMAN_FORD);
    runAlgorithm(AlgorithmType::BELLMAN_FORD, false);
}

void MainWindow::on_button_run_advanced() {
    syncAlgorithmSelection(AlgorithmType::ADVANCED_BMSSP);
    runAlgorithm(AlgorithmType::ADVANCED_BMSSP, false);
}

void MainWindow::runAlgorithm(AlgorithmType type, bool stepMode) {
    hideInlineVisualizePrompt();
    syncAlgorithmSelection(type);

    if (!graph.isValid()) {
        Gtk::MessageDialog dlg(*this, "Chưa tải đồ thị", false, Gtk::MESSAGE_WARNING);
        dlg.set_secondary_text(
            "Vui lòng nạp đồ thị trước khi chạy thuật toán.\n"
        );
        dlg.run();
        showLoadGraphPage();
        return;
    }

    switchToMain();
    m_Main_ContentStack.set_visible_child("log");

    if ((type == AlgorithmType::DIJKSTRA || type == AlgorithmType::ADVANCED_BMSSP)
        && graph.hasNegativeWeights()) {
        Gtk::MessageDialog dlg(*this, "Lỗi đầu vào", false, Gtk::MESSAGE_ERROR);
        dlg.set_secondary_text(
            type == AlgorithmType::DIJKSTRA
                ? "Dijkstra không hỗ trợ cạnh trọng số âm."
                : "BMSSP 2025 chỉ hỗ trợ đồ thị có hướng với trọng số không âm."
        );
        dlg.run();
        return;
    }

    int startInput, endInput;
    try {
        startInput = std::stoi(m_Entry_Start.get_text());
        endInput   = std::stoi(m_Entry_End.get_text());
    } catch (...) {
        Gtk::MessageDialog dlg(*this, "Lỗi nhập liệu", false, Gtk::MESSAGE_ERROR);
        dlg.set_secondary_text("Đỉnh phải là số nguyên.");
        dlg.run();
        return;
    }

    int V = graph.getVertexCount();
    if (startInput < 1 || startInput > V || endInput < 1 || endInput > V) {
        Gtk::MessageDialog dlg(*this, "Đỉnh không hợp lệ", false, Gtk::MESSAGE_ERROR);
        dlg.set_secondary_text("Vui lòng nhập từ 1 đến " + std::to_string(V) + ".");
        dlg.run();
        return;
    }

    int start = startInput - 1;
    int end   = endInput   - 1;

    const bool isDijk = (type == AlgorithmType::DIJKSTRA);
    const bool isBellman = (type == AlgorithmType::BELLMAN_FORD);
    const bool isAdvanced = (type == AlgorithmType::ADVANCED_BMSSP);

    auto t0 = std::chrono::high_resolution_clock::now();

    PathResult result;
    if (isDijk) {
        result = algorithms->dijkstra(start, true);
    } else if (isBellman) {
        result = algorithms->bellmanFord(start, true);
    } else {
        result = advancedAlgorithms->breakingSortingBarrier(start, true);
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    if (result.success) {
        if (isAdvanced) {
            result.shortestPath = advancedAlgorithms->getShortestPath(result, end);
        } else {
            result.shortestPath = algorithms->getShortestPath(result, end);
        }
    }

    m_lastResult = result;
    m_lastAlgoType = type;
    m_lastEndVertex = end;
    m_lastElapsedUs = us;
    m_lastRunStepMode = stepMode;

    refreshVertexStatusTable(result, start, stepMode);
    updateFinalResultPanel(type, start, end, result, us);
    rebuildAlgorithmLogView(result, type, start, end, us, stepMode);
    if (result.success) {
        logMessageColored("[Kết quả] Nhấn [Trực Quan Hóa] để xem animation.", "dim");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Button: Compare
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::on_button_compare() {
    hideInlineVisualizePrompt();
    switchToMain();

    if (!graph.isValid()) {
        Gtk::MessageDialog dlg(*this, "Đồ thị rỗng", false, Gtk::MESSAGE_WARNING);
        dlg.set_secondary_text("Vui lòng tải đồ thị trước.");
        dlg.run();
        return;
    }

    int startInput;
    try {
        startInput = std::stoi(m_Entry_Start.get_text());
    } catch (...) {
        startInput = 1;
    }

    int V = graph.getVertexCount();
    if (startInput < 1 || startInput > V) startInput = 1;
    int start = startInput - 1;

    // Run comparison (also exports ../data/compare_stats.json)
    auto report = comparison->comparePerformance(start, AlgorithmType::BOTH);
    if (report.metrics.size() < 3) {
        clearLog();
        m_Main_ContentStack.set_visible_child("log");
        logMessageColored("✗ Không đủ dữ liệu để so sánh (cần 3 thuật toán).", "red");
        return;
    }

    const auto& d = report.metrics[0];
    const auto& b = report.metrics[1];
    const auto& m = report.metrics[2];

    auto pickMinMaxIndex = [](long long a, long long bValue, long long c, int& minIdx, int& maxIdx) {
        minIdx = 0;
        maxIdx = 0;
        long long vals[3] = {a, bValue, c};
        for (int i = 1; i < 3; ++i) {
            if (vals[i] < vals[minIdx]) minIdx = i;
            if (vals[i] > vals[maxIdx]) maxIdx = i;
        }
    };

    // Dọn section compare cũ
    for (auto* child : m_Page_Compare.get_children()) {
        m_Page_Compare.remove(*child);
    }
    auto* compareGrid = Gtk::manage(new Gtk::Grid());
    compareGrid->set_row_spacing(1);
    compareGrid->set_column_spacing(1);
    compareGrid->set_column_homogeneous(true);
    compareGrid->set_halign(Gtk::ALIGN_FILL);
    compareGrid->set_valign(Gtk::ALIGN_FILL);
    compareGrid->get_style_context()->add_class("compare-table");

    int timeMin = 0, timeMax = 0;
    int memMin = 0, memMax = 0;
    pickMinMaxIndex(d.executionTimeUs, b.executionTimeUs, m.executionTimeUs, timeMin, timeMax);
    pickMinMaxIndex(d.memoryUsageBytes, b.memoryUsageBytes, m.memoryUsageBytes, memMin, memMax);

    auto badgeMarkup = [](const std::string& text, const std::string& bg, const std::string& fg) {
        return "<span background='" + bg + "' foreground='" + fg + "'><b>  "
            + Glib::Markup::escape_text(text) + "  </b></span>";
    };

    auto makeSectionLabel = [&](const std::string& text, const std::string& className, Gtk::Align align) -> Gtk::Label* {
        auto* lbl = Gtk::manage(new Gtk::Label());
        lbl->set_text(text);
        lbl->set_halign(align);
        lbl->set_xalign(align == Gtk::ALIGN_CENTER ? 0.5f : 0.0f);
        lbl->set_line_wrap(true);
        lbl->get_style_context()->add_class(className);
        return lbl;
    };

    auto makeTableLabel = [&](const std::string& markup, bool isHeader, float xalign = 0.5f) -> Gtk::Label* {
        auto* lbl = Gtk::manage(new Gtk::Label());
        lbl->set_use_markup(true);
        lbl->set_markup(markup);
        lbl->set_hexpand(true);
        lbl->set_halign(Gtk::ALIGN_FILL);
        lbl->set_valign(Gtk::ALIGN_FILL);
        lbl->set_xalign(xalign);
        lbl->set_yalign(0.5f);
        lbl->set_line_wrap(true);
        if (isHeader) {
            lbl->get_style_context()->add_class("compare-header");
        } else {
            lbl->get_style_context()->add_class("compare-cell");
        }
        return lbl;
    };

    auto makeMetricCell = [&](long long value,
                              const std::string& unit,
                              const std::string& valueColor,
                              bool addBar,
                              double fraction,
                              const std::string& barClass,
                              const std::string& badgeClass,
                              const std::string& badgeText) -> Gtk::Box* {
        auto* container = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 6));
        container->set_margin_top(6);
        container->set_margin_bottom(6);
        container->set_margin_left(8);
        container->set_margin_right(8);

        if (addBar) {
            auto* progress = Gtk::manage(new Gtk::ProgressBar());
            progress->set_fraction(std::max(0.0, std::min(1.0, fraction)));
            progress->set_show_text(false);
            progress->set_halign(Gtk::ALIGN_FILL);
            progress->set_hexpand(true);
            progress->set_size_request(-1, 8);
            progress->get_style_context()->add_class(barClass);
            container->pack_start(*progress, false, false, 0);
        }

        auto* valueRow = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 6));
        auto* valueLabel = Gtk::manage(new Gtk::Label());
        valueLabel->set_use_markup(true);
        valueLabel->set_markup(
            "<span foreground='" + valueColor + "' size='large'><b>"
            + std::to_string(value) + " " + Glib::Markup::escape_text(unit) + "</b></span>"
        );
        valueLabel->set_halign(Gtk::ALIGN_START);
        valueLabel->set_xalign(0.0f);
        valueRow->pack_start(*valueLabel, false, false, 0);

        if (!badgeText.empty()) {
            auto* badge = Gtk::manage(new Gtk::Label(badgeText));
            badge->set_halign(Gtk::ALIGN_START);
            badge->set_xalign(0.0f);
            badge->get_style_context()->add_class(badgeClass);
            valueRow->pack_start(*badge, false, false, 0);
        }

        container->pack_start(*valueRow, false, false, 0);
        return container;
    };

    auto makeCard = [&](const std::string& title,
                        const std::string& topBadge,
                        const std::string& topBadgeBg,
                        const std::string& topBadgeFg,
                        long long timeUs,
                        const std::string& timeColor,
                        long long memoryBytes,
                        const std::string& cardClass,
                        const std::string& bottomBadgeClass,
                        const std::string& bottomBadgeText) -> Gtk::Frame* {
        auto* frame = Gtk::manage(new Gtk::Frame());
        frame->set_shadow_type(Gtk::SHADOW_NONE);
        frame->get_style_context()->add_class("compare-card");
        frame->get_style_context()->add_class(cardClass);

        auto* box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 8));
        box->set_margin_top(12);
        box->set_margin_bottom(12);
        box->set_margin_left(12);
        box->set_margin_right(12);

        auto* titleLabel = Gtk::manage(new Gtk::Label(title));
        titleLabel->set_halign(Gtk::ALIGN_CENTER);
        titleLabel->set_xalign(0.5f);
        titleLabel->get_style_context()->add_class("compare-card-title");

        auto* badgeLabel = Gtk::manage(new Gtk::Label());
        badgeLabel->set_use_markup(true);
        badgeLabel->set_markup(badgeMarkup(topBadge, topBadgeBg, topBadgeFg));
        badgeLabel->set_halign(Gtk::ALIGN_CENTER);
        badgeLabel->set_xalign(0.5f);
        badgeLabel->get_style_context()->add_class("compare-card-badge");

        auto* timeLabel = Gtk::manage(new Gtk::Label());
        timeLabel->set_use_markup(true);
        timeLabel->set_markup(
            "<span foreground='" + timeColor + "'><b>" + std::to_string(timeUs) + " µs</b></span>"
        );
        timeLabel->set_halign(Gtk::ALIGN_CENTER);
        timeLabel->set_xalign(0.5f);
        timeLabel->get_style_context()->add_class("compare-card-time");

        auto* memLabel = Gtk::manage(new Gtk::Label(std::to_string(memoryBytes) + " B"));
        memLabel->set_halign(Gtk::ALIGN_CENTER);
        memLabel->set_xalign(0.5f);
        memLabel->get_style_context()->add_class("compare-card-mem");

        box->pack_start(*titleLabel, false, false, 0);
        box->pack_start(*badgeLabel, false, false, 0);
        box->pack_start(*timeLabel, false, false, 0);
        if (!bottomBadgeText.empty()) {
            auto* bottomBadge = Gtk::manage(new Gtk::Label(bottomBadgeText));
            bottomBadge->set_halign(Gtk::ALIGN_CENTER);
            bottomBadge->set_xalign(0.5f);
            bottomBadge->get_style_context()->add_class(bottomBadgeClass);
            box->pack_start(*bottomBadge, false, false, 0);
        }
        box->pack_start(*memLabel, false, false, 0);

        frame->add(*box);
        return frame;
    };

    auto* root = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 10));
    root->set_halign(Gtk::ALIGN_FILL);
    root->set_valign(Gtk::ALIGN_FILL);
    root->set_hexpand(true);
    root->set_vexpand(true);

    root->pack_start(*makeSectionLabel("So sánh thuật toán", "compare-title", Gtk::ALIGN_CENTER), false, false, 0);
    root->pack_start(
        *makeSectionLabel(
            "Đánh giá hiệu năng và đặc điểm của các thuật toán trên cùng bộ dữ liệu",
            "compare-subtitle",
            Gtk::ALIGN_CENTER
        ),
        false, false, 0
    );

    auto* infoWrap = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 8));
    infoWrap->set_halign(Gtk::ALIGN_CENTER);
    infoWrap->get_style_context()->add_class("compare-info");

    auto* infoIcon = Gtk::manage(new Gtk::Label());
    infoIcon->set_use_markup(true);
    infoIcon->set_markup("<span foreground='#0EA5A1'><b>●</b></span>");
    infoIcon->set_halign(Gtk::ALIGN_START);

    const std::string graphType = m_isDirected ? "Đồ thị có hướng" : "Đồ thị vô hướng";
    const std::string negativeInfo = graph.hasNegativeWeights() ? "Có cạnh âm" : "Không có cạnh âm";
    auto* infoText = Gtk::manage(new Gtk::Label(
        "Dữ liệu thử nghiệm: " + graphType + " | " + std::to_string(graph.getVertexCount())
        + " đỉnh | " + negativeInfo
    ));
    infoText->set_halign(Gtk::ALIGN_START);
    infoText->set_xalign(0.0f);
    infoText->get_style_context()->add_class("compare-info-text");

    infoWrap->pack_start(*infoIcon, false, false, 0);
    infoWrap->pack_start(*infoText, false, false, 0);
    root->pack_start(*infoWrap, false, false, 2);

    auto* cardRow = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 12));
    cardRow->set_homogeneous(true);
    cardRow->set_halign(Gtk::ALIGN_FILL);

    cardRow->pack_start(
        *makeCard(
            "Dijkstra",
            "🏆 Nhanh nhất",
            "#E7F7EC",
            "#1D7D4F",
            d.executionTimeUs,
            "#1D7D4F",
            d.memoryUsageBytes,
            "card-dijkstra",
            (timeMin == 0 ? "compare-badge-best" : ""),
            (timeMin == 0 ? "Tốt nhất" : "")
        ),
        true, true, 0
    );
    cardRow->pack_start(
        *makeCard(
            "Bellman-Ford",
            "✓ Hỗ trợ cạnh âm",
            "#FDF0D9",
            "#B26A05",
            b.executionTimeUs,
            "#B26A05",
            b.memoryUsageBytes,
            "card-bellman",
            "",
            ""
        ),
        true, true, 0
    );
    cardRow->pack_start(
        *makeCard(
            "BMSSP",
            "⌬ Chiến lược mới",
            "#FBEAEA",
            "#C23B37",
            m.executionTimeUs,
            "#C23B37",
            m.memoryUsageBytes,
            "card-bmssp",
            (timeMax == 2 ? "compare-badge-worst" : ""),
            (timeMax == 2 ? "Cao nhất" : "")
        ),
        true, true, 0
    );
    root->pack_start(*cardRow, false, false, 0);

    auto* tabs = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 8));
    tabs->get_style_context()->add_class("compare-tabs");
    auto* tabBang = Gtk::manage(new Gtk::Button("Bảng"));
    auto* tabTime = Gtk::manage(new Gtk::Button("Biểu đồ thời gian"));
    auto* tabMemory = Gtk::manage(new Gtk::Button("Biểu đồ bộ nhớ"));
    auto* tabRemark = Gtk::manage(new Gtk::Button("Nhận xét"));

    tabBang->get_style_context()->add_class("compare-tab-active");
    tabTime->get_style_context()->add_class("compare-tab");
    tabMemory->get_style_context()->add_class("compare-tab");
    tabRemark->get_style_context()->add_class("compare-tab");
    tabBang->set_sensitive(false);

    tabs->pack_start(*tabBang, false, false, 0);
    tabs->pack_start(*tabTime, false, false, 0);
    tabs->pack_start(*tabMemory, false, false, 0);
    tabs->pack_start(*tabRemark, false, false, 0);
    root->pack_start(*tabs, false, false, 0);

    auto* tableFrame = Gtk::manage(new Gtk::Frame());
    tableFrame->set_shadow_type(Gtk::SHADOW_NONE);
    tableFrame->get_style_context()->add_class("compare-table-wrap");

    auto* tableWrap = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 0));
    tableWrap->set_margin_top(4);
    tableWrap->set_margin_bottom(4);
    tableWrap->set_margin_left(4);
    tableWrap->set_margin_right(4);

    // Header row
    compareGrid->attach(*makeTableLabel("<b>Tiêu chí</b>", true),      0, 0, 1, 1);
    compareGrid->attach(*makeTableLabel("<b>Dijkstra</b>", true),      1, 0, 1, 1);
    compareGrid->attach(*makeTableLabel("<b>Bellman-Ford</b>", true),  2, 0, 1, 1);
    compareGrid->attach(*makeTableLabel("<b>BMSSP</b>", true),         3, 0, 1, 1);

    // Time row
    const long long maxTime = std::max({d.executionTimeUs, b.executionTimeUs, m.executionTimeUs, 1LL});
    compareGrid->attach(*makeTableLabel("<b>Thời gian</b>", false, 0.06f), 0, 1, 1, 1);
    compareGrid->attach(
        *makeMetricCell(
            d.executionTimeUs, "µs", "#1D7D4F", true,
            static_cast<double>(d.executionTimeUs) / static_cast<double>(maxTime),
            "compare-bar-green",
            (timeMin == 0 ? "compare-badge-best" : ""),
            (timeMin == 0 ? "Tốt nhất" : "")
        ),
        1, 1, 1, 1
    );
    compareGrid->attach(
        *makeMetricCell(
            b.executionTimeUs, "µs", "#B26A05", true,
            static_cast<double>(b.executionTimeUs) / static_cast<double>(maxTime),
            "compare-bar-orange",
            "",
            ""
        ),
        2, 1, 1, 1
    );
    compareGrid->attach(
        *makeMetricCell(
            m.executionTimeUs, "µs", "#C23B37", true,
            static_cast<double>(m.executionTimeUs) / static_cast<double>(maxTime),
            "compare-bar-red",
            (timeMax == 2 ? "compare-badge-worst" : ""),
            (timeMax == 2 ? "Cao nhất" : "")
        ),
        3, 1, 1, 1
    );

    // Memory row
    compareGrid->attach(*makeTableLabel("<b>Bộ nhớ</b>", false, 0.06f), 0, 2, 1, 1);
    compareGrid->attach(
        *makeMetricCell(
            d.memoryUsageBytes, "B", "#1D7D4F", false, 0.0, "",
            (memMin == 0 ? "compare-badge-low" : ""),
            (memMin == 0 ? "Thấp nhất" : "")
        ),
        1, 2, 1, 1
    );
    compareGrid->attach(
        *makeMetricCell(
            b.memoryUsageBytes, "B", "#334155", false, 0.0, "",
            "",
            ""
        ),
        2, 2, 1, 1
    );
    compareGrid->attach(
        *makeMetricCell(
            m.memoryUsageBytes, "B", "#B45309", false, 0.0, "",
            "",
            ""
        ),
        3, 2, 1, 1
    );

    // Complexity row
    compareGrid->attach(*makeTableLabel("<b>Độ phức tạp</b>", false, 0.06f), 0, 3, 1, 1);
    compareGrid->attach(*makeTableLabel("O(E log V)", false), 1, 3, 1, 1);
    compareGrid->attach(*makeTableLabel("O(V × E)", false), 2, 3, 1, 1);
    compareGrid->attach(*makeTableLabel("BMSSP 2025", false), 3, 3, 1, 1);

    // Status row
    auto statusMarkup = [](bool success) {
        if (success) {
            return std::string("<span foreground='#1F9D63'><b>✔ Thành công</b></span>");
        }
        return std::string("<span foreground='#DC2626'><b>✖ Thất bại</b></span>");
    };
    compareGrid->attach(*makeTableLabel("<b>Trạng thái</b>", false, 0.06f), 0, 4, 1, 1);
    compareGrid->attach(*makeTableLabel(statusMarkup(d.success), false), 1, 4, 1, 1);
    compareGrid->attach(*makeTableLabel(statusMarkup(b.success), false), 2, 4, 1, 1);
    compareGrid->attach(*makeTableLabel(statusMarkup(m.success), false), 3, 4, 1, 1);

    tableWrap->pack_start(*compareGrid, false, false, 0);
    tableFrame->add(*tableWrap);
    root->pack_start(*tableFrame, false, false, 0);

    auto* conclusion = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 5));
    conclusion->get_style_context()->add_class("compare-conclusion");
    auto* conclusionTitle = makeSectionLabel("Kết luận nhanh", "compare-conclusion-title", Gtk::ALIGN_START);
    auto* line1 = makeSectionLabel("• Dijkstra cho kết quả nhanh nhất trong bộ test này.", "compare-conclusion-text", Gtk::ALIGN_START);
    auto* line2 = makeSectionLabel("• Bellman-Ford chậm hơn nhưng phù hợp khi đồ thị có cạnh âm.", "compare-conclusion-text", Gtk::ALIGN_START);
    auto* line3 = makeSectionLabel("• BMSSP tiêu tốn bộ nhớ cao hơn, phù hợp để nghiên cứu hoặc so sánh thêm.", "compare-conclusion-text", Gtk::ALIGN_START);
    conclusion->pack_start(*conclusionTitle, false, false, 0);
    conclusion->pack_start(*line1, false, false, 0);
    conclusion->pack_start(*line2, false, false, 0);
    conclusion->pack_start(*line3, false, false, 0);
    root->pack_start(*conclusion, false, false, 2);

    m_Page_Compare.pack_start(*root, true, true, 0);

    m_Main_ContentStack.set_visible_child("compare");
    show_all_children();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Button: Visualize
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::on_button_visualize() {
    hideInlineVisualizePrompt();
    switchToMain();

    if (!graph.isValid()) {
        Gtk::MessageDialog dlg(*this, "Chưa tải đồ thị", false, Gtk::MESSAGE_WARNING);
        dlg.set_secondary_text("Vui lòng tải đồ thị và chạy thuật toán trước.");
        dlg.run();
        return;
    }

    // Nếu chưa chạy thuật toán → tự động chạy Dijkstra
    if (m_lastResult.startVertex == -1) {
        logMessageColored("→ Chưa chạy thuật toán. Tự động chạy Dijkstra...", "yellow");
        on_button_run_dijkstra();
        if (m_lastResult.startVertex == -1) return;
    }

    // Chỉ dùng visualizer native C++ trong ứng dụng hiện tại.
    logMessageColored("→ Đang mở chế độ trực quan hóa native...", "blue");
    m_Stack.set_visible_child("visualizer");
    m_GraphCanvas.loadData(graph, m_lastResult, m_isDirected);
    m_GraphCanvas.playAnimation();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Button: Exit
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::on_button_exit() {
    hide();
}
