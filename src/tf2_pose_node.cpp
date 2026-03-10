#include "docking_utils/tf2_pose_node.hpp"
#include <chrono>

using namespace std::chrono_literals;

namespace docking_utils
{
Tf2PoseNode::Tf2PoseNode(const rclcpp::NodeOptions & options)
: Node("tf2_pose_node", options)
{
  // Declare Parameters
  target_frame_ = this->declare_parameter<std::string>("target_frame", "base_link");
  output_frame_ = this->declare_parameter<std::string>("output_frame", "map");
  output_pose_topic_ = this->declare_parameter<std::string>("output_pose_topic", "output_pose");
  timeout_s_ = this->declare_parameter<double>("timeout", 0.1);
  frequency_hz_ = this->declare_parameter<double>("frequency", 10.0);

  // Init TF2
  tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
  tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

  // Init Publisher
  pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>(output_pose_topic_, 10);

  // Init Timer
  auto interval = std::chrono::duration<double>(1.0 / frequency_hz_);
  timer_ = this->create_wall_timer(interval, std::bind(&Tf2PoseNode::timer_callback, this));
}

void Tf2PoseNode::timer_callback()
{
  try {
    // Note: Use lookupTransform (camelCase) in C++
    auto transform = tf_buffer_->lookupTransform(
      output_frame_, 
      target_frame_, 
      tf2::TimePointZero, 
      tf2::durationFromSec(timeout_s_)); // Note: durationFromSec (camelCase)

    geometry_msgs::msg::PoseStamped pose_msg;
    pose_msg.header = transform.header;
    pose_msg.pose.position.x = transform.transform.translation.x;
    pose_msg.pose.position.y = transform.transform.translation.y;
    pose_msg.pose.position.z = transform.transform.translation.z;
    pose_msg.pose.orientation = transform.transform.rotation;

    pose_pub_->publish(pose_msg);
  }
  catch (const tf2::TransformException & ex) {
    RCLCPP_WARN_THROTTLE(this->get_logger(), *this->get_clock(), 5000, 
                         "TF Lookup Failed: %s", ex.what());
  }
}
} // namespace docking_utils

#include "rclcpp_components/register_node_macro.hpp"
RCLCPP_COMPONENTS_REGISTER_NODE(docking_utils::Tf2PoseNode)