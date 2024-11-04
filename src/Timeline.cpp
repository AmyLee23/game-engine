#include "pch.h"
#include "Timeline.h"
#include <iostream>

Timeline::Timeline(Timeline* anchor, int64_t tic)
    : anchor(anchor), tic(tic), paused(false), elapsed_paused_time(0), last_time(0), time_scale(1.0), last_paused_time(0), last_input_time(0), last_movement_time(0) {

    // Get the current time in milliseconds
    auto now = Clock::now();
    auto time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    // Convert time to tics
    start_time = time_in_ms / tic;
    fractional_remainder = time_in_ms % tic;
}

int64_t Timeline::getRawTime() {
    // no mutex so only call internally
    // std::lock_guard<std::mutex> lock(mtx);
    int64_t current_time;

    if (anchor) {
        // Get current time from the anchor timeline
        int64_t anchor_time = anchor->getTime();
        current_time = start_time + (anchor_time * anchor->getTic()) / tic;
    }
    else {
        // Get current system time
        auto now = Clock::now();
        auto time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        current_time = time_in_ms / tic;
    }
    return current_time;
}

int64_t Timeline::getTime() {
    std::lock_guard<std::mutex> lock(mtx);

    int64_t current_time;

    if (anchor) {
        // Get current time from the anchor timeline
        int64_t anchor_time = anchor->getTime();
        current_time = start_time + (anchor_time * anchor->getTic()) / tic;
        // current_time = start_time + (anchor_time * anchor->getTic() + fractional_remainder)/tic;
        // fractional_remainder = (anchor_time * anchor->getTic() + fractional_remainder) % tic;
        // std::cout << "fractional remainder: " << fractional_remainder << " ms" << std::endl;
    }
    else {
        // Get current system time
        auto now = Clock::now();
        auto time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        current_time = time_in_ms / tic;
        fractional_remainder = time_in_ms % tic;
    }

    // Calculate elapsed time
    int64_t elapsed_time = current_time - start_time;
    //std::cout << "elapsed time: " << elapsed_time << " tics" << std::endl;

    // Subtract paused time
    if (paused) {
        //std::cout << "elapsed pause time: " << elapsed_paused_time << " tics" << std::endl;
        int64_t pause_duration = current_time - last_paused_time;
        elapsed_paused_time += pause_duration;
        last_paused_time = current_time;
    }

    int64_t active_time = elapsed_time - elapsed_paused_time;
    //std::cout << "active time: " << active_time << " tics" << std::endl;

    // Avoid negative time
    if (active_time < 0) {
        active_time = 0;
    }

    return active_time;
}

double Timeline::getMovementDt() {
    if (!paused) {
        std::lock_guard<std::mutex> lock(dt_mtx);
        int64_t time = getTime();
        int64_t dt = time - last_movement_time;
        double dt_scaled = static_cast<double>(dt) * time_scale;
        last_movement_time = time;
        //std::cout << "time: " << time << ", movement dt: " << dt_scaled << std::endl;
        return dt_scaled < 0 ? 0 : dt_scaled;
    }
    return 0;
}

double Timeline::getInputDt() {
    if (!paused) {
        std::lock_guard<std::mutex> lock(dt_mtx);
        int64_t time = getTime();
        int64_t dt = time - last_input_time;
        double dt_scaled = static_cast<double>(dt) * time_scale;
        last_input_time = time;
        //std::cout << "time: " << time << ", input dt: " << dt_scaled << std::endl;
        return dt_scaled < 0 ? 0 : dt_scaled;
    }
    return 0;
}

double Timeline::getDt() {
    if (!paused) {
        std::lock_guard<std::mutex> lock(dt_mtx);
        int64_t time = getTime();
        int64_t dt = time - last_time;
        double dt_scaled = static_cast<double>(dt) * time_scale;
        last_time = time;
        return dt_scaled < 0 ? 0 : dt_scaled;
    }
    return 0;
}

double Timeline::getDt(int64_t& custom_last_time) {
    if (!paused) {
        std::lock_guard<std::mutex> lock(dt_mtx);
        int64_t time = getTime();
        int64_t dt = time - custom_last_time;
        double dt_scaled = static_cast<double>(dt) * time_scale;
        custom_last_time = time;
        return dt_scaled < 0 ? 0 : dt_scaled;
    }
    return 0;
}

void Timeline::pause() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!paused) {
        last_paused_time = getRawTime(); // TODO: modify fractional remainder?
        paused = true;
    }
}

void Timeline::unpause() {
    std::lock_guard<std::mutex> lock(mtx);

    if (paused) {
        int64_t pause_duration = getRawTime() - last_paused_time; // TODO: modify fractional remainder?
        elapsed_paused_time += pause_duration;
        paused = false;
    }
}

void Timeline::changeTic(int64_t new_tic) {
    std::lock_guard<std::mutex> lock(mtx);
    last_time = last_time * tic / new_tic;
    tic = new_tic;
}

bool Timeline::isPaused() {
    std::lock_guard<std::mutex> lock(mtx);
    return paused;
}

int64_t Timeline::getTic() {
    std::lock_guard<std::mutex> lock(mtx);
    return tic;
}

double Timeline::getTimeScale() {
    std::lock_guard<std::mutex> lock(mtx);
    return time_scale;
}

void Timeline::setTimeScale(double ts) {
    std::lock_guard<std::mutex> lock(mtx);
    time_scale = ts;
}