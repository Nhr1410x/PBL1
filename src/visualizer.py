"""
visualizer.py — Animation từng bước cho Dijkstra / Bellman-Ford
Đọc file trace.json được xuất bởi C++, hiển thị animation với FuncAnimation.

Màu sắc theo trạng thái:
  node
    gray        → chưa xét
    #FF8C00     → đang Visit (cam đậm)
    #4FC3F7     → đã visited xong (xanh nhạt)
    #FFD700     → đỉnh bắt đầu (vàng)
    #EF5350     → đỉnh đích   (đỏ nhạt)
    #66BB6A     → trên đường đi ngắn nhất (xanh lá)
  edge
    #B0B0B0     → chưa xét (xám)
    #FF3333     → đang Relax (đỏ tươi)
    #66BB6A     → trên đường đi ngắn nhất (xanh lá)
    #FF1744     → chu trình âm   (đỏ thẫm)
"""

import json
import os
import sys

import networkx as nx
import matplotlib
matplotlib.use("TkAgg")          # backend an toàn trên Windows; đổi sang "Qt5Agg" nếu cần
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.animation import FuncAnimation

# ─────────────────────────────────────────
#  Đường dẫn file
# ─────────────────────────────────────────
BASE_DIR   = os.path.dirname(os.path.abspath(__file__))
TRACE_FILE = os.path.normpath(os.path.join(BASE_DIR, "..", "data", "trace.json"))

# ─────────────────────────────────────────
#  Màu sắc
# ─────────────────────────────────────────
C_NODE_DEFAULT  = "#90A4AE"   # xám xanh — chưa xét
C_NODE_VISIT    = "#FF8C00"   # cam — đang visit
C_NODE_DONE     = "#4FC3F7"   # xanh nhạt — đã xử lý xong
C_NODE_START    = "#FFD700"   # vàng — đỉnh bắt đầu
C_NODE_END      = "#EF5350"   # đỏ nhạt — đỉnh đích
C_NODE_PATH     = "#66BB6A"   # xanh lá — đường đi cuối

C_EDGE_DEFAULT  = "#B0B0B0"   # xám — chưa xét
C_EDGE_RELAX    = "#FF3333"   # đỏ — đang relax
C_EDGE_PATH     = "#66BB6A"   # xanh lá — đường đi cuối
C_EDGE_NEG      = "#FF1744"   # đỏ thẫm — chu trình âm

INTERVAL_MS     = 900         # thời gian mỗi frame (ms)
NODE_SIZE       = 1400
FONT_SIZE_NODE  = 10
FONT_SIZE_LABEL = 8


# ─────────────────────────────────────────
#  Đọc trace.json
# ─────────────────────────────────────────
def load_trace(path: str) -> dict:
    if not os.path.exists(path):
        print(f"[ERROR] Khong tim thay file: {path}")
        sys.exit(1)
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


# ─────────────────────────────────────────
#  Xây dựng đồ thị NetworkX
# ─────────────────────────────────────────
def build_graph(trace: dict):
    G = nx.DiGraph()
    for n in trace["nodes"]:
        G.add_node(n["id"], label=n["label"])
    for e in trace["edges"]:
        G.add_edge(e["source"], e["target"], weight=e["weight"])
    return G


# ─────────────────────────────────────────
#  Hàm tính nhãn khoảng cách (d[v])
#  Mỗi frame cần biết d[v] tạm thời
# ─────────────────────────────────────────
def compute_dist_snapshots(trace: dict):
    """
    Trả về list: mỗi phần tử là dict {node_id: dist_value}
    tại thời điểm sau khi xử lý frame đó.
    Index 0 → trạng thái khởi tạo (start=0, còn lại INF).
    """
    INF = float("inf")
    n_nodes  = len(trace["nodes"])
    start_v  = trace["start"]

    cur = {i: INF for i in range(n_nodes)}
    cur[start_v] = 0

    snapshots = [dict(cur)]          # frame 0 = init
    for step in trace["steps"]:
        if step["type"] == "relax":
            to_v      = step["to"]
            new_d     = step["new_dist"]
            cur[to_v] = new_d
        snapshots.append(dict(cur))  # frame sau mỗi step
    return snapshots


# ─────────────────────────────────────────
#  Main
# ─────────────────────────────────────────
def main():
    trace    = load_trace(TRACE_FILE)
    G        = build_graph(trace)
    steps    = trace.get("steps", [])
    start_v  = trace["start"]
    end_v    = trace["end"]
    sp       = trace.get("shortest_path", [])
    sp_edges = set(zip(sp, sp[1:])) if len(sp) > 1 else set()

    algo_name = trace.get("algorithm", "dijkstra").upper().replace("_", "-")

    # Layout cố định (seed cố định → tái lập được)
    pos = nx.spring_layout(G, k=2.2, iterations=60, seed=42)

    # Node labels (hiển thị tên đỉnh)
    node_labels  = {n["id"]: n["label"] for n in trace["nodes"]}

    # Snapshots khoảng cách
    dist_snap = compute_dist_snapshots(trace)

    # ── Figure ──────────────────────────────
    fig, ax = plt.subplots(figsize=(13, 8))
    fig.patch.set_facecolor("#1E1E2E")
    ax.set_facecolor("#1E1E2E")
    plt.tight_layout(pad=2)

    total_frames = len(steps) + 2   # 1 frame init + N steps + 1 frame kết thúc

    # State theo dõi
    state = {
        "visited": set(),            # các đỉnh đã visited xong
        "visiting": -1,              # đỉnh đang visit (frame hiện tại)
        "relaxing_edge": None,       # (u, v) đang relax
        "neg_edges": set(),          # các cạnh chu trình âm
        "done": False,               # animation kết thúc
    }

    def _node_color(n):
        if state["done"]:
            if n in sp:
                return C_NODE_PATH
            if n == start_v:
                return C_NODE_START
            if n == end_v:
                return C_NODE_END
        if n == state["visiting"]:
            return C_NODE_VISIT
        if n in state["visited"]:
            if n == start_v:
                return C_NODE_START
            if n == end_v:
                return C_NODE_END
            return C_NODE_DONE
        if n == start_v:
            return C_NODE_START
        return C_NODE_DEFAULT

    def _edge_color(u, v):
        if state["done"]:
            if (u, v) in sp_edges:
                return C_EDGE_PATH
        if state["relaxing_edge"] == (u, v):
            return C_EDGE_RELAX
        if (u, v) in state["neg_edges"]:
            return C_EDGE_NEG
        return C_EDGE_DEFAULT

    def _edge_width(u, v):
        if state["done"] and (u, v) in sp_edges:
            return 4.5
        if state["relaxing_edge"] == (u, v):
            return 3.5
        return 1.5

    def draw_frame(frame_idx, dist_map, title):
        ax.clear()
        ax.set_facecolor("#1E1E2E")
        ax.axis("off")

        node_colors = [_node_color(n) for n in G.nodes()]
        edge_colors = [_edge_color(u, v) for u, v in G.edges()]
        edge_widths = [_edge_width(u, v) for u, v in G.edges()]

        # Vẽ cạnh
        nx.draw_networkx_edges(
            G, pos, ax=ax,
            edge_color=edge_colors,
            width=edge_widths,
            arrows=True,
            arrowsize=18,
            arrowstyle="->",
            connectionstyle="arc3,rad=0.08",
            min_source_margin=22,
            min_target_margin=22,
        )

        # Nhãn trọng số cạnh
        edge_weight_labels = nx.get_edge_attributes(G, "weight")
        nx.draw_networkx_edge_labels(
            G, pos, ax=ax,
            edge_labels=edge_weight_labels,
            font_size=FONT_SIZE_LABEL,
            font_color="#ECEFF4",
            bbox=dict(boxstyle="round,pad=0.2", facecolor="#2E3440", alpha=0.7),
        )

        # Vẽ node
        nx.draw_networkx_nodes(
            G, pos, ax=ax,
            node_color=node_colors,
            node_size=NODE_SIZE,
        )

        # Nhãn đỉnh (tên + d[v])
        INF = float("inf")
        combined_labels = {}
        for n in G.nodes():
            d = dist_map.get(n, INF)
            d_str = str(d) if d != INF else "INF"
            combined_labels[n] = "{}\nd={}".format(node_labels[n], d_str)

        nx.draw_networkx_labels(
            G, pos, ax=ax,
            labels=combined_labels,
            font_size=FONT_SIZE_NODE,
            font_color="#1E1E2E",
            font_weight="bold",
        )

        # Tiêu đề frame
        frame_info = "Frame {}/{}".format(frame_idx, total_frames - 1)
        ax.set_title(
            "[{}]  {}\n{}".format(algo_name, title, frame_info),
            fontsize=13,
            color="#ECEFF4",
            pad=12,
            fontweight="bold",
        )

        # Legend
        legend_items = [
            mpatches.Patch(color=C_NODE_START,   label="Dinh bat dau"),
            mpatches.Patch(color=C_NODE_END,     label="Dinh dich"),
            mpatches.Patch(color=C_NODE_VISIT,   label="Dang xet (Visit)"),
            mpatches.Patch(color=C_NODE_DONE,    label="Da xu ly xong"),
            mpatches.Patch(color=C_NODE_DEFAULT, label="Chua xet"),
            mpatches.Patch(color=C_EDGE_RELAX,   label="Dang noi long (Relax)"),
            mpatches.Patch(color=C_EDGE_PATH,    label="Duong di ngan nhat"),
        ]
        ax.legend(
            handles=legend_items,
            loc="upper left",
            fontsize=8,
            facecolor="#2E3440",
            labelcolor="#ECEFF4",
            framealpha=0.85,
        )

    # ── FuncAnimation update ─────────────────
    def update(frame):
        if frame == 0:
            state["visited"].clear()
            state["visiting"]      = -1
            state["relaxing_edge"] = None
            state["neg_edges"].clear()
            state["done"] = False
            dist_map = dist_snap[0]
            draw_frame(frame, dist_map, "Khoi tao: tat ca khoang cach = INF, d[start] = 0")
            return

        final_frame = total_frames - 1
        if frame == final_frame or frame > len(steps):
            state["done"] = True
            state["visiting"]      = -1
            state["relaxing_edge"] = None
            dist_map = dist_snap[min(frame, len(dist_snap) - 1)]
            if sp:
                result_desc = "Ket thuc. Duong di: " + " -> ".join(node_labels[v] for v in sp)
            else:
                result_desc = "Ket thuc. Khong co duong di."
            draw_frame(frame, dist_map, result_desc)
            return

        step     = steps[frame - 1]
        dist_map = dist_snap[frame]

        if step["type"] == "visit":
            prev_visit = state["visiting"]
            if prev_visit >= 0:
                state["visited"].add(prev_visit)
            state["visiting"]      = step["node"]
            state["relaxing_edge"] = None
            title = step.get("desc", "Xet dinh {}".format(node_labels.get(step["node"], step["node"])))

        elif step["type"] == "relax":
            state["relaxing_edge"] = (step["from"], step["to"])
            title = step.get("desc", "Noi long {} -> {}".format(step["from"], step["to"]))

        elif step["type"] == "negative_cycle":
            u, v = step["from"], step["to"]
            state["neg_edges"].add((u, v))
            state["relaxing_edge"] = None
            title = step.get("desc", "Phat hien chu trinh am!")

        else:
            title = step.get("desc", "")

        draw_frame(frame, dist_map, title)

    # ── Chạy animation ──────────────────────
    anim = FuncAnimation(
        fig,
        update,
        frames=total_frames,
        interval=INTERVAL_MS,
        repeat=False,
    )

    plt.show()
    return True


if __name__ == "__main__":
    ok = main()
    sys.exit(0 if ok else 1)
