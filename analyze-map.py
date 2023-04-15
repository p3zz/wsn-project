import graphics as g
import csv
import sys
import os
import time

class Position:
    def __init__(self, x, y):
        self.x = x
        self.y = y
    
    def __str__(self):
        return "x: {}\ty: {}".format(self.x, self.y)

class CollectionData:
    def __init__(self, seqn, parent):
        self.seqn = seqn
        self.parent = parent
    
    def __str__(self):
        return "seqn: {}\tparent: {}".format(self.seqn, self.parent)

class Node:
    def __init__(self, id, position, collection_data):
        self.id = id
        self.position = position
        self.collection_data = collection_data
    
    def __str__(self):
        result = "Id: {}\tPosition: {}\n".format(self.id, self.position)
        for data in self.collection_data:
            result += "{}\n".format(data)
        return result

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
            nodes[node_id] = Node(node_id, Position(x,y), [])

def parse_collection(filename, nodes):
    with open(filename, mode='r') as csv_file:
        csv_reader = csv.DictReader(csv_file, delimiter='\t')
        for row in csv_reader:
            node_id = int(row["src"])
            parent = int(row["parent"])
            seqn = int(row["seqn"])
            nodes[node_id].collection_data.append(CollectionData(seqn, parent))
            # print(nodes[node_id])

def draw_link(win, src, dst):
    c = g.Line(g.Point(src.position.x, src.position.y), g.Point(dst.position.x, dst.position.y))
    c.draw(win)

def draw_node(win, node):
    c = g.Circle(g.Point(node.position.x, node.position.y), 8)
    t = g.Text(g.Point(node.position.x, node.position.y), str(node.id))
    t.setSize(8)
    t.setFill("black")
    t.draw(win)
    c.draw(win)

def draw_seqn(win, seqn, x, y):
    t = g.Text(g.Point(x, y), "Seqn: {}".format(seqn))
    t.setSize(20)
    t.setFill("black")
    t.draw(win)

def clear(win):
    for item in win.items[:]:
        item.undraw()
    win.update()

def draw_map(nodes, result_type: ResultType):
    scale = 5 if result_type == ResultType.UDGM else 0.5
    offset = 20
    for key in nodes:
        x_scaled = (nodes[key].position.x * scale) + offset
        y_scaled = (nodes[key].position.y * scale) + offset
        nodes[key] = Node(nodes[key].id, Position(x_scaled, y_scaled), nodes[key].collection_data) 
        
    nds = list(nodes.values())
    height = max(list(map(lambda node: node.position.y, nds)))
    width = max(list(map(lambda node: node.position.x, nds)))
    win = g.GraphWin("nodes map", width + offset, height + offset, autoflush=True)
    index = 0
    while True:
        for node in nds:
            draw_node(win, node)
            collection = list(filter(lambda d: d.seqn == index, node.collection_data))
            for data in collection:
                draw_link(win, node, nodes[data.parent])
        draw_seqn(win, index, width * 0.5, offset)
        index+=1
        time.sleep(0.5)
        clear(win)
        
    win.close()    # Close window when done

if __name__ == '__main__':
    args = sys.argv
    test_path = args[1]
    map_filename = "map.csv"
    recv_filename = "test-recv.csv"
    udgm_dirname = "UDGM"
    mrm_dirname = "MRM"

    udgm_nodes: dict[int, Node] = {}
    mrm_nodes: dict[int, Node] = {}

    udgm_map_path = os.path.join(test_path, udgm_dirname, map_filename)
    mrm_map_path = os.path.join(test_path, mrm_dirname, map_filename)

    udgm_recv_path = os.path.join(test_path, udgm_dirname, recv_filename)
    mrm_recv_path = os.path.join(test_path, mrm_dirname, recv_filename)

    parse_map(udgm_map_path, udgm_nodes)
    parse_map(mrm_map_path, mrm_nodes)

    parse_collection(udgm_recv_path, udgm_nodes)
    parse_collection(mrm_recv_path, mrm_nodes)

    # print(udgm_nodes)
    draw_map(mrm_nodes, ResultType.MRM)
