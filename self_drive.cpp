#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>
#include <nav_msgs/msg/odometry.hpp>

using namespace std::chrono_literals;
static bool initial_motion_set =false;
class SelfDrive : public rclcpp::Node
{
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pose_pub_;
  int step_;
  double desired_distance_;
  double total_distance_;

public:
  SelfDrive() : rclcpp::Node("self_drive"), step_(0), desired_distance_(0.35), total_distance_(0)
  {
    auto lidar_qos_profile = rclcpp::QoS(rclcpp::KeepLast(1));
    lidar_qos_profile.reliability(rclcpp::ReliabilityPolicy::BestEffort);
    auto callback = std::bind(&SelfDrive::subscribe_scan, this, std::placeholders::_1);
    scan_sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>("/scan", lidar_qos_profile,
                                                                       callback);
    auto vel_qos_profile = rclcpp::QoS(rclcpp::KeepLast(1));
    pose_pub_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", vel_qos_profile);
  }

  void subscribe_scan(const sensor_msgs::msg::LaserScan::SharedPtr scan)
  {
    geometry_msgs::msg::Twist vel;
    if (!initial_motion_set)
  {
    vel.linear.x = 0.15;
    vel.angular.z = 0.;
    initial_motion_set = true;
  }
  else
  {
    if(scan->ranges[0]<=0.25||scan->ranges[25]<=0.25)
    {
      vel.linear.x=0.;
      vel.angular.z=-2.;
    }
    else if (scan->ranges[35]>0.5 && scan->ranges[90]<=0.45)
    {
      vel.linear.x=0.;
      vel.angular.z=10.;
    }
    else
    {
      vel.linear.x = 0.15;  // F
      vel.angular.z = 0.;   
    }
  }
    
    RCLCPP_INFO(rclcpp::get_logger("self_drive"),
                "step=%d, range=%1.2f, linear=%1.2f, angular=%1.2f", step_, scan->ranges[0],
                vel.linear.x, vel.angular.z);
    pose_pub_->publish(vel);
    step_++;
  }
};

int main(int argc, char* argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<SelfDrive>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}