#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"

// Semaphore libraries
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

SemaphoreHandle_t testaphore, twostaphore, threestaphore, semafour;
int counter;

#define MAIN_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define MAIN_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

#define SIDE_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define SIDE_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

void setUp(void) {}

void tearDown(void) {}

void test_semaphore_lock ()
{
    xSemaphoreTake(testaphore, portMAX_DELAY);

    bool semaphoreBlock = xSemaphoreTake(testaphore, 100); // Try to take the semaphore again without waiting for too long

    TEST_ASSERT_TRUE_MESSAGE(semaphoreBlock == false, "Semaphore creation incorrectly allowed (expected deadlock)");
    xSemaphoreGive(testaphore); 
}

void test_semaphore_give ()
{
    counter = 0;
    xSemaphoreTake(testaphore, portMAX_DELAY);
    counter++;
    xSemaphoreGive(testaphore);
    bool semaphoreBlock = xSemaphoreTake(testaphore, portMAX_DELAY);
    counter--;
    xSemaphoreGive(testaphore);
    TEST_ASSERT_TRUE_MESSAGE(counter == 0, "The semaphore did not give correctly");
}


void deadlock_thread_one ( void *) {
    xSemaphoreTake(threestaphore, portMAX_DELAY);
    vTaskDelay(200); // Delay to create deadlock

    xSemaphoreTake(semafour, portMAX_DELAY);
}

void deadlock_thread_two ( void *) {
    xSemaphoreTake(semafour, portMAX_DELAY);
    vTaskDelay(200); // Delay to create deadlock

    xSemaphoreTake(threestaphore, portMAX_DELAY);
}


void test_deadlock( void ) {

    TaskHandle_t dl_one, dl_two;
    TaskStatus_t statusDetailOne, statusDetailTwo;
    
    xTaskCreate(deadlock_thread_one, "DEADLOCK_TEST", MAIN_TASK_STACK_SIZE, NULL, 
                            MAIN_TASK_PRIORITY, &dl_one);

    xTaskCreate(deadlock_thread_two, "DEADLOCK_TESTTWO", SIDE_TASK_STACK_SIZE, NULL,
                            SIDE_TASK_PRIORITY, &dl_two);

    vTaskDelay(100);

    
    vTaskGetInfo(dl_one, &statusDetailOne, pdTRUE, eInvalid);
    vTaskGetInfo(dl_two, &statusDetailTwo, pdTRUE, eInvalid);
    
    eTaskState stateOne = statusDetailOne.eCurrentState;
    eTaskState stateTwo = statusDetailTwo.eCurrentState;

    /*
     typedef enum
        {
            eRunning = 0, A task is querying the state of itself, so must be running. 
            eReady,       The task being queried is in a ready or pending ready list. 
            eBlocked,     The task being queried is in the Blocked state. 
            eSuspended,   The task being queried is in the Suspended state, or is in the Blocked state with an infinite time out. 
            eDeleted,     The task being queried has been deleted, but its TCB has not yet been freed. 
            eInvalid      Used as an 'invalid state' value.
        } eTaskState;
     */


    TEST_ASSERT_TRUE_MESSAGE((int)stateOne == 2, "Thread 1 gained access to resource 2.");
    TEST_ASSERT_TRUE_MESSAGE((int)stateTwo == 2, "Thread 2 gained access to resource 1.");

    vTaskSuspend(dl_one);
    vTaskSuspend(dl_two);

    vTaskDelete(dl_one);
    vTaskDelete(dl_two);

}


void orphaned_lock(void*)
{
    while (1) {
        xSemaphoreTake(twostaphore, portMAX_DELAY);
        counter++;
        if (counter % 2) {
            continue;
        }
        //printf("Count %d\n", counter);
        xSemaphoreGive(twostaphore);
    }
}


void test_orphan( void ) {

    TaskHandle_t o_one;
    TaskStatus_t statusDetailOne;
    
    xTaskCreate(orphaned_lock, "DEADLOCK_TEST", MAIN_TASK_STACK_SIZE, NULL, 
                            MAIN_TASK_PRIORITY, &o_one);


    vTaskDelay(100);

    
    vTaskGetInfo(o_one, &statusDetailOne, pdTRUE, eInvalid);
    
    eTaskState stateOne = statusDetailOne.eCurrentState;

    TEST_ASSERT_TRUE_MESSAGE((int)stateOne == 2, "The orphaned lock thread should be blocked.");

    vTaskSuspend(o_one);

    vTaskDelete(o_one);

}


void orphaned_lock_fixed(void*)
{
    while (1) {
        xSemaphoreTake(testaphore, portMAX_DELAY);
        counter++;
        if (counter % 2) {
            xSemaphoreGive(testaphore);
            continue;
        }
        //printf("Count %d\n", counter);
        xSemaphoreGive(testaphore);
    }
}

void test_orphan_fix( void ) {

    TaskHandle_t o_one;
    TaskStatus_t statusDetailOne;
    
    xTaskCreate(orphaned_lock_fixed, "DEADLOCK_TEST", MAIN_TASK_STACK_SIZE, NULL, 
                            MAIN_TASK_PRIORITY, &o_one);


    vTaskDelay(100);

    
    vTaskGetInfo(o_one, &statusDetailOne, pdTRUE, eInvalid);
    
    eTaskState stateOne = statusDetailOne.eCurrentState;
    //printf("Orphaned lock fixed task state: %d\n", (int)stateOne);
    TEST_ASSERT_TRUE_MESSAGE(((int)stateOne == 0) || ((int)stateOne == 1), "The fixed orphaned lock thread should be running or ready"); 

    vTaskSuspend(o_one);

    vTaskDelete(o_one);

}

void thread_testing(void *){
    while (1) {
        /*  uxMaxCount -- The maximum count that can be reached.
            uxInitialCount -- The count value assigned to the semaphore when it is created.
        */
        testaphore    = xSemaphoreCreateCounting(1, 1);
        twostaphore   = xSemaphoreCreateCounting(1, 1);
        threestaphore = xSemaphoreCreateCounting(1, 1);
        semafour      = xSemaphoreCreateCounting(1, 1);

        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_semaphore_lock);
        RUN_TEST(test_semaphore_give);
        RUN_TEST(test_deadlock);
        RUN_TEST(test_orphan);
        RUN_TEST(test_orphan_fix);
        printf("All done!\n");
        sleep_ms(5000);
        UNITY_END();
    }
  
}
int main (void)
{
    stdio_init_all();
    sleep_ms(5000);

    xTaskCreate(thread_testing, "ThreadTesting",
                MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL);

    vTaskStartScheduler(); // This function should never return.
}

