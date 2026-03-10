import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode

def generate_launch_description():
    # Path to the parameters file
    config = os.path.join(
        get_package_share_directory('docking_utils'),
        'config',
        'params.yaml'
    )

    container = ComposableNodeContainer(
        name='docking_container',
        namespace='j100_0921',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            ComposableNode(
                package='docking_utils',
                plugin='docking_utils::Tf2PoseNode',
                name='tf2_pose_node',
                parameters=[config],
            ),
        ],
        output='screen',
        remappings=[
            # ('/detected_dock_pose', 'detected_dock_pose'),
            ('/tf', 'tf'),
            ('/tf_static', 'tf_static'),
        ]
    )

    return LaunchDescription([container])