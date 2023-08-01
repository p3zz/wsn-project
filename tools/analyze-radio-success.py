import matplotlib.pyplot as plt
import sys
import csv

def parse_file(filename: str):
    data_collection_coll = []
    source_routing_coll = []
    topology_report_coll = []
    duty_cycle_coll = []

    with open(filename, mode='r') as csv_file:
        csv_reader = csv.DictReader(csv_file, delimiter=',')
        for row in csv_reader:
            nbnr = int(row["nbnr"]) == 1
            npnr = int(row["npnr"]) == 1
            success_ratio = float(row["success_ratio"])
            data_collection = float(row["data_collection"])
            source_routing = float(row["source_routing"])
            topology_report = float(row["topology_report"])
            duty_cycle = float(row["duty_cycle"])

            data_collection_coll.append((nbnr, npnr, success_ratio, data_collection))
            source_routing_coll.append((nbnr, npnr, success_ratio, source_routing))
            topology_report_coll.append((nbnr, npnr, success_ratio, topology_report))
            duty_cycle_coll.append((nbnr, npnr, success_ratio, duty_cycle))

    csv_file.close()

    return data_collection_coll, source_routing_coll, topology_report_coll, duty_cycle_coll

def plot_collection(collection, name: str):
    plt.title(name)
    x = list(set([elem[2] for elem in collection]))
    x.sort(reverse=True)
    y = [elem[3] for elem in collection if not elem[0] and not elem[1]]
    plt.plot(x, y, label="plain")
    y = [elem[3] for elem in collection if elem[0] and not elem[1]]
    plt.plot(x, y, label="nbnr")
    y = [elem[3] for elem in collection if elem[0] and elem[1]]
    plt.plot(x, y, label="nbnr+npnr")
    plt.legend()
    plt.xlabel("RX/TX success ratio")
    plt.ylabel("PDR %")

if __name__ == "__main__":
    filename = sys.argv[1]
    dc_coll, sr_coll, tr_coll, dcy_coll = parse_file(filename)
    plt.subplot(2,2,1)
    plot_collection(dc_coll, "Data collection PDR")
    plt.subplot(2,2,2)
    plot_collection(sr_coll, "Source routing PDR")
    plt.subplot(2,2,3)
    plot_collection(tr_coll, "Topology report PDR")
    plt.subplot(2,2,4)
    plot_collection(dcy_coll, "Duty Cycle")
    plt.show()