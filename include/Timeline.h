#pragma once
#include <mutex>
#include <chrono>

class Timeline {
private:
    // mutexes for thread safety
    std::mutex mtx;
    std::mutex dt_mtx;
    // init
    using Clock = std::chrono::steady_clock;
    int64_t start_time;
    int64_t tic;       // Time units of anchor timeline per unit of this timeline
    int64_t fractional_remainder;  // To accumulate fractional tics
    // pausing
    int64_t elapsed_paused_time;
    int64_t last_paused_time;
    bool paused;
    // dts
    double time_scale;
    int64_t last_time;
    int64_t last_movement_time;
    int64_t last_input_time;
    
    Timeline* anchor;  // Pointer to the anchor timeline (nullptr for system time)
    int64_t getRawTime();

public:
    Timeline(Timeline* anchor, int64_t tic);
    //Timeline();

    int64_t getTime();
    double getDt(); // old get dt
    double getDt(int64_t& last_time); // for tracking different last times (multi threading, etc)
    double getMovementDt(); // dt for only movement calls
    double getInputDt(); // dt for only input calls
    int64_t getTic();
    void pause();
    void unpause();
    void changeTic(int64_t new_tic);
    bool isPaused();
    double getTimeScale();
    void setTimeScale(double ts);
};

/*
class Timeline {
private:
    std::mutex mtx;  // Mutex for thread safety

    using Clock = std::chrono::steady_clock;
    Clock::time_point start_time;
    std::chrono::milliseconds elapsed_paused_time;
    Clock::time_point last_paused_time;

    int64_t last_time;

    int64_t tic;       // Time units of anchor timeline per unit of this timeline
    int64_t fractional_remainder;  // To accumulate fractional tics
    bool paused;
    Timeline* anchor;  // Pointer to the anchor timeline (nullptr for system time)

public:
    Timeline(Timeline* anchor, int64_t tic);
    //Timeline();

    int64_t getTime();
    int64_t getDt();
    int64_t getTic();
    void pause();
    void unpause();
    void changeTic(int64_t new_tic);
    bool isPaused();
};
*/
