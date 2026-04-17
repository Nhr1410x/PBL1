#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtkmm.h>
#include "Global.h"
#include "AdvancedAlgorithms.h"
#include "graph.h"
#include "algorithms.h"
#include "comparison.h"
#include "GraphCanvas.h"

class MainWindow : public Gtk::ApplicationWindow {
public:
    MainWindow(Graph& g, Algorithms* alg, AdvancedAlgorithms* adv, Comparison* comp);
    virtual ~MainWindow();

protected:
    // ── Signal handlers ────────────────────────────────────────────
    void on_button_load_graph();
    void on_button_run_dijkstra();
    void on_button_run_bellman();
    void on_button_run_advanced();
    void on_button_compare();
    void on_button_visualize();
    void on_button_exit();
    void on_toolbar_run_clicked();
    void on_toolbar_step_clicked();
    void on_toolbar_clear_log_clicked();
    void on_toolbar_log_level_changed();

    // ── Helpers ────────────────────────────────────────────────────
    void logMessage(const std::string& msg);
    void logMessageColored(const std::string& msg, const std::string& colorTag = "");
    void logHeader(const std::string& msg);  // căn giữa + bold + dark blue
    void logInfo(const std::string& msg);    // căn giữa + xám đậm
    void appendStructuredLog(const std::string& level, const std::string& message);
    void clearLog();
    void runAlgorithm(AlgorithmType type, bool stepMode = false);
    void updateGraphSummary();
    void switchToMain();   // Chuyển từ Welcome sang Main page
    void setupCss();       // Load dark theme CSS
    void buildLoadGraphPage();
    void buildWelcomePage();
    void buildMainPage();
    void buildVisualizerPage();
    void populateSampleSimulationData();
    void refreshVertexStatusTable(const PathResult& result, int startVertex, bool stepMode);
    void updateFinalResultPanel(
        AlgorithmType type,
        int startVertex,
        int endVertex,
        const PathResult& result,
        long long elapsedUs
    );
    void rebuildAlgorithmLogView(
        const PathResult& result,
        AlgorithmType type,
        int startVertex,
        int endVertex,
        long long elapsedUs,
        bool stepMode
    );
    void resetFinalResultPanel();
    AlgorithmType getSelectedAlgorithm() const;
    void syncAlgorithmSelection(AlgorithmType type);
    std::string getAlgorithmLabel(AlgorithmType type) const;
    void showLoadGraphPage();
    void updateLoadGraphInputMode();
    void resetLoadGraphForm();
    void clearLoadGraphStatus();
    void setLoadGraphStatus(const std::string& message, const std::string& colorHex);
    void hideInlineVisualizePrompt();
    void finalizeGraphLoad(const std::string& sourceLabel, bool isDirected);
    void updateLoadedSummary(const std::string& sourceLabel, bool isDirected);
    std::string getExeDir(); // Lấy thư mục chứa exe (để mở đúng thư mục dữ liệu mặc định)

    // ── Helpers cho luồng Tải Đồ Thị ──────────────────────────────────────
    // In tóm tắt đồ thị (số đỉnh, cạnh, danh sách cạnh) ra Right Panel
    void printGraphSummaryToLog(bool isDirected);
    void on_button_browse_graph_file();
    void on_button_submit_load_graph();
    void on_button_reset_load_graph_form();
    void on_button_visualize_now_inline();
    void on_button_visualize_later_inline();
    void on_button_loaded_summary_run();
    void on_button_loaded_summary_visualize();

    // ── CSS Provider ───────────────────────────────────────────────
    Glib::RefPtr<Gtk::CssProvider> m_CssProvider;

    // ── Root Layout ────────────────────────────────────────────────
    Gtk::Box     m_Root;          // VERTICAL: header + body
    Gtk::Box     m_Header;        // HORIZONTAL: header bar
    Gtk::Label   m_Header_Title;
    Gtk::Box     m_Body;          // HORIZONTAL: sidebar + stack

    // ── Sidebar (Left Panel) ───────────────────────────────────────
    Gtk::Box     m_Sidebar;
    Gtk::Label   m_Sidebar_Title;
    Gtk::Separator m_Sep1;
    Gtk::Button  m_Button_LoadGraph;
    Gtk::Box     m_Input_Row;     // [Đỉnh Start] [Đỉnh End] cùng dòng
    Gtk::Label   m_Label_Start;
    Gtk::Entry   m_Entry_Start;
    Gtk::Label   m_Label_End;
    Gtk::Entry   m_Entry_End;
    Gtk::Button  m_Button_Dijkstra;
    Gtk::Button  m_Button_Bellman;
    Gtk::Button  m_Button_Advanced;
    Gtk::Button  m_Button_Compare;
    Gtk::Button  m_Button_Visualize;
    Gtk::Separator m_Sep2;
    Gtk::Box     m_GraphInfo_Box;
    Gtk::Label   m_Label_GraphInfo;
    Gtk::Label   m_Label_GraphStats;
    Gtk::Button  m_Button_Exit;

    // ── Content Area (Right) → Gtk::Stack ─────────────────────────
    Gtk::Stack   m_Stack;

    // Page 1: Welcome
    Gtk::Box        m_Page_Welcome;
    Gtk::Label      m_Wlc_University;
    Gtk::Label      m_Wlc_Faculty;
    Gtk::Separator  m_Wlc_Sep;
    Gtk::Label      m_Wlc_ProjectLabel;
    Gtk::Label      m_Wlc_ProjectName;
    Gtk::Separator  m_Wlc_Sep2;
    Gtk::Grid       m_Wlc_InfoGrid;
    Gtk::Label      m_Wlc_SV_Label;
    Gtk::Label      m_Wlc_SV_Name1;
    Gtk::Label      m_Wlc_SV_Name2;
    Gtk::Label      m_Wlc_GVHD_Label;
    Gtk::Label      m_Wlc_GVHD_Name;
    Gtk::Label      m_Wlc_Hint;

    // Page 2: Load Graph
    Gtk::Box        m_Page_LoadGraph;
    Gtk::Box        m_Load_Panel;
    Gtk::Label      m_Load_Title;
    Gtk::Label      m_Load_Subtitle;
    Gtk::Box        m_Load_MethodRow;
    Gtk::RadioButton m_Load_Mode_File;
    Gtk::RadioButton m_Load_Mode_Direct;
    Gtk::Stack      m_Load_FormStack;
    Gtk::Box        m_Load_FileForm;
    Gtk::CheckButton m_Load_FileDirected;
    Gtk::Label      m_Load_FileHint;
    Gtk::Box        m_Load_FilePathRow;
    Gtk::Entry      m_Load_FilePath;
    Gtk::Button     m_Load_FileBrowse;
    Gtk::Box        m_Load_DirectForm;
    Gtk::CheckButton m_Load_DirectDirected;
    Gtk::Box        m_Load_VertexRow;
    Gtk::Label      m_Load_VertexLabel;
    Gtk::Entry      m_Load_VertexEntry;
    Gtk::Label      m_Load_EdgesHint;
    Gtk::ScrolledWindow m_Load_EdgesScroll;
    Gtk::TextView   m_Load_EdgesTextView;
    Glib::RefPtr<Gtk::TextBuffer> m_Load_EdgesBuffer;
    Gtk::Label      m_Load_Status;
    Gtk::Box        m_Load_ActionRow;
    Gtk::Button     m_Load_Confirm;
    Gtk::Button     m_Load_Reset;

    class VertexStatusColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        VertexStatusColumns() {
            add(colVertex);
            add(colDistance);
            add(colParent);
            add(colStatus);
            add(colStatusMarkup);
        }

        Gtk::TreeModelColumn<Glib::ustring> colVertex;
        Gtk::TreeModelColumn<Glib::ustring> colDistance;
        Gtk::TreeModelColumn<Glib::ustring> colParent;
        Gtk::TreeModelColumn<Glib::ustring> colStatus;
        Gtk::TreeModelColumn<Glib::ustring> colStatusMarkup;
    };

    // Page 3: Main (Log output)
    Gtk::Box             m_Page_Main;
    Gtk::Revealer        m_Main_InlinePrompt_Revealer;
    Gtk::Box             m_Main_InlinePrompt;
    Gtk::Label           m_Main_InlinePrompt_Label;
    Gtk::Button          m_Main_InlinePrompt_Visualize;
    Gtk::Button          m_Main_InlinePrompt_Dismiss;
    Gtk::Stack           m_Main_ContentStack;     // loaded_summary | log | compare

    // Main/Loaded Summary view
    Gtk::Box             m_Page_LoadedSummary;
    Gtk::Box             m_LoadedSummaryCard;
    Gtk::Label           m_LoadedSummaryTitle;
    Gtk::Label           m_LoadedSummaryStatus;
    Gtk::Label           m_LoadedSummarySource;
    Gtk::Label           m_LoadedSummaryMeta;
    Gtk::Box             m_LoadedSummaryPreviewBox;
    Gtk::Label           m_LoadedSummaryPreviewTitle;
    Gtk::Label           m_LoadedSummaryEdgePreview;
    Gtk::Box             m_LoadedSummaryActionRow;
    Gtk::Button          m_LoadedSummaryRun;
    Gtk::Button          m_LoadedSummaryVisualize;

    // Main/Simulation module view
    Gtk::Box             m_Page_Simulation;
    Gtk::Frame           m_Sim_ControlFrame;
    Gtk::Box             m_Sim_ControlRow;
    Gtk::Label           m_Sim_AlgoLabel;
    Gtk::ComboBoxText    m_Sim_AlgoCombo;
    Gtk::Button          m_Sim_RunButton;
    Gtk::Button          m_Sim_StepButton;
    Gtk::Button          m_Sim_ClearLogButton;
    Gtk::ComboBoxText    m_Sim_LogLevelCombo;
    Gtk::Button          m_Sim_SettingsButton;

    Gtk::Paned           m_Sim_TopSplit;
    Gtk::Frame           m_Sim_StatusFrame;
    Gtk::Box             m_Sim_StatusBox;
    Gtk::Label           m_Sim_StatusTitle;
    Gtk::ScrolledWindow  m_Sim_StatusScroll;
    Gtk::TreeView        m_Sim_StatusView;
    VertexStatusColumns  m_Sim_StatusColumns;
    Glib::RefPtr<Gtk::ListStore> m_Sim_StatusStore;

    Gtk::Frame           m_Sim_ResultFrame;
    Gtk::Box             m_Sim_ResultBox;
    Gtk::Label           m_Sim_ResultTitle;
    Gtk::Grid            m_Sim_ResultGrid;
    Gtk::Label           m_Result_Label_Algorithm;
    Gtk::Label           m_Result_Label_Source;
    Gtk::Label           m_Result_Label_Target;
    Gtk::Label           m_Result_Label_Path;
    Gtk::Label           m_Result_Label_Cost;
    Gtk::Label           m_Result_Label_Status;
    Gtk::Label           m_Result_Value_Algorithm;
    Gtk::Label           m_Result_Value_Source;
    Gtk::Label           m_Result_Value_Target;
    Gtk::Label           m_Result_Value_Path;
    Gtk::Label           m_Result_Value_Cost;
    Gtk::Label           m_Result_Value_Status;

    Gtk::Frame           m_Sim_LogFrame;
    Gtk::Box             m_Sim_LogBox;
    Gtk::Label           m_Sim_LogTitle;

    // Main/Log view
    Gtk::ScrolledWindow  m_ScrolledWindow_Log;
    Gtk::TextView        m_TextView_Log;
    Glib::RefPtr<Gtk::TextBuffer> m_TextBuffer_Log;

    // Main/Compare view
    Gtk::Box             m_Page_Compare;
    Gtk::Grid            m_Compare_Grid;
    Gtk::Button          m_Button_CompareChart;

    // Page 4: Visualizer Native
    Gtk::Box             m_Page_Visualizer;
    GraphCanvas          m_GraphCanvas;

    // ── Data references ────────────────────────────────────────────
    Graph&       graph;
    Algorithms*  algorithms;
    AdvancedAlgorithms* advancedAlgorithms;
    Comparison*  comparison;

    // ── State ──────────────────────────────────────────────────────
    PathResult    m_lastResult;
    AlgorithmType m_lastAlgoType;
    int           m_lastEndVertex;
    long long     m_lastElapsedUs;
    bool          m_lastRunStepMode;
    bool          m_welcomeShown;
    bool          m_isDirected;   // true = có hướng; dùng chung cho visualize & summary
};

#endif // MAINWINDOW_H
