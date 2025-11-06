#!/bin/bash
# Starts a tmux session that logs output with timestamps to a file

# === Configuration ===
SESSION_NAME="sipm-testbench-run"
LOG_DIR="$HOME/sipm-testbench-logs"

# Create log directory if it doesn't exist
mkdir -p "$LOG_DIR"

# Generate filename with timestamp
TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")
LOG_FILE="$LOG_DIR/${SESSION_NAME}_${TIMESTAMP}.log"

# === Start tmux session ===
tmux new-session -d -s "$SESSION_NAME"

# Set up logging with timestamps
tmux pipe-pane -o -t "$SESSION_NAME" \
  "awk '{ print strftime(\"[%Y-%m-%d %H:%M:%S]\"), \$0; fflush() }' >> '$LOG_FILE'"

# Attach to the session
tmux attach -t "$SESSION_NAME"

