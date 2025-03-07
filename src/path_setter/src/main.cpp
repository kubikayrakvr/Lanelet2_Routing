#include <ros/ros.h>

#include <thread>

#include "route_finder.cpp"
#include "start_lane.cpp"
#include "goal_lane.cpp"
#include "overtake_lane.cpp"

int main(int argc, char** argv){
    ros::init(argc, argv, "path_setter");
    ros::NodeHandle nh_;
    std::thread start_thread(calculateStart, nh_);
    std::thread goal_thread(calculateGoal, nh_);
    std::thread overtake_thread(calculateOvertake, nh_);
    std::thread route_thread(setPath, nh_);
    ros::MultiThreadedSpinner spinner(4);
    spinner.spin();
    start_thread.join();
    goal_thread.join();
    overtake_thread.join();
    route_thread.join();
    ros::spin();
    return 0;
}
