#!/usr/bin/env python3
"""
Simple script to test Arduino communication before launching ROS 2 control.
This helps verify the hardware is working correctly.
"""

import serial
import time
import sys

def test_arduino(port='/dev/ttyUSB0', baud=115200, timeout=5):
    """Test Arduino serial communication."""
    
    print(f"\n{'='*60}")
    print("Arduino LiDAR Communication Test")
    print(f"{'='*60}\n")
    
    print(f"Attempting to connect to Arduino...")
    print(f"  Port: {port}")
    print(f"  Baud: {baud}")
    print(f"  Timeout: {timeout}s\n")
    
    try:
        # Open serial connection
        ser = serial.Serial(port, baud, timeout=1)
        time.sleep(2)  # Wait for Arduino to reset
        
        print("✓ Serial port opened successfully!\n")
        
        # Clear any initial data
        ser.reset_input_buffer()
        
        # Read initial messages
        print("Reading initial Arduino output...")
        start_time = time.time()
        initial_lines = []
        while time.time() - start_time < 3:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    initial_lines.append(line)
                    print(f"  Arduino: {line}")
        
        if not initial_lines:
            print("  ⚠ No output received from Arduino")
            print("  Check if firmware is uploaded and running\n")
        else:
            print(f"\n✓ Received {len(initial_lines)} lines from Arduino\n")
        
        # Test sending command
        print("Testing command interface...")
        test_angles = [0.0, 1.5708, 3.14159, 4.71239]  # 0°, 90°, 180°, 270°
        
        for angle in test_angles:
            cmd = f"cmd:{angle:.6f}\n"
            print(f"\n  Sending: {cmd.strip()}")
            ser.write(cmd.encode())
            time.sleep(0.1)
            
            # Read response
            response_received = False
            start_time = time.time()
            while time.time() - start_time < 1.0:
                if ser.in_waiting:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        print(f"  Response: {line}")
                        if "OK" in line or "angle:" in line:
                            response_received = True
            
            if not response_received:
                print("  ⚠ No response received")
            
            time.sleep(0.5)
        
        # Monitor state feedback for a few seconds
        print("\n" + "="*60)
        print("Monitoring state feedback for 5 seconds...")
        print("(You should see 'angle:' messages every ~50ms)")
        print("="*60 + "\n")
        
        start_time = time.time()
        angle_count = 0
        while time.time() - start_time < 5:
            if ser.in_waiting:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    print(f"  {line}")
                    if "angle:" in line:
                        angle_count += 1
        
        print(f"\n✓ Received {angle_count} angle updates in 5 seconds")
        print(f"  Expected: ~100 (at 50ms intervals)")
        
        if angle_count > 50:
            print("\n" + "="*60)
            print("✅ ARDUINO TEST PASSED!")
            print("="*60)
            print("\nYour Arduino is working correctly!")
            print("You can now launch the ROS 2 control system:\n")
            print("  ros2 launch reccobot_lidar_control lidar_control_standalone.launch.py")
            print(f"  ros2 launch reccobot_lidar_control lidar_control_standalone.launch.py serial_port:={port}\n")
            return True
        else:
            print("\n" + "="*60)
            print("⚠ ARDUINO TEST INCOMPLETE")
            print("="*60)
            print(f"\nReceived only {angle_count} updates (expected ~100)")
            print("Check:")
            print("  - Firmware uploaded correctly")
            print("  - Serial monitor not open in Arduino IDE")
            print("  - Correct port selected\n")
            return False
        
    except serial.SerialException as e:
        print(f"\n❌ ERROR: Could not open serial port")
        print(f"  {e}\n")
        print("Troubleshooting:")
        print("  1. Check if Arduino is connected:")
        print("     ls -l /dev/ttyUSB* /dev/ttyACM*")
        print("  2. Check permissions:")
        print("     sudo usermod -a -G dialout $USER")
        print("     (then logout and login)")
        print("  3. Try a different port:")
        print(f"     python3 {sys.argv[0]} /dev/ttyACM0")
        print("")
        return False
    
    except KeyboardInterrupt:
        print("\n\nTest interrupted by user")
        return False
    
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("\nSerial port closed")


if __name__ == '__main__':
    port = '/dev/ttyUSB0'
    
    if len(sys.argv) > 1:
        port = sys.argv[1]
    
    success = test_arduino(port)
    sys.exit(0 if success else 1)
