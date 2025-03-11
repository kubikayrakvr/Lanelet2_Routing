#include "map_loader.cpp"
#include "map_visual.cpp"

int main(int argc, char *argv[]){
    ros::init(argc, argv, "map_creator");
    ros::NodeHandle nh_;
    loadMap();
    createMap(nh_);
    ros::spinOnce();
    return 0;
}