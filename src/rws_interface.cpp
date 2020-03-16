/***********************************************************************************************************************
 *
 * Copyright (c) 2015, ABB Schweiz AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with
 * or without modification, are permitted provided that
 * the following conditions are met:
 *
 *    * Redistributions of source code must retain the
 *      above copyright notice, this list of conditions
 *      and the following disclaimer.
 *    * Redistributions in binary form must reproduce the
 *      above copyright notice, this list of conditions
 *      and the following disclaimer in the documentation
 *      and/or other materials provided with the
 *      distribution.
 *    * Neither the name of ABB nor the names of its
 *      contributors may be used to endorse or promote
 *      products derived from this software without
 *      specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***********************************************************************************************************************
 */

#include <algorithm>
#include <sstream>

#include "abb_librws/rws_interface.h"
#include "abb_librws/rws_rapid.h"

namespace abb
{
namespace rws
{

typedef SystemConstants::ContollerStates ContollerStates;
typedef SystemConstants::RAPID RAPID;
typedef SystemConstants::RWS::Identifiers Identifiers;
typedef SystemConstants::RWS::XMLAttributes XMLAttributes;

/***********************************************************************************************************************
 * Class definitions: RWSInterface
 */

 /************************************************************
 * Primary methods
 */

RWSInterface::RuntimeInfo RWSInterface::collectRuntimeInfo()
{
  RuntimeInfo runtime_info;

  runtime_info.auto_mode     = isAutoMode();
  runtime_info.motors_on     = isMotorsOn();
  runtime_info.rapid_running = isRAPIDRunning();
  runtime_info.rws_connected = (runtime_info.auto_mode != TriBool::UNKNOWN_VALUE &&
                                runtime_info.motors_on != TriBool::UNKNOWN_VALUE &&
                                runtime_info.rapid_running != TriBool::UNKNOWN_VALUE);

  return runtime_info;
}

RWSInterface::StaticInfo RWSInterface::collectStaticInfo()
{
  StaticInfo static_info;

  static_info.rapid_tasks = getRAPIDTasks();
  static_info.system_info = getSystemInfo();

  return static_info;
}

std::vector<cfg::moc::Arm> RWSInterface::getCFGArms()
{
  std::vector<cfg::moc::Arm> result;

  RWSClient::RWSResult rws_result = rws_client_.getConfigurationInstances(Identifiers::MOC, Identifiers::ARM);

  std::vector<Poco::XML::Node*> instances;
  instances = xmlFindNodes(rws_result.p_xml_document, XMLAttributes::CLASS_CFG_DT_INSTANCE_LI);

  for (size_t i = 0; i < instances.size(); ++i)
  {
    std::vector<Poco::XML::Node*> attributes = xmlFindNodes(instances[i], XMLAttributes::CLASS_CFG_IA_T_LI);

    cfg::moc::Arm arm;

    for (size_t j = 0; j < attributes.size(); ++j)
    {
      Poco::XML::Node* attribute = attributes[j];
      if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, Identifiers::NAME))
      {
        arm.name = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "lower_joint_bound"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> arm.lower_joint_bound;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "upper_joint_bound"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> arm.upper_joint_bound;
      }
    }

    result.push_back(arm);
  }

  return result;
}

std::vector<cfg::moc::MechanicalUnit> RWSInterface::getCFGMechanicalUnits()
{
  std::vector<cfg::moc::MechanicalUnit> result;

  RWSClient::RWSResult rws_result;
  rws_result = rws_client_.getConfigurationInstances(Identifiers::MOC, Identifiers::MECHANICAL_UNIT);

  std::vector<Poco::XML::Node*> instances;
  instances = xmlFindNodes(rws_result.p_xml_document, XMLAttributes::CLASS_CFG_DT_INSTANCE_LI);

  for (size_t i = 0; i < instances.size(); ++i)
  {
    std::vector<Poco::XML::Node*> attributes = xmlFindNodes(instances[i], XMLAttributes::CLASS_CFG_IA_T_LI);

    cfg::moc::MechanicalUnit mechanical_unit;

    for (size_t j = 0; j < attributes.size(); ++j)
    {
      Poco::XML::Node* attribute = attributes[j];
      if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, Identifiers::NAME))
      {
        mechanical_unit.name = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_robot"))
      {
        mechanical_unit.use_robot = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_single_0") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_single_1") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_single_2") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_single_3") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_single_4") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_single_5"))
      {
        mechanical_unit.use_singles.push_back(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
      }
    }

    result.push_back(mechanical_unit);
  }

  return result;
}

std::vector<cfg::sys::MechanicalUnitGroup> RWSInterface::getCFGMechanicalUnitGroups()
{
  std::vector<cfg::sys::MechanicalUnitGroup> result;

  RWSClient::RWSResult rws_result;
  rws_result = rws_client_.getConfigurationInstances(Identifiers::SYS, Identifiers::MECHANICAL_UNIT_GROUP);

  std::vector<Poco::XML::Node*> instances;
  instances = xmlFindNodes(rws_result.p_xml_document, XMLAttributes::CLASS_CFG_DT_INSTANCE_LI);

  for (size_t i = 0; i < instances.size(); ++i)
  {
    std::vector<Poco::XML::Node*> attributes = xmlFindNodes(instances[i], XMLAttributes::CLASS_CFG_IA_T_LI);

    cfg::sys::MechanicalUnitGroup mechanical_unit_group;

    for (size_t j = 0; j < attributes.size(); ++j)
    {
      Poco::XML::Node* attribute = attributes[j];
      if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "Name"))
      {
        mechanical_unit_group.name = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "Robot"))
      {
        mechanical_unit_group.robot = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "MechanicalUnit_1") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "MechanicalUnit_2") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "MechanicalUnit_3") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "MechanicalUnit_4") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "MechanicalUnit_5") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "MechanicalUnit_6"))
      {
        mechanical_unit_group.mechanical_units.push_back(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
      }
    }

    result.push_back(mechanical_unit_group);
  }

  return result;
}

std::vector<cfg::sys::PresentOption> RWSInterface::getCFGPresentOptions()
{
  std::vector<cfg::sys::PresentOption> result;

  RWSClient::RWSResult rws_result = rws_client_.getConfigurationInstances(Identifiers::SYS,
                                                                          Identifiers::PRESENT_OPTIONS);

  std::vector<Poco::XML::Node*> node_list = xmlFindNodes(rws_result.p_xml_document, XMLAttributes::CLASS_CFG_IA_T_LI);

  for (size_t i = 0; i < node_list.size(); i+=2)
  {
    if (i + 1 < node_list.size())
    {
      cfg::sys::PresentOption present_option;
      present_option.name = xmlFindTextContent(node_list.at(i), XMLAttributes::CLASS_VALUE);
      present_option.description = xmlFindTextContent(node_list.at(i+1), XMLAttributes::CLASS_VALUE);
      result.push_back(present_option);
    }
  }

  return result;
}

std::vector<cfg::moc::Robot> RWSInterface::getCFGRobots()
{
  std::vector<cfg::moc::Robot> result;

  RWSClient::RWSResult rws_result = rws_client_.getConfigurationInstances(Identifiers::MOC, Identifiers::ROBOT);

  std::vector<Poco::XML::Node*> instances;
  instances = xmlFindNodes(rws_result.p_xml_document, XMLAttributes::CLASS_CFG_DT_INSTANCE_LI);

  for (size_t i = 0; i < instances.size(); ++i)
  {
     std::vector<Poco::XML::Node*> attributes = xmlFindNodes(instances[i], XMLAttributes::CLASS_CFG_IA_T_LI);

     cfg::moc::Robot robot;

    for (size_t j = 0; j < attributes.size(); ++j)
    {
      Poco::XML::Node* attribute = attributes[j];
      if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, Identifiers::NAME))
      {
        robot.name = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_robot_type"))
      {
        robot.use_robot_type = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_joint_0") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_joint_1") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_joint_2") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_joint_3") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_joint_4") ||
              xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_joint_5"))
      {
        robot.use_joints.push_back(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_pos_x"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> robot.base_frame.pos.x.value;
        robot.base_frame.pos.x.value *= 1000.0;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_pos_y"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> robot.base_frame.pos.y.value;
        robot.base_frame.pos.y.value *= 1000.0;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_pos_z"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> robot.base_frame.pos.z.value;
        robot.base_frame.pos.z.value *= 1000.0;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_orient_u0"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> robot.base_frame.rot.q1.value;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_orient_u1"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> robot.base_frame.rot.q2.value;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_orient_u2"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> robot.base_frame.rot.q3.value;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_orient_u3"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> robot.base_frame.rot.q4.value;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_coordinated"))
      {
        robot.base_frame_moved_by = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
    }

     result.push_back(robot);
  }

  return result;
}

std::vector<cfg::moc::Single> RWSInterface::getCFGSingles()
{
  std::vector<cfg::moc::Single> result;

  RWSClient::RWSResult rws_result = rws_client_.getConfigurationInstances(Identifiers::MOC, Identifiers::SINGLE);

  std::vector<Poco::XML::Node*> instances;
  instances = xmlFindNodes(rws_result.p_xml_document, XMLAttributes::CLASS_CFG_DT_INSTANCE_LI);

  for (size_t i = 0; i < instances.size(); ++i)
  {
    std::vector<Poco::XML::Node*> attributes = xmlFindNodes(instances[i], XMLAttributes::CLASS_CFG_IA_T_LI);

    cfg::moc::Single single;

    for (size_t j = 0; j < attributes.size(); ++j)
    {
      Poco::XML::Node* attribute = attributes[j];
      if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, Identifiers::NAME))
      {
        single.name = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_single_type"))
      {
        single.use_single_type = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "use_joint"))
      {
        single.use_joint = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_pos_x"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> single.base_frame.pos.x.value;
        single.base_frame.pos.x.value *= 1000.0;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_pos_y"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> single.base_frame.pos.y.value;
        single.base_frame.pos.y.value *= 1000.0;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_pos_z"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> single.base_frame.pos.z.value;
        single.base_frame.pos.z.value *= 1000.0;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_orient_u0"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> single.base_frame.rot.q1.value;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_orient_u1"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> single.base_frame.rot.q2.value;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_orient_u2"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> single.base_frame.rot.q3.value;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_orient_u3"))
      {
        std::stringstream ss(xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE));
        ss >> single.base_frame.rot.q4.value;
      }
      else if(xmlNodeHasAttribute(attribute, Identifiers::TITLE, "base_frame_coordinated"))
      {
        single.base_frame_coordinated = xmlFindTextContent(attribute, XMLAttributes::CLASS_VALUE);
      }
    }

    result.push_back(single);
  }

  return result;
}

std::vector<RWSInterface::RobotWareOptionInfo> RWSInterface::getPresentRobotWareOptions()
{
  std::vector<RobotWareOptionInfo> result;

  RWSClient::RWSResult rws_result = rws_client_.getConfigurationInstances(Identifiers::SYS,
                                                                          Identifiers::PRESENT_OPTIONS);

  std::vector<Poco::XML::Node*> node_list = xmlFindNodes(rws_result.p_xml_document, XMLAttributes::CLASS_CFG_IA_T_LI);

  for (size_t i = 0; i < node_list.size(); i+=2)
  {
    if (i + 1 < node_list.size())
    {
      result.push_back(RobotWareOptionInfo(xmlFindTextContent(node_list.at(i), XMLAttributes::CLASS_VALUE),
                                           xmlFindTextContent(node_list.at(i+1), XMLAttributes::CLASS_VALUE)));
    }
  }

  return result;
}

std::string RWSInterface::getIOSignal(const std::string& iosignal)
{
  std::string result;

  RWSClient::RWSResult rws_result = rws_client_.getIOSignal(iosignal);

  if (rws_result.success)
  {
    result = xmlFindTextContent(rws_result.p_xml_document, XMLAttributes::CLASS_LVALUE);
  }

  return result;
}

bool RWSInterface::getMechanicalUnitJointTarget(const std::string& mechunit, JointTarget* p_jointtarget)
{
  bool result = false;

  if (p_jointtarget)
  {
    RWSClient::RWSResult rws_result = rws_client_.getMechanicalUnitJointTarget(mechunit);
    result = rws_result.success;

    if (result)
    {
      std::stringstream ss;

      ss << "[["
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "rax_1")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "rax_2")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "rax_3")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "rax_4")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "rax_5")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "rax_6")) << "], ["
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_a")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_b")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_c")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_d")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_e")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_f")) << "]]";

      p_jointtarget->parseString(ss.str());
    }
  }

  return result;
}

bool RWSInterface::getMechanicalUnitRobTarget(const std::string& mechunit,
                                              RobTarget* p_robtarget,
                                              const RWSClient::Coordinate& coordinate,
                                              const std::string& tool,
                                              const std::string& wobj)
{
  bool result = false;

  if (p_robtarget)
  {
    RWSClient::RWSResult rws_result = rws_client_.getMechanicalUnitRobTarget(mechunit, coordinate, tool, wobj);
    result = rws_result.success;

    if (result)
    {
      std::stringstream ss;

      ss << "[["
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "x")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "y")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "z")) << "], ["
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "q1")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "q2")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "q3")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "q4")) << "], ["
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "cf1")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "cf4")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "cf6")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "cfx")) << "], ["
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_a")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_b")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_c")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_d")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_e")) << ","
         << xmlFindTextContent(rws_result.p_xml_document, XMLAttribute("class", "eax_f")) << "]]";

      p_robtarget->parseString(ss.str());
    }
  }

  return result;
}

bool RWSInterface::setRAPIDSymbolData(const std::string& task,
                                      const std::string& module,
                                      const std::string& name,
                                      const RAPIDSymbolDataAbstract& data)
{
  return rws_client_.setRAPIDSymbolData(RWSClient::RAPIDResource(task, module, name), data).success;
}

bool RWSInterface::setRAPIDSymbolData(const std::string& task,
                                      const RWSClient::RAPIDSymbolResource& symbol,
                                      const RAPIDSymbolDataAbstract& data)
{
  return rws_client_.setRAPIDSymbolData(RWSClient::RAPIDResource(task, symbol), data).success;
}

bool RWSInterface::startRAPIDExecution()
{
  return rws_client_.startRAPIDExecution().success;
}

bool RWSInterface::stopRAPIDExecution()
{
  return rws_client_.stopRAPIDExecution().success;
}

bool RWSInterface::resetRAPIDProgramPointer()
{
  return rws_client_.resetRAPIDProgramPointer().success;
}

bool RWSInterface::setMotorsOn()
{
  return rws_client_.setMotorsOn().success;
}

bool RWSInterface::setMotorsOff()
{
  return rws_client_.setMotorsOff().success;
}

std::vector<RWSInterface::RAPIDModuleInfo> RWSInterface::getRAPIDModulesInfo(const std::string& task)
{
  std::vector<RAPIDModuleInfo> result;

  RWSClient::RWSResult rws_result = rws_client_.getRAPIDModulesInfo(task);
  std::vector<Poco::XML::Node*> node_list = xmlFindNodes(rws_result.p_xml_document,
                                                         XMLAttributes::CLASS_RAP_MODULE_INFO_LI);

  for (size_t i = 0; i < node_list.size(); ++i)
  {
    std::string name = xmlFindTextContent(node_list.at(i), XMLAttributes::CLASS_NAME);
    std::string type = xmlFindTextContent(node_list.at(i), XMLAttributes::CLASS_TYPE);

    result.push_back(RAPIDModuleInfo(name, type));
  }

  return result;
}

std::vector<RWSInterface::RAPIDTaskInfo> RWSInterface::getRAPIDTasks()
{
  std::vector<RAPIDTaskInfo> result;

  RWSClient::RWSResult rws_result = rws_client_.getRAPIDTasks();
  std::vector<Poco::XML::Node*> node_list = xmlFindNodes(rws_result.p_xml_document, XMLAttributes::CLASS_RAP_TASK_LI);

  for (size_t i = 0; i < node_list.size(); ++i)
  {
    std::string name = xmlFindTextContent(node_list.at(i), XMLAttributes::CLASS_NAME);
    bool is_motion_task = xmlFindTextContent(node_list.at(i), XMLAttributes::CLASS_MOTIONTASK) == RAPID::RAPID_TRUE;
    bool is_active = xmlFindTextContent(node_list.at(i), XMLAttributes::CLASS_ACTIVE) == "On";
    std::string temp = xmlFindTextContent(node_list.at(i), XMLAttributes::CLASS_EXCSTATE);

    RAPIDTaskExecutionState execution_state = UNKNOWN;
    if(temp == "read")
    {
      execution_state = READY;
    }
    else if(temp == "stop")
    {
      execution_state = STOPPED;
    }
    else if(temp == "star")
    {
      execution_state = STARTED;
    }
    else if(temp == "unin")
    {
      execution_state = UNINITIALIZED;
    }

    result.push_back(RAPIDTaskInfo(name, is_motion_task, is_active, execution_state));
  }

  return result;
}

RWSInterface::SystemInfo RWSInterface::getSystemInfo()
{
  SystemInfo result;

  RWSClient::RWSResult rws_result = rws_client_.getRobotWareSystem();

  std::vector<Poco::XML::Node*> node_list = xmlFindNodes(rws_result.p_xml_document, XMLAttributes::CLASS_SYS_SYSTEM_LI);
  for (size_t i = 0; i < node_list.size(); ++i)
  {
    result.system_name = xmlFindTextContent(node_list.at(i), XMLAttributes::CLASS_NAME);
    result.robot_ware_version = xmlFindTextContent(node_list.at(i), XMLAttributes::CLASS_RW_VERSION_NAME);
  }

  node_list = xmlFindNodes(rws_result.p_xml_document, XMLAttributes::CLASS_SYS_OPTION_LI);
  for (size_t i = 0; i < node_list.size(); ++i)
  {
    result.system_options.push_back(xmlFindTextContent(node_list.at(i), XMLAttributes::CLASS_OPTION));
  }

  result.system_type = xmlFindTextContent(rws_client_.getContollerService().p_xml_document,
                                          XMLAttributes::CLASS_CTRL_TYPE);

  return result;
}

TriBool RWSInterface::isAutoMode()
{
  return compareSingleContent(rws_client_.getPanelOperationMode(),
                              XMLAttributes::CLASS_OPMODE,
                              ContollerStates::PANEL_OPERATION_MODE_AUTO);
}

TriBool RWSInterface::isMotorsOn()
{
  return compareSingleContent(rws_client_.getPanelControllerState(),
                              XMLAttributes::CLASS_CTRLSTATE,
                              ContollerStates::CONTROLLER_MOTOR_ON);
}

TriBool RWSInterface::isRAPIDRunning()
{
  return compareSingleContent(rws_client_.getRAPIDExecution(),
                              XMLAttributes::CLASS_CTRLEXECSTATE,
                              ContollerStates::RAPID_EXECUTION_RUNNING);
}

bool RWSInterface::setIOSignal(const std::string& iosignal, const std::string& value)
{
  return rws_client_.setIOSignal(iosignal, value).success;
}

bool RWSInterface::getRAPIDSymbolData(const std::string& task,
                                      const std::string& module,
                                      const std::string& name,
                                      RAPIDSymbolDataAbstract* p_data)
{
  return rws_client_.getRAPIDSymbolData(RWSClient::RAPIDResource(task, module, name), p_data).success;
}

bool RWSInterface::getRAPIDSymbolData(const std::string& task,
                                      const RWSClient::RAPIDSymbolResource& symbol,
                                      RAPIDSymbolDataAbstract* p_data)
{
  return rws_client_.getRAPIDSymbolData(RWSClient::RAPIDResource(task, symbol), p_data).success;
}

bool RWSInterface::getFile(const RWSClient::FileResource& resource, std::string* p_file_content)
{
  return rws_client_.getFile(resource, p_file_content).success;
}

bool RWSInterface::uploadFile(const RWSClient::FileResource& resource, const std::string& file_content)
{
  return rws_client_.uploadFile(resource, file_content).success;
}

bool RWSInterface::deleteFile(const RWSClient::FileResource& resource)
{
  return rws_client_.deleteFile(resource).success;
}

bool RWSInterface::startSubscription (const RWSClient::SubscriptionResources& resources)
{
  return rws_client_.startSubscription(resources).success;
}

bool RWSInterface::waitForSubscriptionEvent()
{
  RWSClient::RWSResult rws_result = rws_client_.waitForSubscriptionEvent();

  return (rws_result.success && !rws_result.p_xml_document.isNull());
}

bool RWSInterface::waitForSubscriptionEvent(Poco::AutoPtr<Poco::XML::Document>* p_xml_document)
{
  bool result = false;

  if (p_xml_document)
  {
    RWSClient::RWSResult rws_result = rws_client_.waitForSubscriptionEvent();

    if (rws_result.success && !rws_result.p_xml_document.isNull())
    {
      *p_xml_document = rws_result.p_xml_document;
      result = true;
    }
  }

  return result;
}

bool RWSInterface::endSubscription()
{
  return rws_client_.endSubscription().success;
}

void RWSInterface::forceCloseSubscription()
{
  rws_client_.webSocketShutdown();
}

bool RWSInterface::registerLocalUser(const std::string& username,
                                     const std::string& application,
                                     const std::string& location)
{
  return rws_client_.registerLocalUser(username, application, location).success;
}

bool RWSInterface::registerRemoteUser(const std::string& username,
                                      const std::string& application,
                                      const std::string& location)
{
  return rws_client_.registerRemoteUser(username, application, location).success;
}

std::string RWSInterface::getLogText(const bool verbose)
{
  return rws_client_.getLogText(verbose);
}

std::string RWSInterface::getLogTextLatestEvent(const bool verbose)
{
  return rws_client_.getLogTextLatestEvent(verbose);
}

/************************************************************
 * Auxiliary methods
 */

TriBool RWSInterface::compareSingleContent(const RWSClient::RWSResult& rws_result,
                                           const XMLAttribute& attribute,
                                           const std::string& compare_string)
{
  TriBool result;

  if (rws_result.success)
  {
    std::string temp = xmlFindTextContent(rws_result.p_xml_document, attribute);
    result = (temp == compare_string);
  }

  return result;
}

} // end namespace rws
} // end namespace abb
