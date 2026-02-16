
import networkx as nx
import matplotlib.pyplot as plt
import os
import sys


TEMP_FILE = "../data/temp.txt"
GRAPH_TITLE = "Trực quan hóa đường đi ngắn nhất"
VERTEX_SIZE = 100
FONT_SIZE = 10
EDGE_WIDTH = 2
PATH_WIDTH = 4

def main():
    if not os.path.exists(TEMP_FILE):
        print(f"Error: {TEMP_FILE} not found!")
        return False
    
    try:
        G = nx.DiGraph()

        with open(TEMP_FILE, 'r') as f:
            lines = f.readlines()
        
        if len(lines) < 2:
            print("Error: Invalid graph file format!")
            return False
        
        num_vertices = int(lines[0].strip())
        
        vertex_labels = lines[1].strip().split()
        
        for i, label in enumerate(vertex_labels):
            G.add_node(i, label=label)

        edge_start_idx = 2
        path = []
        path_start_line = -1
        
        for i, line in enumerate(lines[edge_start_idx:], start=edge_start_idx):
            line = line.strip()
            
            if line == "PATH:":
                path_start_line = i + 1
                break
            
            if line:
                parts = line.split()
                if len(parts) >= 3:
                    source = int(parts[0]) - 1
                    dest = int(parts[1]) - 1
                    weight = int(parts[2])
                    if source >= 0 and dest >= 0:
                        G.add_edge(source, dest, weight=weight)
        
        if path_start_line > 0 and path_start_line < len(lines):
            path_parts = lines[path_start_line].strip().split()
            path = []
            for v in path_parts:
                if v.isdigit():
                    idx = int(v) - 1
                    if idx >= 0:
                        path.append(idx)

        plt.figure(figsize=(12, 8))
        plt.title(GRAPH_TITLE, fontsize=16, fontweight='bold')

        pos = nx.spring_layout(G, k=2, iterations=50, seed=42)

        nx.draw_networkx_edges(G, pos, 
                               edgelist=G.edges(),
                               edge_color='gray',
                               width=EDGE_WIDTH,
                               arrows=True,
                               arrowsize=20,
                               arrowstyle='->',
                               connectionstyle='arc3,rad=0.1')

        edge_labels = nx.get_edge_attributes(G, 'weight')
        nx.draw_networkx_edge_labels(G, pos, edge_labels, font_size=8)

        if len(path) > 1:
            path_edges = [(path[i], path[i+1]) for i in range(len(path)-1)]
            nx.draw_networkx_edges(G, pos,
                                  edgelist=path_edges,
                                  edge_color='red',
                                  width=PATH_WIDTH,
                                  arrows=True,
                                  arrowsize=20,
                                  arrowstyle='->',
                                  connectionstyle='arc3,rad=0.1')

        node_colors = []
        for node in G.nodes():
            if len(path) > 0:
                if node == path[0]:
                    node_colors.append('lightgreen')  
                elif node == path[-1]:
                    node_colors.append('lightcoral')  
                elif node in path:
                    node_colors.append('lightyellow') 
                else:
                    node_colors.append('lightblue')  
            else:
                node_colors.append('lightblue')
        
        nx.draw_networkx_nodes(G, pos,
                              node_color=node_colors,
                              node_size=VERTEX_SIZE,
                              node_shape='o')

        labels = {node: G.nodes[node].get('label', str(node)) for node in G.nodes()}
        nx.draw_networkx_labels(G, pos, labels, font_size=FONT_SIZE, font_weight='bold')

        from matplotlib.patches import Patch
        legend_elements = [
            Patch(facecolor='lightgreen', label='Start Node'),
            Patch(facecolor='lightcoral', label='End Node'),
            Patch(facecolor='lightyellow', label='Path Node'),
            Patch(facecolor='lightblue', label='Other Node')
        ]
        plt.legend(handles=legend_elements, loc='upper left', fontsize=10)
        
        plt.axis('off')
        plt.tight_layout()
        
        plt.show()
        
        return True
        
    except Exception as e:
        print(f"Error: {e}")
        return False
if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
