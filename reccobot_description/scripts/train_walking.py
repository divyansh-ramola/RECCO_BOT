#!/usr/bin/env python3
"""
Training script for RECCOBOT quadruped walking using PPO

Requirements:
    pip3 install stable-baselines3 gymnasium torch

Usage:
    # Start Gazebo in another terminal first:
    # ros2 launch reccobot_description gazebo.launch.py
    
    # Then run this script:
    python3 train_walking.py
"""

import os
import sys

# Add parent directory to path
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from reccobot_rl_env import ReccobotEnv
from stable_baselines3 import PPO
from stable_baselines3.common.callbacks import CheckpointCallback, EvalCallback
from stable_baselines3.common.monitor import Monitor
import numpy as np


def train_walking_behavior(
    total_timesteps=1_000_000,
    save_freq=10_000,
    model_dir="models/",
    log_dir="logs/"
):
    """
    Train the quadruped walking behavior using PPO
    
    Args:
        total_timesteps: Total training timesteps
        save_freq: Save model every N steps
        model_dir: Directory to save models
        log_dir: Directory for tensorboard logs
    """
    
    # Create directories
    os.makedirs(model_dir, exist_ok=True)
    os.makedirs(log_dir, exist_ok=True)
    
    print("=" * 60)
    print("RECCOBOT Walking Training")
    print("=" * 60)
    print(f"Total timesteps: {total_timesteps:,}")
    print(f"Model directory: {model_dir}")
    print(f"Log directory: {log_dir}")
    print()
    
    # Create environment
    print("Creating environment...")
    env = ReccobotEnv(max_steps=1000, target_velocity=0.5)
    env = Monitor(env, log_dir)
    
    # Create evaluation environment
    print("Creating evaluation environment...")
    eval_env = ReccobotEnv(max_steps=1000, target_velocity=0.5)
    eval_env = Monitor(eval_env, log_dir + "/eval")
    
    # Create callbacks
    checkpoint_callback = CheckpointCallback(
        save_freq=save_freq,
        save_path=model_dir,
        name_prefix="reccobot_walking"
    )
    
    eval_callback = EvalCallback(
        eval_env,
        best_model_save_path=model_dir + "/best",
        log_path=log_dir + "/eval",
        eval_freq=10_000,
        deterministic=True,
        render=False
    )
    
    # Create PPO model
    print("\nCreating PPO model...")
    model = PPO(
        "MlpPolicy",
        env,
        learning_rate=3e-4,
        n_steps=2048,
        batch_size=64,
        n_epochs=10,
        gamma=0.99,
        gae_lambda=0.95,
        clip_range=0.2,
        ent_coef=0.01,
        verbose=1,
        tensorboard_log=log_dir
    )
    
    print("\nModel architecture:")
    print(model.policy)
    print()
    
    # Train
    print("Starting training...")
    print("Monitor progress with: tensorboard --logdir " + log_dir)
    print()
    
    try:
        model.learn(
            total_timesteps=total_timesteps,
            callback=[checkpoint_callback, eval_callback],
            progress_bar=True
        )
    except KeyboardInterrupt:
        print("\n\nTraining interrupted by user!")
    
    # Save final model
    final_model_path = os.path.join(model_dir, "reccobot_walking_final")
    model.save(final_model_path)
    print(f"\nFinal model saved to: {final_model_path}")
    
    # Close environments
    env.close()
    eval_env.close()
    
    return model


def test_trained_model(model_path, num_episodes=5):
    """
    Test a trained model
    
    Args:
        model_path: Path to saved model
        num_episodes: Number of test episodes
    """
    print("=" * 60)
    print("Testing Trained Model")
    print("=" * 60)
    print(f"Model: {model_path}")
    print(f"Episodes: {num_episodes}")
    print()
    
    # Load model
    print("Loading model...")
    model = PPO.load(model_path)
    
    # Create environment
    env = ReccobotEnv(max_steps=1000, target_velocity=0.5)
    
    # Test episodes
    episode_rewards = []
    
    for episode in range(num_episodes):
        print(f"\nEpisode {episode + 1}/{num_episodes}")
        obs, info = env.reset()
        episode_reward = 0
        step = 0
        
        done = False
        truncated = False
        
        while not (done or truncated):
            # Get action from model
            action, _states = model.predict(obs, deterministic=True)
            
            # Execute action
            obs, reward, done, truncated, info = env.step(action)
            episode_reward += reward
            step += 1
            
            if step % 10 == 0:
                print(f"  Step {step}: reward={reward:.3f}, total={episode_reward:.3f}")
        
        episode_rewards.append(episode_reward)
        print(f"Episode {episode + 1} finished: Total reward = {episode_reward:.3f}")
    
    # Print statistics
    print("\n" + "=" * 60)
    print("Test Results")
    print("=" * 60)
    print(f"Episodes: {num_episodes}")
    print(f"Mean reward: {np.mean(episode_rewards):.3f}")
    print(f"Std reward: {np.std(episode_rewards):.3f}")
    print(f"Min reward: {np.min(episode_rewards):.3f}")
    print(f"Max reward: {np.max(episode_rewards):.3f}")
    
    env.close()


def main():
    """Main function"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Train or test RECCOBOT walking")
    parser.add_argument(
        "--mode",
        type=str,
        choices=["train", "test"],
        default="train",
        help="Mode: train or test"
    )
    parser.add_argument(
        "--model",
        type=str,
        default="models/reccobot_walking_final.zip",
        help="Path to model (for testing)"
    )
    parser.add_argument(
        "--timesteps",
        type=int,
        default=1_000_000,
        help="Total training timesteps"
    )
    parser.add_argument(
        "--episodes",
        type=int,
        default=5,
        help="Number of test episodes"
    )
    
    args = parser.parse_args()
    
    if args.mode == "train":
        train_walking_behavior(total_timesteps=args.timesteps)
    else:
        if not os.path.exists(args.model):
            print(f"Error: Model not found at {args.model}")
            print("Please train a model first with --mode train")
            sys.exit(1)
        test_trained_model(args.model, num_episodes=args.episodes)


if __name__ == '__main__':
    main()
