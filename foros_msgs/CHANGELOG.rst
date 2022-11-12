^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Changelog for package foros_msgs
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

0.4.1 (2022-11-13)
------------------

0.4.0 (2022-10-27)
------------------
* version 0.4.0
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
* foros_msgs: add timestamp in inspector msg
* foros_msgs: add cluster size field in Inspector msg
* foros_msgs: add state in inspector msg
  - add current state field
  - change last_data_index to data_size
* foros_msgs: add inspector message IDL
  define inspector IDL for monitoring nodes in cluster
* Contributors: Wonguk Jeong

0.1.0 (2021-12-22)
------------------
* foros_msgs: refine package dependencies
* foros_msgs: rename fields
  - data -> entries
  - prev_data_term -> prev_log_term
  - prev_data_index -> prev_log_index
* rename failsafe::fsros to failover::foros
* Contributors: Wonguk Jeong
