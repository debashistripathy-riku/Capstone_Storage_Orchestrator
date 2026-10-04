
if [ ! -f "bin/storage_orchestrator" ]; then
    echo "Binaries not found. Building..."
    ./build.sh
fi

echo "Starting daemon..."
./bin/storage_orchestrator &
DAEMON_PID=$!

trap "echo 'Stopping daemon...'; kill $DAEMON_PID 2>/dev/null; exit 0" INT TERM EXIT

sleep 1

echo "Starting CLI..."
./bin/telemetry_cli

wait $DAEMON_PID 2>/dev/null
