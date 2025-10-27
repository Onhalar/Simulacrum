#ifndef PHYSICS_THREAD_HEADER
#define PHYSICS_THREAD_HEADER

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <unistd.h>

inline std::mutex physicsMutex;
inline std::condition_variable physicsCV;
inline std::atomic<bool> physicsRunning(true);
inline std::atomic<bool> pausePhysicsThread(false);

inline double physicsDeltaTime;

inline std::thread physicsThread;

#endif // PHYSICS_THREAD_HEADER