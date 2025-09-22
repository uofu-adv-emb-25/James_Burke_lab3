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
    counter = 0;
    xSemaphoreTake(testaphore, portMAX_DELAY);
    counter++;

    bool semaphoreBlock = xSemaphoreTake(testaphore, portMAX_DELAY);
    counter--;

    TEST_ASSERT_TRUE_MESSAGE(semaphoreBlock == false, "Semaphore creation incorrectly allowed (expected deadlock)");
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

void test_deadlock( void ) {
    TaskHandle_t dl_one, dl_two;

    void * pvParameters = (&threestaphore, &semafour);

    xTaskCreate(deadlock_one, "DEADLOCK_TEST", MAIN_TASK_STACK_SIZE, NULL, 
                            MAIN_TASK_PRIORITY, &dl_one);

    xTaskCreate(deadlock_two, "DEADLOCK_TESTTWO", SIDE_TASK_STACK_SIZE, NULL,
                            SIDE_TASK_PRIORITY, &dl_two);

    vTaskDelay(100);
    xTaskSuspend(deadlock_one);
    xTaskSuspend(deadlock_two);


    TaskStatus_t statusDetailOne;
                    statusDetailTwo;
                    
    vTaskGetInfo(&dl_one, &statusDetailOne, 
                    pdTRUE, eInvalid);
    vTaskGetInfo(&dl_two, &statusDetailOne
                    pdTRUE, eInvalid);

    eTaskState stateOne = statusDetailOne.eCurrentState;
    eTaskState stateOne = statusDetailTwo.eCurrentState;

    printf("Deadlock expected and Task one is currently: ");
    printf("Deadlock expected and Task two is currently: ");

    TEST_ASSERT_TRUE_MESSAGE(true, "Error: reached end of expected code");
}

void deadlock_one ( void ) {
    xSemaphoreTake(threestaphore, portMAX_DELAY);
    vTaskDelay(200); // Delay to create deadlock

    xSemaphoreTake(semafour,      portMAX_DELAY);
}

void deadlock_two ( void ) {
    xSemaphoreTake(semafour, portMAX_DELAY);
    vTaskDelay(200); // Delay to create deadlock

    xSemaphoreTake(threestaphore, portMAX_DELAY);
}


int main (void)
{
    stdio_init_all();
    sleep_ms(5000);

    while (1) {
        /*  uxMaxCount -- The maximum count that can be reached.
            uxInitialCount -- The count value assigned to the semaphore when it is created.
        */
        testaphore    = xSemaphoreCreateCounting(1, 1);
        //twostaphore   = xSemaphoreCreateCounting(1, 1);
        threestaphore = xSemaphoreCreateCounting(1, 1);
        semafour      = xSemaphoreCreateCounting(1, 1);

        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_semaphore_lock);
        RUN_TEST(test_semaphore_give);

        RUN_TEST(test_deadlock);
        sleep_ms(5000);
        UNITY_END();
    }
}
