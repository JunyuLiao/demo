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
RUN make clean && make web-real

# Env for Flask port (Railway will set PORT)
ENV PYTHONUNBUFFERED=1

# Start the polling Flask app (uses PORT env var, defaults to 5001)
CMD ["python", "web_app_simple.py"]

