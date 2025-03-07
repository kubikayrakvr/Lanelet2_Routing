#include <ros/ros.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>

void positionCallback(const geometry_msgs::PoseWithCovarianceStamped::ConstPtr& msg)
{
    double x = msg->pose.pose.position.x;
    double y = msg->pose.pose.position.y;
    double z = msg->pose.pose.position.z;
    ROS_INFO("Received start - Position: [x = %f, y = %f, z = %f]", x, y, z);
    
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "start_fetcher");
    ros::NodeHandle nh;
    ros::Subscriber sub = nh.subscribe("initialpose", 1000, positionCallback);
    ros::spin();
    return 0;
}
