# Thermodynamic Cycle Simulator

A hybrid engineering application designed to model and visualize ideal gas power cycles (Otto, Diesel, Brayton, and Carnot). This project demonstrates a cross-language data pipeline, coupling high-performance C++ backend calculations with a Python visualization layer.

## Features
- **Multi-Cycle Support**: Simulates Otto, Diesel, Brayton, and Carnot cycles.
- **Hybrid Architecture**:
  - **C++ Engine**: High-speed numerical solver for state-property determination ($P, V, T, s, u, h$).
  - **Python Frontend**: GUI orchestration layer using `Tkinter` to capture user inputs.
- **Visualization**: Generates real-time P-V and T-S diagrams using `matplotlib` and `pandas`.

## System Architecture
The application uses a "headless" C++ binary that exports calculated results to CSV files, which are then ingested and rendered by the Python plotter.

## Quick Start
1. **Compile the Backend**:
   `g++ -O3 src/main.cpp -o thermodynamic_simulator`
2. **Install Dependencies**:
   `pip install pandas matplotlib`
3. **Run the Application**:
   `python app.py`

---
*Developed for EN204: Engineering Thermodynamics, IIT Guwahati*
