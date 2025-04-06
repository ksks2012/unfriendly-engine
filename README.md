# Rocket Simulation
A simple rocket simulator built with C++, OpenGL, GLFW, GLM, and Dear ImGui.

## Features

### Control
- Adjustable time scale (Q/E keys).
- Camera zoom (W/S keys).

### Rendering
- Rocket rendering.
- Earth rendering
- Rocket Trajectory Visualization: Displays the rocket's flight path in real-time.

### Simulation Info
The simulation provides real-time data displayed in an ImGui window:

- **Time Scale**: Displays the current time scale of the simulation.
- **Mass**: Shows the rocket's total mass in kilograms.
- **Fuel Mass**: Indicates the remaining fuel mass sin kilograms.
- **Thrust**: Displays the thrust generated by the rocket in newtons.
- **Exhaust Velocity**: Shows the exhaust velocity in meters per second.
- **Position (Geocentric)**: Displays the rocket's position in geocentric coordinates.
- **Velocity**: Shows the rocket's velocity vector.
- **Altitude**: Indicates the rocket's altitude above Earth's surface in meters.
- **Time**: Displays the elapsed simulation time in seconds.
- **Launched**: Indicates whether the rocket has been launched.

# Install

## Dependency

- dev-lib on ubuntu/debian
    ```
    sudo apt-get install libglfw3-dev libglew-dev libglm-dev
    ```

- imgui lib
    ```
    https://github.com/ocornut/imgui
    ```


## Build Instructions
1. Install dependencies: `libglfw3-dev`, `libglew-dev`, `libglm-dev`.   
2. Run `./scripts/build.sh`.
3. Execute `./bin/RocketSimulation`.
