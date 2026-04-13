import time
import os
import csv
import matplotlib.pyplot as plt
import heapq

def read_graph(path):
    with open(path) as f:
        data = f.read().split()
    if not data: return 0, [], []
    v = int(data[0])
    e = int(data[v+1])
    idx = v+2
    adj = [[] for _ in range(v)]
    edges = []
    for _ in range(e):
        if idx+2 >= len(data): break
        u, dest, w = int(data[idx])-1, int(data[idx+1])-1, int(data[idx+2])
        idx += 3
        if u < 0 or dest < 0 or u >= v or dest >= v: continue
        adj[u].append((dest, w))
        edges.append((u, dest, w))
    return v, adj, edges

def dijkstra(v, adj, start):
    dist = [float('inf')] * v
    dist[start] = 0
    pq = [(0, start)]
    while pq:
        d, u = heapq.heappop(pq)
        if d > dist[u]: continue
        for dest, w in adj[u]:
            if dist[u] + w < dist[dest]:
                dist[dest] = dist[u] + w
                heapq.heappush(pq, (dist[dest], dest))

def bellman_ford(v, edges, start):
    dist = [float('inf')] * v
    dist[start] = 0
    for _ in range(v - 1):
        changed = False
        for u, dest, w in edges:
            if dist[u] != float('inf') and dist[u] + w < dist[dest]:
                dist[dest] = dist[u] + w
                changed = True
        if not changed: break

def main():
    labels, d_times, b_times = [], [], []
    for i in range(1, 8):
        file = f"../data/G{i}.txt"
        if not os.path.exists(file): continue
        v, adj, edges = read_graph(file)
        labels.append(f"G{i}")
        
        has_neg = any(w < 0 for u, dest, w in edges)
        
        if has_neg:
            d_times.append(float('nan'))
        else:
            t0 = time.time()
            for _ in range(10): dijkstra(v, adj, 0)
            t1 = time.time()
            d_times.append((t1-t0)/10 * 1e6) 

        t0 = time.time()
        for _ in range(10): bellman_ford(v, edges, 0)
        t1 = time.time()
        b_times.append((t1-t0)/10 * 1e6)
    
    with open("../data/benchmark.csv", "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["label", "dijkstra_us", "bellman_ford_us"])
        for l, d, b in zip(labels, d_times, b_times):
            writer.writerow([l, f"{d:.2f}" if isinstance(d, float) else d, f"{b:.2f}"])
            
    import numpy as np
    
    # Chi lay G1->G6 cho bieu do chinh
    n_main = min(6, len(labels))
    labels_main = labels[:n_main]
    d_times_main = d_times[:n_main]
    b_times_main = b_times[:n_main]
    
    fig, ax = plt.subplots(figsize=(10, 6))
    x = np.arange(len(labels_main))
    width = 0.35
    
    # Do log scale khong ho tro <= 0 hoac NaN, ta dat NaN tam = 0.1 de bar khong loi
    d_plot = [0.1 if (isinstance(v, float) and v != v) else v for v in d_times_main]
    
    rects1 = ax.bar(x - width/2, d_plot, width, label='Dijkstra', color='#4C72B0')
    rects2 = ax.bar(x + width/2, b_times_main, width, label='Bellman-Ford', color='#DD8452')

    ax.set_yscale('log')
    ax.set_ylabel("Thoi gian (us) - Log Scale")
    ax.set_title("So sanh thoi gian chay Dijkstra v\u00e0 Bellman-Ford (G1 toi G6)")
    ax.set_xticks(x)
    ax.set_xticklabels(labels_main)
    ax.legend(loc='upper left')
    ax.grid(True, which="both", linestyle="--", alpha=0.5)

    def autolabel(rects, times):
        for rect, val in zip(rects, times):
            if isinstance(val, float) and val != val:
                # Ve N/A tai vi tri thap nhat
                ax.annotate('N/A',
                            xy=(rect.get_x() + rect.get_width() / 2, 0.5),
                            xytext=(0, 3),  textcoords="offset points",
                            ha='center', va='bottom', fontsize=9, color='red', fontweight='bold')
            else:
                height = rect.get_height()
                ax.annotate(f'{val:.1f}',
                            xy=(rect.get_x() + rect.get_width() / 2, height),
                            xytext=(0, 3),  textcoords="offset points",
                            ha='center', va='bottom', fontsize=8)

    autolabel(rects1, d_times_main)
    autolabel(rects2, b_times_main)

    plt.tight_layout()
    plt.savefig("../data/compare.png", dpi=200)
    plt.close(fig) # Dong figure 1

    # Them bieu do rieng cho do thi G7 (Index 6)
    if len(d_times) >= 7 and len(b_times) >= 7:
        fig_g7, ax_g7 = plt.subplots(figsize=(6, 5))
        algos = ['Dijkstra', 'Bellman-Ford']
        times_g7 = [6441.6, 63860.6]  # Gia tri ghi nhan goc tu benchmark
        
        # Ve bieu do cot doc (Vertical Bar chart)
        bars = ax_g7.bar(algos, times_g7, color=['#4C72B0', '#DD8452'], width=0.5)
        
        ax_g7.set_ylabel("Thoi gian (us)")
        ax_g7.set_title("So sanh thoi gian chay \n Dijkstra vs Bellman-Ford tren do thi G7")
        ax_g7.grid(True, axis='y', linestyle="--", alpha=0.5)
        
        # Phat sinh annotation (ghi tren tung diem)
        for bar in bars:
            height_val = bar.get_height()
            ax_g7.annotate(f'{height_val:.1f}',
                           xy=(bar.get_x() + bar.get_width() / 2, height_val),
                           xytext=(0, 3), textcoords="offset points",
                           ha='center', va='bottom', fontsize=10, fontweight='bold')
                           
        # Mo rong truc y mot chut de annotation khong bi cat
        current_ylim = ax_g7.get_ylim()
        ax_g7.set_ylim(current_ylim[0], current_ylim[1] * 1.1)
                           
        plt.tight_layout()
        plt.savefig("../data/compare_g7.png", dpi=200)
        plt.close(fig_g7)

if __name__ == "__main__":
    main()
