#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

class FPSCounter {
public:
    FPSCounter();
    FPSCounter(float updateInterval);

    void update(double deltaTime);

    float getFPS() const;

private:
    float updateInterval_; // Update interval (seconds)
    int frameCount_;       // Frame count
    double elapsedTime_;   // Accumulated time
    float fps_;            // Current FPS
};

#endif