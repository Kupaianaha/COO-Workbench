import os
import zipfile
import tarfile

class ReportArchiver:
    def __init__(self, config_mgr):
        self.cfg = config_mgr
        self.archive_filename = ""

    def compress_report(self):
        base_name = f"{self.cfg.reports_path}/gecko_{self.cfg.utc_date}".replace(" ", "_").replace("+", "")
        clean_name = base_name.replace(":", "").replace(".", "_")

        if self.cfg.compression == "tar":
            self.archive_filename = clean_name + ".tar.gz"
            context_manager = tarfile.open(self.archive_filename, "w:gz")
        else:
            self.archive_filename = clean_name + ".zip"
            context_manager = zipfile.ZipFile(self.archive_filename, "w", compression=zipfile.ZIP_DEFLATED)

        with context_manager as archive:
            for root, _, files in os.walk(self.cfg.reports_path):
                for file in files:
                    if self.cfg.utc_time in file or file.endswith(".log"):
                        full_path = os.path.join(root, file)
                        arcname = os.path.relpath(full_path, self.cfg.reports_path)
                        
                        if isinstance(archive, tarfile.TarFile):
                            archive.add(full_path, arcname=arcname)
                        else:
                            archive.write(full_path, arcname=arcname)
        
        print(f"Created archive: {self.archive_filename}")
        return self.archive_filename

    def cleanup_reports_dir(self):
        for root, _, files in os.walk(self.cfg.reports_path):
            for filename in files:
                full_path = os.path.join(self.cfg.reports_path, filename)
                if not full_path.endswith(".zip") and not full_path.endswith(".tar.gz"):
                    try:
                        os.remove(full_path)
                    except OSError:
                        pass