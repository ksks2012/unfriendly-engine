#include "ui/fps_counter.h"

FPSCounter::FPSCounter() : updateInterval_(0.5f), frameCount_(0), elapsedTime_(0.0f), fps_(0.0f) {
}

FPSCounter::FPSCounter(float updateInterval = 0.5f) : updateInterval_(updateInterval), frameCount_(0), elapsedTime_(0.0f), fps_(0.0f) {
}

void FPSCounter::update(double deltaTime) {
    frameCount_++;
    elapsedTime_ += deltaTime;
    if (elapsedTime_ >= updateInterval_) {
        fps_ = frameCount_ / elapsedTime_;
        frameCount_ = 0;
        elapsedTime_ = 0.0f;
    }
}

float FPSCounter::getFPS() const {
    return fps_;
}
