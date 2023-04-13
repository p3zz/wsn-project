import re
import sys
import matplotlib.pyplot as plt
import os
import csv

class ParserState:
    Init = 0
    ReadingTrafficData = 1
    ReadingTrafficOverallTx = 2
    ReadingTrafficOverallRx = 3
    ReadingTrafficOverallPdr = 4
    ReadingTrafficOverallPlr = 5
    ReadingDutyCycleData = 6
    ReadingDutyCycleOverallAvg = 7
    ReadingDutyCycleOverallStd = 8
    ReadingDutyCycleOverallMin = 9
    ReadingDutyCycleOverallMax = 10

class DataType:
    DataCollection = 0
    SourceRouting = 1
    DutyCycle = 2

class DutyCycleData:
    def __init__(self, avg, std, min, max):
        self.avg = avg
        self.std = std
        self.min = min
        self.max = max

    def __str__(self):
        return "AVG: {}%\tSTD: {}\tMIN: {}%\tMAX: {}%".format(self.avg, self.std, self.min, self.max)

class TrafficData:
    def __init__(self, rx, tx, pdr, plr):
        self.rx = rx
        self.tx = tx
        self.pdr = pdr
        self.plr = plr

    def __str__(self):
        return "RX: {}\tTX: {}\tPDR: {}%\tPLR: {}%".format(self.rx, self.tx, self.pdr, self.plr)


class Node:
    def __init__(self, id):
        self.id = id
        self.data_collection = None 
        self.source_routing = None
        self.duty_cycle = None
        self.energest = []
    
    def __str__(self):
        result = "ID: {}\nData Collection: {}\nSource routing: {}\nDuty cycle: {}\n".format(self.id, self.data_collection, self.source_routing, self.duty_cycle)
        for ener in self.energest:
            result += "{}\n".format(ener)
        return result

class ResultData:
    def __init__(self, nodes, data_collection_overall, source_routing_overall, duty_cycle_overall):
        self.nodes = nodes
        self.data_collection_overall = data_collection_overall
        self.source_routing_overall = source_routing_overall
        self.duty_cycle_overall = duty_cycle_overall

    def __str__(self):
        return "Data Collection overall: {}\nSource routing overall: {}\nDuty cycle overall: {}\n".format(
            self.data_collection_overall, self.source_routing_overall, self.duty_cycle_overall
        )

class TestData:
    def __init__(self, name: str, udgm: ResultData, mrm: ResultData):
        self.name = name
        self.udgm = udgm
        self.mrm = mrm

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

class ResultType:
    UDGM = 0
    MRM = 1

def plot_test(test_data: TestData):
    plt.figure(num=test_data.name)
    # plot_result(test_data.udgm, 1, ResultType.UDGM)
    # plot_result(test_data.mrm, 4, ResultType.MRM)
    plot_energest(test_data)
    plt.show()

def plot_energest(test_data: TestData):
    nodes = list(test_data.udgm.nodes.values())
    for node in nodes:
        x = list(map(lambda entry: entry.time / 1000000, node.energest))
        y = list(map(lambda entry: entry.rx_time, node.energest))
        plt.plot(x, y, label=str(node.id))
    plt.legend()

def plot_result(result_data: ResultData, idx: int, result_type: ResultType):
    id = list(result_data.nodes.keys())
    nds = list(result_data.nodes.values())
    data_collection_pdr = list(map(lambda node : node.data_collection.pdr if node.data_collection else 0, nds))
    source_routing_pdr = list(map(lambda node : node.source_routing.pdr if node.data_collection else 0, nds))
    duty_cycle = list(map(lambda node : node.duty_cycle if node.duty_cycle else 0, nds))

    # plt.figure(num=filename)

    type = "UDGM" if result_type == ResultType.UDGM else "MRM"

    plt.subplot(2, 3, idx)
    plt.title("Data collection PDR ({})".format(type))
    plt.xlabel("Node ID")
    plt.ylabel("PDR")
    plt.ylim(0, 100)
    plt.xticks(id)
    plt.bar(id, data_collection_pdr, color ='red',
            width = 0.4)

    plt.subplot(2, 3, idx+1)
    plt.title("Source routing PDR ({})".format(type))
    plt.xlabel("Node ID")
    plt.ylabel("PDR")
    plt.ylim(0, 100)
    plt.xticks(id)
    plt.bar(id, source_routing_pdr, color ='blue',
            width = 0.4)
    
    plt.subplot(2, 3, idx+2)
    plt.title("Radio duty cycling ({})".format(type))
    plt.xlabel("Node ID")
    plt.ylabel("Duty cycle")
    plt.xticks(id)
    plt.bar(id, duty_cycle, color ='green',
            width = 0.4)

def parse_file(filename, nodes):

    regex_data_collection_header = re.compile(r"----- Data Collection Node Statistics -----")
    regex_source_routing_header = re.compile(r"----- Source Routing Node Statistics -----")
    regex_node_data = re.compile(r"Node (?P<node_id>\d+): TX Packets = (?P<tx>\d+), RX Packets = (?P<rx>\d+), PDR = (?P<pdr>\d+\.\d+)%, PLR = (?P<plr>\d+\.\d+)%")

    regex_data_collection_overall_header = re.compile(r"----- Data Collection Overall Statistics -----")
    regex_source_routing_overall_header = re.compile(r"----- Source Routing Overall Statistics -----")
    regex_node_overall_data_tx = re.compile(r"Total Number of Packets Sent: (?P<tx>\d+)")
    regex_node_overall_data_rx = re.compile(r"Total Number of Packets Received: (?P<rx>\d+)")
    regex_node_overall_data_pdr = re.compile(r"Overall PDR = (?P<pdr>\d+\.\d+)%")
    regex_node_overall_data_plr = re.compile(r"Overall PLR = (?P<plr>\d+\.\d+)%")

    regex_duty_cycle_header = re.compile(r"----- Duty Cycle Statistics -----")
    regex_duty_cycle_data = re.compile(r"Node (?P<node_id>\d+):  Duty Cycle: (?P<duty_cycle>\d+\.\d+)%")

    regex_duty_cycle_overall_header = re.compile(r"----- Duty Cycle Overall Statistics -----")
    regex_duty_cycle_overall_data_avg = re.compile(r"Average Duty Cycle: (?P<avg>\d+\.\d+)%")
    regex_duty_cycle_overall_data_std = re.compile(r"Standard Deviation: (?P<std>\d+\.\d+)")
    regex_duty_cycle_overall_data_min = re.compile(r"Minimum: (?P<min>\d+\.\d+)%")
    regex_duty_cycle_overall_data_max = re.compile(r"Maximum: (?P<max>\d+\.\d+)%")

    data_collection_overall = None
    data_collection_overall_tx = None
    data_collection_overall_rx = None
    data_collection_overall_pdr = None
    data_collection_overall_plr = None
    source_routing_overall = None
    source_routing_overall_tx = None
    source_routing_overall_rx = None
    source_routing_overall_pdr = None
    source_routing_overall_plr = None
    duty_cycle_overall = None
    duty_cycle_overall_avg = None
    duty_cycle_overall_std = None
    duty_cycle_overall_min = None
    duty_cycle_overall_max = None

    # Parse log file and add data to CSV files
    with open(filename, 'r') as f:
        state = ParserState.Init
        data_type = DataType.DataCollection
        for line in f:
            match state:
                case ParserState.Init:
                    m = regex_data_collection_header.match(line)
                    if m:
                        state = ParserState.ReadingTrafficData
                    continue

                case ParserState.ReadingTrafficData:
                    m = regex_node_data.match(line)
                    if m:
                        d = m.groupdict()
                        node_id = int(d["node_id"])
                        tx = int(d["tx"])
                        rx = int(d["rx"])
                        pdr = float(d["pdr"])
                        plr = float(d["plr"])
                        data = TrafficData(rx, tx, pdr, plr)
                        if not node_id in nodes:
                            nodes[node_id] = Node(node_id)
                        if data_type == DataType.DataCollection:
                            nodes[node_id].data_collection = data
                        else:
                            nodes[node_id].source_routing = data
                        continue

                    md = regex_data_collection_overall_header.match(line)
                    ms = regex_source_routing_overall_header.match(line)
                    if md or ms:
                        state = ParserState.ReadingTrafficOverallTx
                    continue

                case ParserState.ReadingTrafficOverallTx:
                    m = regex_node_overall_data_tx.match(line)
                    if m:
                        d = m.groupdict()
                        tx = int(d["tx"])
                        if data_type == DataType.DataCollection:
                            data_collection_overall_tx = tx
                        else:
                            source_routing_overall_tx = tx
                        state = ParserState.ReadingTrafficOverallRx
                    continue

                case ParserState.ReadingTrafficOverallRx:
                    m = regex_node_overall_data_rx.match(line)
                    if m:
                        d = m.groupdict()
                        rx = int(d["rx"])
                        if data_type == DataType.DataCollection:
                            data_collection_overall_rx = rx
                        else:
                            source_routing_overall_rx = rx
                        state = ParserState.ReadingTrafficOverallPdr
                    continue

                case ParserState.ReadingTrafficOverallPdr:
                    m = regex_node_overall_data_pdr.match(line)
                    if m:
                        d = m.groupdict()
                        pdr = float(d["pdr"])
                        if data_type == DataType.DataCollection:
                            data_collection_overall_pdr = pdr
                        else:
                            source_routing_overall_pdr = pdr
                        state = ParserState.ReadingTrafficOverallPlr
                    continue

                case ParserState.ReadingTrafficOverallPlr:
                    m = regex_node_overall_data_plr.match(line)
                    if m:
                        d = m.groupdict()
                        plr = float(d["plr"])
                        if data_type == DataType.DataCollection:
                            data_collection_overall_plr = plr
                        else:
                            source_routing_overall_plr = plr
                        continue
                    m = regex_source_routing_header.match(line)
                    if m:
                        state = ParserState.ReadingTrafficData
                        data_type = DataType.SourceRouting
                        continue
                    m = regex_duty_cycle_header.match(line)
                    if m:
                        state = ParserState.ReadingDutyCycleData
                        data_type = DataType.DutyCycle
                    continue

                case ParserState.ReadingDutyCycleData:
                    m = regex_duty_cycle_data.match(line)
                    if m:
                        d = m.groupdict()
                        node_id = int(d["node_id"])
                        duty_cycle = float(d["duty_cycle"])
                        if not node_id in nodes:
                            nodes[node_id] = Node(node_id)
                        nodes[node_id].duty_cycle = duty_cycle
                        continue
                    m = regex_duty_cycle_overall_header.match(line)
                    if m:
                        state = ParserState.ReadingDutyCycleOverallAvg
                    continue

                case ParserState.ReadingDutyCycleOverallAvg:
                    m = regex_duty_cycle_overall_data_avg.match(line)
                    if m:
                        d = m.groupdict()
                        avg = float(d["avg"])
                        duty_cycle_overall_avg = avg
                        state = ParserState.ReadingDutyCycleOverallStd
                    continue

                case ParserState.ReadingDutyCycleOverallStd:
                    m = regex_duty_cycle_overall_data_std.match(line)
                    if m:
                        d = m.groupdict()
                        std = float(d["std"])
                        duty_cycle_overall_std = std
                        state = ParserState.ReadingDutyCycleOverallMin
                    continue

                case ParserState.ReadingDutyCycleOverallMin:
                    m = regex_duty_cycle_overall_data_min.match(line)
                    if m:
                        d = m.groupdict()
                        min = float(d["min"])
                        duty_cycle_overall_min = min
                        state = ParserState.ReadingDutyCycleOverallMax
                    continue

                case ParserState.ReadingDutyCycleOverallMax:
                    m = regex_duty_cycle_overall_data_max.match(line)
                    if m:
                        d = m.groupdict()
                        max = float(d["max"])
                        duty_cycle_overall_max = max
                    continue


    f.close()
    data_collection_overall = TrafficData(data_collection_overall_rx, data_collection_overall_tx, data_collection_overall_pdr, data_collection_overall_plr)
    source_routing_overall = TrafficData(source_routing_overall_rx, source_routing_overall_tx, source_routing_overall_pdr, source_routing_overall_plr)
    duty_cycle_overall = DutyCycleData(duty_cycle_overall_avg, duty_cycle_overall_std, duty_cycle_overall_min, duty_cycle_overall_max)
    return data_collection_overall, source_routing_overall, duty_cycle_overall

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

                energest = EnergestData(time, cnt, cpu, lpm, tx, rx)
                nodes[node_id].energest.append(energest)
    
    csv_file.close()

if __name__ == '__main__':

    args = sys.argv
    test_path = args[1]
    energest_filename = "test-energest.csv"
    result_filename = "result.txt"
    udgm_dirname = "UDGM"
    mrm_dirname = "MRM"

    udgm_file_path = os.path.join(test_path, udgm_dirname, result_filename)
    mrm_file_path = os.path.join(test_path, mrm_dirname, result_filename)

    udgm_energest_path = os.path.join(test_path, udgm_dirname, energest_filename)
    mrm_energest_path = os.path.join(test_path, mrm_dirname, energest_filename)

    udgm_nodes = {}
    mrm_nodes = {}

    udgm_data_collection_overall, udgm_source_routing_overall, udgm_duty_cycle_overall = parse_file(udgm_file_path, udgm_nodes)
    mrm_data_collection_overall, mrm_source_routing_overall, mrm_duty_cycle_overall = parse_file(mrm_file_path, mrm_nodes)

    parse_energest(udgm_energest_path, udgm_nodes)
    parse_energest(mrm_energest_path, mrm_nodes)

    udgm_result = ResultData(udgm_nodes, udgm_data_collection_overall, udgm_source_routing_overall, udgm_duty_cycle_overall)
    mrm_result = ResultData(mrm_nodes, mrm_data_collection_overall, mrm_source_routing_overall, mrm_duty_cycle_overall)
    
    test_data = TestData(test_path, udgm_result, mrm_result)
    # print(udgm_result, mrm_result)
    plot_test(test_data)
    
    for i in udgm_nodes:
        print(udgm_nodes[i])


