import re
import sys
import matplotlib.pyplot as plt

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
    Report = 2
    DutyCycle = 3

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
        self.report = None
    
    def __str__(self):
        return "ID: {}\nData Collection: {}\nSource routing: {}\nDuty cycle: {}\n".format(self.id, self.data_collection, self.source_routing, self.duty_cycle)

class ResultData:
    def __init__(self, filename, nodes, data_collection_overall, source_routing_overall, report_overall, duty_cycle_overall):
        self.filename = filename
        self.nodes = nodes
        self.data_collection_overall = data_collection_overall
        self.source_routing_overall = source_routing_overall
        self.duty_cycle_overall = duty_cycle_overall
        self.report_overall = report_overall

    def __str__(self):
        return "Data Collection overall: {}\nSource routing overall: {}\nReport overall: {}\nDuty cycle overall: {}\n".format(
            self.data_collection_overall, self.source_routing_overall, self.report_overall, self.duty_cycle_overall
        )

def plot_test(result: ResultData, path: str):
    plt.figure(path)
    plot_result(result)
    plt.show()

def plot_result(results: list[ResultData]):
    plt.subplot(2, 2, 1)
    offset = 0
    width = 0.25
    colors = ['red', 'green', 'blue']
    i = 0
    for result in results:
        id = list(result.nodes.keys())
        nds = list(result.nodes.values())
        data_collection_pdr = list(map(lambda node : node.data_collection.pdr if node.data_collection else 0, nds))
        data = data_collection_pdr
        id = list(map(lambda id: id + offset, id))
        plt.bar(id, data, width = width, edgecolor ='grey', color=colors[i])
        # add_labels(x, data, fontsize)
        offset+=width
        i+=1
    
    plt.title("Data Collection PDR", fontsize=18)
    plt.ylim(0, 100)
    plt.xticks([i + width for i in range(len(id)+1)], [i for i in range(len(id)+1)])
    
    plt.subplot(2, 2, 2)
    offset = 0
    i = 0
    for result in results:
        id = list(result.nodes.keys())
        nds = list(result.nodes.values())
        source_routing_pdr = list(map(lambda node : node.source_routing.pdr if node.data_collection else 0, nds))
        # report_pdr = list(map(lambda node : node.report.pdr if node.report else 0, nds))
        # duty_cycle = list(map(lambda node : node.duty_cycle if node.duty_cycle else 0, nds))

        data = source_routing_pdr
        id = list(map(lambda id: id + offset, id))
        plt.bar(id, data, width = width, edgecolor ='grey', color=colors[i])
        # add_labels(x, data, fontsize)
        offset+=width
        i+=1
    
    plt.title("Source Routing PDR", fontsize=18)
    plt.ylim(0, 100)
    plt.xticks([i + width for i in range(len(id)+1)], [i for i in range(len(id)+1)])

    plt.subplot(2, 2, 3)
    offset = 0
    i = 0
    for result in results:
        id = list(result.nodes.keys())
        nds = list(result.nodes.values())
        report_pdr = list(map(lambda node : node.report.pdr if node.report else 0, nds))
        # duty_cycle = list(map(lambda node : node.duty_cycle if node.duty_cycle else 0, nds))

        data = report_pdr
        id = list(map(lambda id: id + offset, id))
        plt.bar(id, data, width = width, edgecolor ='grey', color=colors[i])
        # add_labels(x, data, fontsize)
        offset+=width
        i+=1
    
    plt.title("Report PDR", fontsize=18)
    plt.ylim(0, 100)
    plt.xticks([i + width for i in range(len(id)+1)], [i for i in range(len(id)+1)])

    plt.subplot(2, 2, 4)
    offset = 0
    i = 0
    for result in results:
        id = list(result.nodes.keys())
        nds = list(result.nodes.values())
        duty_cycle = list(map(lambda node : node.duty_cycle if node.duty_cycle else 0, nds))

        data = duty_cycle
        id = list(map(lambda id: id + offset, id))
        plt.bar(id, data, width = width, edgecolor ='grey', color=colors[i])
        # add_labels(x, data, fontsize)
        offset+=width
        i+=1
    
    plt.title("Duty Cycle", fontsize=18)
    plt.xticks([i + width for i in range(len(id)+1)], [i for i in range(len(id)+1)])


def add_labels(x,y,fontsize):
    for i in range(len(x)):
        plt.text(x[i],y[i],"{}%".format(y[i]), ha='center', fontsize=fontsize)

def plot_comparison(results: list[ResultData]):
    # Set position of bar on X axis
    bar_width = 0.25
    fontsize=14
    colors = ['red', 'green', 'blue']
    labels = ['plain', 'nbnr', 'nbnr + npnp']
    plt.subplot(2, 1, 1)
    x = [i for i in range(3)]
    i = 0
    for result in results:
        data = [result.data_collection_overall.pdr, result.source_routing_overall.pdr, result.report_overall.pdr]
        print(data)
        plt.bar(x, data, color = colors[i], width = bar_width, edgecolor ='grey', label = labels[i])
        add_labels(x, data, fontsize)
        x = [elem + bar_width for elem in x]
        i+=1
    
    plt.legend(loc='lower right')
    plt.title("PDR", fontsize=18)
    plt.legend(loc='lower right')
    plt.ylim(0, 100)
    plt.xticks([r + bar_width for r in range(3)],
        ['Data collection', 'Source routing', 'Report'], fontsize=14)

    plt.subplot(2, 1, 2)
    i = 0
    data = []
    for result in results:
        x = [i]
        data = [result.duty_cycle_overall.avg]
        plt.bar(x, data, color = colors[i], width = bar_width, edgecolor ='grey', label = labels[i])
        print(x,data)
        add_labels(x, data, fontsize)
        x = [elem + bar_width for elem in x]
        i+=1

    plt.title("Duty Cycle", fontsize=18)
    plt.xticks([r for r in range(len(labels))], labels, fontsize=14)
    plt.show()


def parse_file(filename, nodes):

    regex_data_collection_header = re.compile(r"----- Data Collection Node Statistics -----")
    regex_source_routing_header = re.compile(r"----- Source Routing Node Statistics -----")
    regex_report_header = re.compile(r"----- Report Statistics -----")
    regex_node_data = re.compile(r"Node (?P<node_id>\d+): TX Packets = (?P<tx>\d+), RX Packets = (?P<rx>\d+), PDR = (?P<pdr>\d+\.\d+)%, PLR = (?P<plr>\d+\.\d+)%")

    regex_data_collection_overall_header = re.compile(r"----- Data Collection Overall Statistics -----")
    regex_source_routing_overall_header = re.compile(r"----- Source Routing Overall Statistics -----")
    regex_report_overall_header = re.compile(r"----- Report Overall Statistics -----")
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
    report_overall = None
    report_overall_tx = None
    report_overall_rx = None
    report_overall_pdr = None
    report_overall_plr = None
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
                        elif data_type == DataType.Report:
                            nodes[node_id].report = data
                        else:
                            nodes[node_id].source_routing = data
                        continue

                    md = regex_data_collection_overall_header.match(line)
                    ms = regex_source_routing_overall_header.match(line)
                    mr = regex_report_overall_header.match(line)
                    if md or ms or mr:
                        state = ParserState.ReadingTrafficOverallTx
                    continue

                case ParserState.ReadingTrafficOverallTx:
                    m = regex_node_overall_data_tx.match(line)
                    if m:
                        d = m.groupdict()
                        tx = int(d["tx"])
                        if data_type == DataType.DataCollection:
                            data_collection_overall_tx = tx
                        elif data_type == DataType.Report:
                            report_overall_tx = tx
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
                        elif data_type == DataType.Report:
                            report_overall_rx = rx
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
                        elif data_type == DataType.Report:
                            report_overall_pdr = pdr
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
                        elif data_type == DataType.Report:
                            report_overall_plr = plr
                        else:
                            source_routing_overall_plr = plr
                        continue
                    ms = regex_source_routing_header.match(line)
                    mr = regex_report_header.match(line)
                    if ms or mr:
                        state = ParserState.ReadingTrafficData
                        data_type = DataType.SourceRouting if ms else DataType.Report
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
    report_overall = TrafficData(report_overall_rx, report_overall_tx, report_overall_pdr, report_overall_plr)
    duty_cycle_overall = DutyCycleData(duty_cycle_overall_avg, duty_cycle_overall_std, duty_cycle_overall_min, duty_cycle_overall_max)
    return data_collection_overall, source_routing_overall, report_overall, duty_cycle_overall

if __name__ == '__main__':

    args = sys.argv
    mode = args[1]
    option = args[2]
    result_paths = option.split(',')
    results = []
    for result_path in result_paths:
        nodes = {}
        data_collection_overall, source_routing_overall, report_overall, duty_cycle_overall = parse_file(result_path, nodes)
        result = ResultData(result_path, nodes, data_collection_overall, source_routing_overall, report_overall, duty_cycle_overall)
        results.append(result)
    if mode == 'analyze':
        plot_test(results, "")
    else:
        plot_comparison(results)