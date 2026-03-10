#include <memory>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "rclcpp_components/register_node_macro.hpp" // Required for Composition
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

namespace docking_utils {

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class LifecycleTagBridge : public rclcpp_lifecycle::LifecycleNode {
public:
  // Composable nodes MUST take NodeOptions in the constructor
  explicit LifecycleTagBridge(const rclcpp::NodeOptions & options)
  : rclcpp_lifecycle::LifecycleNode("lifecycle_tag_bridge", options) {
    this->declare_parameter("target_tag_frame", "tag36h11:0");
    this->declare_parameter("output_frame", "odom");
  }

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) override {
    RCLCPP_INFO(get_logger(), "Configuring Component...");
    
    target_frame_ = this->get_parameter("target_tag_frame").as_string();
    output_frame_ = this->get_parameter("output_frame").as_string();

    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    // Lifecycle publisher
    pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("detected_dock_pose", 10);

    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State &) override {
    RCLCPP_INFO(get_logger(), "Activating Component.");
    pose_pub_->on_activate();
    
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(100), 
      std::bind(&LifecycleTagBridge::publish_tag_pose, this));

    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override {
    RCLCPP_INFO(get_logger(), "Deactivating Component.");
    timer_.reset();
    pose_pub_->on_deactivate();
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) override {
    RCLCPP_INFO(get_logger(), "Cleaning up.");
    tf_listener_.reset();
    tf_buffer_.reset();
    pose_pub_.reset();
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) override {
    return CallbackReturn::SUCCESS;
  }

private:
  void publish_tag_pose() {
    try {
      auto transformStamped = tf_buffer_->lookupTransform(
        output_frame_, target_frame_, tf2::TimePointZero);

      geometry_msgs::msg::PoseStamped dock_pose;
      dock_pose.header.stamp = this->now();
      dock_pose.header.frame_id = output_frame_;
      dock_pose.pose.position.x = transformStamped.transform.translation.x;
      dock_pose.pose.position.y = transformStamped.transform.translation.y;
      dock_pose.pose.position.z = transformStamped.transform.translation.z;
      dock_pose.pose.orientation = transformStamped.transform.rotation;

      pose_pub_->publish(dock_pose);
    } catch (const tf2::TransformException & ex) {
      return; // Tag likely not in view; skip silently
    }
  }

  std::string target_frame_;
  std::string output_frame_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<geometry_msgs::msg::PoseStamped>> pose_pub_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
};

} // namespace docking_utils

// Register as a component
RCLCPP_COMPONENTS_REGISTER_NODE(docking_utils::LifecycleTagBridge)