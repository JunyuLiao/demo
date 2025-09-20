FROM python:3.10-slim

# Install build tools and GLPK
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    make \
    g++ \
    libglpk-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy dependency lists first for better caching
COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

# Copy the rest of the app
COPY . .

# Build the web-real C++ binary
# Build with sanitizers enabled per Makefile
RUN make clean && make web-real

# Env for Flask port (Railway will set PORT)
ENV PYTHONUNBUFFERED=1
# Conservative defaults to reduce memory/CPU usage (can override in Railway vars)
ENV HD_NUM_QUESTIONS=60 \
    HD_K=8

# Start the polling Flask app (uses PORT env var, defaults to 5001)
CMD ["python", "web_app_simple.py"]

