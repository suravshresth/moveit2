/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2019, PickNik Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of PickNik Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: Henning Kayser
   Desc: A high level interface that does not require the use of ROS Actions, Services, and Messages to access the
   core MoveIt functionality
*/

#pragma once

#include <rclcpp/rclcpp.hpp>
#include <moveit/controller_manager/controller_manager.h>
#include <moveit/planning_scene_monitor/planning_scene_monitor.h>
#include <moveit/planning_pipeline/planning_pipeline.h>
#include <moveit/trajectory_execution_manager/trajectory_execution_manager.h>
#include <moveit/robot_state/robot_state.h>
#include <tf2_ros/buffer.h>

namespace moveit_cpp
{
MOVEIT_CLASS_FORWARD(MoveItCpp);  // Defines MoveItCppPtr, ConstPtr, WeakPtr... etc

class MoveItCpp
{
public:
  /// Specification of options to use when constructing the MoveItCpp class
  struct PlanningSceneMonitorOptions
  {
    void load(const rclcpp::Node::SharedPtr& node)
    {
      const std::string ns = "planning_scene_monitor_options";
      node->get_parameter_or(ns + ".name", name, std::string("planning_scene_monitor"));
      node->get_parameter_or(ns + ".robot_description", robot_description, std::string("robot_description"));
      node->get_parameter_or(ns + ".joint_state_topic", joint_state_topic,
                             planning_scene_monitor::PlanningSceneMonitor::DEFAULT_JOINT_STATES_TOPIC);
      node->get_parameter_or(ns + ".attached_collision_object_topic", attached_collision_object_topic,
                             planning_scene_monitor::PlanningSceneMonitor::DEFAULT_ATTACHED_COLLISION_OBJECT_TOPIC);
      node->get_parameter_or(ns + ".monitored_planning_scene_topic", monitored_planning_scene_topic,
                             planning_scene_monitor::PlanningSceneMonitor::MONITORED_PLANNING_SCENE_TOPIC);
      node->get_parameter_or(ns + ".publish_planning_scene_topic", publish_planning_scene_topic,
                             planning_scene_monitor::PlanningSceneMonitor::DEFAULT_PLANNING_SCENE_TOPIC);
      node->get_parameter_or(ns + ".wait_for_initial_state_timeout", wait_for_initial_state_timeout, 0.0);
    }
    std::string name;
    std::string robot_description;
    std::string joint_state_topic;
    std::string attached_collision_object_topic;
    std::string monitored_planning_scene_topic;
    std::string publish_planning_scene_topic;
    double wait_for_initial_state_timeout;
  };

  /// struct contains the the variables used for loading the planning pipeline
  struct PlanningPipelineOptions
  {
    void load(const rclcpp::Node::SharedPtr& node)
    {
      const std::string ns = "planning_pipelines.";
      node->get_parameter(ns + "pipeline_names", pipeline_names);
      node->get_parameter(ns + "namespace", parent_namespace);
    }
    std::vector<std::string> pipeline_names;
    std::string parent_namespace;
  };

  /// Parameter container for initializing MoveItCpp
  struct Options
  {
    Options(const rclcpp::Node::SharedPtr& node)
    {
      planning_scene_monitor_options.load(node);
      planning_pipeline_options.load(node);
    }

    PlanningSceneMonitorOptions planning_scene_monitor_options;
    PlanningPipelineOptions planning_pipeline_options;
  };

  /** \brief Constructor */
  MoveItCpp(const rclcpp::Node::SharedPtr& node);
  MoveItCpp(const rclcpp::Node::SharedPtr& node, const Options& options);

  /**
   * @brief This class owns unique resources (e.g. action clients, threads) and its not very
   * meaningful to copy. Pass by references, move it, or simply create multiple instances where
   * required.
   */
  MoveItCpp(const MoveItCpp&) = delete;
  MoveItCpp& operator=(const MoveItCpp&) = delete;

  MoveItCpp(MoveItCpp&& other) = default;
  MoveItCpp& operator=(MoveItCpp&& other) = default;

  /** \brief Destructor */
  ~MoveItCpp();

  /** \brief Get the RobotModel object. */
  moveit::core::RobotModelConstPtr getRobotModel() const;

  /** \brief Get the ROS node this instance operates on */
  const rclcpp::Node::SharedPtr& getNode() const;

  /** \brief Get the current state queried from the current state monitor
      \param wait_seconds the time in seconds for the state monitor to wait for a robot state. */
  bool getCurrentState(moveit::core::RobotStatePtr& current_state, double wait_seconds);

  /** \brief Get the current state queried from the current state monitor
      \param wait_seconds the time in seconds for the state monitor to wait for a robot state. */
  moveit::core::RobotStatePtr getCurrentState(double wait_seconds = 0.0);

  /** \brief Get all loaded planning pipeline instances mapped to their reference names */
  const std::unordered_map<std::string, planning_pipeline::PlanningPipelinePtr>& getPlanningPipelines() const;

  /** \brief Get the stored instance of the planning scene monitor */
  planning_scene_monitor::PlanningSceneMonitorConstPtr getPlanningSceneMonitor() const;
  planning_scene_monitor::PlanningSceneMonitorPtr getPlanningSceneMonitorNonConst();

  std::shared_ptr<const tf2_ros::Buffer> getTFBuffer() const;
  std::shared_ptr<tf2_ros::Buffer> getTFBuffer();

  /** \brief Get the stored instance of the trajectory execution manager */
  trajectory_execution_manager::TrajectoryExecutionManagerConstPtr getTrajectoryExecutionManager() const;
  trajectory_execution_manager::TrajectoryExecutionManagerPtr getTrajectoryExecutionManagerNonConst();

  /** \brief Execute a trajectory on the planning group specified by the robot's trajectory using the trajectory
   * execution manager.
   *  \param [in] robot_trajectory Contains trajectory info as well as metadata such as a RobotModel.
   *  \param [in] controllers An optional list of ros2_controllers to execute with. If none, MoveIt will attempt to find
   * a controller. The exact behavior of finding a controller depends on which MoveItControllerManager plugin is active.
   *  \return moveit_controller_manager::ExecutionStatus::SUCCEEDED if successful
   */
  [[deprecated("MoveItCpp::execute() no longer requires a blocking parameter")]] moveit_controller_manager::ExecutionStatus
  execute(const robot_trajectory::RobotTrajectoryPtr& robot_trajectory, bool blocking,
          const std::vector<std::string>& controllers = std::vector<std::string>());

  moveit_controller_manager::ExecutionStatus
  execute(const robot_trajectory::RobotTrajectoryPtr& robot_trajectory,
          const std::vector<std::string>& controllers = std::vector<std::string>());

  /** \brief Utility to terminate the given planning pipeline */
  bool terminatePlanningPipeline(const std::string& pipeline_name);

private:
  //  Core properties and instances
  rclcpp::Node::SharedPtr node_;
  planning_scene_monitor::PlanningSceneMonitorPtr planning_scene_monitor_;

  // Planning
  std::unordered_map<std::string, planning_pipeline::PlanningPipelinePtr> planning_pipelines_;
  std::unordered_map<std::string, std::set<std::string>> groups_algorithms_map_;

  // Execution
  trajectory_execution_manager::TrajectoryExecutionManagerPtr trajectory_execution_manager_;

  /** \brief Initialize and setup the planning scene monitor */
  bool loadPlanningSceneMonitor(const PlanningSceneMonitorOptions& options);

  /** \brief Initialize and setup the planning pipelines */
  bool loadPlanningPipelines(const PlanningPipelineOptions& options);
};
}  // namespace moveit_cpp
