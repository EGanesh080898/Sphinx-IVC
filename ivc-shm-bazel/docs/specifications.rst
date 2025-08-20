Specifications
==============

.. spec:: Priority message queue
   :id: SPEC_001
   :status: in progress
   :tags: queue, design
   :links: TEST_001
   :source: ixfcm/src/driver/hv/acrn/ivcreaderwriter/IvcAcrnRW.cpp

   Implement a priority queue in IvcAcrnRW for handling messages based on
   urgency and importance.

.. spec:: Reader/Writer lock mechanism
   :id: SPEC_002
   :status: planned
   :tags: reliability
   :links: TEST_002
   :source: ixfcm/src/driver/hv/acrn/shmem-hal/lgivc_shmem_hal_rwlock.cpp

   Implement read/write lock in shared memory handling to prevent data
   corruption under concurrent access.
