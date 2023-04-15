import csv
import matplotlib.pyplot as plt
import os
import sys
import math

class Node:
    def __init__(self, id):
        self.id = id
        self.energest = []

class ResultType:
    UDGM = 0
    MRM = 1

class EnergestData:
    def __init__(self, time: int, cnt: int, cpu_time: int, lpm_time: int, tx_time: int, rx_time: int):
        self.time = time
        self.cnt = cnt
        self.cpu_time = cpu_time
        self.lpm_time = lpm_time
        self.tx_time = tx_time
        self.rx_time = rx_time
    
    def __str__(self):
        return "Time: {}\tCnt: {}\tCPU time: {}\tLPM time: {}\tTX time: {}\tRX time: {}\n".format(
            self.time, self.cnt, self.cpu_time, self.lpm_time, self.tx_time, self.rx_time
        )

def parse_energest(filename: str, nodes):
    with open(filename, mode='r') as csv_file:
        csv_reader = csv.DictReader(csv_file, delimiter='\t')
        for row in csv_reader:
            node_id = int(row["node"])
            if node_id:
                time = int(row["time"])
                cnt = int(row["cnt"])
                cpu = int(row["cpu"])
                lpm = int(row["lpm"])
                tx = int(row["tx"])
                rx = int(row["rx"])

                if node_id not in nodes:
                    nodes[node_id] = Node(node_id)

                energest = EnergestData(time, cnt, cpu, lpm, tx, rx)
                nodes[node_id].energest.append(energest)
    
    csv_file.close()

def plot_energest(nodes: dict[int, Node], result_type: ResultType):
    nds = nodes.values()
    cols = 2
    rows =  math.ceil(len(nds) / cols)
    cnt = 1
    for node in nds:
        time = list(map(lambda entry: entry.time / 1000000, node.energest))
        # cpu = list(map(lambda entry: entry.cpu_time, node.energest))
        rx = list(map(lambda entry: entry.rx_time, node.energest))
        tx = list(map(lambda entry: entry.tx_time, node.energest))

        type = "UDGM" if result_type == ResultType.UDGM else "MRM"

        plt.subplot(rows, cols, cnt)
        # plt.title("Energest ({})".format(node.id))
        plt.xlabel("time")
        # plt.plot(time, cpu, color="green", label="[{}] cpu time".format(node.id))
        plt.plot(time, rx, color="red", label="[{}] rx time".format(node.id))
        plt.plot(time, tx, color="blue", label="[{}] tx time".format(node.id))
        plt.legend()

        cnt+=1
    
    plt.show()

if __name__ == '__main__':

    args = sys.argv
    test_path = args[1]
    energest_filename = "test-energest.csv"
    udgm_dirname = "UDGM"
    mrm_dirname = "MRM"

    udgm_nodes: dict[int, Node] = {}
    mrm_nodes: dict[int, Node] = {}

    udgm_energest_path = os.path.join(test_path, udgm_dirname, energest_filename)
    mrm_energest_path = os.path.join(test_path, mrm_dirname, energest_filename)

    parse_energest(udgm_energest_path, udgm_nodes)
    parse_energest(mrm_energest_path, mrm_nodes)

    plot_energest(udgm_nodes, ResultType.UDGM)


