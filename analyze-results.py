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
    regex_node_overall_data_pdr = re.compile(r"Overall PDR = (?P<pdr>\d+\.\d+)")
    regex_node_overall_data_plr = re.compile(r"Overall PLR = (?P<plr>\d+\.\d+)")

    regex_duty_cycle_header = re.compile(r"----- Duty Cycle Statistics -----")
    regex_duty_cycle_data = re.compile(r"Node (?P<node_id>\d+):  Duty Cycle: (?P<duty_cycle>\d+\.\d+)")

    regex_duty_cycle_overall_header = re.compile(r"----- Duty Cycle Overall Statistics -----")
    regex_duty_cycle_overall_data_avg = re.compile(r"Average Duty Cycle: (?P<avg>\d+\.\d+)")
    regex_duty_cycle_overall_data_std = re.compile(r"Standard Deviation: (?P<std>\d+\.\d+)")
    regex_duty_cycle_overall_data_max = re.compile(r"Minimum: (?P<max>\d+\.\d+)")
    regex_duty_cycle_overall_data_min = re.compile(r"Maximum: (?P<min>\d+\.\d+)")



    # Parse log file and add data to CSV files
    with open(file, 'r') as f:
        state = ParserState.Init
        for line in f:
            match state:
                case ParserState.Init:
                    m = regex_data_collection_header.match(line)
                    if m:
                        state = ParserState.ReadingDataCollectionData
                case ParserState.ReadingDataCollectionData:
                    m = regex_node_data.match(line)
                    if m:
                        d = m.groupdict()
                        node_id = int(d["node_id"])
                        tx = int(d["tx"])
                        rx = int(d["rx"])
                        pdr = float(d["pdr"])
                        plr = float(d["plr"])
                        print(node_id, tx, rx, pdr, plr)
    f.close()

class ParserState:
    Init = 0
    ReadingDataCollectionData = 1
    ReadingDataCollectionOverallData = 2
    ReadingSourceRoutingData = 3
    ReadingSourceRoutingOverallData = 4

if __name__ == '__main__':

    args = sys.argv
    parse_file(args[1])

