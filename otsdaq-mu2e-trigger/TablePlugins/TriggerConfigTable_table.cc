#include "otsdaq-mu2e-trigger/TablePlugins/TriggerConfigTable.h"
#include "otsdaq/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq/Macros/TablePluginMacros.h"
//#include "otsdaq/tools/otsdaq_load_json_document.cc"

#include <stdio.h>
#include <sys/stat.h>  //for mkdir
#include <fstream>     // std::fstream
#include <iostream>
#include <regex>

using namespace ots;

#define ARTDAQ_FCL_PATH std::string(getenv("OTS_SCRATCH")) + "/TriggerConfigurations/"
#define ARTDAQ_FILE_PREAMBLE "boardReader"

//helpers
#define OUT out << tabStr << commentStr
#define PUSHTAB tabStr += "\t"
#define POPTAB tabStr.resize(tabStr.size() - 1)
#define PUSHCOMMENT commentStr += "# "
#define POPCOMMENT commentStr.resize(commentStr.size() - 2)

//========================================================================================================================
TriggerConfigTable::TriggerConfigTable(void) : TableBase("TriggerConfigTable")
{
	//////////////////////////////////////////////////////////////////////
	//WARNING: the names used in C++ MUST match the Table INFO  //
	//////////////////////////////////////////////////////////////////////
	__COUTS__(10) << "[TriggerConfigTable::TriggerConfigTable] Initializing the "
	            "TriggerConfigTable plugin..."
	         << __E__;
	//  exit(0);
	__COUTS__(10) << StringMacros::stackTrace() << __E__;
}  //end constructor

//========================================================================================================================
TriggerConfigTable::~TriggerConfigTable(void) {}

//========================================================================================================================
void TriggerConfigTable::init(ConfigurationManager* configManager)
{
	isFirstAppInContext_ = configManager->isOwnerFirstAppInContext();

	__COUTV__(isFirstAppInContext_);
	if(!isFirstAppInContext_)
		return;

	//make directory just in case
	mkdir((ARTDAQ_FCL_PATH).c_str(), 0755);

	std::string trigEpilogsDir;
	std::string fcl_dir = "TriggerEpilogs";
	trigEpilogsDir      = ARTDAQ_FCL_PATH + fcl_dir;
	mkdir(trigEpilogsDir.c_str(), 0755);

	auto childrenMap = configManager->__SELF_NODE__.getChildren();
	__COUTS__(10) << "printing children content" << __E__;
	__COUTS__(10) << "children map size" << childrenMap.size() << __E__;

	auto& topLevelPair = childrenMap.at(0);
	__COUTS__(10) << "Main table name '" << topLevelPair.first << "'" << __E__;

	//now download from MONGO-Db the trigger table to be used
	std::string getTableFromMongoDb = "otsdaq_load_json_document ";
	std::string triggerTableName    = topLevelPair.second.getNode("TriggerDocName").getValue();     //" testTriggerDoc ";
	std::string triggerTableVersion = topLevelPair.second.getNode("TriggerConfigTag").getValue();  //" 6 ";
	std::string outputFileName      = ARTDAQ_FCL_PATH + "/physMenu.json";

	getTableFromMongoDb +=
	    triggerTableName + " " + triggerTableVersion + " " + outputFileName;
	__COUTS__(10) << "otsdaq_load_json command: " << getTableFromMongoDb << __E__;
	
	int statusCode = system(getTableFromMongoDb.c_str());
	__COUTVS__(10,statusCode);

	__COUTS__(10) << StringMacros::stackTrace() << __E__;

	std::string command =
	    "python ${SPACK_ENV}/mu2e-trig-config/python/generateMenuFromJSON.py";
	std::string menuFile = " -mf " + outputFileName;
	std::string output   = " -o " + trigEpilogsDir;
	std::string evtMode  = " -evtMode all";

	command += menuFile + output + evtMode;
	statusCode = system(command.c_str());
	__COUTVS__(10,statusCode);

	// //create the fcl file
	// std::ofstream      triggerFclFile, epilogFclFile, subEpilogFclFile, allPathsFile;
	// std::string        fclFileName, skelethonName, allPathsFileName;
	// std::string        epilogName;
	// //skelethon to clone
	// //	skelethonName = Form("%s/main.fcl", (ARTDAQ_FCL_PATH).c_str());
	// skelethonName = ARTDAQ_FCL_PATH + "/main.fcl" ;
	// //file to be edited
	// //	fclFileName = Form("%s/runTriggerExample.fcl", (ARTDAQ_FCL_PATH).c_str());
	// fclFileName = ARTDAQ_FCL_PATH+"/runTriggerExample.fcl";

	// //file that will house all the includes necessary to run the trigger paths
	// allPathsFileName = trigEpilogsDir+ "/allPaths.fcl";

	// std::ifstream      mainFclFile;
	// mainFclFile   .open(skelethonName);
	// triggerFclFile.open(fclFileName);
	// allPathsFile  .open(allPathsFileName);

	// std::string        line;
	// while (std::getline(mainFclFile, line, '\n') ) triggerFclFile << line << '\n';

	// //we need to append the line where include the fcl that will contain all the trigger paths
	// triggerFclFile << "#include \"Trigger_epilogs/allPaths.fcl\""<<__E__;

	// __COUTS__(10) << "*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*" << std::endl;
	// __COUTS__(10) << configManager->__SELF_NODE__ << std::endl;

	// auto childrenMap = configManager->__SELF_NODE__.getChildren();
	// __COUTS__(10) <<"printing children content"<<__E__;
	// __COUTS__(10) <<"children map size"<<childrenMap.size() << __E__;

	// for (auto &topLevelPair : childrenMap)
	//   {
	//     __COUTS__(10)       << "Main table name '" << topLevelPair.first << "'" << __E__;
	//     auto triggerPaths = topLevelPair.second.getNode("LinkToTriggerPathsTable").getChildren();
	//     for (auto &triggerPathPair : triggerPaths)
	// 	{
	// 	  __COUTS__(10)       << "children LOOP" << __E__;
	// 	  __COUTS__(10)       << "children Path '" << triggerPathPair.first << "'" << __E__;
	// 	}
	//   }
	// __COUTS__(10) <<"children map printed..." <<__E__;

	// for (auto &topLevelPair : childrenMap)
	//   {
	//     __COUTS__(10)       << "top LOOP" << __E__;
	//     __COUTS__(10)       << "Top Level '" << topLevelPair.first << "'" << __E__;
	//     // triggerFclFile << "Top Level '" << topLevelPair.first << "'" << __E__;
	//     // triggerFclFile << "Node name '" << topLevelPair.second << "'" << __E__;

	//     auto triggerPaths = topLevelPair.second.getNode("LinkToTriggerPathsTable").getChildren();
	//     size_t children_map_size = triggerPaths.size();
	//     size_t counter(0);
	//     std::vector<int> list_of_pathIDs;

	//     for (auto &triggerPathPair : triggerPaths)
	// 	{
	// 	  __COUTS__(10)       << "internal LOOP" << __E__;
	// 	  __COUTS__(10)       << "Trigger Path '" << triggerPathPair.first << "'" << __E__;
	// 	  __COUTS__(10)       << "Trigger Name '" << triggerPathPair.second.getNode("TriggerName").getValue() << "'" << __E__;

	// 	  std::string trigger_status = triggerPathPair.second.getNode("Status").getValue();
	// 	  if (trigger_status == "Off"){
	// 	    __COUTS__(10)       << "Trigger status is Off" << __E__;
	// 	    continue;
	// 	  }
	// 	  ots::ConfigurationTree singlePath = triggerPathPair.second.getNode("LinkToTriggerTable");
	// 	  __COUTS__(10)       << "singlePath : " << singlePath << __E__;
	// 	  __COUTS__(10)       << "singlePath.isDisconnected : " << singlePath.isDisconnected() << __E__;
	// 	  // __COUTS__(10)       << "singlePath.getNode : " << StringMacros::vectorToString(singlePath.getChildrenNames()) << __E__;
	// 	  // __COUTS__(10)       << "singlePath.getConfigurationManager " << singlePath.getConfigurationManager() << __E__;
	// 	  // __COUTS__(10)       << "singlePath.getConfigurationManager()->getTableByName " << singlePath.getConfigurationManager()->getTableByName("TriggerParameterTable") << __E__;

	// 	  std::string  triggerType = triggerPathPair.second.getNode("TriggerType").getValue<std::string>();
	// 	  int          pathID      = triggerPathPair.second.getNode("PathID").getValue<int>();

	// 	  __COUTS__(10)       << "Trigger Type '" << triggerType << "'" << __E__;

	// 	  //create the fcl housing the trigger-path configurations
	// 	  epilogName = trigEpilogsDir + "/" + triggerPathPair.first + ".fcl";
	// 	  allPathsFile << "#include \"Trigger_epilogs/" << triggerPathPair.first<<".fcl\"" << __E__;
	// 	  //we need to append the line where we instantiate the given TriggerPath
	// 	  //	  allPathsFile << "art.physics." << triggerPathPair.first  << "_trigger  : [ @sequence::Trigger.paths."<< triggerPathPair.first<< " ]\n" << __E__;
	// 	  // allPathsFile << "art.physics." << triggerPathPair.first  << "_trigger  : [ makeSD, CaloDigiMaker, @sequence::Trigger.paths."<< triggerPathPair.first<< " ]\n" << __E__;
	// 	  //allPathsFile << "art.physics." << triggerPathPair.first  << "_trigger  : [ dtcEventVerifier, artFragFromDTCEvents, makeSD, @sequence::Trigger.paths."<< triggerPathPair.first<< " ]" << __E__;
	// 	  allPathsFile << "art.physics." << triggerPathPair.first  << "_trigger  : [ artFragFromDTCEvents, makeSD, @sequence::Trigger.paths."<< triggerPathPair.first<< " ]" << __E__;
	// 	  allPathsFile << "art.physics.trigger_paths["<< pathID <<"] : " << triggerPathPair.first  << "_trigger \n"<< __E__;

	// 	  epilogFclFile.open(epilogName.c_str());

	// 	  //check if the pathID is already usd by another trigger chain
	// 	  for (auto & id : list_of_pathIDs){
	// 	    if (id == pathID){
	// 	      __SS__ << "Attempt to use twice the same PathID : "<< pathID << std::endl;
	// 	      __COUT_ERR__ << ss.str() << std::endl;
	// 	      __SS_THROW__;
	// 	    }
	// 	  }

	// 	  //create the directory that will house all the epilogs of a given triggerPath
	// 	  std::string               singlePathEpilogsDir, singlePathPairFclName;
	// 	  //		singlePathEpilogsDir = "%sTrigger_epilogs/%s", (ARTDAQ_FCL_PATH).c_str(), triggerPathPair.first.c_str());
	// 	  singlePathEpilogsDir = ARTDAQ_FCL_PATH + "Trigger_epilogs/" + triggerPathPair.first;
	// 	  mkdir(singlePathEpilogsDir.c_str(), 0755);
	// 	  __COUTS__(10)       << "single path epilogs dir " << singlePathEpilogsDir << __E__;

	// 	  //set the general prescale factor at the beginning of the path
	// 	  createPrescaleEpilog         (epilogFclFile, singlePathEpilogsDir, triggerPathPair.first, triggerPathPair.second); //singlePath);

	// 	  if (triggerType == "TrackSeed")
	// 	    {
	// 	      //to set up the Tracking filters we need to loop over the children of the corresponding node
	// 	      createTrackingFiltersEpilog  (epilogFclFile, singlePathEpilogsDir, triggerPathPair.first, singlePath);
	// 	    }
	// 	  else if (triggerType == "Helix")
	// 	    {
	// 	      singlePath.getNode("LinkToDigiFilterParameterTable");
	// 	      createHelixFiltersEpilog  (epilogFclFile, singlePathEpilogsDir, triggerPathPair.first, singlePath);
	// 	    }
	// 	  else if (triggerType == "DigiCount")
	// 	    {
	// 	      createDigiCountFiltersEpilog  (epilogFclFile, singlePathEpilogsDir, triggerPathPair.first, singlePath);
	// 	    }

	// 	  __COUTS__(10) <<" closing epilogFclFile" <<__E__;

	// 	  epilogFclFile.close();
	// 	  __COUTS__(10) << "epilogFclFile closed... " << __E__;

	// 	  ++counter;
	// 	  __COUTS__(10) <<"counter = " << counter <<", map-size = " << children_map_size <<__E__;
	// 	  // if (counter == children_map_size-2) break;
	// 	}//end loop over triggerPathPair

	//     __COUTS__(10) <<" end of main LOOP" <<__E__;

	//   }//end loop over topLevelPair

	// __COUTS__(10) << "loop completed closed... " << __E__;

	// triggerFclFile.close();
	// __COUTS__(10) << "triggerFclFile closed... " << __E__;
}

//----------------------------------------------------------------------------------------------------
// this function creates a string usde to set the module names in a given trigPath
//----------------------------------------------------------------------------------------------------
std::string TriggerConfigTable::GetModuleNameFromPath(std::string& TrigPath)
{
	std::string del("_");
	std::string name(TrigPath);
	std::string newName = "";
	__COUTS__(10) << "TrigPath = " << TrigPath << __E__;

	auto pos = name.find(del);
	if(pos == std::string::npos)
	{
		newName = name;
	}
	else
	{
		newName = name.substr(0, pos);
		name.erase(0, pos + del.length());
		__COUTS__(10) << "newName = " << newName << ", name = " << name << __E__;

		//while(name.length() >0)
		do
		{
			auto        pos   = name.find(del);
			std::string token = name.substr(0, pos);
			if((pos == std::string::npos) && (name.length() > 0))
			{
				token = name;
				name  = "";
			}
			if((newName != "") &&
			   (token.find("timing") ==
			    std::string::npos))  //convert to upper case the first letter
			{
				token[0] = token[0] - 32;
				newName += token;
			}
			name.erase(0, pos + del.length());
			__COUTS__(10) << "[while ] newName = " << newName << ", name = " << name << __E__;

		} while(name.length() > 0);  //name.find(del) != std::string::npos);
	}
	return newName;
}

DEFINE_OTS_TABLE(TriggerConfigTable)
