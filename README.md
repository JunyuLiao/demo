# High-Dimensional Regret Minimization Demo: An advanced interactive recommendation system

This app provides an interactive web UI (Flask) that runs a C++ high‑dimensional regret minimization algorithm.

## Quick start (local)

- Prereqs: macOS/Linux with make, C++17 toolchain, GLPK installed (Homebrew/MacPorts on macOS)
- Build the web binary:
  - `make web-real`
- Run the simple server:
  - `python3 start.py`
- Open: http://localhost:5001

If you want to run on a custom port: `PORT=8080 python3 web_app_simple.py` then open http://localhost:8080

## Dataset

- `car.txt` is at the repo root. The app references it directly as `car.txt`.

## Session isolation

- Each browser session gets its own algorithm process, keyed by a generated `session_id`. Inputs are not shared between users.
- Invalid inputs are handled on the frontend, backend, and C++ side. Enter option numbers in range, `0` to skip, or `-99` to stop early.

## Data persistence

- Feedback is stored as a JSON array in a writable volume directory:
  - File path: `/data/user_feedback.json` (configurable via `DATA_DIR` env)
  - Public endpoint to inspect: `/data/feedback.json`
- Saved record fields: `startTime`, `endTime`, `questions`, `rating`, `ip`, `submission_time`.

## Endpoints (HTTP)

- `POST /start_algorithm` `{ session_id, use_real: true }` → starts the C++ process
- `POST /send_input` `{ session_id, input }` → forwards answer to the running process
- `POST /get_status` `{ session_id }` → returns `{ running, output[] }` (polled by UI)
- `POST /stop_algorithm` `{ session_id }` → stops and cleans up
- `GET  /data/feedback.json` → returns saved feedback JSON array

## Docker

Build and run using Docker:

```
docker build -t highdim-demo .
docker run -p 5001:5001 -e PORT=5001 highdim-demo
```

Notes:
- The image installs GLPK and builds the C++ binary during the Docker build.
- Runtime directories are created inside the image: `/app/output`, `/data`.

## Railway deployment

- Live demo (example): https://highdim-rm-demo-production.up.railway.app
- Deploy with the CLI from the repo root:
  1) `railway login`
  2) `railway init` (create/link a project)
  3) `railway up -s <service-name>`

The server binds to `0.0.0.0` and respects the `PORT` env variable supplied by Railway. Mount a volume at `/data` for persisted feedback.