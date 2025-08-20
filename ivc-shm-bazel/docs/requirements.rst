Requirements
============

.. req:: IVC must transmit messages under 100 ms
   :id: REQ_001
   :status: open
   :tags: performance, critical
   :links: SPEC_001

   Messages between Reader and Writer must arrive in under 100 milliseconds
   under normal CPU load.

.. req:: Shared memory must be lock-safe
   :id: REQ_002
   :status: draft
   :tags: reliability
   :links: SPEC_002

   All concurrent access to shared memory must be safe and free from deadlocks.

Traceability
------------

.. needflow::
   :filter: id in ["REQ_001", "REQ_002", "SPEC_001", "SPEC_002", "TEST_001", "TEST_002"]
   :link_types: links
   :show: id, title, status, tags
