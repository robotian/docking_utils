from launch import LaunchDescription
from launch_ros.actions import Node, LifecycleNode
from launch.actions import EmitEvent, RegisterEventHandler
from launch_ros.event_handlers import OnStateTransition
from launch_ros.events.lifecycle import ChangeState
from launch.event_handlers import OnProcessStart
import lifecycle_msgs.msg

def generate_launch_description():
    ns = 'j100_0921'  # Namespace for the node, adjust as needed
    node_name = 'lifecycle_tag_bridge'
    # Define the bridge node action
    bridge_node = LifecycleNode(
        package='docking_utils',
        executable='tag_to_dock_bridge',
        name=node_name,  # This name is key
        namespace=ns,
        output='screen',
        parameters=[{
            'target_tag_frame': 'jackal_charger_april',
            'output_frame': 'odom',
            # 'tag_to_dockpose_transform': [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0] # [x, y, z, x, y, z, w]
        }],
        remappings=[
            ('/detected_dock_pose', 'detected_dock_pose'),
            ('/tf', 'tf'),
            ('/tf_static', 'tf_static'),
        ]
    )

    # Use a string matcher instead of the object to avoid the 'not callable' error
    configure_event = RegisterEventHandler(
        OnProcessStart(
            target_action=bridge_node,
            on_start=[
                EmitEvent(
                    event=ChangeState(
                        lifecycle_node_matcher=lambda node: node.node_name == f'/{ns}/{node_name}',
                        transition_id=lifecycle_msgs.msg.Transition.TRANSITION_CONFIGURE,
                    )
                ),
            ],
        )
    )

    activate_event = RegisterEventHandler(
        OnStateTransition(
            target_lifecycle_node=bridge_node,
            goal_state='inactive',
            entities=[
                EmitEvent(
                    event=ChangeState(
                        lifecycle_node_matcher=lambda node: node.node_name == f'/{ns}/{node_name}',
                        transition_id=lifecycle_msgs.msg.Transition.TRANSITION_ACTIVATE,
                    )
                ),
            ],
        )
    )

    return LaunchDescription([
        bridge_node,
        configure_event,
        activate_event,
    ])