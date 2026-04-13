import argparse
import csv
import heapq
import math
import os
import re
import sys
import time

DEFAULT_CSV = os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "data", "benchmark.csv")
)

_LABEL_KEYS = {
    "label",
    "graph",
    "dataset",
    "name",
    "ten",
    "ten_do_thi",
    "do_thi",
}

_DIJKSTRA_KEYS = {
    "dijkstra",
    "dijkstra_us",
    "dijkstra_ms",
    "dijkstra_s",
    "dijkstra_time",
    "dijkstra_time_us",
    "dijkstra_time_ms",
    "dijkstra_time_s",
}

_BELLMAN_KEYS = {
    "bellman",
    "bellman_ford",
    "bellman_ford_us",
    "bellman_ford_ms",
    "bellman_ford_s",
    "bellman_time",
    "bellman_time_us",
    "bellman_time_ms",
    "bellman_time_s",
    "bellmanford",
    "bellmanford_us",
    "bellmanford_ms",
    "bellmanford_s",
}

_HEADER_KEYS = _LABEL_KEYS | _DIJKSTRA_KEYS | _BELLMAN_KEYS


def _norm_key(text):
    s = text.strip().lower().replace(" ", "_").replace("-", "_")
    s = re.sub(r"[^a-z0-9_]", "", s)
    return s


def _find_index(header, keys):
    normed = [_norm_key(h) for h in header]
    for i, h in enumerate(normed):
        if h in keys:
            return i
    return None


def _infer_unit_from_header(header_value):
    s = header_value.lower()
    if "ms" in s:
        return "ms"
    if "us" in s or "\u03bcs" in s:
        return "us"
    if "ns" in s:
        return "ns"
    if " s" in s or s.endswith("s"):
        return "s"
    return None


def _parse_float(text, context):
    s = text.strip().replace(",", "")
    if not s:
        raise ValueError(f"Missing value for {context}.")
    try:
        return float(s)
    except ValueError as exc:
        raise ValueError(f"Invalid number for {context}: {text}") from exc


def _read_csv(path):
    rows = []
    with open(path, newline="", encoding="utf-8") as handle:
        reader = csv.reader(handle)
        for row in reader:
            if not row:
                continue
            if row[0].strip().startswith("#"):
                continue
            cleaned = [c.strip() for c in row]
            if all(not c for c in cleaned):
                continue
            rows.append(cleaned)

    if not rows:
        raise ValueError("CSV file is empty.")

    header = rows[0]
    header_norm = [_norm_key(h) for h in header]
    is_header = any(h in _HEADER_KEYS for h in header_norm)

    if is_header:
        data = rows[1:]
        idx_label = _find_index(header, _LABEL_KEYS)
        idx_d = _find_index(header, _DIJKSTRA_KEYS)
        idx_b = _find_index(header, _BELLMAN_KEYS)
        if idx_d is None or idx_b is None:
            raise ValueError("CSV header missing required columns for Dijkstra/Bellman-Ford.")
        if idx_label is None:
            idx_label = 0
        inferred_unit = _infer_unit_from_header(header[idx_d]) or _infer_unit_from_header(header[idx_b])
    else:
        data = rows
        if len(data[0]) < 3:
            raise ValueError("CSV must have at least 3 columns: label,dijkstra,bellman.")
        idx_label, idx_d, idx_b = 0, 1, 2
        inferred_unit = None

    labels = []
    d_values = []
    b_values = []
    for row_idx, row in enumerate(data, start=(2 if is_header else 1)):
        if len(row) <= max(idx_label, idx_d, idx_b):
            raise ValueError(f"Row {row_idx} has too few columns.")
        label = row[idx_label] or f"Case {row_idx}"
        d_val = _parse_float(row[idx_d], f"row {row_idx} Dijkstra")
        b_val = _parse_float(row[idx_b], f"row {row_idx} Bellman-Ford")
        labels.append(label)
        d_values.append(d_val)
        b_values.append(b_val)

    return labels, d_values, b_values, inferred_unit


def _read_graph_file(path, undirected=False):
    with open(path, encoding="utf-8") as handle:
        tokens = handle.read().split()

    if not tokens:
        raise ValueError(f"Graph file is empty: {path}")

    idx = 0
    try:
        v_count = int(tokens[idx])
    except ValueError as exc:
        raise ValueError(f"Invalid vertex count in {path}.") from exc
    idx += 1

    if v_count < 1:
        raise ValueError(f"Invalid vertex count in {path}.")

    if idx + v_count > len(tokens):
        raise ValueError(f"Missing vertex labels in {path}.")
    idx += v_count

    if idx >= len(tokens):
        raise ValueError(f"Missing edge count in {path}.")
    try:
        e_count = int(tokens[idx])
    except ValueError as exc:
        raise ValueError(f"Invalid edge count in {path}.") from exc
    idx += 1

    adj = [dict() for _ in range(v_count)]
    edge_map = {}

    def add_edge(u, v, w):
        adj[u][v] = w
        edge_map[(u, v)] = w

    for _ in range(e_count):
        if idx + 2 >= len(tokens):
            raise ValueError(f"Not enough edge data in {path}.")
        try:
            u = int(tokens[idx]) - 1
            v = int(tokens[idx + 1]) - 1
            w = int(tokens[idx + 2])
        except ValueError as exc:
            raise ValueError(f"Invalid edge data in {path}.") from exc
        idx += 3

        if u < 0 or v < 0 or u >= v_count or v >= v_count:
            continue
        add_edge(u, v, w)
        if undirected:
            add_edge(v, u, w)

    adj_list = [list(neigh.items()) for neigh in adj]
    edges = [(u, v, w) for (u, v), w in edge_map.items()]
    return v_count, adj_list, edges


def _dijkstra(adj, start):
    dist = [float("inf")] * len(adj)
    dist[start] = 0.0
    pq = [(0.0, start)]
    while pq:
        d, u = heapq.heappop(pq)
        if d != dist[u]:
            continue
        for v, w in adj[u]:
            nd = d + w
            if nd < dist[v]:
                dist[v] = nd
                heapq.heappush(pq, (nd, v))
    return dist


def _bellman_ford(v_count, edges, start):
    dist = [float("inf")] * v_count
    dist[start] = 0.0
    for _ in range(v_count - 1):
        changed = False
        for u, v, w in edges:
            if dist[u] != float("inf") and dist[u] + w < dist[v]:
                dist[v] = dist[u] + w
                changed = True
        if not changed:
            break
    return dist


def _measure_seconds(fn, repeat):
    if repeat < 1:
        repeat = 1
    start = time.perf_counter_ns()
    for _ in range(repeat):
        fn()
    end = time.perf_counter_ns()
    return (end - start) / repeat / 1e9


def _convert_unit(values, unit):
    factors = {
        "s": 1.0,
        "ms": 1e3,
        "us": 1e6,
        "ns": 1e9,
    }
    factor = factors.get(unit, 1.0)
    return [v * factor for v in values]


def _benchmark_graphs(paths, start_vertex, repeat, undirected):
    labels = []
    d_values = []
    b_values = []

    for path in paths:
        v_count, adj, edges = _read_graph_file(path, undirected)
        if v_count < 1:
            raise ValueError(f"Graph has no vertices: {path}")

        start = min(max(0, start_vertex), v_count - 1)
        labels.append(os.path.splitext(os.path.basename(path))[0])

        has_negative = any(w < 0 for _, _, w in edges)
        if has_negative:
            d_time = float("nan")
        else:
            d_time = _measure_seconds(lambda: _dijkstra(adj, start), repeat)
        b_time = _measure_seconds(lambda: _bellman_ford(v_count, edges, start), repeat)
        d_values.append(d_time)
        b_values.append(b_time)

    return labels, d_values, b_values


def _format_value(value):
    abs_val = abs(value)
    if abs_val >= 1000:
        return f"{value:,.0f}"
    if abs_val >= 100:
        return f"{value:.1f}"
    if abs_val >= 1:
        return f"{value:.2f}"
    return f"{value:.3f}"


def plot_compare(labels, d_values, b_values, unit, title, out_path, show):
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("Missing dependency: matplotlib. Install with: pip install matplotlib")
        return 1

    count = len(labels)
    xs = list(range(count))
    fig_w = max(6.0, 1.2 * count)
    fig_h = 4.5

    fig, ax = plt.subplots(figsize=(fig_w, fig_h))
    ax.plot(xs, d_values, marker="o", linewidth=2.0, label="Dijkstra")
    ax.plot(xs, b_values, marker="o", linewidth=2.0, label="Bellman-Ford")

    ax.set_xticks(xs)
    ax.set_xticklabels(labels, rotation=20, ha="right")
    ax.set_ylabel(f"Thời gian ({unit})")
    ax.set_title(title)
    ax.legend()
    ax.grid(axis="y", linestyle="--", alpha=0.4)

    finite_values = [v for v in (d_values + b_values) if math.isfinite(v)]
    max_val = max(finite_values) if finite_values else 0.0
    y_offset = max_val * 0.03
    if y_offset == 0:
        y_offset = 0.01

    for x, value in zip(xs, d_values):
        if not math.isfinite(value):
            continue
        ax.text(x, value + y_offset, _format_value(value), ha="center", va="bottom", fontsize=8)
    for x, value in zip(xs, b_values):
        if not math.isfinite(value):
            continue
        ax.text(x, value + y_offset, _format_value(value), ha="center", va="bottom", fontsize=8)

    plt.tight_layout()

    if out_path:
        fig.savefig(out_path, dpi=200, bbox_inches="tight")
        print(f"Saved chart to: {out_path}")

    if show:
        plt.show()

    return 0


def main():
    parser = argparse.ArgumentParser(
        description="Compare Dijkstra vs Bellman-Ford execution time from a CSV file."
    )
    parser.add_argument("--csv", default=DEFAULT_CSV, help="Path to CSV file.")
    parser.add_argument("--graphs", nargs="*", default=None,
                        help="Graph files to benchmark (overrides CSV).")
    parser.add_argument("--graph-dir", default=None,
                        help="Directory containing graph files (e.g., data).")
    parser.add_argument("--graph-prefix", default="G",
                        help="Prefix for graph files in graph-dir (default: G).")
    parser.add_argument("--graph-count", type=int, default=0,
                        help="Number of graph files in graph-dir (e.g., 6 -> G1..G6).")
    parser.add_argument("--start-vertex", type=int, default=1,
                        help="Start vertex for timing (1-based).")
    parser.add_argument("--repeat", type=int, default=200,
                        help="Repeat count for averaging timing.")
    parser.add_argument("--undirected", action="store_true",
                        help="Treat graphs as undirected (add reverse edges).")
    parser.add_argument("--write-csv", default=None,
                        help="Write computed timings to CSV.")
    parser.add_argument("--unit", default=None, help="Unit label (us, ms, s, ns).")
    parser.add_argument("--scale", type=float, default=1.0, help="Scale factor applied to values.")
    parser.add_argument("--title", default="Biểu đồ so sánh thời gian chạy của hai thuật toán Dijkstra và Bellman-Ford và Bellman-Ford trên đồ thị vô hướng", help="Chart title.")
    parser.add_argument("--label-prefix", default=None,
                        help="Override labels using prefix + index (e.g., G -> G1..Gn).")
    parser.add_argument("--out", default=None, help="Save chart to file (png, jpg, ...).")
    parser.add_argument("--no-show", action="store_true", help="Do not open a window.")

    args = parser.parse_args()
    csv_path = args.csv
    graph_paths = None

    if args.graphs:
        graph_paths = args.graphs
    elif args.graph_dir and args.graph_count > 0:
        graph_paths = [
            os.path.join(args.graph_dir, f"{args.graph_prefix}{i}.txt")
            for i in range(1, args.graph_count + 1)
        ]

    inferred_unit = None
    if graph_paths:
        for path in graph_paths:
            if not os.path.exists(path):
                print(f"Graph not found: {path}")
                return 1
        try:
            labels, d_values, b_values = _benchmark_graphs(
                graph_paths,
                args.start_vertex - 1,
                args.repeat,
                args.undirected,
            )
        except Exception as exc:
            print(f"Failed to benchmark graphs: {exc}")
            return 1
    else:
        if not os.path.exists(csv_path):
            print(f"CSV not found: {csv_path}")
            print("Create a CSV with columns: label,dijkstra_us,bellman_us")
            return 1
        try:
            labels, d_values, b_values, inferred_unit = _read_csv(csv_path)
        except Exception as exc:
            print(f"Failed to read CSV: {exc}")
            return 1

    unit = args.unit or inferred_unit or ("s" if graph_paths else "us")

    if graph_paths:
        d_values = _convert_unit(d_values, unit)
        b_values = _convert_unit(b_values, unit)

    if args.scale != 1.0:
        d_values = [v * args.scale for v in d_values]
        b_values = [v * args.scale for v in b_values]

    if args.label_prefix:
        labels = [f"{args.label_prefix}{i + 1}" for i in range(len(labels))]

    if graph_paths and args.write_csv:
        header = ["label", f"dijkstra_{unit}", f"bellman_ford_{unit}"]
        try:
            with open(args.write_csv, "w", newline="", encoding="utf-8") as handle:
                writer = csv.writer(handle)
                writer.writerow(header)
                for label, d_val, b_val in zip(labels, d_values, b_values):
                    writer.writerow([label, f"{d_val:.6g}", f"{b_val:.6g}"])
        except Exception as exc:
            print(f"Failed to write CSV: {exc}")
            return 1

    show = not args.no_show

    return plot_compare(labels, d_values, b_values, unit, args.title, args.out, show)


if __name__ == "__main__":
    sys.exit(main())
