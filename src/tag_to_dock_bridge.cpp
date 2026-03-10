// #include <memory>
// #include <string>

// #include "rclcpp/rclcpp.hpp"
// #include "geometry_msgs/msg/pose_stamped.hpp"
// #include "std_msgs/msg/bool.hpp"
// #include "tf2_ros/transform_listener.h"
// #include "tf2_ros/buffer.h"
// #include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

// using std::placeholders::_1;

// class TagToDockBridge : public rclcpp::Node {
// public:
//   TagToDockBridge() : Node("tag_to_dock_bridge"), is_active_(false) {
//     // Declare and get parameters
//     this->declare_parameter("target_tag_frame", "tag36h11:0");
//     this->declare_parameter("output_frame", "odom");
//     this->declare_parameter("dock_pose_topic", "/detected_dock_pose");

//     target_frame_ = this->get_parameter("target_tag_frame").as_string();
//     output_frame_ = this->get_parameter("output_frame").as_string();
//     std::string topic = this->get_parameter("dock_pose_topic").as_string();

//     // TF Setup
//     tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
//     tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

//     // Communication
//     pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>(topic, 10);
    
//     enable_sub_ = this->create_subscription<std_msgs::msg::Bool>(
//       "/enable_tag_docking", 10, std::bind(&TagToDockBridge::enable_callback, this, _1));

//     // Timer: Runs at 10Hz
//     timer_ = this->create_wall_timer(
//       std::chrono::milliseconds(100), std::bind(&TagToDockBridge::timer_callback, this));

//     RCLCPP_INFO(this->get_logger(), "C++ Tag Bridge Ready. Monitoring: %s", target_frame_.c_str());
//   }

// private:
//   void enable_callback(const std_msgs::msg::Bool::SharedPtr msg) {
//     if (msg->data != is_active_) {
//       is_active_ = msg->data;
//       RCLCPP_INFO(this->get_logger(), "Tag Bridge: %s", is_active_ ? "ACTIVE" : "INACTIVE");
//     }
//   }

//   void timer_callback() {
//     if (!is_active_) {
//       return;
//     }

//     try {
//       // Look up transform from tag to odom
//       geometry_msgs::msg::TransformStamped transformStamped;
//       transformStamped = tf_buffer_->lookupTransform(
//         output_frame_, target_frame_, tf2::TimePointZero);

//       // Create and publish PoseStamped
//       geometry_msgs::msg::PoseStamped dock_pose;
//       dock_pose.header.stamp = this->now();
//       dock_pose.header.frame_id = output_frame_;
      
//       dock_pose.pose.position.x = transformStamped.transform.translation.x;
//       dock_pose.pose.position.y = transformStamped.transform.translation.y;
//       dock_pose.pose.position.z = transformStamped.transform.translation.z;
//       dock_pose.pose.orientation = transformStamped.transform.rotation;

//       pose_pub_->publish(dock_pose);
//     }
//     catch (const tf2::TransformException & ex) {
//       // Silently skip if tag is not visible
//       return;
//     }
//   }

//   // Members
//   bool is_active_;
//   std::string target_frame_;
//   std::string output_frame_;
  
//   rclcpp::TimerBase::SharedPtr timer_;
//   rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pose_pub_;
//   rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr enable_sub_;
  
//   std::shared_ptr<tf2_ros::TransformListener> tf_listener_{nullptr};
//   std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
// };

// int main(int argc, char * argv[]) {
//   rclcpp::init(argc, argv);
//   rclcpp::spin(std::make_shared<TagToDockBridge>());
//   rclcpp::shutdown();
//   return 0;
// }

#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class LifecycleTagBridge : public rclcpp_lifecycle::LifecycleNode {
public:
  LifecycleTagBridge() : rclcpp_lifecycle::LifecycleNode("lifecycle_tag_bridge") {
    this->declare_parameter("target_tag_frame", "tag36h11:0");
    this->declare_parameter("output_frame", "odom");
    this->declare_parameter<std::vector<double>>("tag_to_dockpose_transform", {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0});
  }

  // --- Lifecycle Transitions ---

  CallbackReturn on_configure(const rclcpp_lifecycle::State &) {
    RCLCPP_INFO(get_logger(), "Configuring: Setting up TF and Publishers...");
    
    target_frame_ = this->get_parameter("target_tag_frame").as_string();
    output_frame_ = this->get_parameter("output_frame").as_string();
    this->get_parameter("tag_to_dockpose_transform", this->tag_to_dockpose_transform_);

    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    // Lifecycle publishers only send data when the node is in the 'Active' state
    pose_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("/detected_dock_pose", 10);

    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_activate(const rclcpp_lifecycle::State & state) {
    RCLCPP_INFO(get_logger(), "Activating: Starting Tag Polling.");
    
    // Explicitly activate the lifecycle publisher
    pose_pub_->on_activate();

    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(100), std::bind(&LifecycleTagBridge::publish_tag_pose, this));

    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_deactivate(const rclcpp_lifecycle::State & state) {
    RCLCPP_INFO(get_logger(), "Deactivating: Stopping Tag Polling.");
    
    timer_.reset();
    pose_pub_->on_deactivate();

    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_cleanup(const rclcpp_lifecycle::State &) {
    RCLCPP_INFO(get_logger(), "Cleaning up resources.");
    tf_listener_.reset();
    tf_buffer_.reset();
    pose_pub_.reset();
    return CallbackReturn::SUCCESS;
  }

  CallbackReturn on_shutdown(const rclcpp_lifecycle::State &) {
    return CallbackReturn::SUCCESS;
  }

private:
  void publish_tag_pose() {
    try {
      // RCLCPP_INFO(get_logger(), "Publishing tag pose.");
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
      RCLCPP_INFO(get_logger(), "Transform exception: %s", ex.what());
      return; // Tag not visible
    }
  }

  std::string target_frame_;
  std::string output_frame_;
  std::vector<double> tag_to_dockpose_transform_; // [x, y, z, x, y, z, w]
  rclcpp::TimerBase::SharedPtr timer_;
  std::shared_ptr<rclcpp_lifecycle::LifecyclePublisher<geometry_msgs::msg::PoseStamped>> pose_pub_;
  std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
  std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
};

int main(int argc, char * argv[]) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<LifecycleTagBridge>();
  rclcpp::spin(node->get_node_base_interface());
  rclcpp::shutdown();
  return 0;
}