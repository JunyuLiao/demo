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
# --------- Custom JSON writer to keep each interaction on one line ---------
def write_sessions_with_singleline_interactions(records, filepath):
    try:
        lines = ['[']
        for idx, rec in enumerate(records):
            lines.append('  {')
            # Write non-interaction keys first, preserving order
            items = [(k, v) for k, v in rec.items() if k != 'interaction']
            for i, (k, v) in enumerate(items):
                import json as _json
                key = _json.dumps(k)
                val = _json.dumps(v, ensure_ascii=False)
                comma = ',' if (i < len(items) - 1 or ('interaction' in rec)) else ''
                lines.append(f'    {key}: {val}{comma}')
            # Write interaction array with one-line objects
            if 'interaction' in rec and isinstance(rec['interaction'], list):
                lines.append('    "interaction": [')
                for j, item in enumerate(rec['interaction']):
                    import json as _json
                    item_str = _json.dumps(item, separators=(',', ':'), ensure_ascii=False)
                    comma = ',' if j < len(rec['interaction']) - 1 else ''
                    lines.append(f'      {item_str}{comma}')
                lines.append('    ]')
            lines.append('  }' + (',' if idx < len(records) - 1 else ''))
        lines.append(']')
        with open(filepath, 'w') as f:
            f.write('\n'.join(lines) + ('\n' if lines and not lines[-1].endswith('\n') else ''))
    except Exception:
        # Fallback: dump compact if custom formatting fails
        try:
            import json as _json
            with open(filepath, 'w') as f:
                _json.dump(records, f)
        except Exception:
            pass


# --------- Data storage helpers (volume-aware) ---------
def get_data_dir():
    """Return a writable data directory, preferring /data or $DATA_DIR."""
    preferred = os.environ.get('DATA_DIR', '/data')
    try:
        os.makedirs(preferred, exist_ok=True)
        test_path = os.path.join(preferred, '.write_test')
        with open(test_path, 'w') as f:
            f.write('ok')
        os.remove(test_path)
        return preferred
    except Exception:
        # Fallback to local directory
        os.makedirs('.', exist_ok=True)
        return '.'

def read_json_array(filepath):
    try:
        with open(filepath, 'r') as f:
            return json.load(f)
    except (FileNotFoundError, json.JSONDecodeError):
        return []

def append_json_array(filepath, item):
    data = read_json_array(filepath)
    data.append(item)
    # Ensure directory exists
    os.makedirs(os.path.dirname(filepath) or '.', exist_ok=True)
    with open(filepath, 'w') as f:
        json.dump(data, f, indent=2)

class AlgorithmRunner:
    def __init__(self):
        self.process = None
        self.is_running = False
        self.output_lines = []
        
    def start_algorithm(self, dataset_path="car.txt", use_real=False):
        """Start the C++ algorithm process"""
        try:
            # Ensure output directory exists for C++ intermediate files
            os.makedirs('output', exist_ok=True)
            
            # Build only if binary is missing and not explicitly skipped
            skip_build = os.environ.get('SKIP_BUILD', '0') == '1'
            if not skip_build and not os.path.exists('./run_web'):
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
        
        # Mark not running and log exit code for diagnosis
        exit_code = None
        try:
            exit_code = self.process.poll() if self.process else None
        except Exception:
            exit_code = None
        print(f"Algorithm process ended. exit_code={exit_code}")
        self.is_running = False
        self.process = None
    
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

    # Initialize a per-session scratch file path for interactions
    try:
        data_dir = get_data_dir()
        feedback_file = os.path.join(data_dir, 'user_feedback.json')
        try:
            with open(feedback_file, 'r') as f:
                existing = json.load(f)
        except Exception:
            existing = []

        if not isinstance(existing, list):
            existing = []

        # Only append a new skeleton if the last record already has core fields,
        # or file is empty. This avoids creating multiple skeletons in a row.
        need_new = True
        if len(existing) > 0 and isinstance(existing[-1], dict):
            last = existing[-1]
            if 'interaction' in last and not (('startTime' in last) or ('rating' in last) or ('ip' in last)):
                need_new = False
        if need_new:
            # Just append an empty stub to mark a new session; interactions will
            # accumulate in DATA_DIR/sessions/<SESSION_ID>.json until feedback is submitted
            existing.append({})

        write_sessions_with_singleline_interactions(existing, feedback_file)

        # Export SESSION_ID to the C++ process via environment so it can write to
        # DATA_DIR/sessions/<SESSION_ID>.json. (Already set in browser, but ensure env var exists here too.)
        os.environ['SESSION_ID'] = session_id
    except Exception:
        # Non-fatal: continue even if logging init fails
        pass

    return jsonify({'success': success, 'message': message})

@app.route('/send_input', methods=['POST'])
def send_input():
    """Send user input to the algorithm for a specific session"""
    data = request.get_json() or {}
    session_id = data.get('session_id')
    if not session_id:
        return jsonify({'success': False, 'message': 'session_id is required'}), 400
    raw_input = str(data.get('input', '')).strip()

    # Validate integer input
    try:
        user_input = int(raw_input)
    except Exception:
        return jsonify({'success': False, 'message': 'Please enter an integer.'}), 400

    runner = get_runner(session_id)
    if not runner or not runner.is_running:
        return jsonify({'success': False, 'message': 'No active session'}), 400

    # Forward input
    success = runner.send_input(str(user_input))
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


# --------- Data inspection endpoints ---------
@app.route('/data/feedback.json')
def view_feedback():
    data_dir = get_data_dir()
    feedback_file = os.path.join(data_dir, 'user_feedback.json')
    return jsonify(read_json_array(feedback_file))

@app.route('/data/completions.json')
def view_completions():
    return jsonify({'error': 'Not stored'}), 404

@app.route('/study_completion', methods=['POST'])
def study_completion():
    """Record study completion data"""
    try:
        # Accept but do not persist separate completion records.
        data = request.get_json() or {}
        print("Study completion ping:")
        print(f"  Start Time: {data.get('startTime', 'N/A')}")
        print(f"  End Time: {data.get('endTime', 'N/A')}")
        print(f"  Questions Answered: {data.get('questionsAnswered', 0)}")
        print(f"  Completed: {data.get('completed', False)}")
        return jsonify({'success': True, 'message': 'OK'})
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500

@app.route('/submit_feedback', methods=['POST'])
def submit_feedback():
    """Collect user feedback rating"""
    try:
        data = request.get_json() or {}
        
        # Validate ratings
        rating1 = data.get('rating1')
        rating2 = data.get('rating2')
        if (not rating1 or rating1 < 1 or rating1 > 10) or (not rating2 or rating2 < 1 or rating2 > 10):
            return jsonify({'success': False, 'error': 'Invalid rating'}), 400
        
        # Extract study info
        study = data.get('studyData', {}) or {}
        start_time = study.get('startTime', '')
        end_time = study.get('endTime', '')
        questions = study.get('questionsAnswered', 0)
        # Client IP (consider X-Forwarded-For when behind proxy)
        ip = request.headers.get('X-Forwarded-For', request.remote_addr)
        if ip and ',' in ip:
            ip = ip.split(',')[0].strip()
        
        # Prepare minimal record
        record = {
            'startTime': start_time,
            'endTime': end_time,
            'questions': questions,
            'rating1': rating1,
            'rating2': rating2,
            'ip': ip,
            'submission_time': datetime.now().isoformat()
        }
        
        # Merge this feedback into the latest session record so that
        # startTime/endTime/questions/ip/rating live at the same level as
        # the "interaction" array collected during the run.
        data_dir = get_data_dir()
        feedback_file = os.path.join(data_dir, 'user_feedback.json')

        # Load existing array (or create if missing)
        try:
            with open(feedback_file, 'r') as f:
                existing = json.load(f)
        except Exception:
            existing = []

        # Merge interactions from the per-session scratch file, then clear it
        try:
            sessions_dir = os.path.join(get_data_dir(), 'sessions')
            os.makedirs(sessions_dir, exist_ok=True)
            session_file = os.path.join(sessions_dir, os.environ.get('SESSION_ID', 'default') + '.json')
            interactions = []
            if os.path.exists(session_file):
                with open(session_file, 'r') as sf:
                    tmp_obj = json.load(sf)
                    interactions = tmp_obj.get('interaction', []) if isinstance(tmp_obj, dict) else []
                # Clear the session scratch after reading
                try:
                    os.remove(session_file)
                except Exception:
                    pass
            if interactions:
                record['interaction'] = interactions
        except Exception:
            pass

        # Decide whether to update the last record or append a new one
        if isinstance(existing, list) and len(existing) > 0 and isinstance(existing[-1], dict):
            last = existing[-1]
            # If last record doesn't yet have core fields, treat it as the current session
            if 'startTime' not in last and 'rating' not in last:
                last.update(record)
            else:
                existing.append(record)
        else:
            existing = [record]

        # Persist with one-line interactions
        write_sessions_with_singleline_interactions(existing, feedback_file)

        return jsonify({'success': True, 'message': 'Feedback recorded successfully', 'file': feedback_file})
        
    except Exception as e:
        return jsonify({'success': False, 'error': str(e)}), 500

if __name__ == '__main__':
    # Create templates directory if it doesn't exist
    os.makedirs('templates', exist_ok=True)
    os.makedirs('static', exist_ok=True)
    
    # Get port from environment variable (for Railway/Heroku)
    port = int(os.environ.get('PORT', 5001))
    
    print("Starting simple web application...")
    print(f"Open your browser and go to: http://localhost:{port}")
    app.run(debug=False, host='0.0.0.0', port=port)
