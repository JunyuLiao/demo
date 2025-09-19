#!/usr/bin/env python3
"""
Mock algorithm for deployment when GLPK is not available.
This simulates the real C++ algorithm behavior for demonstration purposes.
"""

import json
import random
import time
import sys

def load_dataset(dataset_path):
    """Load and parse the dataset"""
    try:
        with open(dataset_path, 'r') as f:
            lines = f.readlines()
        
        # Skip header line
        data_lines = lines[1:]
        
        points = []
        for line in data_lines:
            if line.strip():
                values = [float(x) for x in line.strip().split()]
                points.append(values)
        
        return points
    except Exception as e:
        print(f"Error loading dataset: {e}")
        return []

def generate_question(points, question_num):
    """Generate a mock question with 2 options"""
    if len(points) < 2:
        return None
    
    # Select 2 random points
    option1_idx = random.randint(0, len(points) - 1)
    option2_idx = random.randint(0, len(points) - 1)
    while option2_idx == option1_idx:
        option2_idx = random.randint(0, len(points) - 1)
    
    option1 = points[option1_idx]
    option2 = points[option2_idx]
    
    # Select random dimensions to show (7 dimensions per question)
    num_dims = min(7, len(option1))
    dims = random.sample(range(len(option1)), num_dims)
    
    print(f"Please choose the option you favor more:")
    print("|   Options|" + "".join([f"{d:10d}|" for d in dims]))
    print("-" * (11 + 11 * len(dims)))
    print("|   Option1|" + "".join([f"{option1[d]:10.3f}|" for d in dims]))
    print("|   Option2|" + "".join([f"{option2[d]:10.3f}|" for d in dims]))
    print("-" * (11 + 11 * len(dims)))
    print("Your choice (0 for not interested): ", end="", flush=True)
    
    return {
        'option1': option1,
        'option2': option2,
        'dims': dims,
        'option1_idx': option1_idx,
        'option2_idx': option2_idx
    }

def generate_final_recommendations(points, num_recommendations=10):
    """Generate mock final recommendations"""
    if len(points) < num_recommendations:
        num_recommendations = len(points)
    
    # Select random points as recommendations
    selected_indices = random.sample(range(len(points)), num_recommendations)
    
    print("\n=== FINAL RECOMMENDATION ===")
    for i, idx in enumerate(selected_indices):
        point = points[idx]
        print(f"Option {idx}: ", end="")
        for j, value in enumerate(point):
            print(f"dim {j}: {value:.4f} ", end="")
        print()

def main():
    if len(sys.argv) != 2:
        print("Usage: python mock_algorithm.py <dataset_path>")
        sys.exit(1)
    
    dataset_path = sys.argv[1]
    print("=== Mock High-Dimensional Regret Minimization Algorithm ===")
    print(f"Loading dataset: {dataset_path}")
    
    points = load_dataset(dataset_path)
    if not points:
        print("Failed to load dataset")
        sys.exit(1)
    
    print(f"Dataset loaded: {len(points)} points, {len(points[0]) if points else 0} dimensions")
    print("Starting mock interactive algorithm...")
    print("Parameters: d_prime=3, d_hat=7, d_hat_2=6, K=10, s=2")
    print("Initializing algorithm phases...")
    print("=" * 40)
    print("Phase 1: Initializing dimension selection...")
    
    # Generate 5 mock questions
    for question_num in range(1, 6):
        print(f"\nQuestion {question_num}:")
        question_data = generate_question(points, question_num)
        if not question_data:
            break
        
        # Wait for user input
        try:
            choice = input().strip()
            if choice == "-99":
                print("Stopping early...")
                break
        except EOFError:
            print("No input received, stopping...")
            break
    
    print("\nPhase 2: Refining dimension selection...")
    print("Phase 3: Starting interactive questioning...")
    print("number of final dimensions: 57")
    print("number of rounds: 2")
    print("\nPhase 3B completed")
    
    # Generate final recommendations
    generate_final_recommendations(points)

if __name__ == "__main__":
    main()
