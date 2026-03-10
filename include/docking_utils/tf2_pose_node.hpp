#ifndef DOCKING_UTILS__TF2_POSE_NODE_HPP_
#define DOCKING_UTILS__TF2_POSE_NODE_HPP_

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"

namespace docking_utils
{
class Tf2PoseNode : public rclcpp::Node
{
public:
  explicit Tf2PoseNode(const rclcpp::NodeOptions & options);

private:
  void timer_callback();

  // Parameters
  std::string target_frame_;
  std::string output_frame_;
  std::string output_pose_topic_;
  double timeout_s_;
  double frequency_hz_;

  // TF2
  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  
  // ROS Interfaces
  rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_pub_;
  rclcpp::TimerBase::SharedPtr timer_;
};
} // namespace docking_utils

#endif // DOCKING_UTILS__TF2_POSE_NODE_HPP_