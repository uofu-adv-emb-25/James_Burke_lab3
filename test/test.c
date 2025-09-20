#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"

// Semaphore libraries
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

SemaphoreHandle_t testaphore, twostaphore;
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


int main (void)
{
    stdio_init_all();
    sleep_ms(5000);

    while (1) {
        /*  uxMaxCount -- The maximum count that can be reached.
            uxInitialCount -- The count value assigned to the semaphore when it is created.
        */
        testaphore = xSemaphoreCreateCounting(1, 1);
    
        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_semaphore_lock);
        RUN_TEST(test_semaphore_give);
        sleep_ms(5000);
        UNITY_END();
    }
}
