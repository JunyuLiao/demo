#!/usr/bin/env python3
"""
Startup script for the Simple High-Dimensional Regret Minimization Web Application
"""

import os
import sys
import subprocess

def check_dependencies():
    """Check if required dependencies are installed"""
    try:
        import flask
        print("✓ Flask is installed")
        return True
    except ImportError as e:
        print(f"✗ Missing dependency: {e}")
        print("Please install dependencies with: pip install Flask")
        return False

def build_algorithm():
    """Build the C++ algorithm"""
    print("Building C++ algorithm...")
    try:
        result = subprocess.run(["make", "web-real"], capture_output=True, text=True)
        if result.returncode == 0:
            print("✓ Algorithm built successfully")
            return True
        else:
            print(f"✗ Build failed: {result.stderr}")
            return False
    except Exception as e:
        print(f"✗ Build error: {e}")
        return False

def main():
    print("=== High-Dimensional Regret Minimization Web App (Simple) ===")
    print()
    
    # Check dependencies
    if not check_dependencies():
        return 1
    
    # Build algorithm
    if not build_algorithm():
        return 1
    
    print()
    print("Starting simple web application...")
    print("Open your browser and go to: http://localhost:5001")
    print("Press Ctrl+C to stop the server")
    print()
    
    # Start the web app
    try:
        from web_app_simple import app
        app.run(debug=False, host='0.0.0.0', port=5001)
    except KeyboardInterrupt:
        print("\nShutting down...")
    except Exception as e:
        print(f"Error starting web app: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
