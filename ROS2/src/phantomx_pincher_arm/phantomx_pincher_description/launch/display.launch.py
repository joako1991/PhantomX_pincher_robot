from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

import os
import xacro


def generate_launch_description():

    package_name = 'phantomx_pincher_description'
    package_path = get_package_share_directory(package_name)

    xacro_file = os.path.join(
        package_path,
        'urdf',
        'phantomx_pincher.urdf.xacro'
    )
    rviz_config_path = os.path.join(package_path, 'rviz', 'dummy_display_config.rviz')

    robot_description = xacro.process_file(xacro_file).toxml()

    return LaunchDescription([
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            parameters=[
                {'robot_description': robot_description}
            ],
            output='screen'
        ),

        Node(
            package='joint_state_publisher_gui',
            executable='joint_state_publisher_gui',
            output='screen'
        ),

        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d', rviz_config_path],
            output='screen'
        )
    ])
