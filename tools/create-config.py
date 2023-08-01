import re
import sys
import os

def parse_simulation_config(filename: str):
    regex_random_seed = re.compile(r"^(.*)<randomseed>(?P<random_seed>\d+)</randomseed>")
    regex_mote_delay = re.compile(r"^(.*)<motedelay_us>(?P<mote_delay>\d+)</motedelay_us>")
    regex_success_ratio_tx = re.compile(r"^(.*)<success_ratio_tx>(?P<ratio_tx>\d+(\.\d+)?)</success_ratio_tx>")
    regex_success_ratio_rx = re.compile(r"^(.*)<success_ratio_rx>(?P<ratio_rx>\d+(\.\d+)?)</success_ratio_rx>")

    random_seed = None
    mote_delay = None
    success_ratio_tx = None
    success_ratio_rx = None

    with open(filename, 'r') as f:
        for line in f:
            m = regex_random_seed.match(line)
            if(m):
                d = m.groupdict()
                random_seed = d["random_seed"]
                continue
            m = regex_mote_delay.match(line)
            if(m):
                d = m.groupdict()
                mote_delay = d["mote_delay"]
                continue
            m = regex_success_ratio_tx.match(line)
            if(m):
                d = m.groupdict()
                success_ratio_tx = d["ratio_tx"]
                continue
            m = regex_success_ratio_rx.match(line)
            if(m):
                d = m.groupdict()
                success_ratio_rx = d["ratio_rx"]
                continue
    
    f.close()
    return random_seed, mote_delay, success_ratio_rx, success_ratio_tx

def parse_project_config(filename: str):

    regex_rdc = re.compile(r"^#define NETSTACK_CONF_RDC (?P<rdc>\w+)")

    rdc = None

    with open(filename, 'r') as f:
        for line in f:
            m = regex_rdc.match(line)
            if(m):
                d = m.groupdict()
                rdc = d["rdc"]
                continue
    
    f.close()
    return rdc

def parse_my_collect(filename: str):

    regex_report_period = re.compile(r"^#define TOPOLOGY_REPORT_PERIOD \((?P<report_period>\d+)")
    regex_report_delay = re.compile(r"^#define TOPOLOGY_REPORT_DELAY \((?P<report_delay>\d+)")
    regex_report_enabled = re.compile(r"^#define TOPOLOGY_REPORT_ENABLED (?P<report_enabled>\d)")
    regex_beacon_period = re.compile(r"^#define BEACON_INTERVAL \((?P<beacon_period>\d+)")
    regex_nbnr_enabled = re.compile(r"^#define NBNR_ENABLED (?P<nbnr_enabled>\d)")
    regex_npnr_enabled = re.compile(r"^#define NPNR_ENABLED (?P<npnr_enabled>\d)")

    report_period = None
    report_delay = None
    report_enabled = None
    beacon_period = None

    with open(filename, 'r') as f:
        for line in f:
            m = regex_report_period.match(line)
            if(m):
                d = m.groupdict()
                report_period = int(d["report_period"])
                continue
            m = regex_report_delay.match(line)
            if(m):
                d = m.groupdict()
                report_delay = int(d["report_delay"])
                continue
            m = regex_report_enabled.match(line)
            if(m):
                d = m.groupdict()
                report_enabled = int(d["report_enabled"])
                continue
            m = regex_beacon_period.match(line)
            if(m):
                d = m.groupdict()
                beacon_period = int(d["beacon_period"])
                continue
            m = regex_nbnr_enabled.match(line)
            if(m):
                d = m.groupdict()
                nbnr_enabled = int(d["nbnr_enabled"])
                continue
            m = regex_npnr_enabled.match(line)
            if(m):
                d = m.groupdict()
                npnr_enabled = int(d["npnr_enabled"])
                continue

    f.close()
    return report_period, report_delay, report_enabled, beacon_period, nbnr_enabled, npnr_enabled

def parse_app(filename: str):

    regex_upward_traffic = re.compile(r"^#define APP_UPWARD_TRAFFIC (?P<upward_traffic>\d)")
    regex_downward_traffic = re.compile(r"^#define APP_DOWNWARD_TRAFFIC (?P<downward_traffic>\d)")
    regex_msg_period = re.compile(r"^#define MSG_PERIOD \((?P<msg_period>\d+)")
    regex_msg_delay = re.compile(r"^#define MSG_DELAY \((?P<msg_delay>\d+)")
    regex_sr_msg_period = re.compile(r"^#define SR_MSG_PERIOD \((?P<sr_msg_period>\d+)")
    regex_sr_msg_delay = re.compile(r"^#define SR_MSG_DELAY \((?P<sr_msg_delay>\d+)")

    upward_traffic = None
    downward_traffic = None
    msg_period = None
    msg_delay = None
    sr_msg_period = None
    sr_msg_delay = None

    with open(filename, 'r') as f:
        for line in f:
            m = regex_upward_traffic.match(line)
            if(m):
                d = m.groupdict()
                upward_traffic = int(d["upward_traffic"])
                continue
            m = regex_downward_traffic.match(line)
            if(m):
                d = m.groupdict()
                downward_traffic = int(d["downward_traffic"])
                continue
            m = regex_msg_period.match(line)
            if(m):
                d = m.groupdict()
                msg_period = int(d["msg_period"])
                continue
            m = regex_msg_delay.match(line)
            if(m):
                d = m.groupdict()
                msg_delay = int(d["msg_delay"])
                continue
            m = regex_sr_msg_period.match(line)
            if(m):
                d = m.groupdict()
                sr_msg_period = int(d["sr_msg_period"])
                continue
            m = regex_sr_msg_delay.match(line)
            if(m):
                d = m.groupdict()
                sr_msg_delay = int(d["sr_msg_delay"])
                continue
            
    f.close()
    return upward_traffic, downward_traffic, msg_period, msg_delay, sr_msg_period, sr_msg_delay


if __name__ == '__main__':

    args = sys.argv
    cooja_config_path = args[1]
    app_filename = "app.c"
    my_collect_filename = "my_collect.h"
    project_config_filename = "project-conf.h"
    
    upward_traffic, downward_traffic, msg_period, msg_delay, sr_msg_period, sr_msg_delay = parse_app(app_filename)
    report_period, report_delay, report_enabled, beacon_period, nbnr_enabled, npnr_enabled = parse_my_collect(my_collect_filename)
    rdc = parse_project_config(project_config_filename)
    random_seed, mote_delay, success_ratio_rx, success_ratio_tx = parse_simulation_config(cooja_config_path)

    output = ""
    output += "UPWARD_TRAFFIC={}\n".format(upward_traffic)
    output += "DOWNWARD_TRAFFIC={}\n".format(downward_traffic)
    output += "MSG_PERIOD={}\n".format(msg_period)
    output += "MSG_DELAY={}\n".format(msg_delay)
    output += "SR_MSG_PERIOD={}\n".format(sr_msg_period)
    output += "SR_MSG_DELAY={}\n".format(sr_msg_delay)
    output += "REPORT_PERIOD={}\n".format(report_period)
    output += "REPORT_DELAY={}\n".format(report_delay)
    output += "REPORT_ENABLED={}\n".format(report_enabled)
    output += "BEACON_PERIOD={}\n".format(beacon_period)
    output += "RDC={}\n".format(rdc)
    output += "MOTE_DELAY={}\n".format(mote_delay)
    output += "RANDOM_SEED={}\n".format(random_seed)
    output += "SUCCESS_RATIO_RX={}\n".format(success_ratio_rx)
    output += "SUCCESS_RATIO_TX={}\n".format(success_ratio_tx)
    output += "NBNR_ENABLED={}\n".format(nbnr_enabled)
    output += "NPNR_ENABLED={}\n".format(npnr_enabled)

    print(output)
