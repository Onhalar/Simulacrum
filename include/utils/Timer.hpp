#ifndef GLFW_SCHEDULER_HEADER
#define GLFW_SCHEDULER_HEADER

#include <map>
#include <chrono>

// Use high_resolution_clock and time_point directly
using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
using Nanoseconds = std::chrono::nanoseconds;

typedef void (*callback_function)(void);

// Schedule map: maps execution times to callbacks
std::map<TimePoint, callback_function> schedule;

// Schedules a callback to run after a given timeout
void Timer(Nanoseconds delay, callback_function callback) {
    schedule[Clock::now() + delay] = callback;
}

// Call this once per frame to check and run due callbacks
void handleSchedule() {
    TimePoint now = Clock::now();

    for (auto it = schedule.begin(); it != schedule.end(); ) {
        if (it->first <= now) {
            if (it->second) it->second(); // Call the callback
            it = schedule.erase(it);      // Remove after running
        } else {
            break; // Remaining events are scheduled for later
        }
    }
}

#endif // GLFW_SCHEDULER_HEADER
