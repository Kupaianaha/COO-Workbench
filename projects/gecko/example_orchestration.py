#!/usr/bin/env python3
import sys
import argparse
from src.config import ConfigManager
from src.system_info import SystemDiagnostics
from src.log_processor import LogProcessor
from src.archiver import ReportArchiver
from src.notifier import NotificationManager

def main():
    parser = argparse.ArgumentParser(description="Gecko Observatory Triage Tool")
    parser.add_argument("-config", default="config.ini", help="Path to config ini file")
    parser.add_argument("-init", action="store_true", help="Initialize and configure metrics")
    parser.add_argument("-msg", default="Manual system triage triggered.", help="Message text summary for dump report")
    args = parser.parse_args()

    # 1. Config Loading & Validation Context
    cfg = ConfigManager(config_path=args.config, message=args.msg, init=args.init)
    if args.init:
        print("Initialization completed successfully.")
        return

    # 2. Setup Orchestrator Modules
    diagnostics = SystemDiagnostics(cfg)
    processor = LogProcessor(cfg)
    archiver = ReportArchiver(cfg)
    notifier = NotificationManager(cfg)

    # 3. Running Sequence Pipeline
    print("Gathering basic diagnostic machine states...")
    diagnostics.gather_system_info()
    diagnostics.take_screenshots()

    print("Combing relevant application logs...")
    processor.gather_logs()
    processor.comb_logs()
    processor.grab_science_image()

    print("Archiving metrics...")
    archive_file = archiver.compress_report()

    print("Dispatching external notifications...")
    notifier.send_report(archive_file)

    print("Cleaning local telemetry cache workspace...")
    archiver.cleanup_reports_dir()
    print("Done!")

if __name__ == "__main__":
    main()
