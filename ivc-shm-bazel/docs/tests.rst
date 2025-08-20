Tests
=====

.. test:: Measure latency between two nodes
   :id: TEST_001
   :status: passed
   :tags: performance
   :links: REQ_001, SPEC_001
   :source: ixfcm/src/interface/readerwriter/tests/ivcrw-latency-test.cpp

   Run latency test to ensure messages meet the 100 ms requirement.

.. test:: Simulate concurrent access
   :id: TEST_002
   :status: failed
   :tags: reliability
   :links: REQ_002, SPEC_002
   :source: ixfcm/src/driver/hv/acrn/tests/ivcrw-get-shmeminfo.cpp

   Simulate concurrent read/write operations to validate lock safety.
