# rmw_ndn

Implementation of the ROS Middleware (rmw) Interface using NDN

# Install

```bash
mkdir -p ros2_ndn_ws/src
cd ros2_ndn_ws
wget https://raw.githubusercontent.com/astralien3000/rmw_ndn/master/ros2.repos
vcs import src < ros2.repos
./src/ament/ament_tools/scripts/ament.py build --symlink-install
```

# Use

## Run and configure NFD

```bash
nfd-start
nfdc strategy set prefix /ros2/discovery strategy /localhost/nfd/strategy/multicast
```

## Run publisher

```bash
. ./install/setup.bash
RMW_IMPLEMENTATION=rmw_ndn ros2 run examples_rclcpp_minimal_publisher publisher_member_function
```

## Run subscriber

```bash
. ./install/setup.bash
RMW_IMPLEMENTATION=rmw_ndn ros2 run examples_rclcpp_minimal_subscriber subscriber_member_function
```
