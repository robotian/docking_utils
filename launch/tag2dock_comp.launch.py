# from launch import LaunchDescription
# from launch_ros.actions import ComposableNodeContainer, LifecycleNode
# from launch_ros.descriptions import ComposableNode
# from launch_ros.events.lifecycle import ChangeState
# from launch_ros.event_handlers import OnStateTransition
# from launch.actions import EmitEvent, RegisterEventHandler
# import lifecycle_msgs.msg

# def generate_launch_description():
#     # 1. Define the Composable Node
#     tag_bridge_node = ComposableNode(
#         package='docking_utils',  # Replace with your actual package name
#         plugin='docking_utils::LifecycleTagBridge',
#         name='lifecycle_tag_bridge',
#         namespace='j100_0921',  # Adjust namespace as needed
#         parameters=[{
#             'target_tag_frame': 'jackal_charger_april',
#             'output_frame': 'odom',
#         }]
#     )

#     # 2. Define the Container to hold the node
#     container = ComposableNodeContainer(
#         name='tag_bridge_container',
#         namespace='j100_0921',  # Adjust namespace as needed
#         package='rclcpp_components',
#         executable='component_container',
#         composable_node_descriptions=[tag_bridge_node],
#         output='screen',
#     )

#     # 3. Logic to automatically "Configure" the node once it starts
#     configure_event = EmitEvent(
#         event=ChangeState(
#             lifecycle_node_matcher=lambda node: node.node_name == 'lifecycle_tag_bridge',
#             transition_id=lifecycle_msgs.msg.Transition.TRANSITION_CONFIGURE,
#         )
#     )

#     # 4. Logic to automatically "Activate" the node once "Configure" is successful
#     activate_event = RegisterEventHandler(
#       OnStateTransition(
#         target_lifecycle_node='lifecycle_tag_bridge',
#         goal_state='inactive',
#         entities=[
#           EmitEvent(
#             event=ChangeState(
#                     lifecycle_node_matcher=lambda node: node.node_name == 'lifecycle_tag_bridge',
#                     transition_id=lifecycle_msgs.msg.Transition.TRANSITION_ACTIVATE,
#             )
#           )
#         ]
#       )
#     )

#     return LaunchDescription([
#         container,
#         # configure_event,
#         # activate_event
#     ])

from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode
from launch_ros.events.lifecycle import ChangeState
from launch_ros.event_handlers import OnStateTransition
from launch.actions import EmitEvent, RegisterEventHandler, TimerAction
import lifecycle_msgs.msg

def generate_launch_description():
    ns = 'j100_0921'
    # Important: The matcher needs the full path including the namespace
    full_node_name = f'/{ns}/lifecycle_tag_bridge'

    tag_bridge_node = ComposableNode(
        package='docking_utils',
        plugin='docking_utils::LifecycleTagBridge',
        name='lifecycle_tag_bridge',
        namespace=ns,
        parameters=[{
            'target_tag_frame': 'jackal_charger_april',
            'output_frame': 'odom',
        }]
    )

    container = ComposableNodeContainer(
        name='tag_bridge_container',
        namespace=ns,
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[tag_bridge_node],
        output='screen',
    )

    # Use a TimerAction to give the container a moment to load the component
    configure_event = TimerAction(
        period=2.0,
        actions=[
            EmitEvent(
                event=ChangeState(
                    lifecycle_node_matcher=lambda node: node.node_name == full_node_name,
                    transition_id=lifecycle_msgs.msg.Transition.TRANSITION_CONFIGURE,
                )
            )
        ]
    )

    activate_event = RegisterEventHandler(
        OnStateTransition(
            target_lifecycle_node=full_node_name,
            goal_state='inactive',
            entities=[
                EmitEvent(
                    event=ChangeState(
                        lifecycle_node_matcher=lambda node: node.node_name == full_node_name,
                        transition_id=lifecycle_msgs.msg.Transition.TRANSITION_ACTIVATE,
                    )
                )
            ]
        )
    )

    return LaunchDescription([
        container,
        # configure_event,
        # activate_event
    ])