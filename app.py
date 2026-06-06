import tkinter as tk
from tkinter import ttk
from tkinter import messagebox
import subprocess
import os
from plotter import plot_cycle

class ThermoApp:
    def __init__(self, root):
        self.root = root
        self.root.title("Thermodynamic Cycle Simulator")

        self.cpp_executable = "./thermodynamic_simulator"
        if os.name == 'nt':
            self.cpp_executable = "thermodynamic_simulator.exe"

        main_frame = ttk.Frame(root, padding="20")
        main_frame.grid(row=0, column=0, sticky=(tk.W, tk.E, tk.N, tk.S))

        ttk.Label(main_frame, text="Select Cycle:", font=("Arial", 12)).grid(row=0, column=0, sticky=tk.W, pady=5)
        self.cycle_var = tk.StringVar()
        cycle_menu = ttk.Combobox(main_frame, textvariable=self.cycle_var, state="readonly", width=30)
        cycle_menu['values'] = ('otto', 'diesel', 'brayton', 'carnot')
        cycle_menu.grid(row=0, column=1, columnspan=2, sticky=(tk.W, tk.E), pady=5)
        cycle_menu.bind('<<ComboboxSelected>>', self.update_fields)
        
        self.inputs = {}
        self.input_frame = ttk.Frame(main_frame, padding="10 0")
        self.input_frame.grid(row=1, column=0, columnspan=3, sticky=(tk.W, tk.E))
        
        ttk.Label(main_frame, text="Results:", font=("Arial", 12)).grid(row=2, column=0, sticky=tk.W, pady=10)
        self.result_text = tk.StringVar()
        result_label = ttk.Label(main_frame, textvariable=self.result_text, font=("Courier", 11), foreground="blue")
        result_label.grid(row=3, column=0, columnspan=3, sticky=tk.W)

        run_button = ttk.Button(main_frame, text="Run Simulation & Plot", command=self.run_simulation)
        run_button.grid(row=4, column=0, columnspan=3, pady=20)
        
        cycle_menu.current(0)
        self.update_fields(None)

    def update_fields(self, event):
        for widget in self.input_frame.winfo_children():
            widget.destroy()
        self.inputs.clear()

        cycle = self.cycle_var.get()
        
        params = {
            'otto': ["P1 (Pa):", "T1 (K):", "Comp. Ratio:"],
            'diesel': ["P1 (Pa):", "T1 (K):", "Comp. Ratio:", "Cutoff Ratio:"],
            'brayton': ["P1 (Pa):", "T1 (K):", "Pressure Ratio:", "T3 Max (K):"],
            'carnot': ["T_High (K):", "T_Low (K):", "P1 (Pa):", "V1 (m^3/kg):", "V2 (m^3/kg):"]
        }
        
        for i, label_text in enumerate(params.get(cycle, [])):
            label = ttk.Label(self.input_frame, text=label_text)
            label.grid(row=i, column=0, sticky=tk.W, padx=5, pady=5)
            
            entry = ttk.Entry(self.input_frame, width=25)
            entry.grid(row=i, column=1, sticky=(tk.W, tk.E), padx=5, pady=5)
            self.inputs[label_text] = entry

    def run_simulation(self):
        cycle = self.cycle_var.get()
        command = [self.cpp_executable, cycle]
        
        try:
            for key, entry in self.inputs.items():
                command.append(entry.get())
            
            result = subprocess.run(command, capture_output=True, text=True, check=True, shell=False)
            
            self.result_text.set(result.stdout)
            
            plot_cycle()

        except subprocess.CalledProcessError as e:
            messagebox.showerror("Simulation Error", f"The C++ program failed:\n{e.stderr}")
        except FileNotFoundError:
            messagebox.showerror("Error", f"Executable not found at:\n{self.cpp_executable}\nPlease re-compile main.cpp.")
        except Exception as e:
            messagebox.showerror("Error", f"An error occurred:\n{e}")

if __name__ == "__main__":
    root = tk.Tk()
    app = ThermoApp(root)
    root.mainloop()