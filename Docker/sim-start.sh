#!/bin/bash

ROOTDIR=$(dirname $(readlink -f "$0"))
cd ${ROOTDIR}

function show_help {
    printf "Usage:\n" > /dev/tty
    printf "                sim-start.sh --ros_server_ip SERVER_IP --ros_client_ip CLIENT_IP \n" > /dev/tty
    printf "\n" > /dev/tty
    printf " --ros_server_ip              Specify the IP address of the ROS core. This will enable ROS\n" > /dev/tty
    printf "                           remote connectivity between the different sensors.\n" > /dev/tty
    printf "\n" > /dev/tty
    printf " --ros_client_ip              Specify the IP address of the ROS client (machine where this script is run).\n" > /dev/tty
    printf "                           Without this option, the remote connectivity won't work.\n" > /dev/tty
    printf " --image                      Specify the name of the Docker image to use.\n" > /dev/tty
    printf "\n" > /dev/tty
}

DOCKER_IMG=""
ROS_SERVER_IP=""
ROS_CLIENT_IP=""
DOCKER_INIT_FLAGS="-ti --rm --privileged --device=/dev/dri --group-add video --group-add dialout -v /tmp/.X11-unix:/tmp/.X11-unix -v /dev:/dev --gpus all -e LIBVA_DRIVER_NAME=i965 "
while [ $# -gt 0 ]; do
    case "$1" in
        --ros_server_ip)
            ROS_SERVER_IP="$2"
            shift;shift
            ;;
        --ros_client_ip)
            ROS_CLIENT_IP="$2"
            shift;shift
            ;;
        -h|--help )
            show_help
            exit 0
            ;;
        -i|--image )
            DOCKER_IMG="$2"
            shift;shift
            ;;

        *)
            echo "Invalid option $1"
            show_help
            exit 0
            ;;
    esac
done

if [[ "${ROS_CLIENT_IP}" == "" ]] || [[ "${ROS_SERVER_IP}" == "" ]]; then
    echo "ERROR: YOU MUST SPECIFY THE BOTH: ros_client_ip and ros_server_ip OPTIONS"
    echo ""
    show_help
    exit -1
fi

if [[ "${DOCKER_IMG}" == "" ]]; then
    echo "ERROR: YOU MUST SPECIFY THE DOCKER IMAGE NAME TO USE."
    echo ""
    show_help
    exit -1
fi

sudo docker run  ${DOCKER_INIT_FLAGS} \
    --env="ROS_SERVER_IP=${ROS_SERVER_IP}" \
    --env="ROS_CLIENT_IP=${ROS_CLIENT_IP}" \
    --env="DISPLAY=$DISPLAY" \
    --env="SHELL=$SHELL" \
    --env="USER=$USER" \
    -v /home/${USER}/docker_dev/:/home/${USER}/docker_dev \
    --network=host \
    --ipc=host \
    --user $USER \
    --entrypoint /home/$USER/docker-entrypoint.sh \
    ${DOCKER_IMG} \
    bash
