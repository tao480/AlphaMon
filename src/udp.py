import socket
import sys
import os
import logging
import msvcrt  # Windows-specific console input handler

# --- Network Settings - Telemetry Data Only ---
UDP_IP_LISTEN = "0.0.0.0"      # Listen on all local LAN interfaces
UDP_PORT_DATA = 4220           # AlphaMon's default Data Xchange port
BUFFER_SIZE = 2048             # Safe allocation window for long inverter logs

# --- Logging Configuration ---
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
LOG_FILE_PATH = os.path.join(SCRIPT_DIR, "udp.log")

class DeciSecondFormatter(logging.Formatter):
    def formatTime(self, record, datefmt=None):
        base_time = self.formatTimeSimple(record, "%H:%M:%S")
        deci_second = int(record.msecs // 100)
        return f"{base_time}.{deci_second}"
        
    def formatTimeSimple(self, record, fmt):
        import time
        ct = self.converter(record.created)
        return time.strftime(fmt, ct)

logger = logging.getLogger("AlphaMonLogger")
logger.setLevel(logging.INFO)
shared_formatter = DeciSecondFormatter("%(asctime)s - %(message)s")

# File Output configuration
file_handler = logging.FileHandler(LOG_FILE_PATH, encoding="utf-8")
file_handler.setFormatter(shared_formatter)
logger.addHandler(file_handler)

# Screen Output configuration
console_handler = logging.StreamHandler(sys.stdout)
console_handler.setFormatter(shared_formatter)
logger.addHandler(console_handler)


def run_telemetry_monitor():
    """Single-threaded loop to intercept telemetry with spacebar pause control."""
    data_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    data_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    # Crucial: Allow socket to step away if no packets arrive, checking the spacebar
    data_sock.settimeout(0.1)
    
    try:
        data_sock.bind((UDP_IP_LISTEN, UDP_PORT_DATA))
    except Exception as e:
        print(f"[!] Critical Error: Unable to bind to Port {UDP_PORT_DATA}: {e}")
        sys.exit(1)

    print("** ALPHAMON UDP DATA XCHANGE MONITOR **")
    print("=======================================")
    print(f" Listen for packets on data port: {UDP_PORT_DATA}")
    print(f" Writing logs to: {LOG_FILE_PATH}")
    print(" Press [SPACEBAR] to pause/resume logging.")
    print(" Press Ctrl+Break to terminate.")
    print("---------------------------------------\n")

    paused = False

    try:
        while True:
            # 1. Non-blocking key check for Windows Consoles
            if msvcrt.kbhit():
                key = msvcrt.getch()
                if key == b' ':  # Spacebar detected
                    paused = not paused
                    if paused:
                        print(f"\n*** LOGGING PAUSED *** (Incoming packets are being dropped)")
                    else:
                        print(f"\n*** LOGGING RESUMED ***")
                    sys.stdout.flush()

            # 2. Packet reception block
            try:
                data, addr = data_sock.recvfrom(BUFFER_SIZE)
                
                # If paused is True, explicitly drop the packet by ignoring it
                if paused:
                    continue 
                
                decoded_line = data.decode("utf-8", errors="ignore").rstrip()
                if decoded_line:
                    logger.info(decoded_line)
                    
            except socket.timeout:
                # No data received within 0.1s; cycle loop safely to catch keypresses
                continue
            except Exception as format_err:
                print(f"\n[WARN] String format parsing exception: {format_err}")
                
    except KeyboardInterrupt:
        print("\n[INFO] Intercept loop interrupted by user. Closing data streams...")
    finally:
        data_sock.close()
        print("[SUCCESS] Network socket safely terminated.")

if __name__ == '__main__':
    run_telemetry_monitor()
