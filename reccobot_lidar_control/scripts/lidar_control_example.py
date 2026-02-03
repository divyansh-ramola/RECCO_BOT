#!/usr/bin/env python3
"""
Example node demonstrating how to control the LiDAR position controller.

This node shows different control patterns:
1. Move to specific angles
2. Continuous scanning
3. Custom scan patterns
"""

import rclpy
from rclpy.node import Node
from std_msgs.msg import Float64
import math
import time


class LidarControlExample(Node):
    """Example node for controlling LiDAR rotation."""

    def __init__(self):
        super().__init__('lidar_control_example')
        
        # Publisher for sending position commands
        self.command_pub = self.create_publisher(
            Float64,
            '/lidar_position_controller/command',
            10
        )
        
        # Subscriber for monitoring current position
        self.state_sub = self.create_subscription(
            Float64,
            '/lidar_position_controller/state',
            self.state_callback,
            10
        )
        
        self.current_position = 0.0
        
        self.get_logger().info('LiDAR Control Example Node Started')

    def state_callback(self, msg):
        """Callback for monitoring current LiDAR position."""
        self.current_position = msg.data

    def move_to_angle(self, angle_rad):
        """
        Move LiDAR to specific angle.
        
        Args:
            angle_rad: Target angle in radians (0 to 2*pi)
        """
        msg = Float64()
        msg.data = float(angle_rad)
        self.command_pub.publish(msg)
        self.get_logger().info(f'Commanded angle: {angle_rad:.3f} rad ({math.degrees(angle_rad):.1f}°)')

    def move_to_degrees(self, angle_deg):
        """
        Move LiDAR to specific angle in degrees.
        
        Args:
            angle_deg: Target angle in degrees (0 to 360)
        """
        angle_rad = math.radians(angle_deg)
        self.move_to_angle(angle_rad)

    def scan_pattern_simple(self):
        """Perform a simple 4-point scan pattern."""
        self.get_logger().info('Starting simple scan pattern (0°, 90°, 180°, 270°)')
        
        angles = [0, 90, 180, 270]
        for angle in angles:
            self.move_to_degrees(angle)
            time.sleep(2.0)  # Wait for movement
        
        self.get_logger().info('Simple scan pattern complete')

    def scan_pattern_detailed(self):
        """Perform a detailed 8-point scan pattern."""
        self.get_logger().info('Starting detailed scan pattern')
        
        angles = [0, 45, 90, 135, 180, 225, 270, 315]
        for angle in angles:
            self.move_to_degrees(angle)
            time.sleep(1.5)  # Wait for movement
        
        self.get_logger().info('Detailed scan pattern complete')

    def scan_pattern_custom(self, angles_deg, dwell_time=2.0):
        """
        Perform custom scan pattern.
        
        Args:
            angles_deg: List of angles in degrees
            dwell_time: Time to wait at each angle in seconds
        """
        self.get_logger().info(f'Starting custom scan pattern: {angles_deg}')
        
        for angle in angles_deg:
            self.move_to_degrees(angle)
            time.sleep(dwell_time)
        
        self.get_logger().info('Custom scan pattern complete')

    def continuous_sweep(self, num_steps=36, sweep_range=(0, 360), reverse=False):
        """
        Perform continuous sweep with specified number of steps.
        
        Args:
            num_steps: Number of steps in sweep
            sweep_range: Tuple of (start_angle, end_angle) in degrees
            reverse: If True, sweep from end to start
        """
        start, end = sweep_range
        if reverse:
            start, end = end, start
        
        self.get_logger().info(
            f'Starting continuous sweep: {start}° to {end}° in {num_steps} steps'
        )
        
        angles = [start + (end - start) * i / (num_steps - 1) 
                  for i in range(num_steps)]
        
        for angle in angles:
            self.move_to_degrees(angle)
            time.sleep(0.1)  # Fast sweep
        
        self.get_logger().info('Continuous sweep complete')


def example_1_fixed_positions(node):
    """Example 1: Move to fixed positions."""
    print("\n=== Example 1: Fixed Positions ===")
    print("Moving to 0°, 90°, 180°, 270°")
    
    node.move_to_degrees(0)
    time.sleep(2)
    
    node.move_to_degrees(90)
    time.sleep(2)
    
    node.move_to_degrees(180)
    time.sleep(2)
    
    node.move_to_degrees(270)
    time.sleep(2)
    
    print("Example 1 complete\n")


def example_2_simple_scan(node):
    """Example 2: Simple scan pattern."""
    print("\n=== Example 2: Simple Scan Pattern ===")
    node.scan_pattern_simple()
    print("Example 2 complete\n")


def example_3_detailed_scan(node):
    """Example 3: Detailed scan pattern."""
    print("\n=== Example 3: Detailed Scan Pattern ===")
    node.scan_pattern_detailed()
    print("Example 3 complete\n")


def example_4_custom_scan(node):
    """Example 4: Custom scan pattern."""
    print("\n=== Example 4: Custom Scan Pattern ===")
    print("Scanning forward-facing angles only")
    
    # Scan only forward hemisphere (-90° to +90°)
    custom_angles = [-90, -60, -30, 0, 30, 60, 90]
    # Convert to 0-360 range
    custom_angles = [(angle + 360) % 360 for angle in custom_angles]
    
    node.scan_pattern_custom(custom_angles, dwell_time=1.5)
    print("Example 4 complete\n")


def example_5_continuous_sweep(node):
    """Example 5: Continuous sweep."""
    print("\n=== Example 5: Continuous Sweep ===")
    print("Fast 360° sweep")
    
    node.continuous_sweep(num_steps=72, sweep_range=(0, 360))
    
    print("\nReverse sweep")
    node.continuous_sweep(num_steps=72, sweep_range=(0, 360), reverse=True)
    
    print("Example 5 complete\n")


def main(args=None):
    rclpy.init(args=args)
    
    node = LidarControlExample()
    
    print("\n" + "="*60)
    print("LiDAR Position Controller - Example Usage")
    print("="*60)
    
    try:
        # Run examples (uncomment the ones you want to run)
        
        # Example 1: Basic fixed positions
        example_1_fixed_positions(node)
        
        # Example 2: Simple 4-point scan
        # example_2_simple_scan(node)
        
        # Example 3: Detailed 8-point scan
        # example_3_detailed_scan(node)
        
        # Example 4: Custom scan pattern
        # example_4_custom_scan(node)
        
        # Example 5: Continuous sweep
        # example_5_continuous_sweep(node)
        
        print("\nExamples complete! Node will continue running to monitor state.")
        print("Press Ctrl+C to exit.\n")
        
        # Keep node running to monitor state
        rclpy.spin(node)
        
    except KeyboardInterrupt:
        print("\nShutting down...")
    
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
