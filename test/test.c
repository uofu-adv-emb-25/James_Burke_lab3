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

void test_main_thread (void* pvParams) 
{
    xSemaphoreTake(twostaphore, portMAX_DELAY);
    if (counter < 100) {
        printf("printing counter from %s, counter = %d", "main", counter);
        counter++;
    }

    else
        xSemaphoreGive(twostaphore); 
}

void test_side_thread (void* pvParams) 
{
    xSemaphoreTake(twostaphore, portMAX_DELAY);
    if (counter < 100)
        printf("printing counter from %s, counter = %d", "main", counter);    
    else
        xSemaphoreGive(twostaphore);
}

void test_threads_func () {
    counter = 0;
    TaskHandle_t main, side;
    twostaphore = xSemaphoreCreateCounting(1, 1);

    xTaskCreate(test_main_thread, "TestMainThread",
                MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, &main);
    xTaskCreate(test_side_thread, "TestSideThread",
                SIDE_TASK_STACK_SIZE, NULL, SIDE_TASK_PRIORITY, &side);
    //vTaskStartScheduler();
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
        RUN_TEST(test_threads_func);

        sleep_ms(5000);
        UNITY_END();
    }
}
