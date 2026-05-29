import os
import re
import shutil
from datetime import datetime

class LogProcessor:
    def __init__(self, config_mgr):
        self.cfg = config_mgr
        self.regex_pattern = r"^.*error.*$|^.*warning.*$"

    def gather_logs(self):
        for subdir, _, files in os.walk(self.cfg.log_dir):
            for file in files:
                log_file = os.path.join(subdir, file)
                try:
                    shutil.copy2(log_file, self.cfg.reports_path)
                except Exception as e:
                    print(f"Skipping log {file} due to: {e}")

    def comb_logs(self):
        for subdir, _, files in os.walk(self.cfg.log_dir):
            for file in files:
                if not file.lower().endswith(".log"):
                    continue
                log_path = os.path.join(subdir, file)
                
                with open(self.cfg.report_name, 'a', encoding='utf-8') as rep:
                    rep.write(f"\n\n====={log_path}=====\n")
                
                try:
                    with open(log_path, 'r', encoding='utf-8', errors='ignore') as lf:
                        full_log = lf.read()
                    
                    matches = re.findall(self.regex_pattern, full_log, re.IGNORECASE | re.MULTILINE)
                    timeframe_matches = [
                        match for match in matches
                        if (m := re.match(self.cfg.time_pattern, match))
                        and datetime.fromisoformat(m.group(1)) >= self.cfg.cutoff
                    ]

                    with open(self.cfg.report_name, 'a', encoding='utf-8') as rep:
                        for match in timeframe_matches:
                            rep.write(f"{match}\n")
                except Exception as e:
                    print(f"Error reading {log_path}: {e}")

    def grab_science_image(self):
        image_dirs = [os.path.join(self.cfg.science_dir, "acam"), os.path.join(self.cfg.science_dir, "slicecam"), self.cfg.science_dir]
        for i_dir in image_dirs:
            if not os.path.exists(i_dir):
                continue
            try:
                files = [os.path.join(i_dir, f) for f in os.listdir(i_dir) if os.path.isfile(os.path.join(i_dir, f))]
                if files:
                    newest_file = max(files, key=os.path.getmtime)
                    shutil.copy(newest_file, self.cfg.reports_path)
            except Exception as e:
                print(f"Could not grab science image from {i_dir}: {e}")