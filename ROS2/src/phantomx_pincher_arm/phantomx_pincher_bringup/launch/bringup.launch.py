from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

import os
import xacro


def generate_launch_description():

    description_package = get_package_share_directory(
        'phantomx_pincher_description'
    )

    bringup_package = get_package_share_directory(
        'phantomx_pincher_bringup'
    )

    xacro_file = os.path.join(
        description_package,
        'urdf',
        'phantomx_pincher.urdf.xacro'
    )

    controllers_file = os.path.join(
        bringup_package,
        'config',
        'controllers.yaml'
    )

    robot_description = xacro.process_file(
        xacro_file
    ).toxml()

    return LaunchDescription([
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[
                {'robot_description': robot_description}
            ],
            output='screen',
        ),

        Node(package="controller_manager",
            executable="ros2_control_node",
            parameters=[controllers_file],
            remappings=[("~/robot_description", "/robot_description")],
            output="screen",
        ),
        Node(
            package='controller_manager',
            executable='spawner',
            arguments=[
                'joint_state_broadcaster',
                '--controller-manager',
                '/controller_manager'
            ],
            output='screen'
        ),
        Node(
            package='controller_manager',
            executable='spawner',
            arguments=[
                'arm_controller',
                '--controller-manager',
                '/controller_manager'
            ],
            output='screen'
        ),

    ])
