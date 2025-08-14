These test apps are meant to test intervm communication and performance.
So, the test apps have to be run on the VMs with respect to which communication is being tested.
Most the apps work in pair(reader and writer).
So if one or more reader is launched on VM1, same number of writers have to launched on VM2.
Some of the test apps expect user to press "Ctrl+C" to stop the test and display the results.
Run apps with --help options to check for args to be passed.
Good luck!!



EXAMPLES:

1. Throughput Test (1MB chunk) - ivcrw-throughput-test.cpp
# On Reader Side(VM1), run the command
$ ./bin/ivcrw-throughput-test -t r -c 1 -tp test -bs 1048576
# On Writer Side(VM2), run the command
$ ./bin/ivcrw-throughput-test -t w -c 1 -tp test -bs 1048576

2. Generic test (1MB chunk) - ivcrw-generic-test.cpp
# On Reader Side(VM1), run the command
$ ./bin/ivcrw-generic-test -t r -c 1 -tp test -bs 1048576
# On Writer Side(VM2), run the command
$ ./bin/ivcrw-generic-test -t w -c 1 -tp test -bs 1048576

3. CPU Load test (1MB chunk) - ivcrw-cpuload-test.cpp
# On Reader Side(VM1), run the command
./bin/ivcrw-cpuload-test -t r -c 1 -tp test -bs 1048576
# On Writer Side(VM2), run the command
./bin/ivcrw-cpuload-test -t w -c 1 -tp test -bs 1048576

4. Latency test (1MB chunk) - ivcrw-latency-test.cpp
# On Reader Side(VM1), run the command
./bin/ivcrw-latency-test -t r -c 1 -tp test -bs 1048576
# On Writer Side(VM2), run the command
./bin/ivcrw-latency-test -t w -c 1 -tp test -bs 1048576

5. Long Duration test (1MB chunk) - ivcrw-longduration-test
# On Reader Side(VM1), run the command
./bin/ivcrw-longduration-test -t r -c 1 -tp test -bs 1048576 -dt 11s
# On Writer Side(VM2), run the command
./bin/ivcrw-longduration-test -t w -c 1 -tp test -bs 1048576 -dt 11s

6. Memory Allocation test - ivcrw-memoryallocation-test
# On any VM, run the command
./bin/ivcrw-memoryallocation-test -count 2 -topic test

7. Memory Usage test (1MB chunk) - ivcrw-memusage-test
# On Reader Side(VM1), run the command
./bin/ivcrw-memusage-test -t r -c 1 -tp test -bs 1048576
# On Writer Side(VM2), run the command
./bin/ivcrw-memusage-test -t w -c 1 -tp test -bs 1048576

8. ReaderWriter Up/Down test (1MB chunk) - ivcrw-updown-test
# On Reader Side(VM1), run the command
./bin/ivcrw-updown-test -t r -c 1 -tp test -bs 1048576
# On Writer Side(VM2), run the command
./bin/ivcrw-updown-test -t w -c 1 -tp test -bs 1048576

9. API Sanity test - ivcrw-complete-apisanity-test
# On Reader Side(VM1), run the command
./bin/ivcrw-complete-apisanity-test -type r -count 1 -topic test -bufsize 1048576
# On Writer Side(VM2), run the command
./bin/ivcrw-complete-apisanity-test -type w -count 1 -topic test -bufsize 1048576
# Select the test case to be executed and check if the test case is passing
