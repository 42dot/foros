^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package foros_examples
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.4.1 (2022-11-13)
------------------

0.4.0 (2022-10-27)
------------------
* version 0.4.0
* foros_examples: fix missing exception handler
* Contributors: Wonguk Jeong

0.3.1 (2022-04-27)
------------------
* Version 0.3.1
  - inspector UI enhanced
  - unit test updated
* Contributors: Wonguk Jeong

0.3.0 (2022-04-25)
------------------
* Version 0.3.0
  - inspector introduced
  - locking added for multithreading
* foros_examples: support ros2 run
  change install directory from 'bin' to 'lib/foros_examples'
* foros_examples: rename node
* Contributors: Wonguk Jeong

0.1.0 (2021-12-22)
------------------
* foros_examples: add missing dependency
  std_msgs dependency added in package.xml, CMakeLists.txt
* foros_examples: log replication test updated
  - "clsuter_log_replication_with_commit" commit every 1 sec
  - "cluster_log_replication_withou_commit" check replication by callback
* foros_example/cluster_service: support test with given cluster size
* foros_example/cluster_publisher: support test with given cluster size
* foros_examples/cluster_election: support test with given cluster size
* foros: use rclcpp logger not std::cerr, std::cout
* foros: data commit API implemented
* foros: refine cluster node data itnerface
  Do not use internal terminology in APIs
  - index -> id
  - term -> sub_id
* foros: update commit index, next index as needed
  1) update last commit index,
  - on local commit
  - on rollback commit
  2) update next indeice of other nodes, when the node becomes leader
* foros: data replication logic added (phase 1)
  - implemented replication using broadcast
  - example added
  TODO:
  - bug fix
  - implement replication using data commit API
* foros: add lifecycle callback register APIs
  The virtual function is not guarenteed to be invoked in the constructor.
  Therefore, uses register APIs not overriding
  As-Is: overriding using interface
  To-Be: register using API explicitly
* foros: add data replicatoin logic in RPC response
* fsros: add prev commt info in Data class
  To achieve raft data duplication, need to pass previous commit index
  and previous commit term.
* foros: data commit interface added
  for data replication,
  1) add data interface API
  2) data commit API added in raft context
* foros: align coding style (CommitData, CommitResponse)
* foros: change commit data API to asnyc API
  This API need to make a consensus in the cluster.
  Therefore, change to async API.
  - use promise-future
  - for now, return result immediately
  until the business logic is implemented.
* rename failsafe::fsros to failover::foros
* Contributors: Wonguk Jeong
