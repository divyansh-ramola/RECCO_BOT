#include <memory>
#include <vector>
#include <cmath>
#include <unordered_set>
#include <sstream>
#include <limits>

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/point_cloud2_iterator.hpp"

#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.h"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/LinearMath/Matrix3x3.h"

using Point3 = std::array<float,3>;

static inline double normalizeAngle(double a) {
  // normalize to [-pi, pi)
  while (a >= M_PI) a -= 2.0 * M_PI;
  while (a < -M_PI) a += 2.0 * M_PI;
  return a;
}

class RotatingLidarAccumulator : public rclcpp::Node
{
public:
  RotatingLidarAccumulator()
  : Node("rotating_lidar_accumulator"),
    tf_buffer_(this->get_clock()),
    tf_listener_(tf_buffer_),
    cumulative_rotation_(0.0),
    last_angle_valid_(false)
  {
    // parameters
    this->declare_parameter<std::string>("lidar_frame", "lidarl_sensor");
    this->declare_parameter<std::string>("fixed_frame", "base_link");
    this->declare_parameter<std::string>("publish_topic", "/lidar_3d");
    this->declare_parameter<double>("voxel_size", 0.0); // 0 -> no downsample
    this->declare_parameter<int>("min_points_to_publish", 50);
    this->declare_parameter<int>("max_points_buffer", 5'000'000);

    this->get_parameter("lidar_frame", lidar_frame_);
    this->get_parameter("fixed_frame", fixed_frame_);
    this->get_parameter("publish_topic", publish_topic_);
    this->get_parameter("voxel_size", voxel_size_);
    this->get_parameter("min_points_to_publish", min_points_to_publish_);
    this->get_parameter("max_points_buffer", max_points_buffer_);

    // subscription & publisher
    scan_sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
      "/scan", rclcpp::SensorDataQoS(),
      std::bind(&RotatingLidarAccumulator::scanCallback, this, std::placeholders::_1)
    );

    pc_pub_ = this->create_publisher<sensor_msgs::msg::PointCloud2>(publish_topic_, rclcpp::QoS(10));

    RCLCPP_INFO(this->get_logger(), "RotatingLidarAccumulator started (C++)");
    RCLCPP_INFO(this->get_logger(), " lidar_frame: %s, fixed_frame: %s, publish: %s",
                lidar_frame_.c_str(), fixed_frame_.c_str(), publish_topic_.c_str());

    if (voxel_size_ > 0.0) {
      RCLCPP_INFO(this->get_logger(), " voxel_size enabled: %.4f m", voxel_size_);
    }
  }

private:
  void scanCallback(const sensor_msgs::msg::LaserScan::SharedPtr scan_msg)
  {
    // Lookup the transform from lidar frame to fixed frame (at latest available time)
    geometry_msgs::msg::TransformStamped tf;
    try {
      tf = tf_buffer_.lookupTransform(
        fixed_frame_, lidar_frame_,
        tf2::TimePointZero
      );
    } catch (const tf2::TransformException &ex) {
      RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 2000,
                           "TF lookup failed: %s", ex.what());
      return;
    }

    // compute current yaw (rotation about Z) of lidar frame in fixed_frame
    tf2::Quaternion q(
      tf.transform.rotation.x,
      tf.transform.rotation.y,
      tf.transform.rotation.z,
      tf.transform.rotation.w
    );
    double roll, pitch, yaw;
    tf2::Matrix3x3(q).getRPY(roll, pitch, yaw);
    double current_angle = yaw; // radians in [-pi, pi)

    if (!last_angle_valid_) {
      last_angle_ = current_angle;
      last_angle_valid_ = true;
      cumulative_rotation_ = 0.0;
      // we still want to collect the first scan
    } else {
      double delta = normalizeAngle(current_angle - last_angle_);
      cumulative_rotation_ += std::fabs(delta);
      last_angle_ = current_angle;
    }

    // convert scan into points in lidar frame, then transform to fixed/world frame
    const size_t N = scan_msg->ranges.size();
    double angle = scan_msg->angle_min;

    // Prepare a transform matrix once per callback
    double tx = tf.transform.translation.x;
    double ty = tf.transform.translation.y;
    double tz = tf.transform.translation.z;
    // rotation matrix from quaternion
    tf2::Matrix3x3 R(q);

    for (size_t i = 0; i < N; ++i, angle += scan_msg->angle_increment) {
      float r = scan_msg->ranges[i];
      if (std::isnan(r) || r < scan_msg->range_min || r > scan_msg->range_max) {
        continue;
      }

      // point in lidar frame
      float lx = r * std::cos(angle);
      float ly = r * std::sin(angle);
      float lz = 0.0f;

      // rotate using R and translate
      tf2::Vector3 lv(lx, ly, lz);
      tf2::Vector3 wv = R * lv + tf2::Vector3(tx, ty, tz);

      // append
      if ((int)accumulated_points_.size() < max_points_buffer_) {
        accumulated_points_.push_back({static_cast<float>(wv.x()),
                                       static_cast<float>(wv.y()),
                                       static_cast<float>(wv.z())});
      } else {
        // buffer full: warn and drop incoming points to avoid OOM
        if (((int)drop_warn_count_) % 50 == 0) {
          RCLCPP_WARN(this->get_logger(), "accumulated buffer reached max (%d). Dropping points.",
                      max_points_buffer_);
        }
        drop_warn_count_++;
      }
    }

    // If we've integrated enough rotation (close to 360 deg), publish the accumulated cloud
    const double full_rotation_threshold = 2.0 * M_PI * 0.9; // 90% of full rotation
    if (cumulative_rotation_ >= full_rotation_threshold) {
      publishAccumulatedCloud(scan_msg->header);
      // reset for next sweep
      accumulated_points_.clear();
      cumulative_rotation_ = 0.0;
      last_angle_valid_ = true; // keep last_angle as current
    }
  }

  void publishAccumulatedCloud(const std_msgs::msg::Header &in_header)
  {
    if ((int)accumulated_points_.size() < min_points_to_publish_) {
      RCLCPP_WARN(this->get_logger(), "Not enough points (%zu) to publish (min %d)",
                  accumulated_points_.size(), min_points_to_publish_);
      return;
    }

    std::vector<Point3> output_points;
    output_points.reserve(accumulated_points_.size());

    if (voxel_size_ > 0.0) {
      // simple voxel grid downsample (hash map using integer keys)
      struct Key {
        int64_t x, y, z;
        bool operator==(Key const &o) const { return x==o.x && y==o.y && z==o.z; }
      };
      struct KeyHash {
        std::size_t operator()(Key const &k) const noexcept {
          // combine
          uint64_t a = (uint64_t)(k.x) * 73856093u;
          uint64_t b = (uint64_t)(k.y) * 19349663u;
          uint64_t c = (uint64_t)(k.z) * 83492791u;
          return (std::size_t)(a ^ b ^ c);
        }
      };

      std::unordered_set<Key, KeyHash> seen;
      double inv_vs = 1.0 / voxel_size_;
      for (const auto &p : accumulated_points_) {
        int64_t ix = (int64_t)std::floor(p[0] * inv_vs);
        int64_t iy = (int64_t)std::floor(p[1] * inv_vs);
        int64_t iz = (int64_t)std::floor(p[2] * inv_vs);
        Key k{ix, iy, iz};
        if (seen.find(k) == seen.end()) {
          seen.insert(k);
          output_points.push_back(p);
        }
      }
    } else {
      output_points = accumulated_points_;
    }

    // Build PointCloud2
    sensor_msgs::msg::PointCloud2 cloud_msg;
    cloud_msg.header = in_header;
    cloud_msg.header.frame_id = fixed_frame_;
    cloud_msg.height = 1;
    cloud_msg.width = static_cast<uint32_t>(output_points.size());

    sensor_msgs::PointCloud2Modifier modifier(cloud_msg);
    modifier.setPointCloud2FieldsByString(1, "xyz");
    modifier.resize(output_points.size());

    sensor_msgs::PointCloud2Iterator<float> iter_x(cloud_msg, "x");
    sensor_msgs::PointCloud2Iterator<float> iter_y(cloud_msg, "y");
    sensor_msgs::PointCloud2Iterator<float> iter_z(cloud_msg, "z");

    for (const auto &p : output_points) {
      *iter_x = p[0];
      *iter_y = p[1];
      *iter_z = p[2];
      ++iter_x; ++iter_y; ++iter_z;
    }

    pc_pub_->publish(cloud_msg);
    RCLCPP_INFO(this->get_logger(), "Published accumulated cloud with %zu points (after voxel %zu)",
                output_points.size(), output_points.size());
  }

  // ROS
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pc_pub_;

  // TF
  tf2_ros::Buffer tf_buffer_;
  tf2_ros::TransformListener tf_listener_;

  // parameters
  std::string lidar_frame_;
  std::string fixed_frame_;
  std::string publish_topic_;
  double voxel_size_;
  int min_points_to_publish_;
  int max_points_buffer_;

  // accumulation state
  std::vector<Point3> accumulated_points_;
  double cumulative_rotation_;
  double last_angle_;
  bool last_angle_valid_;
  int drop_warn_count_ = 0;
};

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<RotatingLidarAccumulator>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
