import os
import glob
import subprocess
import psutil

class SystemDiagnostics:
    def __init__(self, config_mgr):
        self.cfg = config_mgr

    def gather_system_info(self):
        cpu_detailed_usage = psutil.cpu_times_percent(interval=3, percpu=True)
        cpu_count_logical = psutil.cpu_count(logical=True)
        cpu_count_physical = psutil.cpu_count(logical=False)
        cpu_freq = psutil.cpu_freq(percpu=True)
        cpu_stats = psutil.cpu_stats()
        cpu_times = psutil.cpu_times()
        temps = psutil.sensors_temperatures()
        virtual_memory = psutil.virtual_memory()
        
        os_str = f"{self.cfg.config['System']['os']} :: {self.cfg.config['System']['os_version']}"

        with open(self.cfg.report_name, 'a', encoding='utf-8') as f:
            f.write("\n\n=====System Information=====\n")
            f.write(f'{os_str}\n')
            f.write(f'Logical CPUs: {cpu_count_logical}\n')
            f.write(f'Physical CPUs: {cpu_count_physical}\n')
            f.write('Detailed CPU Usage:\n')
            for cpu in cpu_detailed_usage:
                f.write(f"{cpu}\n")
            f.write('Detailed CPU freq stat:\n')
            for freq in cpu_freq:
                f.write(f"{freq}\n")
            f.write(f'CPU Stats:\n{cpu_stats}\n')
            f.write(f'CPU Times:\n{cpu_times}\n')
            f.write('CPU Temps:\n')
            for key, value in temps.items():
                f.write(f'Key: {key}\n    Value: {value}\n')
            f.write(f"{virtual_memory}\n")

    def take_screenshots(self):
        displays = sorted([":" + os.path.basename(p)[1:] for p in glob.glob("/tmp/.X11-unix/X*")], key=lambda d: int(d[1:]))
        if not displays:
            print("No X11 displays found")
            return

        for display in displays:
            try:
                screenshot_name = f"gecko_screenshot_{display}_{self.cfg.utc_time}.png"
                screenshot_path = os.path.join(self.cfg.reports_path, screenshot_name)
                
                xwd = subprocess.Popen(["xwd", "-root", "-silent", "-display", display], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
                convert = subprocess.Popen(["convert", "xwd:-", screenshot_path], stdin=xwd.stdout, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                xwd.stdout.close()
                convert.wait()
                print(f"Captured Session: {display} -> {screenshot_path}")
            except Exception as e:
                print(f"Failed to capture display {display}: {e}")