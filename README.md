# rmw_ndn

Implementation of the ROS Middleware (rmw) Interface using NDN

# Install

## Install ndn-cxx

### Prerequisites

```bash
sudo apt install pkg-config libsqlite3-dev libssl-dev libboost-all-dev
```

### Build and install

```bash
git clone https://github.com/named-data/ndn-cxx.git -b ndn-cxx-0.6.1
cd ndn-cxx
./waf configure
./waf
sudo ./waf install
```

## Install ROS2

### Prerequisites

[See ROS2 documentation](https://github.com/ros2/ros2/wiki/Linux-Development-Setup)

### Build and install

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
