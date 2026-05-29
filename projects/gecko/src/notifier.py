import os
import glob
import smtplib
import subprocess
from email.message import EmailMessage

class NotificationManager:
    def __init__(self, config_mgr):
        self.cfg = config_mgr

    def send_report(self, archive_filename):
        if not self.cfg.email_alerts:
            return

        msg = EmailMessage()
        msg['Subject'] = f'Gecko Report {self.cfg.utc_date}'
        msg['From'] = self.cfg.machine_name
        msg['To'] = self.cfg.target_email
        
        compressed_file_path = os.path.join(self.cfg.reports_path, archive_filename)
        body = f"{self.cfg.message}\n\nTo view logs and images:\n  1. Download attachment\n  2. Extract it\n\nLocation:\n{compressed_file_path}"
        msg.set_content(body)

        # Attach raw main report summary
        if os.path.exists(self.cfg.report_name):
            with open(self.cfg.report_name, 'rb') as f:
                msg.add_attachment(f.read(), maintype='text', subtype='plain', filename=os.path.basename(self.cfg.report_name))

        # Attach bundle package
        if self.cfg.attach_report and os.path.exists(archive_filename):
            subtype = 'gzip' if self.cfg.compression == "tar" else 'zip'
            with open(archive_filename, 'rb') as f:
                msg.add_attachment(f.read(), maintype='application', subtype=subtype, filename=os.path.basename(archive_filename))

        # Attach images separately if required
        if self.cfg.include_images:
            for img_path in glob.glob(os.path.join(self.cfg.reports_path, '**', f'*{self.cfg.utc_time}.png'), recursive=True):
                with open(img_path, 'rb') as fp:
                    msg.add_attachment(fp.read(), maintype='image', subtype='png', filename=os.path.basename(img_path))

        # Direct Mail Submission
        if self.cfg.machine_email:
            try:
                subprocess.run(["/usr/sbin/sendmail", "-t", "-oi"], input=msg.as_bytes(), check=True)
                print(f"Report sent to {self.cfg.target_email} via local sendmail.")
            except Exception as e:
                print(f"Failed to execute local sendmail binary: {e}")
        else:
            # Client Relay Submission
            smtp_settings = {
                "gmail.com": ("smtp.gmail.com", 587),
                "yahoo.com": ("smtp.mail.yahoo.com", 587),
                "outlook.com": ("smtp-mail.outlook.com", 587)
            }
            domain = self.cfg.sender_email.split('@')[-1].lower()
            server_host, server_port = smtp_settings.get(domain, ("smtp-mail.outlook.com", 587))

            try:
                with smtplib.SMTP(server_host, server_port) as smtp:
                    smtp.starttls()
                    smtp.login(self.cfg.sender_email, self.cfg.sender_password)
                    smtp.send_message(msg)
                print(f"Report successfully sent via SMTP to {self.cfg.target_email}")
            except Exception as e:
                print(f"Failed to send email via SMTP relay: {e}")