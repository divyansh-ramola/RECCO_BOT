#!/usr/bin/env python3
"""
Gymnasium Environment for RECCOBOT Quadruped Walking

This environment interfaces with the reccobot via ROS2 topics for
reinforcement learning training.

Usage:
    from reccobot_rl_env import ReccobotEnv
    env = ReccobotEnv()
    obs, info = env.reset()
    action = env.action_space.sample()
    obs, reward, done, truncated, info = env.step(action)
"""

import gymnasium as gym
import numpy as np
import rclpy
from rclpy.node import Node
from std_msgs.msg import Float64MultiArray
from sensor_msgs.msg import JointState
import time


class ReccobotEnv(gym.Env):
    """Gym environment for reccobot quadruped walking"""
    
    metadata = {'render.modes': ['human']}
    
    def __init__(self, max_steps=1000, target_velocity=0.5):
        super().__init__()
        
        # Initialize ROS2
        if not rclpy.ok():
            rclpy.init()
        self.node = Node('reccobot_rl_env')
        
        # Configuration
        self.max_steps = max_steps
        self.target_velocity = target_velocity
        self.current_step = 0
        
        # State tracking
        self.joint_states = None
        self.last_joint_positions = np.zeros(12)
        
        # Publishers
        self.leg_pub = self.node.create_publisher(
            Float64MultiArray, '/leg_controller/commands', 10)
        
        # Subscribers
        self.node.create_subscription(
            JointState, '/joint_states', self.joint_callback, 10)
        
        # Action space: 12 leg joints
        # Joint limits: [-1.396, 1.396] radians for most joints
        self.action_space = gym.spaces.Box(
            low=-1.396, 
            high=1.396, 
            shape=(12,), 
            dtype=np.float32
        )
        
        # Observation space: 
        # - 12 joint positions
        # - 12 joint velocities
        # - 3 base orientation (roll, pitch, yaw)
        self.observation_space = gym.spaces.Box(
            low=-np.inf, 
            high=np.inf, 
            shape=(27,), 
            dtype=np.float32
        )
        
        print("ReccobotEnv initialized!")
        print(f"Action space: {self.action_space.shape}")
        print(f"Observation space: {self.observation_space.shape}")
    
    def joint_callback(self, msg):
        """Callback for joint state updates"""
        self.joint_states = msg
    
    def step(self, action):
        """Execute one step in the environment"""
        self.current_step += 1
        
        # Clip action to valid range
        action = np.clip(action, self.action_space.low, self.action_space.high)
        
        # Send action to robot
        msg = Float64MultiArray()
        msg.data = action.tolist()
        self.leg_pub.publish(msg)
        
        # Wait for physics update (10 Hz control rate)
        time.sleep(0.1)
        rclpy.spin_once(self.node, timeout_sec=0.01)
        
        # Get observation
        obs = self._get_observation()
        
        # Calculate reward
        reward = self._calculate_reward(action)
        
        # Check if episode is done
        done = self._check_done()
        truncated = self.current_step >= self.max_steps
        
        info = {
            'step': self.current_step,
            'joint_positions': obs[:12] if obs is not None else None
        }
        
        return obs, reward, done, truncated, info
    
    def _get_observation(self):
        """Get current observation from robot state"""
        if self.joint_states is None:
            return np.zeros(27, dtype=np.float32)
        
        # Extract leg joint states (first 12 joints)
        positions = np.array(self.joint_states.position[:12], dtype=np.float32)
        velocities = np.array(self.joint_states.velocity[:12], dtype=np.float32) if len(self.joint_states.velocity) > 0 else np.zeros(12, dtype=np.float32)
        
        # TODO: Get base orientation from IMU or odometry
        # For now, use zeros (assumes robot is level)
        base_orientation = np.zeros(3, dtype=np.float32)
        
        # Concatenate all observations
        obs = np.concatenate([positions, velocities, base_orientation])
        
        return obs
    
    def _calculate_reward(self, action):
        """Calculate reward for current state and action"""
        if self.joint_states is None:
            return 0.0
        
        reward = 0.0
        
        # Get current joint positions
        current_positions = np.array(self.joint_states.position[:12])
        
        # 1. Forward motion reward
        # TODO: Get actual base velocity from odometry
        # For now, reward joint movement (proxy for walking)
        joint_movement = np.abs(current_positions - self.last_joint_positions).mean()
        forward_reward = min(joint_movement * 10.0, 1.0)  # Cap at 1.0
        reward += forward_reward
        
        # 2. Stability reward (penalize extreme joint angles)
        stability_penalty = -np.sum(np.abs(current_positions) > 1.2) * 0.1
        reward += stability_penalty
        
        # 3. Energy efficiency (penalize large actions)
        energy_penalty = -np.sum(np.abs(action)) * 0.01
        reward += energy_penalty
        
        # 4. Smooth motion (penalize large joint velocity changes)
        velocities = np.array(self.joint_states.velocity[:12]) if len(self.joint_states.velocity) > 0 else np.zeros(12)
        smoothness_penalty = -np.sum(np.abs(velocities)) * 0.01
        reward += smoothness_penalty
        
        # Update last positions
        self.last_joint_positions = current_positions.copy()
        
        return reward
    
    def _check_done(self):
        """Check if episode should terminate"""
        if self.joint_states is None:
            return False
        
        # Episode ends if robot falls or extreme joint angles
        positions = np.array(self.joint_states.position[:12])
        
        # Check for extreme joint angles (potential fall)
        if np.any(np.abs(positions) > 1.5):
            print("Episode ended: Extreme joint angle detected")
            return True
        
        # TODO: Add check for base height (if robot fell)
        # if base_height < 0.05:
        #     return True
        
        return False
    
    def reset(self, seed=None, options=None):
        """Reset the environment to initial state"""
        super().reset(seed=seed)
        
        self.current_step = 0
        
        # Reset to neutral standing position
        # Legs slightly bent for stability
        initial_pose = np.array([
            # Leg 1: coxa, femur, tibia
            0.0, -0.2, 0.4,
            # Leg 2
            0.0, -0.2, 0.4,
            # Leg 3
            0.0, -0.2, 0.4,
            # Leg 4
            0.0, -0.2, 0.4
        ], dtype=np.float32)
        
        # Send robot to initial position
        msg = Float64MultiArray()
        msg.data = initial_pose.tolist()
        self.leg_pub.publish(msg)
        
        # Wait for robot to stabilize
        time.sleep(0.5)
        rclpy.spin_once(self.node, timeout_sec=0.1)
        
        # Initialize last positions
        self.last_joint_positions = initial_pose.copy()
        
        obs = self._get_observation()
        info = {'step': 0}
        
        return obs, info
    
    def render(self, mode='human'):
        """Render the environment (gazebo handles visualization)"""
        pass
    
    def close(self):
        """Clean up resources"""
        self.node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


def test_environment():
    """Test the environment with random actions"""
    print("Testing ReccobotEnv...")
    
    env = ReccobotEnv(max_steps=100)
    
    print("Resetting environment...")
    obs, info = env.reset()
    print(f"Initial observation shape: {obs.shape}")
    print(f"Initial observation: {obs}")
    
    print("\nRunning 10 random steps...")
    for i in range(10):
        action = env.action_space.sample()
        obs, reward, done, truncated, info = env.step(action)
        print(f"Step {i+1}: reward={reward:.3f}, done={done}, truncated={truncated}")
        
        if done or truncated:
            print("Episode finished, resetting...")
            obs, info = env.reset()
    
    env.close()
    print("\nTest complete!")


if __name__ == '__main__':
    test_environment()
