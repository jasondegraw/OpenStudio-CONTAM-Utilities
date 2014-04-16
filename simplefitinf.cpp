/**********************************************************************
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
#include <contam/SimFile.hpp>
#include <model/Model.hpp>
#include <model/Space.hpp>
#include <model/Space_Impl.hpp>
#include <model/ThermalZone.hpp>
#include <model/SpaceInfiltrationDesignFlowRate.hpp>
#include <model/SpaceInfiltrationDesignFlowRate_Impl.hpp>
#include <model/SpaceInfiltrationEffectiveLeakageArea.hpp>
#include <model/SpaceInfiltrationEffectiveLeakageArea_Impl.hpp>
#include <osversion/VersionTranslator.hpp>
#include <utilities/core/CommandLine.hpp>
#include <utilities/core/Path.hpp>

#include <QProcess>

#include <map>

void usage( boost::program_options::options_description desc)
{
  std::cout << "Usage: simplefitinf --input-path=./path/to/input.osm" << std::endl;
  std::cout << "   or: simplefitinf input.osm" << std::endl;
  std::cout << desc << std::endl;
}

int main(int argc, char *argv[])
{
  std::string inputPathString;
  std::string outputPathString = "simple-fit-infiltration.osm";
  std::string leakageDescriptorString="Average";
  int ndirs=4;
  double flow=27.1;
  double returnSupplyRatio=1.0;
  double density = 1.2041;
  bool setLevel = true;
  bool verbose = true;
  boost::program_options::options_description desc("Allowed options");

  desc.add_options()
    ("flow,f", boost::program_options::value<double>(&flow), "leakage flow rate per envelope area [m^3/h/m^2]")
    ("ndirs,n", boost::program_options::value<int>(&ndirs), "number of directions to use (default: 4)")
    ("help,h", "print help message and exit")
    ("input-path,i", boost::program_options::value<std::string>(&inputPathString), "path to input OSM file")
    ("level,l", boost::program_options::value<std::string>(&leakageDescriptorString), "airtightness: Leaky|Average|Tight (default: Average)")
    ("output-path,o", boost::program_options::value<std::string>(&outputPathString), "path to output OSM file")
    ("no-osm", "suppress output of OSM file")
    ("quiet,q", "suppress progress output");

  boost::program_options::positional_options_description pos;
  pos.add("input-path", -1);

  boost::program_options::variables_map vm;
  // The following try/catch block is necessary to avoid uncaught
  // exceptions when the program is executed with more than one
  // "positional" argument - there's got to be a better way.
  try
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

  // The usual ugly hard coded locations of the executables
  openstudio::path contamExe = openstudio::toPath("C:\\Program Files (x86)\\NIST\\CONTAM 3.1\\ContamX3.exe");
  openstudio::path simreadxExe = openstudio::toPath("C:\\Users\\jwd131\\Software\\CONTAM\\simreadx.exe");

  if(vm.count("help"))
  {
    usage(desc);
    return EXIT_SUCCESS;
  }

  if(vm.count("quiet"))
  {
    verbose = false;
  }

  if(!vm.count("input-path"))
  {
    std::cout << "No input path given." << std::endl << std::endl;
    usage(desc);
    return EXIT_FAILURE;
  }

  if(vm.count("flow"))
  {
    // Probably should do a sanity check of input - but maybe later
    setLevel = false;
  }

  if(ndirs < 0)
  {
    if(verbose)
    {
      std::cout << "Bad ndirs value '" << ndirs << "', using ndirs=4" << std::endl;
    }
    ndirs = 4;
  }

  // Open the model
  openstudio::path inputPath = openstudio::toPath(inputPathString);
  openstudio::osversion::VersionTranslator vt;
  boost::optional<openstudio::model::Model> model = vt.loadModel(inputPath);

  if(!model)
  {
    std::cout << "Unable to load file '"<< inputPathString << "' as an OpenStudio model." << std::endl;
    return EXIT_FAILURE;
  }

  // Create a vector of the directions we'll need
  double delta = 360.0/(double)ndirs;
  QVector<double> direction;
  if(verbose)
  {
    std::cout << "Simulation conditions:" << std::endl;
  }
  for(double angle=0.0;angle<360.0;angle+=delta)
  {
    direction << angle;
    if(verbose)
    {
      std::cout << "\tDirection: " << angle << std::endl;
    }
  }

  // Now the speeds
  QVector<double> speed;
  speed << 4.4704 << 8.9408;
  if(verbose)
  {
    std::cout << "\tSpeed: " << 4.4704 << std::endl;
    std::cout << "\tSpeed: " << 8.9408 << std::endl;
  }

  // Create a storage vector
  QVector<QVector<double> > results;
  // Note we are assuming one space per zone! (maybe relax this later)
  unsigned int nzones = model->getConcreteModelObjects<openstudio::model::Space>().size();
  results << QVector<double>(nzones,0.0) << QVector<double>(nzones,0.0);

  // Translate the model
  openstudio::contam::ForwardTranslator translator;
  if(setLevel)
  {
    QVector<std::string> known;
    known << "Tight" << "Average" << "Leaky";
    if(!known.contains(leakageDescriptorString))
    {
      std::cout << "Unknown airtightness level '" << leakageDescriptorString << "'" << std::endl;
      return EXIT_FAILURE;
    }
    translator.setAirtightnessLevel(leakageDescriptorString);
  }
  else
  {
    translator.setExteriorFlowRate(flow,0.65,75.0);
  }
  translator.setTranslateHVAC(false);
  boost::optional<openstudio::contam::IndexModel> cx = translator.translateModel(model.get());
  if(!cx)
  {
     std::cout << "Translation failed, check errors and warnings for more information." << std::endl;
     return EXIT_FAILURE;
  }
  if(!cx->valid())
  {
     std::cout << "Translation returned an invalid model, check errors and warnings for more information." << std::endl;
     return EXIT_FAILURE;
  }

  // Set the model for steady-state simulation
  cx->rc().setSim_af(0);

  // If we have made it this far, we should be good to go - loop over the cases and run ContamX
  for(int i=0;i<speed.size();i++)
  {
    for(int j=0;j<direction.size();j++)
    {
      // Set the wind speed and direction
      cx->ssWeather().setWindspd(speed[i]);
      cx->ssWeather().setWinddir(direction[j]);
      QString fileName = QString("temporary-%1-%2.prj").arg(speed[i]).arg(direction[j]);

      QFile file(fileName);
      if(!file.open(QFile::WriteOnly))
      {
        std::cout << "Failed to open file '"<< fileName.toStdString() << "'." << std::endl;
        std::cout << "Check that this file location is accessible and may be written." << std::endl;
        return EXIT_FAILURE;
      }
      QTextStream textStream(&file);
      if(verbose)
      {
        std::cout << "Writing file " << fileName.toStdString() << std::endl;
      }
      boost::optional<std::string> output = cx->toString();
      textStream << openstudio::toQString(*output);
      file.close();
      //
      // Run ContamX
      //
      QProcess contamProcess;
      contamProcess.start(openstudio::toQString(contamExe), QStringList() << fileName);
      if(!contamProcess.waitForStarted(-1))
      {
        std::cout << "Failed to start ContamX process." << std::endl;
        return EXIT_FAILURE;
      }
      if(!contamProcess.waitForFinished(-1))
      {
        std::cout << "Failed to complete ContamX process." << std::endl;
        return EXIT_FAILURE;
      }
      if(verbose)
      {
        std::cout << "Successfully ran ContamX" << std::endl;
      }
      //
      // Run SimRead - this will hopefully go away at some point
      //
      QProcess simreadProcess;
      simreadProcess.start(openstudio::toQString(simreadxExe), QStringList() << "-a" << fileName);
      if(!simreadProcess.waitForStarted(-1))
      {
        std::cout << "Failed to start SimReadX process." << std::endl;
        return EXIT_FAILURE;
      }
      if(!simreadProcess.waitForFinished(-1))
      {
        std::cout << "Failed to complete SimReadX process." << std::endl;
        return EXIT_FAILURE;
      }
      if(verbose)
      {
        std::cout << "Successfully ran SimReadX" << std::endl;
      }
      // Get the results
      openstudio::path simPath = openstudio::toPath(fileName.replace(".prj",".sim"));
      openstudio::contam::SimFile sim(simPath);
      // Use the convenience routine to figure out what is what
      std::vector<openstudio::TimeSeries> infiltration = cx->zoneInfiltration(&sim);
      if(infiltration.size() != nzones)
      {
        std::cout << "Failed to complete SimReadX process." << std::endl;
        return EXIT_FAILURE;
      }
      for(unsigned int k=0;k<infiltration.size();k++)
      {
        // Check to make sure the values vector has 1 entry
        if(infiltration[k].values().size() != 1)
        {
          std::cout << "Unexpected time series data." << std::endl;
          return EXIT_FAILURE;
        }
        results[i][k] += infiltration[k].value(0);
      }
    }
  }

  // Average over the various directions
  for(int i=0;i<speed.size();i++)
  {
    for(int j=0;j<results[i].size();j++)
    {
      results[i][j] /= (double)ndirs;
    }
  }
  if(verbose)
  {
    for(int j=0;j<results[0].size();j++)
    {
      std::cout << j << " " << results[0][j] << " " << results[1][j] << std::endl;
    }
  }
  QVector<double> C;
  QVector<double> D;
  for(int j=0;j<results[0].size();j++)
  {
    // Use Cramer's rule to get the coefficients we want
    double denom = 4.4704*8.9408*8.9408 - 8.9408*4.4704*4.4704;
    C << (8.9408*8.9408 - 4.4704*4.4704*results[1][j]/results[0][j])/denom;
    D << (4.4704*results[1][j]/results[0][j] - 8.9408)/denom;
  }
  if(verbose)
  {
    for(int j=0;j<results[0].size();j++)
    {
      std::cout << j << " " << C[j] << " " << D[j] << std::endl;
    }
  }

  if(verbose)
  {
    for(int j=0;j<results[0].size();j++)
    {
      double calcQ10 = results[0][j]*(C[j]*4.4704 + D[j]*4.4704*4.4704);
      double calcQ20 = results[0][j]*(C[j]*8.9408 + D[j]*8.9408*8.9408);
      //std::cout<<openstudio::toString(spaces[i].handle())<<", 10 mph error: "<<results[0][j]-calcQ10<<std::endl;
      //std::cout<<openstudio::toString(spaces[i].handle())<<", 20 mph error: "<<results[1][j]-calcQ20<<std::endl;
      std::cout<<j<<", 10 mph error: "<<results[0][j]-calcQ10<<std::endl;
      std::cout<<j<<", 20 mph error: "<<results[1][j]-calcQ20<<std::endl;
    }
  }

  // Remove previous infiltration objects
  std::vector<openstudio::model::SpaceInfiltrationDesignFlowRate> dfrInf = model->getConcreteModelObjects<openstudio::model::SpaceInfiltrationDesignFlowRate>();
  BOOST_FOREACH(openstudio::model::SpaceInfiltrationDesignFlowRate inf, dfrInf)
  {
    inf.remove();
  }
  std::vector<openstudio::model::SpaceInfiltrationEffectiveLeakageArea> elaInf = model->getConcreteModelObjects<openstudio::model::SpaceInfiltrationEffectiveLeakageArea>();
  BOOST_FOREACH(openstudio::model::SpaceInfiltrationEffectiveLeakageArea inf, elaInf)
  {
    inf.remove();
  }

  // Build a map to the index - this will need to be changed significantly if we want more than one
  // space per zone.
  std::map <openstudio::Handle, int> zoneMap = translator.zoneMap();
  std::vector<openstudio::model::Space> spaces = model->getConcreteModelObjects<openstudio::model::Space>();
  std::map<openstudio::Handle,int> spaceMap;
  BOOST_FOREACH(openstudio::model::Space space, spaces)
  {
    boost::optional<openstudio::model::ThermalZone> thermalZone = space.thermalZone();
    if(!thermalZone)
    {
      std::cout << "Warning: Unattached space '" << openstudio::toString(space.handle()) << "'" << std::endl;
    }
    else
    {
      if(zoneMap.count(thermalZone->handle()) > 0)
      {
        spaceMap[space.handle()] = zoneMap[thermalZone->handle()];
      }
      else
      {
        std::cout << "Warning: lookup failed for zone '" << openstudio::toString(thermalZone->handle()) << "'" << std::endl;
      }
    }
  }

  // Generate infiltration objects and attach to spaces
  std::pair <openstudio::Handle,int> handleInt;
  BOOST_FOREACH(handleInt, spaceMap)
  {
    //std::cout << openstudio::toString(handleInt.first) << " " << handleInt.second << std::endl;
    boost::optional<openstudio::model::Space> space = model->getModelObject<openstudio::model::Space>(handleInt.first);
    if(!space)
    {
      std::cout << "Failed to find space " << openstudio::toString(handleInt.first)<< std::endl;
      return EXIT_FAILURE;
    }
    int index = handleInt.second-1;
    openstudio::model::SpaceInfiltrationDesignFlowRate infObj(*model);
    infObj.setDesignFlowRate(density*results[0][index]);
    infObj.setConstantTermCoefficient(0.0);
    infObj.setTemperatureTermCoefficient(0.0);
    infObj.setVelocityTermCoefficient(C[index]);
    infObj.setVelocitySquaredTermCoefficient(D[index]);
    infObj.setSpace(*space);
  }

  // Write out new OSM
  if(!vm.count("no-osm"))
  {
    openstudio::path outPath = openstudio::toPath(outputPathString);
    if(!model->save(outPath,true))
    {
      std::cout << "Failed to write OSM file." << std::endl;
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
