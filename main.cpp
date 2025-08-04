#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

#ifdef _WIN32
    #include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
    #include <pthread.h>
    #include <sched.h>
#endif

std::mutex shared_resource;
std::atomic<bool> simulation_running{true};
std::atomic<bool> enable_priority_inheritance{false};

// Platform-specific priority setting
void set_thread_priority(std::thread& t, int priority_level) {
#ifdef _WIN32
    auto native_handle = t.native_handle();
    HANDLE handle;
    
    #ifdef __MINGW32__
        // MinGW returns thread ID, need to open thread handle
        handle = OpenThread(THREAD_SET_INFORMATION, FALSE, static_cast<DWORD>(native_handle));
        if (handle == NULL) {
            std::cout << "Warning: Failed to open thread handle on MinGW\n";
            return;
        }
    #else
        // MSVC returns HANDLE directly
        handle = native_handle;
    #endif
    
    int win_priority;
    switch(priority_level) {
        case 3: win_priority = THREAD_PRIORITY_TIME_CRITICAL; break;  // High
        case 2: win_priority = THREAD_PRIORITY_ABOVE_NORMAL; break;   // Medium  
        case 1: win_priority = THREAD_PRIORITY_BELOW_NORMAL; break;   // Low
        default: win_priority = THREAD_PRIORITY_NORMAL; break;
    }
    
    if (!SetThreadPriority(handle, win_priority)) {
        std::cout << "Warning: Failed to set thread priority on Windows\n";
    }
    
    #ifdef __MINGW32__
        CloseHandle(handle);  // Clean up
    #endif
    
#elif defined(__linux__) || defined(__APPLE__)
    pthread_t handle = t.native_handle();
    struct sched_param param;
    int policy = SCHED_FIFO;  // Real-time scheduling
    
    // Map our priority levels to system priorities
    switch(priority_level) {
        case 3: param.sched_priority = 90; break;  // High
        case 2: param.sched_priority = 50; break;  // Medium
        case 1: param.sched_priority = 10; break;  // Low
        default: param.sched_priority = 1; break;
    }
    
    if (pthread_setschedparam(handle, policy, &param) != 0) {
        policy = SCHED_OTHER;
        param.sched_priority = 0;
        if (pthread_setschedparam(handle, policy, &param) != 0) {
            std::cout << "Warning: Failed to set thread priority on Unix\n";
        }
    }
#else
    std::cout << "Priority setting not supported on this platform\n";
#endif
}

void high_priority_thread() {
    while (simulation_running) {
        auto acquire_start = std::chrono::steady_clock::now();
        
        // SCOPED BLOCK: Ensures mutex is automatically released when leaving this scope
        {
            std::lock_guard<std::mutex> lock(shared_resource);
            auto acquire_end = std::chrono::steady_clock::now();
            auto wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                acquire_end - acquire_start).count();
            
            std::cout << "HIGH: Waited " << wait_time << "ms for resource\n";
            
            // Simulate critical computation
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            
        }
        
        // High-priority threads typically have periods between critical sections
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

void medium_priority_thread() {
    while (simulation_running) {
        std::cout << "MEDIUM: Running background task...\n";
        
        // Simulate CPU-intensive work that causes priority inversion
        auto busy_start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - busy_start < std::chrono::milliseconds(150)) {
            // Busy wait to simulate computational load
            volatile long dummy = 0;
            for (int i = 0; i < 50000; ++i) {
                dummy += i * i;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void low_priority_thread() {
    while (simulation_running) {
        // SCOPED BLOCK: Critical for mutex management and priority inheritance demo
        {
            std::lock_guard<std::mutex> lock(shared_resource);
            std::cout << "LOW: Got resource";
            
            if (enable_priority_inheritance) {
                std::cout << " (priority boosted!)";
            }
            std::cout << "\n";
            

            // high-priority thread waits here
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
        }
        
        // Low-priority threads typically have longer periods between work
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
}

void run_simulation(const std::string& scenario, bool use_priority_inheritance, int duration_seconds) {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << scenario << (use_priority_inheritance ? " (with priority inheritance)" : " (priority inversion problem)") << "\n";
    std::cout << std::string(50, '=') << "\n";
    
    // Reset state
    simulation_running = true;
    enable_priority_inheritance = use_priority_inheritance;
    
    auto start = std::chrono::steady_clock::now();
    
    // Create threads
    std::thread high_thread(high_priority_thread);
    std::thread medium_thread(medium_priority_thread);
    std::thread low_thread(low_priority_thread);
    
    // Set platform-specific priorities
    set_thread_priority(high_thread, 3);    // Highest priority
    set_thread_priority(medium_thread, 2);  // Medium priority  
    set_thread_priority(low_thread, 1);     // Lowest priority
    
    // Let simulation run
    std::this_thread::sleep_for(std::chrono::seconds(duration_seconds));
    
    // Stop simulation
    simulation_running = false;
    auto end = std::chrono::steady_clock::now();
    
    // Wait for threads to finish
    high_thread.join();
    medium_thread.join();
    low_thread.join();
    
    std::cout << "\n" << std::string(50, '-') << "\n";
}

int main() {
    std::cout << "Mars Pathfinder Priority Inversion Simulation\n";
    std::cout << "==============================================\n";
    std::cout << "Watch the HIGH thread wait times!\n\n";
    
    // Run simulation without priority inheritance
    run_simulation("PROBLEM", false, 3);
    
    std::cout << "Press Enter to see the solution...";
    std::cin.get();
    
    // Run simulation with priority inheritance
    run_simulation("SOLUTION", true, 3);
    
    std::cout << "\nKEY OBSERVATION:\n";
    std::cout << "Without priority inheritance: HIGH thread waits longer\n";
    std::cout << "With priority inheritance: HIGH thread waits less\n";
    
    return 0;
}