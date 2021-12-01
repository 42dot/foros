# FOROS

**Failover ROS framework**

[![pipeline status](https://gitlab.42dot.ai/engineering/ak/akitos/failover/foros/badges/main/pipeline.svg)](https://gitlab.42dot.ai/engineering/ak/akitos/failover/foros/-/commits/main) [![coverage report](https://gitlab.42dot.ai/engineering/ak/akitos/failover/foros/badges/main/coverage.svg)](https://gitlab.42dot.ai/engineering/ak/akitos/failover/foros/-/commits/main)

## Purpose
---
[FOROS](https://gitlab.42dot.ai/engineering/ak/akitos/failover/foros) is a [ROS2](https://docs.ros.org/en/galactic/index.html) framework that provides the ability to construct a active-standby cluster  based on the [RAFT](https://raft.github.io/) consensus algorithm. 

### Key Features
| Feature           | Description                                                              |
| ----------------- | ------------------------------------------------------------------------ |
| Node Redundancy   | Maintain a cluster of nodes runing in parallel to acheive a common goal. |
| State Replication | Replicate a state of nodes for implementing a fault-tolerant service.    |

### Goals
This framework can tolerate fail-stop failures equal to the cluster size minus the quorum.
| Cluster size (N) | Quorum (Q = N / 2 + 1) | Number of fault tolerant nodes (N - Q) |
| :--------------: | :--------------------: | :------------------------------------: |
|        1         |           1            |                 **0**                  |
|        2         |           2            |                 **0**                  |
|        3         |           2            |                 **1**                  |
|        4         |           3            |                 **1**                  |
|        5         |           3            |                 **2**                  |

## Prerequisites (Ubuntu 20.04)
---
### Install ROS2 galactic
Please refer to the [official site](https://docs.ros.org/en/galactic/Installation/Ubuntu-Install-Debians.html) to install.

### Install leveldb
```bash
sudo apt install libleveldb-dev
```

## Build
---
> If you want to install to existing ROS2 workspace, please clone this source in the workspace in advance.

```bash
. /opt/ros/galactic/setup.bash
colcon build
```

## Getting Started
---
TBD (Hello World guide)

## Technical Articles
---
TBD (include API reference documents)