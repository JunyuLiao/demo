# Use Ubuntu base image with GLPK pre-installed
FROM ubuntu:20.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV PYTHONUNBUFFERED=1

# Install system dependencies
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    libglpk-dev \
    python3 \
    python3-pip \
    python3-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy requirements and install Python dependencies
COPY requirements.txt .
RUN pip3 install -r requirements.txt

# Copy source code
COPY . .

# Build the C++ algorithm
RUN make web-real

# Expose port
EXPOSE 5000

# Start the application
CMD ["python3", "web_app_simple.py"]
