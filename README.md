# High-Dimensional Regret Minimization Demo

This app exposes a web UI and Python Flask server that runs the C++ algorithm.

## Local

- Build: `make web-real`
- Run: `python3 start.py` then open http://localhost:5001

## Dataset

- The dataset `car.txt` lives at the repo root. The app references it directly as `car.txt`.

## Docker

Build and run using Docker:

```
docker build -t highdim-demo .
docker run -p 5001:5001 -e PORT=5001 highdim-demo
```

## Railway

This project is deployable to Railway using the Dockerfile.

1. Install Railway CLI
2. `railway login` (or set `RAILWAY_TOKEN`)
3. `railway init` (or `railway up` in the repo root)

The server binds to `0.0.0.0` and reads `PORT` env var, so it will run the same way as local.