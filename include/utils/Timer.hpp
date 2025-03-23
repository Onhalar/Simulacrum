#pragma once
#include <config.hpp>

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
