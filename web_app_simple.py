#!/usr/bin/env python3
"""
Simple web application for running the high-dimensional regret minimization algorithm.
This version uses polling instead of WebSockets to avoid compatibility issues.
"""

import os
import sys
import subprocess
import threading
import time
import json
from datetime import datetime
from flask import Flask, render_template, request, jsonify
import queue

app = Flask(__name__)
# Feedback storage path resolution (supports Railway volume at /data)
def resolve_feedback_file() -> str:
    feedback_dir = os.environ.get('FEEDBACK_DIR')
    if feedback_dir and os.path.isdir(feedback_dir):
        return os.path.join(feedback_dir, 'user_feedback.json')
    if os.path.isdir('/data'):
        return os.path.join('/data', 'user_feedback.json')
    return 'user_feedback.json'

FEEDBACK_FILE = resolve_feedback_file()

def migrate_feedback_file_if_needed():
    legacy_path = 'user_feedback.json'
    try:
        # Only migrate if target is different from legacy and legacy exists but target does not
        if FEEDBACK_FILE != legacy_path and os.path.exists(legacy_path) and not os.path.exists(FEEDBACK_FILE):
            print(f"Migrating legacy feedback file from {legacy_path} to {FEEDBACK_FILE}...")
            try:
                os.makedirs(os.path.dirname(FEEDBACK_FILE), exist_ok=True)
            except Exception:
                pass
            with open(legacy_path, 'r') as src:
                content = src.read()
            with open(FEEDBACK_FILE, 'w') as dst:
                dst.write(content)
            print("Migration completed.")
    except Exception as e:
        print(f"Feedback migration skipped due to error: {e}")

migrate_feedback_file_if_needed()

# Utilities to handle both legacy JSON array and NDJSON formats
def _read_feedback_records(path: str):
    records = []
    try:
        with open(path, 'r') as f:
            content = f.read()
        # Try whole-file JSON first
        try:
            parsed = json.loads(content)
            if isinstance(parsed, list):
                records.extend(parsed)
            elif isinstance(parsed, dict):
                records.append(parsed)
            return records
        except json.JSONDecodeError:
            pass

        # Try to extract a JSON array substring if present
        try:
            start = content.find('[')
            end = content.rfind(']')
            if start != -1 and end != -1 and end > start:
                arr_text = content[start:end+1]
                parsed = json.loads(arr_text)
                if isinstance(parsed, list):
                    records.extend(parsed)
        except json.JSONDecodeError:
            pass

        # Parse remaining as NDJSON (one JSON object per line)
        for line in content.splitlines():
            line_stripped = line.strip()
            if not line_stripped or not line_stripped.startswith('{'):
                continue
            try:
                records.append(json.loads(line_stripped))
            except json.JSONDecodeError:
                continue
    except FileNotFoundError:
        return []
    return records

def _migrate_to_ndjson_if_array_file(path: str):
    try:
        if not os.path.exists(path):
            return
        with open(path, 'r') as f:
            first_non_ws = f.read(1)
            if first_non_ws != '[':
                return
        # Convert legacy/mixed file to pure NDJSON
        records = _read_feedback_records(path)
        os.makedirs(os.path.dirname(path) or '.', exist_ok=True)
        with open(path, 'w') as f:
            for rec in records:
                f.write(json.dumps(rec) + '\n')
        print(f"Migrated feedback file at {path} to NDJSON format")
    except Exception as e:
        print(f"NDJSON migration skipped due to error: {e}")


# Legacy globals (no longer used with session-based runners)
current_process = None
output_queue = queue.Queue()
input_queue = queue.Queue()
algorithm_output = []
algorithm_running = False

# Session-based runner registry
sessions = {}
sessions_lock = threading.Lock()

def get_runner(session_id, create_if_missing=False):
    if not session_id:
        return None
    with sessions_lock:
        runner = sessions.get(session_id)
        if runner is None and create_if_missing:
            runner = AlgorithmRunner()
            sessions[session_id] = runner
        return runner

class AlgorithmRunner:
    def __init__(self):
        self.process = None
        self.is_running = False
        self.output_lines = []
        
    def start_algorithm(self, dataset_path="car.txt", use_real=False):
        """Start the C++ algorithm process"""
        try:
            # Always build the algorithm to ensure correct version
            print("Building algorithm (real version)...")
            result = subprocess.run(["make", "web-real"], capture_output=True, text=True)
            if result.returncode != 0:
                return False, f"Build failed: {result.stderr}"
            
            # Start the algorithm process with the specified dataset
            self.process = subprocess.Popen(
                ["./run_web", dataset_path],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
                universal_newlines=True
            )
            
            self.is_running = True
            self.output_lines = []
            
            # Start output reading thread
            threading.Thread(target=self._read_output, daemon=True).start()
            
            return True, "Algorithm started successfully"
            
        except Exception as e:
            return False, f"Failed to start algorithm: {str(e)}"
    
    def _read_output(self):
        """Read output from the algorithm process"""
        while self.is_running and self.process:
            try:
                line = self.process.stdout.readline()
                if line:
                    self.output_lines.append(line.rstrip())
                elif self.process.poll() is not None:
                    # Process has ended
                    break
            except Exception as e:
                print(f"Error reading output: {e}")
                break
        
        self.is_running = False
    
    def send_input(self, user_input):
        """Send user input to the algorithm process"""
        if self.process and self.is_running:
            try:
                self.process.stdin.write(user_input + '\n')
                self.process.stdin.flush()
                return True
            except Exception as e:
                print(f"Error sending input: {e}")
                return False
        return False
    
    def stop_algorithm(self):
        """Stop the algorithm process"""
        self.is_running = False
        if self.process:
            self.process.terminate()
            self.process = None
    
    def get_output(self):
        """Get current output lines"""
        return self.output_lines.copy()
    
    def get_status(self):
        """Get current status"""
        return {
            'running': self.is_running,
            'output': self.output_lines
        }

# Removed single global runner in favor of per-session runners

@app.route('/')
def index():
    """Main page - check for consent first"""
    return render_template('index_simple.html')

@app.route('/consent')
def consent():
    """Consent page"""
    return render_template('consent.html')

@app.route('/feedback')
def feedback():
    """Feedback page"""
    return render_template('feedback.html')

@app.route('/thank_you')
def thank_you():
    """Thank you page"""
    return render_template('thank_you.html')


@app.route('/start_algorithm', methods=['POST'])
def start_algorithm():
    """Start the algorithm for a specific session"""
    data = request.get_json() or {}
    session_id = data.get('session_id')
    if not session_id:
        return jsonify({'success': False, 'message': 'session_id is required'}), 400
    dataset = 'car.txt'  # Always use car.txt at repo root
    use_real = data.get('use_real', False)

    runner = get_runner(session_id, create_if_missing=True)
    # If a previous process exists, terminate before starting a new one
    if runner.is_running:
        runner.stop_algorithm()
        time.sleep(0.2)

    success, message = runner.start_algorithm(dataset, use_real)
    return jsonify({'success': success, 'message': message})

@app.route('/send_input', methods=['POST'])
def send_input():
    """Send user input to the algorithm for a specific session"""
    data = request.get_json() or {}
    session_id = data.get('session_id')
    if not session_id:
        return jsonify({'success': False, 'message': 'session_id is required'}), 400
    user_input = data.get('input', '')

    runner = get_runner(session_id)
    if not runner or not runner.is_running:
        return jsonify({'success': False, 'message': 'No active session'}), 400

    success = runner.send_input(user_input)
    return jsonify({'success': success, 'message': 'Input sent' if success else 'Failed to send input'})

@app.route('/stop_algorithm', methods=['POST'])
def stop_algorithm():
    """Stop the algorithm for a specific session"""
    data = request.get_json() or {}
    session_id = data.get('session_id')
    if not session_id:
        return jsonify({'success': False, 'message': 'session_id is required'}), 400
    runner = get_runner(session_id)
    if runner:
        runner.stop_algorithm()
        with sessions_lock:
            sessions.pop(session_id, None)
    return jsonify({'success': True, 'message': 'Algorithm stopped'})

@app.route('/get_status', methods=['POST'])
def get_status():
    """Get current algorithm status and output for a specific session"""
    data = request.get_json() or {}
    session_id = data.get('session_id')
    if not session_id:
        return jsonify({'running': False, 'output': [], 'error': 'session_id is required'}), 400
    runner = get_runner(session_id)
    if not runner:
        return jsonify({'running': False, 'output': []})
    return jsonify(runner.get_status())

@app.route('/get_datasets')
def get_datasets():
    """Get list of available datasets. With root-level car.txt, return it if present."""
    datasets = []
    # Prefer root-level car.txt
    if os.path.exists('car.txt'):
        datasets.append('car.txt')
    # Backward compatibility: also include any .txt files in datasets/ if it exists
    datasets_dir = 'datasets'
    if os.path.isdir(datasets_dir):
        for file in os.listdir(datasets_dir):
            if file.endswith('.txt'):
                datasets.append(os.path.join(datasets_dir, file))
    return jsonify({'datasets': datasets})

@app.route('/map.json')
def get_dimension_map():
    """Serve the dimension mapping file"""
    try:
        with open('map.json', 'r') as f:
            import json
            return json.load(f)
    except FileNotFoundError:
        return jsonify({'error': 'Dimension mapping file not found'}), 404

@app.route('/study_completion', methods=['POST'])
def study_completion():
    """Record study completion data"""
    try:
        data = request.get_json()
        
        # Log completion data (in a real study, you might save this to a database)
        print("Study Completion Data:")
        print(f"  Start Time: {data.get('startTime', 'N/A')}")
        print(f"  End Time: {data.get('endTime', 'N/A')}")
        print(f"  Questions Answered: {data.get('questionsAnswered', 0)}")
        print(f"  Completed: {data.get('completed', False)}")
        
        return jsonify({'success': True, 'message': 'Study completion recorded'})
        
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/submit_feedback', methods=['POST'])
def submit_feedback():
    """Collect user feedback rating"""
    try:
        data = request.get_json()
        
        # Validate rating
        rating = data.get('rating')
        if not rating or rating < 1 or rating > 10:
            return jsonify({'success': False, 'error': 'Invalid rating'}), 400

        # Extract minimal fields
        study_data = data.get('studyData', {}) or {}
        num_questions = int(study_data.get('questionsAnswered', 0))
        # Best-effort client IP and region from headers
        xff = request.headers.get('X-Forwarded-For', '')
        client_ip = (xff.split(',')[0].strip() if xff else request.remote_addr) or ''
        country = request.headers.get('CF-IPCountry') or request.headers.get('X-Appengine-Country') or request.headers.get('X-Country-Code') or ''

        feedback_record = {
            'time': datetime.now().isoformat(),
            'rating': int(rating),
            'questionsAnswered': num_questions,
            'ip': client_ip,
            'country': country
        }

        # Before writing, if the existing file is a JSON array, migrate it to NDJSON
        _migrate_to_ndjson_if_array_file(FEEDBACK_FILE)

        # Append line-delimited JSON (one record per line)
        feedback_file = FEEDBACK_FILE
        os.makedirs(os.path.dirname(feedback_file) or '.', exist_ok=True)
        with open(feedback_file, 'a') as f:
            f.write(json.dumps(feedback_record) + '\n')

        print("User Feedback Recorded (NDJSON):")
        print(f"  {feedback_record}")
        print(f"  Saved to: {feedback_file}")

        return jsonify({'success': True, 'message': 'Feedback recorded successfully'})
        
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/admin/feedback', methods=['GET'])
def admin_feedback():
    """Return stored feedback JSON directly for quick inspection.

    Response shape: { path: str, count: int|None, data: list|object }
    """
    try:
        path = FEEDBACK_FILE
        data = []
        if os.path.exists(path):
            data = _read_feedback_records(path)
        else:
            # Fallback check legacy path if current path missing
            legacy = 'user_feedback.json'
            if os.path.exists(legacy):
                data = _read_feedback_records(legacy)
                path = legacy
        count = len(data)
        return jsonify({ 'path': path, 'count': count, 'data': data })
    except Exception as e:
        return jsonify({ 'error': str(e), 'path': FEEDBACK_FILE }), 500

if __name__ == '__main__':
    # Create templates directory if it doesn't exist
    os.makedirs('templates', exist_ok=True)
    os.makedirs('static', exist_ok=True)
    
    # Get port from environment variable (for Railway/Heroku)
    port = int(os.environ.get('PORT', 5001))
    
    print("Starting simple web application...")
    print(f"Open your browser and go to: http://localhost:{port}")
    app.run(debug=False, host='0.0.0.0', port=port)
