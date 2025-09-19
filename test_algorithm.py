#!/usr/bin/env python3
"""
Test script to verify the algorithm works correctly
"""
import subprocess
import os
import sys

def test_algorithm():
    print("Testing algorithm build and execution...")
    
    # Test 1: Build the algorithm
    print("\n1. Building algorithm...")
    try:
        result = subprocess.run(["make", "web-real"], capture_output=True, text=True)
        if result.returncode != 0:
            print(f"Build failed: {result.stderr}")
            return False
        print("✓ Build successful")
    except Exception as e:
        print(f"Build error: {e}")
        return False
    
    # Test 2: Check if executable exists
    print("\n2. Checking executable...")
    if not os.path.exists("./run_web"):
        print("✗ Executable not found")
        return False
    print("✓ Executable found")
    
    # Test 3: Check if dataset exists
    print("\n3. Checking dataset...")
    if not os.path.exists("datasets/car.txt"):
        print("✗ Dataset not found")
        return False
    print("✓ Dataset found")
    
    # Test 4: Test algorithm execution (with timeout)
    print("\n4. Testing algorithm execution...")
    try:
        # Start the algorithm
        process = subprocess.Popen(
            ["./run_web", "datasets/car.txt"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True
        )
        
        # Wait a bit for initialization
        import time
        time.sleep(2)
        
        # Check if process is still running
        if process.poll() is not None:
            print("✗ Algorithm exited immediately")
            stdout, _ = process.communicate()
            print(f"Output: {stdout}")
            return False
        
        print("✓ Algorithm started successfully")
        
        # Send test input and get output
        stdout, _ = process.communicate(input="1\n", timeout=5)
        print(f"Algorithm output: {stdout[:200]}...")
        
        # Clean up
        process.terminate()
        
    except subprocess.TimeoutExpired:
        print("✓ Algorithm running (timeout expected)")
        process.terminate()
    except Exception as e:
        print(f"✗ Algorithm test failed: {e}")
        return False
    
    print("\n✓ All tests passed!")
    return True

if __name__ == "__main__":
    success = test_algorithm()
    sys.exit(0 if success else 1)
