# Use Ubuntu 22.04 base image
FROM ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV PYTHONUNBUFFERED=1

# Install system dependencies including GLPK
RUN apt-get update && \
    apt-get install -y \
    build-essential \
    wget \
    curl \
    python3 \
    python3-pip \
    python3-dev \
    && apt-get install -y libglpk-dev || \
    (echo "libglpk-dev not available, installing from source..." && \
     cd /tmp && \
     wget https://ftp.gnu.org/gnu/glpk/glpk-5.0.tar.gz && \
     tar -xzf glpk-5.0.tar.gz && \
     cd glpk-5.0 && \
     ./configure --prefix=/usr/local && \
     make -j$(nproc) && \
     make install && \
     ldconfig) && \
    rm -rf /var/lib/apt/lists/*

# Verify GLPK installation
RUN echo "Checking GLPK installation..." && \
    find /usr -name "glpk.h" 2>/dev/null || find /usr/local -name "glpk.h" 2>/dev/null || \
    (echo "GLPK not found!" && exit 1)

# Set working directory
WORKDIR /app

# Copy requirements and install Python dependencies
COPY requirements.txt .
RUN pip3 install -r requirements.txt

# Copy source code
COPY . .

# Debug: Check what was copied
RUN echo "Checking copied files..." && \
    ls -la && \
    echo "Checking if datasets directory exists..." && \
    if [ -d "datasets" ]; then \
        echo "datasets directory exists" && \
        ls -la datasets/ && \
        if [ -f "datasets/car.txt" ]; then \
            echo "car.txt exists" && \
            ls -la datasets/car.txt; \
        else \
            echo "car.txt does not exist"; \
        fi; \
    else \
        echo "datasets directory does not exist"; \
    fi

# Build the C++ algorithm
RUN make web-real

# Expose port
EXPOSE 5000

# Start the application
CMD ["python3", "web_app_simple.py"]
