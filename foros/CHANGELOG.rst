^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package foros
^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.4.1 (2022-11-13)
------------------
* foros: update package.xml test_depend
  add ament_cmake_gtest, ament_cmake_gmock
* foros: update package.xml depend
  libleveldb-dev -> leveldb
* Contributors: Wonguk Jeong

0.4.0 (2022-10-27)
------------------
* version 0.4.0
* foros: remove private member access from public method
* Added humble branch (#1)
  foros: migration to humble hawksbill
  - asnyc_send_request() returns a std::future instead of a std::shared_future
  - get_callback_group() is replaced with for_each_callback_group()
  - publish() is not a virtual function anymore
* foros: fix unused variable, uninitialize fields
  fix CID 257409, 257410, 257421, 257426, 257433
* docs: integrate coverity scan, remove dead link
* Contributors: Taehun Lim, Wonguk Jeong

0.3.1 (2022-04-27)
------------------
* Version 0.3.1
  - inspector UI enhanced
  - unit test updated
* foros: unittest updated
  add raft unittest when inspector is enabled
* docs: inspector screenshot updated
* Contributors: Wonguk Jeong

0.3.0 (2022-04-25)
------------------
* docs: doxygen updated
  show test result using inspector not console
* Version 0.3.0
  - inspector introduced
  - locking added for multithreading
* foros: remove unnecessary logs
* foros: add default period of instpector
* foros: fill timstamp in inspector msg
* foros: send cluster_size to inspector
* foros: inspector implemented
  enable inspector with environment variables
  - FOROS_INSPECTOR : 1 to enable
  - FOROS_INSPECTOR_PERIOD : inspector message publish period
  if inspector is enabled, publish inspector message every
  FOROS_INSPECTOR_PERIOD second(s)
* foros: add mutex for supporting multithread
  add mutex for
  - callback of observable
  - state transition of state machine
  - data getter/setter of context store
  - commit/revert callback of context
  - index of other node
* docs: fix test failure of doxygen mainpage
  add new line at the end of foros/docs/foros.hpp
* docs: doxygen main page added
* Contributors: Wonguk Jeong

0.1.0 (2021-12-22)
------------------
* foros: raft state machine TC added
* foros: more unit TC
* foros: unit TC added
  - add more test for cluster node class
  - add raft context, context store test cases
* foros: refine functio signature
* foros: raft TC added, coverage script added
* foros: context store ldata loading bug fix
* foros_msgs: rename fields
  - data -> entries
  - prev_data_term -> prev_log_term
  - prev_data_index -> prev_log_index
* foros: maintain command logs by framework not user
  as-is: maintains command logs (data) outside (by user)
  to-be: maintains command logs internally using leveldb (by framework)
* foros: maintain persistent data using leveldb
  election_term, voted_for, voted values should be
  stored in storage. (deleted on reboot)
  Therefore,
  1) created raft context store using leveldb
  2) add options to set temp directory. default is
  std::filesystem::temp_directory_path()
  TODO: add command logs in store
* foros: check validity of raft RPC
  check whether the RAFT message is sent
  by node which is not included in the cluster.
* foros: quorum calculation changed
  as-is: calculate quorum from the number of available candidates
  to-be: calculate quorum from full cluster
* foros: use rclcpp logger not std::cerr, std::cout
* foros: data commit API implemented
* foros: fix method signiture mismatch
* foros: refine cluster node data itnerface
  Do not use internal terminology in APIs
  - index -> id
  - term -> sub_id
* foros: align doxygen style
* foros: add data constructor
* foros: specify missing const arg of function
* foros: update commit index, next index as needed
  1) update last commit index,
  - on local commit
  - on rollback commit
  2) update next indeice of other nodes, when the node becomes leader
* foros: send current commit index in the vote request RPC
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
* foros: rename callbacks of data interface
* foros: initialize the last commit index
* foros: add data replicatoin logic in RPC response
* foros: the lastest data request interface added
* foros: CommitInfo class added
  CommitInfo stores the index, the term, request counts.
  It used for storing commit information in the pending commits map.
* fsros: add prev commt info in Data class
  To achieve raft data duplication, need to pass previous commit index
  and previous commit term.
* foros: store term, request count in pending commit
* foros: rename interfaces, add more doxygen
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
