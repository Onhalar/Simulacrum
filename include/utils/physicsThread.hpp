#ifndef PHYSICS_THREAD_HEADER
#define PHYSICS_THREAD_HEADER

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>

std::mutex physicsMutex;
std::condition_variable physicsCV;
std::atomic<bool> physicsRunning(true);
std::atomic<bool> physicsStepReady(false);

double physicsDeltaTime;

std::thread physicsThread;

#endif // PHYSICS_THREAD_HEADER