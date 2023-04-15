import re
import graphics as g

class Node:
    def __init__(self, id, x, y):
        self.id = id
        self.x = x
        self.y = y
    
    def __str__(self):
        return "Id: {}\tx: {}\ty: {}".format(self.id, self.x, self.y)

class ParserState:
    Init = 0
    WaitingMoteOpenOrSimulationClosed = 1
    WaitingMoteX = 2
    WaitingMoteY = 3
    WaitingMoteId = 4
    WaitingMoteClose = 5
    End = 6

def parse_config(filename: str, nodes: dict[int, Node]):
    regex_simulation_open = re.compile(r"^(.*)<simulation>")
    regex_simulation_close = re.compile(r"^(.*)</simulation>")
    regex_mote_open = re.compile(r"^(.*)<mote>")
    regex_mote_close = re.compile(r"^(.*)</mote>")
    regex_mote_x = re.compile(r"^(.*)<x>(?P<x>\d+\.\d+)</x>")
    regex_mote_y = re.compile(r"^(.*)<y>(?P<y>\d+\.\d+)</y>")
    regex_mote_id = re.compile(r"^(.*)<id>(?P<id>\d+)</id>")

    x = None
    y = None

    with open(filename, mode='r') as f:
        state = ParserState.Init
        for line in f:
            match state:
                case ParserState.Init:
                    m = regex_simulation_open.match(line)
                    if m:
                        state = ParserState.WaitingMoteOpenOrSimulationClosed
                    continue
                
                case ParserState.WaitingMoteOpenOrSimulationClosed:
                    m = regex_mote_open.match(line)
                    if m:
                        state = ParserState.WaitingMoteX
                        continue
                    m = regex_simulation_close.match(line)
                    if m:
                        state = ParserState.End
                    continue

                case ParserState.WaitingMoteX:
                    m = regex_mote_x.match(line)
                    if m:
                        d = m.groupdict()
                        x = float(d["x"])
                        state = ParserState.WaitingMoteY
                    continue

                case ParserState.WaitingMoteY:
                    m = regex_mote_y.match(line)
                    if m:
                        d = m.groupdict()
                        y = float(d["y"])
                        state = ParserState.WaitingMoteId
                    continue

                case ParserState.WaitingMoteId:
                    m = regex_mote_id.match(line)
                    if m:
                        d = m.groupdict()
                        id = int(d["id"])
                        nodes[id] = Node(id, x, y)
                        state = ParserState.WaitingMoteClose
                    continue

                case ParserState.WaitingMoteClose:
                    m = regex_mote_close.match(line)
                    if m:
                        state = ParserState.WaitingMoteOpenOrSimulationClosed
                    continue

                case ParserState.End:
                    break
                
    return

def draw_node(win, node):
    c = g.Circle(g.Point(node.x, node.y), 8)
    t = g.Text(g.Point(node.x, node.y), str(node.id))
    t.setSize(8)
    t.setFill("black")
    t.draw(win)
    c.draw(win)

def draw_map(nodes):
    scale = 3
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

def print_map(nodes):
    result = ""
    result += "id\tx\ty\n"
    for node in list(nodes.values()):
        result+="{}\t{}\t{}\n".format(node.id, node.x, node.y)
    print(result)

if __name__ == '__main__':

    config_filename = "test_nogui.csc"
    nodes = {}
    parse_config(config_filename, nodes)
    # for i in nodes:
        # print(nodes[i])
    print_map(nodes)
    draw_map(nodes)