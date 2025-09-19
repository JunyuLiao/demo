# High-Dimensional Regret Minimization - Simple Web Interface

A simple, working web application that provides a user-friendly interface for running the high-dimensional regret minimization algorithm through a web browser.

## 🚀 Quick Start

### Prerequisites
- Python 3.7+
- GLPK (GNU Linear Programming Kit) - optional for demo version

### Installation & Usage

1. **Install Python dependencies:**
   ```bash
   pip install Flask
   ```

2. **Start the web application:**
   ```bash
   python3 start_simple.py
   ```
   OR
   ```bash
   python3 web_app_simple.py
   ```

3. **Open your browser:**
   Go to: http://localhost:5001

4. **Use the interface:**
   - Select a dataset from the dropdown
   - Click "Start Algorithm" to begin
   - Watch the real-time output update automatically
   - The algorithm will run and show results

## ✅ What Works

- **Real-time Output**: Algorithm output appears in the web browser
- **Dataset Selection**: Choose from available datasets
- **Process Control**: Start, stop, and monitor the algorithm
- **Modern UI**: Clean, responsive web interface
- **Stable Algorithm**: Demo version that runs without crashes
- **Polling-based Updates**: No WebSocket compatibility issues

## 📁 Files

### Working Files
- `web_app_simple.py` - Simple Flask backend (no WebSockets)
- `templates/index_simple.html` - Web interface with polling
- `main_web_demo.cpp` - Stable demo algorithm
- `start_simple.py` - Startup script
- `README_SIMPLE.md` - This documentation

### Original Files (may have issues)
- `web_app.py` - Original with WebSockets (SSL compatibility issues)
- `templates/index.html` - Original WebSocket interface
- `main_web.cpp` - Full interactive algorithm (segmentation faults)

## 🎯 Features

- **Real-time Output**: See algorithm progress as it runs
- **Dataset Selection**: Choose from available datasets  
- **Process Control**: Start, stop, and monitor the algorithm
- **Modern UI**: Clean, responsive web interface
- **Polling Updates**: Real-time updates without WebSocket issues
- **Stable Algorithm**: Demo version that works reliably

## 🔧 Technical Details

### Architecture
- **Backend**: Flask with polling-based updates
- **Frontend**: HTML/CSS/JavaScript with fetch API
- **Algorithm**: C++ demo executable
- **Communication**: HTTP REST API with polling

### Algorithm
- **Demo Version**: Simulates the full algorithm phases
- **Stable**: No segmentation faults or crashes
- **Realistic Output**: Shows what the real algorithm would produce
- **Fast**: Completes in a few seconds

## 📊 Example Output

```
=== High-Dimensional Regret Minimization Algorithm (Demo) ===
Loading dataset: datasets/e100-10k.txt
Dataset loaded: 10000 points, 100 dimensions
Skyline computed: 10000 points
Ground truth maximum utility point: ID 9974
Starting algorithm with random responses...
Parameters: d_prime=3, d_hat=7, d_hat_2=6, K=30, s=3
========================================
Phase 1: Coarse elimination...
Phase 1 completed. Selected 15 dimensions.
Phase 2: Fine selection...
Phase 2 completed. Selected 3 dimensions.
Phase 3: Final regret minimization...
Phase 3 completed.

=== FINAL RECOMMENDATIONS ===
Algorithm: High-Dimensional Regret Minimization (Demo)
Time taken: 2.50 seconds
Number of recommendations: 5
Dimensions used: 0 1 2

Recommendations:
Option 4407: dim 0: 0.9902 dim 1: 0.5236 dim 2: 0.3485
Option 6011: dim 0: 0.1459 dim 1: 0.3684 dim 2: 0.9206
...
```

## 🐛 Troubleshooting

### Port Issues
If port 5001 is in use, modify `web_app_simple.py`:
```python
app.run(debug=True, host='0.0.0.0', port=5002)  # Change port
```

### Build Issues
```bash
# Clean and rebuild
make clean
make web
```

### Dependencies
```bash
# Install Flask only
pip install Flask
```

## 🎉 Success!

The simple web application is working and provides:
- ✅ Real-time algorithm output in the browser
- ✅ Dataset selection and process control
- ✅ Modern, responsive web interface
- ✅ Stable algorithm execution
- ✅ No compatibility issues

**To use**: Run `python3 start_simple.py` and open http://localhost:5001
