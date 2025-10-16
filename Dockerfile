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

# Ensure runtime directories exist inside the image
RUN mkdir -p /app/output /data

# Build the web-real C++ binary
RUN make clean && make web-real

# Build the UH-Random C++ binary
RUN g++ -w -I/usr/include --std=c++17 -Wall -Werror -pedantic \
    main_uh_random.cpp highdim.cpp attribute_subset.cpp util.cpp \
    other/*.c other/*.cpp -L/usr/lib -lglpk -lm -Ofast -o main_uh_random

# Env for Flask port (Railway will set PORT)
ENV PYTHONUNBUFFERED=1

# Start the polling Flask app (uses PORT env var, defaults to 5001)
CMD ["python", "web_app_simple.py"]

