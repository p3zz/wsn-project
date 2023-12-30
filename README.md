# Low-power Wireless Networking for IoT project
A firmware written in C above Contiki OS. The whole purpose of this project is to emulate a network of nodes represented by a tree with a root, in which every node exchanges different kind of data using 2 traffic patterns:

- from root to other nodes
- from a single node to root

The topology of the network is kept by the root node, using a table where each row contains the coupling between a node and its parent. Every node must have a single parent, but a single node can be parent of several nodes.

The simulation environment used is Cooja. The csc folder contains a bunch of simulation examples that uses different topologies and RX/TX ratio, noise etc.

## Build
```bash
make
```
## Simulate
```bash
cooja <csc_file>
```
or
```bash
cooja-nogui <csc_file>
```
## Automation tools
The repo provides a bunch of tools to simplify the simulation and analysis process

### simulate.sh
takes a folder containing 2 configuration files (udgm.csc, mrm.csc) as argument,
creates the test folder inside test/ with the current datetime as name,
then proceeds to run both simulations.
Once the simulations are complete, the folder struct will look like this:

    datetime/
        foldername/
            UDGM/
            MRM/

This script runs "simulate2.st" under the hood.

As an example, the csc/ folder is available inside the root directory of the project, while the resulting tests are available at test/

Use:
```bash
./simulate.sh <csc_folder>
```

Example:
```bash
./simulate.sh csc/plain/
```


### simulate2.sh
takes a .csc configuration file as argument, create the current state file of the project (values of some constants), create the map.csv, then run the no-gui simulation and writes the resulting file inside result.txt.

This script is used by simulate.sh

Use:
```bash
./simulate2.sh <csc_file> > result.txt
```

Example:
```bash
./simulate2.sh csc/plain/udgm.csc > result.txt
```

### parse-stats
Analyze a .log file created by cooja and returns a summary of the performances of the solution

Use:
```bash
python3 parse-stats.py <test.log>
```

### create-config and create-map
Both scripts search for specific files in order to create a snapshot of the current project configuration.

Use:
```bash
python3 create-config.py <cooja_config_path> > config.txt
python3 create-map.py <cooja_config_path> > map.csv
```

### analyze-energest, analyze-map, analyze-radio-success, analyze-result
Every script takes a specific file coming from parse-stats.py, parse its content and plot data.

Use:
```bash
python3 analyze-energest.py <energest_file>
python3 analyze-map.py <map_file>
python3 analyze-radio-success.py <map_file>
python3 analyze-result.py analyze <result_file> # use analyze for node-to-node analysis
python3 analyze-result.py <result_file1>,<result_file2>,<result_file3> # use compare for comparing more files

```

