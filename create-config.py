import re

def parse_simulation_config(filename: str):
    regex_random_seed = re.compile(r"^(.*)<randomseed>(?P<random_seed>\d+)</randomseed>")
    regex_mote_delay = re.compile(r"^(.*)<motedelay_us>(?P<mote_delay>\d+)</motedelay_us>")

    random_seed = None
    mote_delay = None

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

    return random_seed, mote_delay

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

    return rdc

def parse_my_collect(filename: str):

    regex_report_period = re.compile(r"^#define TOPOLOGY_REPORT_PERIOD \((?P<report_period>\d+)")
    regex_report_delay = re.compile(r"^#define TOPOLOGY_REPORT_DELAY \((?P<report_delay>\d+)")
    regex_report_enabled = re.compile(r"^#define TOPOLOGY_REPORT_ENABLED (?P<report_enabled>\d)")
    regex_beacon_period = re.compile(r"^#define BEACON_INTERVAL \((?P<beacon_period>\d+)")

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

    return report_period, report_delay, report_enabled, beacon_period

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

    return upward_traffic, downward_traffic, msg_period, msg_delay, sr_msg_period, sr_msg_delay


if __name__ == '__main__':

    app_filename = "app.c"
    my_collect_filename = "my_collect.h"
    project_config_filename = "project-conf.h"
    udgm_config_filename = "test_nogui.csc"
    mrm_config_filename = "test_nogui_mrm.csc"

    upward_traffic, downward_traffic, msg_period, msg_delay, sr_msg_period, sr_msg_delay = parse_app(app_filename)
    report_period, report_delay, report_enabled, beacon_period = parse_my_collect(my_collect_filename)
    rdc = parse_project_config(project_config_filename)
    ugdm_random_seed, udgm_mote_delay = parse_simulation_config(udgm_config_filename)
    mrm_random_seed, mrm_mote_delay = parse_simulation_config(mrm_config_filename)

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
    output += "UDGM_RANDOM_SEED={}\n".format(ugdm_random_seed)
    output += "UDGM_MOTE_DELAY={}\n".format(udgm_mote_delay)
    output += "MRM_RANDOM_SEED={}\n".format(mrm_random_seed)
    output += "MRM_MOTE_DELAY={}\n".format(mrm_mote_delay)

    print(output)