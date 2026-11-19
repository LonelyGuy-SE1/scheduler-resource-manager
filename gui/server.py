#!/usr/bin/env python3
"""
CampusCompute GUI Server & Backend API
Bridges the compiled C binary execution with the frontend.
Runs the exact sub-case requested and parses output dynamically.
"""
import http.server
import json
import os
import re
import socketserver
import subprocess

PORT = 8080
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_DIR = os.path.dirname(BASE_DIR)
BINARY_PATH = os.path.join(PROJECT_DIR, "build", "campuscompute")

def strip_cli_menu(text):
    """
    Strips out the repetitive interactive CLI ASCII banners, menus,
    and trailing exit messages so the terminal displays only clean execution logs.
    """
    text = re.sub(r'CampusCompute [—\-] shared lab OS concepts prototype\n*', '', text)
    # Strip full ASCII menu blocks
    menu_pattern = r'={5,}\s*CampusCompute\s*={5,}.*?Enter choice:\s*'
    text = re.sub(menu_pattern, '', text, flags=re.DOTALL)
    # Remove any trailing "Enter choice:" and "Goodbye."
    text = re.sub(r'Enter choice:.*', '', text, flags=re.DOTALL)
    text = re.sub(r'Goodbye\.\s*', '', text)
    return text.strip()

def parse_scheduler_output(raw_text):
    gantt_data = {"fcfs": [], "rr": [], "priority": []}
    metrics = {"fcfs_wt": "0.00", "rr_wt": "0.00", "priority_wt": "0.00"}

    fcfs_gantt_match = re.search(r'\[1/3\] FCFS.*?Gantt:\s*\|(.*)', raw_text, re.DOTALL)
    if fcfs_gantt_match:
        line = fcfs_gantt_match.group(1).split("\n")[0]
        tokens = [t.strip() for t in line.split("|") if t.strip()]
        if tokens:
            gantt_data["fcfs"] = tokens[:-1]

    rr_gantt_match = re.search(r'\[2/3\] Round Robin.*?Gantt.*?:.*?\|(.*)', raw_text, re.DOTALL)
    if rr_gantt_match:
        line = rr_gantt_match.group(1).split("\n")[0]
        tokens = [t.strip() for t in line.split("|") if t.strip()]
        if tokens:
            gantt_data["rr"] = tokens[:-1]

    prio_gantt_match = re.search(r'\[3/3\] Priority.*?Gantt:\s*\|(.*)', raw_text, re.DOTALL)
    if prio_gantt_match:
        line = prio_gantt_match.group(1).split("\n")[0]
        tokens = [t.strip() for t in line.split("|") if t.strip()]
        if tokens:
            gantt_data["priority"] = tokens[:-1]

    fcfs_m = re.search(r'FCFS\s+([\d\.]+)\s+([\d\.]+)', raw_text)
    if fcfs_m:
        metrics["fcfs_wt"] = fcfs_m.group(2)

    rr_m = re.search(r'Round Robin\s+([\d\.]+)\s+([\d\.]+)', raw_text)
    if rr_m:
        metrics["rr_wt"] = rr_m.group(2)

    prio_m = re.search(r'Priority\s+([\d\.]+)\s+([\d\.]+)', raw_text)
    if prio_m:
        metrics["priority_wt"] = prio_m.group(2)

    return {"gantt": gantt_data, "metrics": metrics}

class CampusComputeHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=BASE_DIR, **kwargs)

    def do_POST(self):
        if self.path == "/api/run":
            length = int(self.headers.get("Content-Length", 0))
            body = self.rfile.read(length).decode("utf-8")
            data = json.loads(body) if body else {}
            choice = data.get("choice", 7)
            subchoice = data.get("subchoice", None)

            if not os.path.exists(BINARY_PATH):
                subprocess.run(["make"], cwd=PROJECT_DIR, check=True)

            # Route input accurately based on what the user wants to test
            if choice == 1:
                # Option 1: Scheduling (pick built-in sample 1, then compare all 4, then exit 0)
                input_str = "1\n1\n4\n0\n"
            elif choice == 4:
                # Banker's algorithm: can be subchoice 1 (safe), 2 (unsafe), or both (1 then 2)
                if subchoice == 1:
                    input_str = "4\n1\n0\n"
                elif subchoice == 2:
                    input_str = "4\n2\n0\n"
                else:
                    # Run safe, then run unsafe, then exit
                    input_str = "4\n1\n4\n2\n0\n"
            else:
                input_str = f"{choice}\n0\n"

            try:
                proc = subprocess.run(
                    [BINARY_PATH],
                    input=input_str,
                    text=True,
                    capture_output=True,
                    cwd=PROJECT_DIR,
                    timeout=10
                )
                output = proc.stdout
                error = proc.stderr
            except Exception as e:
                output = ""
                error = str(e)

            clean_output = strip_cli_menu(output)
            parsed = None
            if choice == 1 or choice == 7:
                parsed = parse_scheduler_output(output)

            resp = {
                "success": True,
                "output": clean_output,
                "error": error,
                "choice": choice,
                "parsed": parsed
            }

            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            self.wfile.write(json.dumps(resp).encode("utf-8"))
        else:
            self.send_error(404, "Not Found")

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.end_headers()

def run_server():
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), CampusComputeHandler) as httpd:
        print(f"CampusCompute backend server running on http://localhost:{PORT}")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            pass

if __name__ == "__main__":
    run_server()
