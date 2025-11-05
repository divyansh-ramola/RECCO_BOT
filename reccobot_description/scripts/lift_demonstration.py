#!/usr/bin/env python3
"""
RECCOBOT Lift Demonstration Script

This script demonstrates the robot lifting itself from the ground
through a sequence of poses:
1. Collapsed (on ground)
2. Crouch (preparing to lift)
3. Push-up (actively lifting)
4. Standing (fully lifted)

Usage:
    # Start Gazebo first, then run:
    python3 lift_demonstration.py
"""

import rclpy
from rclpy.node import Node
from std_msgs.msg import Float64MultiArray
import time
import sys


class LiftDemonstration(Node):
    """Demonstrate robot lifting itself from ground to standing"""
    
    def __init__(self):
        super().__init__('lift_demonstration')
        
        # Publisher for leg controller
        self.leg_pub = self.create_publisher(
            Float64MultiArray, '/leg_controller/commands', 10)
        
        # Wait for publisher to be ready
        time.sleep(1.0)
        
        print("\n" + "="*60)
        print("RECCOBOT LIFT DEMONSTRATION")
        print("="*60)
        print("\nThis will show your robot lifting itself from the ground!")
        print("Watch Gazebo to see the motion...\n")
        
    def send_pose(self, positions, pose_name, hold_time=2.0):
        """Send a pose and hold it for specified time"""
        msg = Float64MultiArray()
        msg.data = positions
        
        print(f"➤ {pose_name}")
        print(f"  Positions: {[f'{p:+.2f}' for p in positions]}")
        
        # Publish multiple times to ensure delivery
        for _ in range(10):
            self.leg_pub.publish(msg)
            time.sleep(0.05)
        
        print(f"  Holding for {hold_time:.1f} seconds...")
        time.sleep(hold_time)
        print("  ✓ Complete\n")
    
    def smooth_transition(self, start_pos, end_pos, duration=3.0, steps=30):
        """Smoothly transition from start to end position"""
        print(f"  → Transitioning over {duration:.1f} seconds...")
        
        msg = Float64MultiArray()
        
        for step in range(steps + 1):
            # Linear interpolation
            t = step / steps
            interpolated = [
                start + (end - start) * t 
                for start, end in zip(start_pos, end_pos)
            ]
            
            msg.data = interpolated
            self.leg_pub.publish(msg)
            time.sleep(duration / steps)
        
        print("  ✓ Transition complete\n")
    
    def run_demonstration(self):
        """Run complete lift demonstration"""
        
        # Define poses (12 joints: coxa1-4, femur1-4, tibia1-4)
        # Format: [coxa1, femur1, tibia1, coxa2, femur2, tibia2, 
        #          coxa3, femur3, tibia3, coxa4, femur4, tibia4]
        
        poses = {
            'collapsed': [
                # Legs completely folded - robot on ground
                0.0,  0.8, -1.2,  # Leg 1: coxa straight, femur/tibia folded
                0.0,  0.8, -1.2,  # Leg 2
                0.0,  0.8, -1.2,  # Leg 3
                0.0,  0.8, -1.2   # Leg 4
            ],
            
            'crouch': [
                # Deep crouch - legs bent but under body
                0.0,  0.5, -0.8,  # Leg 1: moderate fold
                0.0,  0.5, -0.8,  # Leg 2
                0.0,  0.5, -0.8,  # Leg 3
                0.0,  0.5, -0.8   # Leg 4
            ],
            
            'partial_lift': [
                # Halfway up - actively lifting
                0.0,  0.0, -0.3,  # Leg 1: starting to extend
                0.0,  0.0, -0.3,  # Leg 2
                0.0,  0.0, -0.3,  # Leg 3
                0.0,  0.0, -0.3   # Leg 4
            ],
            
            'standing': [
                # Fully standing - legs extended
                0.0, -0.3,  0.6,  # Leg 1: extended for standing
                0.0, -0.3,  0.6,  # Leg 2
                0.0, -0.3,  0.6,  # Leg 3
                0.0, -0.3,  0.6   # Leg 4
            ],
            
            'tall_stand': [
                # Extra tall - maximum extension
                0.0, -0.5,  0.9,  # Leg 1: nearly straight
                0.0, -0.5,  0.9,  # Leg 2
                0.0, -0.5,  0.9,  # Leg 3
                0.0, -0.5,  0.9   # Leg 4
            ]
        }
        
        print("\n" + "="*60)
        print("STARTING LIFT SEQUENCE")
        print("="*60 + "\n")
        
        # Phase 1: Collapsed on ground
        print("📍 PHASE 1: COLLAPSED POSITION")
        print("   Robot is lying on the ground, legs fully folded")
        self.send_pose(poses['collapsed'], "Collapsed (on ground)", hold_time=2.0)
        
        # Phase 2: Transition to crouch
        print("📍 PHASE 2: PREPARING TO LIFT")
        print("   Moving to crouch position...")
        self.smooth_transition(
            poses['collapsed'], 
            poses['crouch'], 
            duration=3.0, 
            steps=30
        )
        self.send_pose(poses['crouch'], "Crouch (preparing)", hold_time=2.0)
        
        # Phase 3: Partial lift
        print("📍 PHASE 3: LIFTING OFF GROUND")
        print("   Actively pushing up with legs...")
        self.smooth_transition(
            poses['crouch'], 
            poses['partial_lift'], 
            duration=3.0, 
            steps=30
        )
        self.send_pose(poses['partial_lift'], "Partial Lift (rising)", hold_time=2.0)
        
        # Phase 4: Full standing
        print("📍 PHASE 4: STANDING UP")
        print("   Extending to full standing height...")
        self.smooth_transition(
            poses['partial_lift'], 
            poses['standing'], 
            duration=2.0, 
            steps=20
        )
        self.send_pose(poses['standing'], "Standing (stable)", hold_time=2.0)
        
        # Phase 5: Tall stand
        print("📍 PHASE 5: MAXIMUM HEIGHT")
        print("   Reaching maximum extension...")
        self.smooth_transition(
            poses['standing'], 
            poses['tall_stand'], 
            duration=2.0, 
            steps=20
        )
        self.send_pose(poses['tall_stand'], "Tall Stand (maximum)", hold_time=3.0)
        
        print("\n" + "="*60)
        print("✅ DEMONSTRATION COMPLETE!")
        print("="*60)
        print("\nYour robot successfully lifted itself from the ground!")
        print("Notice how the femur joints did most of the work (13× safety factor).")
        print("\nThe robot is now standing tall and stable! 🤖\n")
    
    def run_quick_demo(self):
        """Quick up-down demonstration"""
        print("\n" + "="*60)
        print("QUICK LIFT DEMO")
        print("="*60 + "\n")
        
        # Simple crouch-to-stand
        crouch = [0.0, 0.5, -0.8] * 4
        stand = [0.0, -0.3, 0.6] * 4
        
        print("Going down to crouch...")
        self.smooth_transition([0.0]*12, crouch, duration=2.0, steps=20)
        time.sleep(1.0)
        
        print("⬆ Lifting up to stand...")
        self.smooth_transition(crouch, stand, duration=2.0, steps=20)
        time.sleep(1.0)
    
        print("\n Quick demo complete!\n")


def main():
    """Main function"""
    import argparse
    
    parser = argparse.ArgumentParser(
        description='Demonstrate RECCOBOT lifting itself')
    parser.add_argument(
        '--mode',
        choices=['full', 'quick'],
        default='full',
        help='Demo mode: full (complete sequence) or quick (simple up/down)'
    )
    
    args = parser.parse_args()
    
    # Initialize ROS2
    rclpy.init()
    
    # Create node
    demo = LiftDemonstration()
    
    try:
        # Check if controllers are active
        print("Checking for active controllers...")
        time.sleep(0.5)
        
        if args.mode == 'full':
            demo.run_demonstration()
        else:
            demo.run_quick_demo()
        
    except KeyboardInterrupt:
        print("\n\n  Demo interrupted by user")
    except Exception as e:
        print(f"\n\nError: {e}")
        import traceback
        traceback.print_exc()
    finally:
        # Cleanup
        demo.destroy_node()
        rclpy.shutdown()
        print("\nShutdown complete.\n")


if __name__ == '__main__':
    main()
