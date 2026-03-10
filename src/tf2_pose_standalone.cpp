#include "rclcpp/rclcpp.hpp"
#include "docking_utils/tf2_pose_node.hpp"

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<docking_utils::Tf2PoseNode>(rclcpp::NodeOptions());
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}