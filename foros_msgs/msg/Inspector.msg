builtin_interfaces/Time stamp   # timestamp
string cluster_name             # cluster name
uint32 cluster_size             # cluster size
uint32 id                       # id
uint8 state                     # current state
uint64 term                     # current term
uint64 data_size                # data size
uint32 voted_for                # id voted by this node

uint8 UNKNOWN = 0
uint8 STANDBY = 1
uint8 FOLLOWER = 2
uint8 CANDIDATE = 3
uint8 LEADER = 4
string TOPIC_NAME = "foros/inspector"
