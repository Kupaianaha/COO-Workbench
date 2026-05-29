"""
Description:
    Module for managing configuration settings for the Gecko application. 
    This includes loading, initializing, and validating configuration 
    parameters from a config file. The ConfigManager class provides 
    methods to handle these operations, ensuring that the application 
    has the necessary settings to function correctly. It also handles 
    user input during initialization and organizes report generation 
    based on the current date and time.

Last Edited:
    May 29, 2026

Author:
    Elijah A-B/Kupaianaha
"""
import os
import sys
import configparser
import re
from datetime import datetime, timezone, timedelta

class ConfigManager:
    def __init__(self, config_path: str, message: str = "", init: bool = False):
        self.config_file = config_path
        self.config = configparser.ConfigParser()
        self.message = message

        self.utc_date = datetime.now(timezone.utc)
        self.current_utc_date = str(self.utc_date.date())
        self.utc_time = str(self.utc_date.time())
        self.cutoff = datetime.now().replace(tzinfo=None) - timedelta(hours=24)
        
        if init:
            self.initialize()
        else:
            self.load_config()

    def initialize(self):
        if not os.path.exists(self.config_file):
            raise FileNotFoundError(f"Configuration file {self.config_file} not found")
        self.config.read(self.config_file)

        for section in self.config.sections():
            print(f"\n======  Initializing Section: {section}  ======")
            for key, value in self.config[section].items():
                if key in ["initialized", "help_text"]:
                    if key == "help_text":
                        print(f"{value.rstrip()}\n")
                    continue
                clean_value = value.split('#')[0].strip()
                prompt = f"Enter value for '{key}'" + (f" (current: {clean_value}): " if clean_value else ": ")
                user_input = input(prompt).strip() or clean_value
                self.config[section][key] = user_input

        self.config["System"]["initialized"] = "true"
        with open(self.config_file, "w", encoding="utf-8") as f:
            self.config.write(f)

    def load_config(self):
        if not os.path.exists(self.config_file):
            raise FileNotFoundError(f"Configuration file {self.config_file} not found")
        self.config.read(self.config_file)

        if not self.config.getboolean("System", "initialized", fallback=False):
            print("\nYou have not Initialized this application! Please run: 'gecko -init'\n")
            sys.exit(0)

        # Unpack configurations into a clean namespace or attributes
        self.email_alerts = self.config.getboolean("Report", "email_alerts")
        if self.email_alerts:
            self.machine_email = self.config.getboolean("Report", "machine_email")
            self.machine_name = self.config.get("Report", "machine_name", fallback="Unknown Machine")
            self.sender_email = self.config.get("Report", "sender_email", fallback=None)
            self.sender_password = self.config.get("Report", "sender_password", fallback=None)
            self.target_email = self.config["Report"]["recipient_email"]
            self.include_images = self.config.getboolean("Report", "include_images")
            self.attach_report = self.config.getboolean("Report", "attach_report")

        self.report_incident = self.config.getboolean("Report", "report_incident")
        self.compression = self.config["Report"]["compression"].lower()
        
        # Paths setup
        self.r_path = self.config["Report"]["report_path"]
        self.log_dir = self.config["Logs"]["logs_dir"]
        self.science_dir = self.config["Logs"]["science_dir"]
        
        self.reports_path = os.path.join(self.r_path, self.current_utc_date)
        os.makedirs(self.reports_path, exist_ok=True)
        
        self.report_name = os.path.join(self.reports_path, f"gecko_report_{self.utc_time}.txt")
        
        try:
            self.time_pattern = re.compile(self.config["Report"]["time_pattern"])
        except re.error as e:
            raise ValueError(f"Invalid regex in config: {e}")

        # Initialize base text report
        with open(self.report_name, 'w', encoding='utf-8') as f:
            f.write("=========Reported Error From User==========\n")
            f.write(f"{self.message}\n\n")