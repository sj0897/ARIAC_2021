#include "utility.h"

#include <ros/ros.h>

#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h> //--needed for tf2::Matrix3x3
#include <geometry_msgs/TransformStamped.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/static_transform_broadcaster.h>
#include <tf2_ros/transform_listener.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <array>

namespace Utility
{
  bool is_same_part(const nist_gear::Model& part1, const nist_gear::Model& part2, double tolerance = 0.05)
  {
    // double epsilon = 0.1; 

    auto x1 = part1.pose.position.x; 
    auto y1 = part1.pose.position.y; 
    auto z1 = part1.pose.position.z; 

    auto x2 = part2.pose.position.x; 
    auto y2 = part2.pose.position.y; 
    auto z2 = part2.pose.position.z; 

    // auto dist = sqrt(pow((x2-x1), 2) + pow((y2-y1), 2) + pow((z2-z1), 2)); 
    // ROS_INFO("distance: %f", dist); 

    if (sqrt(pow((x2-x1), 2) + pow((y2-y1), 2)) < tolerance)
      return true; 
    else
      return false; 
  }

  void print_part_pose(const nist_gear::Model& part)
  {
    double roll, pitch, yaw;
    std::tie(roll, pitch, yaw) = Utility::quat_to_rpy(part.pose.orientation); 

    ROS_INFO("%s in /world frame: [%f,%f,%f], [%f,%f,%f]",
              part.type.c_str(), 
              part.pose.position.x,
              part.pose.position.y,
              part.pose.position.z,
              roll,
              pitch,
              yaw); 

  }

  void print_part_pose(const nist_gear::Product& part)
  {
    double roll, pitch, yaw;
    std::tie(roll, pitch, yaw) = Utility::quat_to_rpy(part.pose.orientation); 

    ROS_INFO("%s in /world frame: [%f,%f,%f], [%f,%f,%f]",
              part.type.c_str(), 
              part.pose.position.x,
              part.pose.position.y,
              part.pose.position.z,
              roll,
              pitch,
              yaw); 

  }

  void print_pose(const geometry_msgs::Pose& pose)
  {
    ROS_INFO("Pose in /world frame: [%f,%f,%f], [%f,%f,%f,%f]",
              pose.position.x,
              pose.position.y,
              pose.position.z,
              pose.orientation.x,
              pose.orientation.y,
              pose.orientation.z,
              pose.orientation.w); 
  }


  std::tuple<double, double, double> quat_to_rpy(const geometry_msgs::Quaternion& quat)
  {
      // Transform msgs quaternion to tf2 quaternion 
      tf2::Quaternion q_tf;
      tf2::convert(quat, q_tf); 
      q_tf.normalize(); 

      //Convert Quaternion to Euler angles
      tf2::Matrix3x3 m(q_tf);
      double roll, pitch, yaw;
      m.getRPY(roll, pitch, yaw);

      return std::make_tuple(roll, pitch, yaw); 
  }

  namespace motioncontrol
  {

    void print(const tf2::Quaternion& quat) {
        ROS_INFO("[x: %f, y: %f, z: %f, w: %f]",
            quat.getX(), quat.getY(), quat.getZ(), quat.getW());
    }

    void print(const geometry_msgs::Pose& pose) {
        auto rpy = eulerFromQuaternion(pose);

        ROS_INFO("position: [x: %f, y: %f, z: %f]\norientation(quat): [x: %f, y: %f, z: %f, w: %f]\norientation(rpy): [roll: %f, pitch: %f, yaw: %f]",
            pose.position.x, pose.position.y, pose.position.z,
            pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w,
            rpy[0], rpy[1], rpy[2]);
    }

    tf2::Quaternion quaternionFromEuler(double r, double p, double y) {
        tf2::Quaternion q;
        q.setRPY(r, p, y);
        // ROS_INFO("quat: [%f, %f, %f, %f]",
        //     q.getX(),
        //     q.getY(),
        //     q.getZ(),
        //     q.getW());

        return q;
    }

    std::array<double, 3> eulerFromQuaternion(const tf2::Quaternion& quat) {

        tf2::Quaternion q(
            quat.getX(),
            quat.getY(),
            quat.getZ(),
            quat.getW());
        tf2::Matrix3x3 m(q);


        double roll, pitch, yaw;
        m.getRPY(roll, pitch, yaw);

        ROS_INFO("[%f, %f, %f]", roll, pitch, yaw);

        std::array<double, 3> rpy_array{ roll, pitch, yaw };
        return rpy_array;
    }

    std::array<double, 3> eulerFromQuaternion(
        const geometry_msgs::Pose& pose) {
        tf2::Quaternion q(
            pose.orientation.x,
            pose.orientation.y,
            pose.orientation.z,
            pose.orientation.w);
        tf2::Matrix3x3 m(q);


        double roll, pitch, yaw;
        m.getRPY(roll, pitch, yaw);
        ROS_INFO("[%f, %f, %f]", roll, pitch, yaw);

        std::array<double, 3> rpy_array{ roll, pitch, yaw };
        return rpy_array;
    }

    std::array<double, 3> eulerFromQuaternion(
        double x, double y, double z, double w) {
        tf2::Quaternion q(x, y, z, w);
        tf2::Matrix3x3 m(q);


        double roll, pitch, yaw;
        m.getRPY(roll, pitch, yaw);

        ROS_INFO("%f, %f, %f", roll, pitch, yaw);

        std::array<double, 3> rpy_array{ roll, pitch, yaw };
        return rpy_array;
    }

    geometry_msgs::Pose transformToWorldFrame(std::string part_in_camera_frame) {
        tf2_ros::Buffer tfBuffer;
        tf2_ros::TransformListener tfListener(tfBuffer);
        ros::Rate rate(10);
        ros::Duration timeout(1.0);


        geometry_msgs::TransformStamped world_target_tf;
        geometry_msgs::TransformStamped ee_target_tf;


        for (int i = 0; i < 10; i++) {
            try {
                world_target_tf = tfBuffer.lookupTransform("world", part_in_camera_frame,
                    ros::Time(0), timeout);
            }
            catch (tf2::TransformException& ex) {
                ROS_WARN("%s", ex.what());
                ros::Duration(1.0).sleep();
                continue;
            }
        }

        geometry_msgs::Pose world_target{};
        world_target.position.x = world_target_tf.transform.translation.x;
        world_target.position.y = world_target_tf.transform.translation.y;
        world_target.position.z = world_target_tf.transform.translation.z;
        world_target.orientation.x = world_target_tf.transform.rotation.x;
        world_target.orientation.y = world_target_tf.transform.rotation.y;
        world_target.orientation.z = world_target_tf.transform.rotation.z;
        world_target.orientation.w = world_target_tf.transform.rotation.w;

        return world_target;
    }
    

    geometry_msgs::Pose transformToWorldFrame(
        const geometry_msgs::Pose& target,
        std::string agv) {
        static tf2_ros::StaticTransformBroadcaster br;
        geometry_msgs::TransformStamped transformStamped;

        std::string kit_tray;
        if (agv.compare("agv1") == 0)
            kit_tray = "kit_tray_1";
        else if (agv.compare("agv2") == 0)
            kit_tray = "kit_tray_2";
        else if (agv.compare("agv3") == 0)
            kit_tray = "kit_tray_3";
        else if (agv.compare("agv4") == 0)
            kit_tray = "kit_tray_4";

        transformStamped.header.stamp = ros::Time::now();
        transformStamped.header.frame_id = kit_tray;
        transformStamped.child_frame_id = "target_frame";
        transformStamped.transform.translation.x = target.position.x;
        transformStamped.transform.translation.y = target.position.y;
        transformStamped.transform.translation.z = target.position.z;
        transformStamped.transform.rotation.x = target.orientation.x;
        transformStamped.transform.rotation.y = target.orientation.y;
        transformStamped.transform.rotation.z = target.orientation.z;
        transformStamped.transform.rotation.w = target.orientation.w;


        for (int i{ 0 }; i < 15; ++i)
            br.sendTransform(transformStamped);

        tf2_ros::Buffer tfBuffer;
        tf2_ros::TransformListener tfListener(tfBuffer);
        ros::Rate rate(10);
        ros::Duration timeout(1.0);


        geometry_msgs::TransformStamped world_pose_tf;
        geometry_msgs::TransformStamped ee_target_tf;


        for (int i = 0; i < 10; i++) {
            try {
                world_pose_tf = tfBuffer.lookupTransform("world", "target_frame",
                    ros::Time(0), timeout);
            }
            catch (tf2::TransformException& ex) {
                ROS_WARN("%s", ex.what());
                ros::Duration(1.0).sleep();
                continue;
            }
        }

        geometry_msgs::Pose world_pose{};
        world_pose.position.x = world_pose_tf.transform.translation.x;
        world_pose.position.y = world_pose_tf.transform.translation.y;
        world_pose.position.z = world_pose_tf.transform.translation.z;
        world_pose.orientation.x = world_pose_tf.transform.rotation.x;
        world_pose.orientation.y = world_pose_tf.transform.rotation.y;
        world_pose.orientation.z = world_pose_tf.transform.rotation.z;
        world_pose.orientation.w = world_pose_tf.transform.rotation.w;

        return world_pose;
    }

    geometry_msgs::Pose transformToTrayFrame(
        const geometry_msgs::Pose& target,
        std::string agv) {
        static tf2_ros::StaticTransformBroadcaster br;
        geometry_msgs::TransformStamped transformStamped;

        std::string kit_tray;
        if (agv.compare("agv1") == 0)
            kit_tray = "kit_tray_1";
        else if (agv.compare("agv2") == 0)
            kit_tray = "kit_tray_2";
        else if (agv.compare("agv3") == 0)
            kit_tray = "kit_tray_3";
        else if (agv.compare("agv4") == 0)
            kit_tray = "kit_tray_4";

        transformStamped.header.stamp = ros::Time::now();
        transformStamped.header.frame_id = "world";
        transformStamped.child_frame_id = "target_frame";
        transformStamped.transform.translation.x = target.position.x;
        transformStamped.transform.translation.y = target.position.y;
        transformStamped.transform.translation.z = target.position.z;
        transformStamped.transform.rotation.x = target.orientation.x;
        transformStamped.transform.rotation.y = target.orientation.y;
        transformStamped.transform.rotation.z = target.orientation.z;
        transformStamped.transform.rotation.w = target.orientation.w;


        for (int i{ 0 }; i < 15; ++i)
            br.sendTransform(transformStamped);

        tf2_ros::Buffer tfBuffer;
        tf2_ros::TransformListener tfListener(tfBuffer);
        ros::Rate rate(10);
        ros::Duration timeout(1.0);


        geometry_msgs::TransformStamped tray_pose_tf;


        for (int i = 0; i < 10; i++) {
            try {
                tray_pose_tf = tfBuffer.lookupTransform(kit_tray, "target_frame",
                    ros::Time(0), timeout);
            }
            catch (tf2::TransformException& ex) {
                ROS_WARN("%s", ex.what());
                ros::Duration(1.0).sleep();
                continue;
            }
        }

        geometry_msgs::Pose tray_pose{};
        tray_pose.position.x = tray_pose_tf.transform.translation.x;
        tray_pose.position.y = tray_pose_tf.transform.translation.y;
        tray_pose.position.z = tray_pose_tf.transform.translation.z;
        tray_pose.orientation.x = tray_pose_tf.transform.rotation.x;
        tray_pose.orientation.y = tray_pose_tf.transform.rotation.y;
        tray_pose.orientation.z = tray_pose_tf.transform.rotation.z;
        tray_pose.orientation.w = tray_pose_tf.transform.rotation.w;

        return tray_pose; 
    }
}
}
