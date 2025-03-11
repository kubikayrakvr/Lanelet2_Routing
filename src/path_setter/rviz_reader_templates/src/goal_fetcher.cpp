#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>

void positionCallback(const geometry_msgs::PoseStamped::ConstPtr& msg)
{
    double x = msg->pose.position.x;
    double y = msg->pose.position.y;
    double z = msg->pose.position.z;
    ROS_INFO("Received goal - Position: [x = %f, y = %f, z = %f]", x, y, z);
    
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "goal_fetcher");
    ros::NodeHandle nh;
    ros::Subscriber sub = nh.subscribe("move_base_simple/goal", 1000, positionCallback);
    ros::spin();
    return 0;
}
