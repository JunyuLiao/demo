#!/usr/bin/env python3
"""
Check Railway deployment status
"""
import requests
import time
import sys

def check_deployment(railway_url):
    print(f"Checking deployment at: {railway_url}")
    
    try:
        # Test basic connectivity
        response = requests.get(railway_url, timeout=10)
        print(f"Status Code: {response.status_code}")
        
        if response.status_code == 200:
            print("✓ Deployment is working!")
            return True
        else:
            print(f"✗ Deployment returned status {response.status_code}")
            return False
            
    except requests.exceptions.Timeout:
        print("✗ Request timed out - deployment may be starting up")
        return False
    except requests.exceptions.ConnectionError:
        print("✗ Connection failed - deployment may be down")
        return False
    except Exception as e:
        print(f"✗ Error: {e}")
        return False

if __name__ == "__main__":
    # Replace with your actual Railway URL
    railway_url = "https://your-app-name.railway.app"
    print("Please update the railway_url variable with your actual Railway URL")
    print("Example: https://highdim-rm-demo.railway.app")
    sys.exit(1)
