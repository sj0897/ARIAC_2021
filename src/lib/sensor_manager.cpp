#include "sensor_manager.h"

SensorManager::SensorManager(ros::NodeHandle* nodehandle):
  m_nh{*nodehandle}
{
  // All Logical cameras in the environment
  for (auto& camera_id: m_logical_cameras) {
    std::string prefix = "logical_camera_"; 
    m_logical_cameras_dict[camera_id] = std::make_unique<LogicalCamera>(nodehandle, prefix += camera_id); 
  }

  // All quality sensors in the environment
  for (auto& camera_id: m_quality_sensors) {
    m_quality_sensors_dict[camera_id] = std::make_unique<LogicalCamera>(nodehandle, camera_id); 
  }

}

void SensorManager::update_parts()
{
  for (auto& camera_id: m_logical_cameras) {
    m_logical_cameras_dict[camera_id]->update_parts(m_parts_database);  
  }

  for (auto& data: m_parts_database) {
    auto key = data.first; 
    ROS_INFO("Key: %s", key.c_str()); 
  }
}
