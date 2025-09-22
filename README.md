# Lab 3
This is the Lab 3 submission for Burke Dambly and James Semerad.
The submission contains the test.c file in the test/ directory, along with other FreeRTOS setup files. 

## test.c
test.c contains tests for semaphore and task functionality. Below are descriptions of their purpose.

### test_semaphore_lock
Tests basic semaphore functionality, disallowing a task from using a resource using xSemaphoreTake() without xSemaphoreGive().

### test_semaphore_give
Tests semaphore ability to give after it has taken a resource, using counter variable.

### test_deadlock
Tests for classic deadlock, where thread 1 locks resource A, looks for resource B
                                     thread 2 locks resource B, looks for resource A

### test_orphan
Creates an orphaned lock, ensures that it is blocked

### test_orphan_fix
Tests fixing an orphaned lock by calling xSemaphoreGive() intermittently as the thread operates.