#!/usr/bin/env python3
"""
Web application for running the high-dimensional regret minimization algorithm.
This creates a simple web interface that can execute the C++ algorithm and stream output.
"""

import os
import sys
import subprocess
import threading
import time
import json
from flask import Flask, render_template, request, jsonify, Response
from flask_socketio import SocketIO, emit
import queue

app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-secret-key-here'
socketio = SocketIO(app, cors_allowed_origins="*")

# Global variables for process management
current_process = None
output_queue = queue.Queue()
input_queue = queue.Queue()

class AlgorithmRunner:
    def __init__(self):
        self.process = None
        self.is_running = False
        
    def start_algorithm(self, dataset_path="datasets/e100-10k.txt"):
        """Start the C++ algorithm process"""
        try:
            # Build the web algorithm if needed
            if not os.path.exists("run_web"):
                print("Building web algorithm...")
                result = subprocess.run(["make", "web"], capture_output=True, text=True)
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
                    # Send output to web interface
                    socketio.emit('algorithm_output', {'data': line.rstrip()})
                elif self.process.poll() is not None:
                    # Process has ended
                    break
            except Exception as e:
                print(f"Error reading output: {e}")
                break
        
        self.is_running = False
        socketio.emit('algorithm_finished', {'status': 'completed'})
    
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

# Global algorithm runner instance
algorithm_runner = AlgorithmRunner()

@app.route('/')
def index():
    """Main page"""
    return render_template('index.html')

@app.route('/start_algorithm', methods=['POST'])
def start_algorithm():
    """Start the algorithm"""
    data = request.get_json()
    dataset = data.get('dataset', 'datasets/e100-10k.txt')
    
    success, message = algorithm_runner.start_algorithm(dataset)
    
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

@socketio.on('connect')
def handle_connect():
    """Handle client connection"""
    print('Client connected')
    emit('status', {'message': 'Connected to server'})

@socketio.on('disconnect')
def handle_disconnect():
    """Handle client disconnection"""
    print('Client disconnected')

if __name__ == '__main__':
    # Create templates directory if it doesn't exist
    os.makedirs('templates', exist_ok=True)
    os.makedirs('static', exist_ok=True)
    
    print("Starting web application...")
    print("Open your browser and go to: http://localhost:5000")
    socketio.run(app, debug=True, host='0.0.0.0', port=5000, allow_unsafe_werkzeug=True)
