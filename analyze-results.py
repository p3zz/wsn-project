import re
import sys

def parse_file(file):

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



    # Parse log file and add data to CSV files
    with open(file, 'r') as f:
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
                        print(node_id, tx, rx, pdr, plr)
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
                        print(tx)
                        state = ParserState.ReadingTrafficOverallRx
                    continue

                case ParserState.ReadingTrafficOverallRx:
                    m = regex_node_overall_data_rx.match(line)
                    if m:
                        d = m.groupdict()
                        rx = int(d["rx"])
                        print(rx)
                        state = ParserState.ReadingTrafficOverallPdr
                    continue

                case ParserState.ReadingTrafficOverallPdr:
                    m = regex_node_overall_data_pdr.match(line)
                    if m:
                        d = m.groupdict()
                        pdr = float(d["pdr"])
                        print(pdr)
                        state = ParserState.ReadingTrafficOverallPlr
                    continue

                case ParserState.ReadingTrafficOverallPlr:
                    m = regex_node_overall_data_plr.match(line)
                    if m:
                        d = m.groupdict()
                        plr = float(d["plr"])
                        print(plr)
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
                        print(node_id, duty_cycle)
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
                        print(avg)
                        state = ParserState.ReadingDutyCycleOverallStd
                    continue

                case ParserState.ReadingDutyCycleOverallStd:
                    m = regex_duty_cycle_overall_data_std.match(line)
                    if m:
                        d = m.groupdict()
                        std = float(d["std"])
                        print(std)
                        state = ParserState.ReadingDutyCycleOverallMin
                    continue

                case ParserState.ReadingDutyCycleOverallMin:
                    m = regex_duty_cycle_overall_data_min.match(line)
                    if m:
                        d = m.groupdict()
                        min = float(d["min"])
                        print(min)
                        state = ParserState.ReadingDutyCycleOverallMax
                    continue

                case ParserState.ReadingDutyCycleOverallMax:
                    m = regex_duty_cycle_overall_data_max.match(line)
                    if m:
                        d = m.groupdict()
                        max = float(d["max"])
                        print(max)
                        continue


    f.close()

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

if __name__ == '__main__':

    args = sys.argv
    parse_file(args[1])

