#!/usr/bin/env python3
import sys
import time
import serial

def highlight_diff(sent, received):
    """Return the received string with any differing chars highlighted in red."""
    out = []
    length = max(len(sent), len(received))
    is_diff = False
    for i in range(length):
        s_char = sent[i] if i < len(sent) else ""
        r_char = received[i] if i < len(received) else ""
        if s_char != r_char:
            is_diff = True
            out.append(f"\033[31m{r_char}\033[0m")  # highlight in red
        else:
            out.append(r_char)
    return (is_diff, "".join(out))

def reset(ser):
    while True:
        ser.write(b"R\r\n")
        time.sleep(0.1)
        device_line = ser.read_until(b"\n").decode(errors="ignore").rstrip("\r\n")
        if device_line:
            print(device_line)
            break

def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} /dev/ttyXXX test.dat")
        sys.exit(1)

    port = sys.argv[1]
    infile = sys.argv[2]
    
    ser = serial.Serial(port, 9600, timeout=0.05, rtscts=False, dsrdtr=False)
    
    ser.reset_input_buffer()

    reset(ser)
    
    had_diff = False
    with open(infile, "r") as f:
        for line in f:
            line = line.strip()
            
            if not line:
                continue
            
            to_send = line + "\n"
            ser.write(to_send.encode("ascii"))
            ser.flush()
            time.sleep(0.1)

            # Read device response
            device_line = ser.read_until(b"\n").decode(errors="ignore").rstrip("\r\n")
            
            print(f"-> {line}")
            
            if not device_line:
                # No response? Just keep going or break if needed
                continue

            # The device response typically starts with 'O:' or 'X:'.
            prefix = ""
            if len(device_line) > 2 and device_line[1] == ":":
                prefix = device_line[:2]  # e.g. 'O:' or 'X:'
                rest = device_line[2:]
            else:
                rest = device_line

            # Highlight differences in the body portion against the original line
            (is_diff, highlighted) = highlight_diff(line, rest)
            
            if is_diff:
                had_diff = True

            # Print response line
            print(f"<- {prefix}{highlighted}")

    reset(ser)
    ser.close()
    
    if had_diff:
        sys.exit(1)

if __name__ == "__main__":
    main()
