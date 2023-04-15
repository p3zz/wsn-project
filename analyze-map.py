import graphics as g
import csv
import sys
import os

class Node:
    def __init__(self, id, x, y):
        self.id = id
        self.x = x
        self.y = y
    
    def __str__(self):
        return "Id: {}\tx: {}\ty: {}".format(self.id, self.x, self.y)

class ResultType:
    UDGM = 0
    MRM = 1

def parse_map(filename, nodes):
    with open(filename, mode='r') as csv_file:
        csv_reader = csv.DictReader(csv_file, delimiter='\t')
        for row in csv_reader:
            node_id = int(row["id"])
            x = float(row["x"])
            y = float(row["y"])
            nodes[node_id] = Node(node_id, x, y)

def draw_node(win, node):
    c = g.Circle(g.Point(node.x, node.y), 8)
    t = g.Text(g.Point(node.x, node.y), str(node.id))
    t.setSize(8)
    t.setFill("black")
    t.draw(win)
    c.draw(win)

def draw_map(nodes, result_type: ResultType):
    scale = 5 if result_type == ResultType.UDGM else 0.5
    offset = 20
    nds = list(nodes.values())
    nds_scaled = list(map(lambda node: Node(node.id, (node.x * scale) + offset, (node.y * scale) + offset), nds))
    height = max(list(map(lambda node: node.y, nds_scaled)))
    width = max(list(map(lambda node: node.x, nds_scaled)))
    win = g.GraphWin("nodes map", width + offset, height + offset)
    for node in nds_scaled:
        draw_node(win, node)
    keyString = ""
    while keyString!="q":
        keyString = win.getKey() # Pause to view result
    win.close()    # Close window when done

if __name__ == '__main__':
    args = sys.argv
    test_path = args[1]
    map_filename = "map.csv"
    udgm_dirname = "UDGM"
    mrm_dirname = "MRM"

    udgm_nodes: dict[int, Node] = {}
    mrm_nodes: dict[int, Node] = {}

    udgm_map_path = os.path.join(test_path, udgm_dirname, map_filename)
    mrm_map_path = os.path.join(test_path, mrm_dirname, map_filename)

    parse_map(udgm_map_path, udgm_nodes)
    parse_map(mrm_map_path, mrm_nodes)

    draw_map(udgm_nodes, ResultType.UDGM)
