"""
turn_test.py — drive the car chassis through forward, backward and both spin turns over USB serial.

Usage:
    & "$env:USERPROFILE\.platformio\penv\Scripts\python.exe" tools\turn_test.py
    (lift the wheels first; close any serial monitor on the port)
"""

# %% Imports
import time

import serial


# %% Config
PORT        = "COM7"
BAUD        = 115200
SPEED       = 255          # sketch caps the PWM itself (SPEED_LIMIT in main.cpp)
STEP_S      = 3.0          # seconds per move
PAUSE_S     = 2.0          # seconds stopped between moves
RESEND_S    = 0.2          # must stay well under the sketch's 500 ms watchdog
BOOT_WAIT_S = 2.5          # Uno reset on port open: bootloader plus sketch start

# Motor order matches the sketch: front-left, front-right, back-left, back-right.
MOVES = [
    ("all forward",               ( SPEED,  SPEED,  SPEED,  SPEED)),
    ("all backward",              (-SPEED, -SPEED, -SPEED, -SPEED)),
    ("spin left (anticlockwise)", (-SPEED,  SPEED, -SPEED,  SPEED)),
    ("spin right (clockwise)",    ( SPEED, -SPEED,  SPEED, -SPEED)),
]


# %% Helpers
def send(port: serial.Serial, line: str) -> str:
    port.write((line + "\n").encode())
    return port.readline().decode(errors="replace").strip()


def run_move(port: serial.Serial, name: str, speeds: tuple) -> None:
    cmd   = "M " + " ".join(str(s) for s in speeds)
    bad   = 0
    t_end = time.monotonic() + STEP_S
    print(f"{name:<28} {cmd}")
    while time.monotonic() < t_end:
        if send(port, cmd) != "OK":
            bad += 1
        time.sleep(RESEND_S)
    send(port, "S")
    print(f"{'':<28} stopped, bad replies: {bad}")
    time.sleep(PAUSE_S)


# %% Main function
def main(config: dict) -> None:
    with serial.Serial(config["port"], config["baud"], timeout=0.5) as port:
        time.sleep(config["boot_wait_s"])      # opening the port resets the Uno; wait out its boot banner
        port.reset_input_buffer()
        try:
            for name, speeds in config["moves"]:
                run_move(port, name, speeds)
        finally:
            send(port, "S")                    # always leave the motors stopped
    print("done")


# %% Entry point
if __name__ == "__main__":

    config = dict(
        port        = PORT,
        baud        = BAUD,
        boot_wait_s = BOOT_WAIT_S,
        moves       = MOVES,
    )

    main(config)
