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

# Global variables for process management
current_process = None
output_queue = queue.Queue()
input_queue = queue.Queue()
algorithm_output = []
algorithm_running = False

class AlgorithmRunner:
    def __init__(self):
        self.process = None
        self.is_running = False
        self.output_lines = []
        
    def start_algorithm(self, dataset_path="datasets/car.txt", use_real=False):
        """Start the C++ algorithm process with fallback to mock"""
        try:
            # Try to build the real algorithm first
            print("Building algorithm (real version)...")
            result = subprocess.run(["make", "web-real"], capture_output=True, text=True)
            if result.returncode != 0:
                print(f"Real algorithm build failed: {result.stderr}")
                print("Falling back to mock algorithm...")
                
                # Fallback to mock algorithm
                self.process = subprocess.Popen(
                    ["python3", "mock_algorithm.py", dataset_path],
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
                
                return True, "Mock algorithm started (real algorithm build failed)"
            
            print("Real algorithm built successfully")
            
            # Start the real algorithm process
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
            
            return True, "Real algorithm started successfully"
            
        except Exception as e:
            print(f"Real algorithm failed: {str(e)}")
            print("Falling back to mock algorithm...")
            
            try:
                # Fallback to mock algorithm
                self.process = subprocess.Popen(
                    ["python3", "mock_algorithm.py", dataset_path],
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
                
                return True, "Mock algorithm started (real algorithm failed)"
            except Exception as e2:
                return False, f"Failed to start both real and mock algorithms: {str(e2)}"
    
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

# Global algorithm runner instance
algorithm_runner = AlgorithmRunner()

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
    """Start the algorithm"""
    data = request.get_json()
    dataset = 'datasets/car.txt'  # Always use car.txt
    use_real = data.get('use_real', False)
    
    success, message = algorithm_runner.start_algorithm(dataset, use_real)
    
    return jsonify({
        'success': success,
        'message': message
    })

@app.route('/send_input', methods=['POST'])
def send_input():
    """Send user input to the algorithm"""
    data = request.get_json()
    user_input = data.get('input', '')
    
    success = algorithm_runner.send_input(user_input)
    
    return jsonify({
        'success': success,
        'message': 'Input sent' if success else 'Failed to send input'
    })

@app.route('/stop_algorithm', methods=['POST'])
def stop_algorithm():
    """Stop the algorithm"""
    algorithm_runner.stop_algorithm()
    return jsonify({
        'success': True,
        'message': 'Algorithm stopped'
    })

@app.route('/get_status')
def get_status():
    """Get current algorithm status and output"""
    status = algorithm_runner.get_status()
    return jsonify(status)

@app.route('/get_datasets')
def get_datasets():
    """Get list of available datasets"""
    datasets = []
    datasets_dir = "datasets"
    
    if os.path.exists(datasets_dir):
        for file in os.listdir(datasets_dir):
            if file.endswith('.txt'):
                datasets.append(file)
    
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
        
        # Prepare feedback data for saving
        feedback_data = {
            'rating': rating,
            'timestamp': data.get('timestamp', ''),
            'study_data': data.get('studyData', {}),
            'submission_time': datetime.now().isoformat()
        }
        
        # Save to feedback file
        feedback_file = 'user_feedback.json'
        
        # Read existing feedback data
        existing_feedback = []
        try:
            with open(feedback_file, 'r') as f:
                existing_feedback = json.load(f)
        except (FileNotFoundError, json.JSONDecodeError):
            existing_feedback = []
        
        # Add new feedback
        existing_feedback.append(feedback_data)
        
        # Write back to file
        with open(feedback_file, 'w') as f:
            json.dump(existing_feedback, f, indent=2)
        
        # Also log to console for immediate visibility
        print("User Feedback Data:")
        print(f"  Rating: {rating}/10")
        print(f"  Timestamp: {data.get('timestamp', 'N/A')}")
        print(f"  Study Data: {data.get('studyData', {})}")
        print(f"  Saved to: {feedback_file}")
        
        return jsonify({'success': True, 'message': 'Feedback recorded successfully'})
        
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
