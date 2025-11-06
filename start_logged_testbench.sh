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

COMMANDS=$(cat <<'EOF'
cat /root/system_wrapper.bit > /dev/xdevcfg
sleep 1
/root/sipm-testbench/bin/testbench
EOF
)

# === Start tmux session running those commands ===
tmux new-session -d -s "$SESSION_NAME" "bash -c '$COMMANDS'"


tmux pipe-pane -o "sed -r 's/\x1B\[([0-9]{1,3}(;[0-9]{1,3})*)?[mGK]//g' > output.log"


# Attach to the session
tmux attach -t "$SESSION_NAME"


