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
                time = float(row["time"])
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

def plot_energest(nodes: dict[int, Node]):
    nds = nodes.values()
    plt.subplot(1,3,1)
    for node in nds:
        time = list(map(lambda entry: entry.time / 1000000, node.energest))
        rx = list(map(lambda entry: entry.rx_time, node.energest))
        rx = [sum(rx[:y]) for y in range(1, len(rx) + 1)]
        plt.plot(time, rx, label=node.id)
    
    plt.title("RX Time")
    plt.legend()
    plt.xlabel("Time")
    plt.ylabel("RX Time")

    plt.subplot(1,3,2)
    for node in nds:
        time = list(map(lambda entry: entry.time / 1000000, node.energest))
        tx = list(map(lambda entry: entry.tx_time, node.energest))
        tx = [sum(tx[:y]) for y in range(1, len(tx) + 1)]
        plt.plot(time, tx, label=node.id)
    
    plt.title("TX Time")
    plt.legend()
    plt.xlabel("Time")
    plt.ylabel("TX Time")
    
    plt.subplot(1,3,3)
    ids = list(map(lambda node: node.id, nds))
    cpu_time = [sum([entry.cpu_time for entry in node.energest]) for node in nds]
    plt.bar(ids, cpu_time)
    plt.xticks(ids)
    plt.title("CPU Time")
    plt.legend()
    plt.xlabel("Node ID")
    plt.ylabel("CPU Time")
    plt.show()

if __name__ == '__main__':

    args = sys.argv
    energest_path = args[1]

    nodes: dict[int, Node] = {}

    parse_energest(energest_path, nodes)

    plot_energest(nodes)


