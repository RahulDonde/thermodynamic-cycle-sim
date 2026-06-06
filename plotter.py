import pandas as pd
import matplotlib.pyplot as plt
import os
import sys

def plot_cycle():
    """
    Reads all CSV data from the current folder and generates
    P-V and T-S diagrams.
    """
    try:
        # Read P-V data
        pv12 = pd.read_csv('pv_process12.csv')
        pv23 = pd.read_csv('pv_process23.csv')
        pv34 = pd.read_csv('pv_process34.csv')
        pv41 = pd.read_csv('pv_process41.csv')
        
        # Read T-S data
        ts12 = pd.read_csv('ts_process12.csv')
        ts23 = pd.read_csv('ts_process23.csv')
        ts34 = pd.read_csv('ts_process34.csv')
        ts41 = pd.read_csv('ts_process41.csv')
        
        states = pd.read_csv('states.csv')
    
    except FileNotFoundError as e:
        print(f"Error: Could not find data file '{e.filename}'.")
        print("Please run the C++ simulator program first.")
        return # Exit the function, don't crash
    except Exception as e:
        print(f"An error occurred reading files: {e}")
        return

    try:
        # --- P-V Diagram Plotting ---
        plt.figure(figsize=(9, 6))
        plt.plot(pv12['v'], pv12['p'], label='Process 1-2')
        plt.plot(pv23['v'], pv23['p'], label='Process 2-3')
        plt.plot(pv34['v'], pv34['p'], label='Process 3-4')
        plt.plot(pv41['v'], pv41['p'], label='Process 4-1')
        plt.plot(states['v'], states['p'], 'o', color='black', markersize=8)
        for i, row in states.iterrows():
            plt.text(row['v'], row['p'] * 1.05, str(row['label']), fontsize=14, ha='center')
        plt.xlabel('Volume ($m^3/kg$)', fontsize=12)
        plt.ylabel('Pressure ($Pa$)', fontsize=12)
        plt.title('Ideal P-V Diagram', fontsize=16)
        plt.grid(True)
        plt.legend()
        plt.tight_layout()

        # --- T-S Diagram Plotting ---
        plt.figure(figsize=(9, 6))
        plt.plot(ts12['s'], ts12['t'], label='Process 1-2')
        plt.plot(ts23['s'], ts23['t'], label='Process 2-3')
        plt.plot(ts34['s'], ts34['t'], label='Process 3-4')
        plt.plot(ts41['s'], ts41['t'], label='Process 4-1')
        plt.plot(states['s'], states['t'], 'o', color='black', markersize=8)
        for i, row in states.iterrows():
            plt.text(row['s'], row['t'] * 1.03, str(row['label']), fontsize=14, ha='center')
        plt.xlabel('Entropy ($J/(kg \cdot K)$)', fontsize=12)
        plt.ylabel('Temperature ($K$)', fontsize=12)
        plt.title('Ideal T-S Diagram', fontsize=16)
        plt.grid(True)
        plt.legend()
        plt.tight_layout()

        # Show both plots
        plt.show()
    
    except Exception as e:
        print(f"An error occurred during plotting: {e}")

# This part allows the script to be run by itself
# OR imported as a module by our GUI
if __name__ == '__main__':
    print("Running plotter as standalone script...")
    plot_cycle()