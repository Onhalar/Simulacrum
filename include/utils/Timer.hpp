#ifndef GLFW_SCHEDULER_HEADER
#define GLFW_SCHEDULER_HEADER

#include <map>
#include <GLFW/glfw3.h>

typedef void (*callback_function)(void);

std::map<float, callback_function> schedule;

void Timer(float timeOutMiliSecnods, callback_function callback) {
    schedule[timeOutMiliSecnods + glfwGetTime() * 1000] = callback;
}

void handleSchedule() {
    float currentTime = (glfwGetTime() * 1000);

    for (auto it = schedule.begin(); it != schedule.end(); ) {
        if (it->first <= currentTime) {
            it->second(); 
            it = schedule.erase(it); 
        } else {
            ++it; 
        }
    }
}

#endif // GLFW_SCHEDULER_HEADER