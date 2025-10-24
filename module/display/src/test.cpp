#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <lvgl_input_device.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
#include <malloc.h>
#include <zephyr/random/random.h>
#include <iostream>
#include <vector>
#include <string>

LOG_MODULE_REGISTER(test_app);

class Test
{
public:
    int a;
    std::string name;

    Test(int num, std::string n)
    {
        a = num;
        name = n;
    }

    void print()
    {
        std::cout << name << " " << a++ << std::endl;
    }
};

void foo()
{
    std::vector<Test> vec{Test{1, "moo"}, Test{2, "blah"}, Test{3, "oink"}};

    while (true)
    {
        k_sleep(K_MSEC(100));
        for (auto& it : vec)
        {
            it.print();
        }
    }
}

K_THREAD_DEFINE(test_thread, 2048,
                foo, NULL, NULL, NULL,
                2, 0, 0);