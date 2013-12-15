/**********************************************************************
 *  Copyright (c) 2008-2010, Alliance for Sustainable Energy.
 *  Copyright (c) 2013, The Pennsylvania State University.
 *  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/

#include <contam/ForwardTranslator.hpp>
#include <energyplus/ReverseTranslator.hpp>
#include <model/Model.hpp>
#include <model/WeatherFile.hpp>
#include <model/Building.hpp>
#include <model/Building_Impl.hpp>
#include <model/ThermalZone.hpp>
#include <model/ThermalZone_Impl.hpp>
#include <model/Space.hpp>
#include <model/SpaceType.hpp>
#include <model/SizingZone.hpp>
#include <model/DesignSpecificationOutdoorAir.hpp>
#include <model/BuildingStory.hpp>
#include <model/ThermostatSetpointDualSetpoint.hpp>
#include <model/ThermostatSetpointDualSetpoint_Impl.hpp>
#include <model/HVACTemplates.hpp>
#include <model/AirLoopHVAC.hpp>
#include <model/AirLoopHVAC_Impl.hpp>
#include <model/SetpointManagerSingleZoneReheat.hpp>
#include <model/SetpointManagerSingleZoneReheat_Impl.hpp>
#include <osversion/VersionTranslator.hpp>
#include <utilities/geometry/Point3d.hpp>

#include <utilities/core/CommandLine.hpp>
#include <utilities/core/Path.hpp>
#include <utilities/sql/SqlFile.hpp>

#include <string>
#include <iostream>

void usage( boost::program_options::options_description desc)
{
  std::cout << "Usage: demomodel [options]" << std::endl;
  std::cout << desc << std::endl;
}

/*
inline openstudio::path contamTemplatePath()
{
  return openstudio::toPath("CONTAMTemplate.osm");
}
*/

boost::optional<openstudio::model::Model> buildDemoModel(openstudio::model::Model model)
{
   // load model from template
  //openstudio::osversion::VersionTranslator vt;
  //boost::optional<openstudio::model::Model> optionalModel = vt.loadModel(contamTemplatePath());
  //if(!optionalModel)
  //{
  //  return boost::optional<openstudio::model::Model>();
  //}
  //openstudio::model::Model model = optionalModel.get();

  // add design days
  //openstudio::path ddyPath = (resourcesPath() / openstudio::toPath("weatherdata") / openstudio::toPath("USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.ddy"));
  //boost::optional<Workspace> ddyWorkspace = Workspace::load(ddyPath);
  //ASSERT_TRUE(ddyWorkspace);

  //openstudio::energyplus::ReverseTranslator rt;
  //boost::optional<openstudio::model::Model> ddyModel = rt.translateWorkspace(*ddyWorkspace);
  //ASSERT_TRUE(ddyModel);
  //BOOST_FOREACH(openstudio::model::DesignDay designDay, ddyModel->getModelObjects<openstudio::model::DesignDay>())
  //{
  //  model.addObject(designDay);
  //}

  // set outdoor air specifications
  openstudio::model::Building building = model.getUniqueModelObject<openstudio::model::Building>();
  boost::optional<openstudio::model::SpaceType> spaceType = building.spaceType();
  if(!spaceType)
  {
    return boost::optional<openstudio::model::Model>();
  }
  boost::optional<openstudio::model::DesignSpecificationOutdoorAir> oa = spaceType->designSpecificationOutdoorAir();
  if(!oa)
  {
    return boost::optional<openstudio::model::Model>();
  }

  if(!oa->setOutdoorAirMethod("Sum"))
  {
    return boost::optional<openstudio::model::Model>();
  }
  if(!oa->setOutdoorAirFlowperPerson(0.0))
  {
    return boost::optional<openstudio::model::Model>();
  }
  if(!oa->setOutdoorAirFlowperFloorArea(0.00508)) // 1 cfm/ft^2 = 0.00508 m/s
  {
    return boost::optional<openstudio::model::Model>();
  }
  if(!oa->setOutdoorAirFlowRate(0.0))
  {
    return boost::optional<openstudio::model::Model>();
  }
  if(!oa->setOutdoorAirFlowAirChangesperHour(0.0))
  {
    return boost::optional<openstudio::model::Model>();
  }

  double floorHeight = 3.0;

  openstudio::model::BuildingStory story1(model);
  story1.setName("Story 1");
  story1.setNominalZCoordinate(0.0);
  story1.setNominalFloortoFloorHeight(floorHeight);

  std::vector<openstudio::Point3d> points;
  points.push_back(openstudio::Point3d(0,0,0));
  points.push_back(openstudio::Point3d(0,17,0));
  points.push_back(openstudio::Point3d(8,17,0));
  points.push_back(openstudio::Point3d(8,10,0));
  points.push_back(openstudio::Point3d(8,0,0));

  boost::optional<openstudio::model::Space> library = openstudio::model::Space::fromFloorPrint(points, floorHeight, model);
  if(!library)
  {
    return boost::optional<openstudio::model::Model>();
  }
  library->setName("Library");

  points.clear();
  points.push_back(openstudio::Point3d(8,10,0));
  points.push_back(openstudio::Point3d(8,17,0));
  points.push_back(openstudio::Point3d(18,17,0));
  points.push_back(openstudio::Point3d(18,10,0));
  points.push_back(openstudio::Point3d(11,10,0));

  boost::optional<openstudio::model::Space> office2 = openstudio::model::Space::fromFloorPrint(points, floorHeight, model);
  if(!office2)
  {
    return boost::optional<openstudio::model::Model>();
  }
  office2->setName("Office 2");

  points.clear();
  points.push_back(openstudio::Point3d(8,0,0));
  points.push_back(openstudio::Point3d(8,10,0));
  points.push_back(openstudio::Point3d(11,10,0));
  points.push_back(openstudio::Point3d(11,0,0));

  boost::optional<openstudio::model::Space> hallway = openstudio::model::Space::fromFloorPrint(points, floorHeight, model);
  if(!hallway)
  {
    return boost::optional<openstudio::model::Model>();
  }
  hallway->setName("Hallway");

  points.clear();
  points.push_back(openstudio::Point3d(11,0,0));
  points.push_back(openstudio::Point3d(11,10,0));
  points.push_back(openstudio::Point3d(18,10,0));
  points.push_back(openstudio::Point3d(18,0,0));

  boost::optional<openstudio::model::Space> office1 = openstudio::model::Space::fromFloorPrint(points, floorHeight, model);
  if(!office1)
  {
    return boost::optional<openstudio::model::Model>();
  }
  office1->setName("Office 1");

  library->matchSurfaces(*office2);
  library->matchSurfaces(*hallway);
  hallway->matchSurfaces(*office1);
  hallway->matchSurfaces(*office2);
  office1->matchSurfaces(*office2);

  // find thermostat
  boost::optional<openstudio::model::ThermostatSetpointDualSetpoint> thermostat;
  BOOST_FOREACH(openstudio::model::ThermostatSetpointDualSetpoint t,
    model.getModelObjects<openstudio::model::ThermostatSetpointDualSetpoint>())
  {
    thermostat = t;
    break;
  }
  if(!thermostat)
  {
    return boost::optional<openstudio::model::Model>();
  }
  
  // create  thermal zones
  openstudio::model::ThermalZone libraryZone(model);
  openstudio::model::SizingZone librarySizing(model, libraryZone);
  libraryZone.setName("Library Zone");
  libraryZone.setThermostatSetpointDualSetpoint(*thermostat);
  library->setThermalZone(libraryZone);
  library->setBuildingStory(story1);

  openstudio::model::ThermalZone hallwayZone(model);
  //model::SizingZone hallwaySizing(model, hallwayZone);
  hallwayZone.setName("Hallway Zone");
  //hallwayZone.setThermostatSetpointDualSetpoint(*thermostat);
  hallway->setThermalZone(hallwayZone);
  hallway->setBuildingStory(story1);

  openstudio::model::ThermalZone office1Zone(model);
  openstudio::model::SizingZone office1Sizing(model, office1Zone);
  office1Zone.setName("Office 1 Zone");
  office1Zone.setThermostatSetpointDualSetpoint(*thermostat);
  office1->setThermalZone(office1Zone);
  office1->setBuildingStory(story1);

  openstudio::model::ThermalZone office2Zone(model);
  openstudio::model::SizingZone office2Sizing(model, office2Zone);
  office2Zone.setName("Office 2 Zone");
  office2Zone.setThermostatSetpointDualSetpoint(*thermostat);
  office2->setThermalZone(office2Zone);
  office2->setBuildingStory(story1);

  // add the air system
  openstudio::model::Loop loop = openstudio::model::addSystemType3(model);
  openstudio::model::AirLoopHVAC airLoop = loop.cast<openstudio::model::AirLoopHVAC>();
  airLoop.addBranchForZone(libraryZone);
  airLoop.addBranchForZone(office1Zone);
  airLoop.addBranchForZone(office2Zone);

  boost::optional<openstudio::model::SetpointManagerSingleZoneReheat> setpointManager;
  BOOST_FOREACH(openstudio::model::SetpointManagerSingleZoneReheat t, 
    model.getModelObjects<openstudio::model::SetpointManagerSingleZoneReheat>())
  {
    setpointManager = t;
    break;
  }
  if(!setpointManager)
  {
    return boost::optional<openstudio::model::Model>();
  }
  setpointManager->setControlZone(libraryZone);

  return boost::optional<openstudio::model::Model>(model);
}

int main(int argc, char *argv[])
{
  std::string inputPathString;
  std::string outputPathString="CONTAMDemo.osm";
  
  boost::program_options::options_description desc("Allowed options");

  desc.add_options()
    ("help,h", "print help message and exit")
    ("input-path,i", boost::program_options::value<std::string>(&inputPathString), "path to template OSM file")
    ("output-path,o", boost::program_options::value<std::string>(&outputPathString), "path to write OSM file to");
    //("quiet,q", "suppress progress output");

  boost::program_options::positional_options_description pos;
  //pos.add("input-path", -1);  // Maybe make the output path positional?
    
  boost::program_options::variables_map vm;
  // The following try/catch block is necessary to avoid uncaught
  // exceptions when the program is executed with more than one
  // "positional" argument - there's got to be a better way.
  try // Is this still needed? Was it ever?
  {
    boost::program_options::store(boost::program_options::command_line_parser(argc,
      argv).options(desc).positional(pos).run(), vm);
    boost::program_options::notify(vm);
  }
  catch(std::exception&)
  {
    std::cout << "Execution failed: check arguments and retry."<< std::endl << std::endl;
    usage(desc);
    return EXIT_FAILURE;
  }
  
  if(vm.count("help"))
  {
    usage(desc);
    return EXIT_SUCCESS;
  }

  //if(!vm.count("input-path"))
  //{
  //  std::cout << "No input path given." << std::endl << std::endl;
  //  usage(desc);
  //  return EXIT_FAILURE;
  //}
  
  // Open the model
  boost::optional<openstudio::model::Model> optionalModel;
  openstudio::osversion::VersionTranslator vt;
  if(vm.count("input-path"))
  {
    optionalModel = vt.loadModel(openstudio::toPath(inputPathString));
    if(!optionalModel)
    {
      std::cout << "Failed to open input template model '" << inputPathString << "'." << std::endl;
      std::cout << "Using default template." << std::endl;
    }
  }
  if(!optionalModel)
  {
    QFile fp(":/templates/CONTAMTemplate.osm");
    if(!fp.open(QFile::ReadOnly))
    {
      std::cout << "Failed to open default template model." << std::endl;
      return EXIT_FAILURE;
    }
    QTextStream stream(&fp);
    QString string = stream.readAll();
    fp.close();
    optionalModel = vt.loadModel(std::istringstream(string.toStdString()));
    if(!optionalModel)
    {
      std::cout << "Failed to read default template model." << std::endl;
      return EXIT_FAILURE;
    }
  }


  //openstudio::path inputPath = openstudio::toPath(inputPathString);
  //openstudio::osversion::VersionTranslator vt;
  //boost::optional<openstudio::model::Model> model = vt.loadModel(inputPath);

  //openstudio::osversion::VersionTranslator vt;
  //boost::optional<openstudio::model::Model> optionalModel = vt.loadModel(contamTemplatePath());
  //if(!optionalModel)
  //{
  //  return boost::optional<openstudio::model::Model>();
  //}
  openstudio::model::Model model = optionalModel.get();

  optionalModel = buildDemoModel(model);

  if(optionalModel)
  {
    if(!optionalModel->save(openstudio::toPath(outputPathString),true))
    {
      std::cout << "Failed to write OSM file." << std::endl;
      return EXIT_FAILURE;
    }
  }
  else
  {
    std::cout << "Failed to build OpenStudio model." << std::endl;
    return EXIT_FAILURE;
  }

  //if(!model)
  //{
  //  std::cout << "Unable to load file '"<< inputPathString << "' as an OpenStudio model." << std::endl;
  //  return EXIT_FAILURE;
  //}
  
  return EXIT_SUCCESS;
}
