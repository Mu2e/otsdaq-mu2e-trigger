#include "otsdaq-mu2e-trigger/TablePluginDataFormats/OfflineDatabaseTable.h"
#include "otsdaq/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq/Macros/TablePluginMacros.h"
// #include "otsdaq/tools/otsdaq_load_json_document.cc"

#include <stdio.h>
#include <sys/stat.h>  //for mkdir
#include <fstream>     // std::fstream
#include <iostream>
#include <regex>

using namespace ots;

//Note: this could be used to inspect the path of the trigger offline db (declaration example from otsdaq-mu2e-calorimeter/otsdaq-mu2e-calorimeter/TablePlugins/SubsystemCalorimeterParametersTable_table.cc)
// const std::string SubsystemCalorimeterParametersTable::PATH_TO_TRIGGER_OFFLINE_DB = getenv("PATH_TO_TRIGGER_OFFLINE_DB") ? getenv("PATH_TO_TRIGGER_OFFLINE_DB") : "";

#define ARTDAQ_FCL_PATH std::string(getenv("OTS_SCRATCH")) + "/TriggerConfigurations/"
#define ARTDAQ_FILE_PREAMBLE "boardReader"

// helpers
#define OUT out << tabStr << commentStr
#define PUSHTAB tabStr += "\t"
#define POPTAB tabStr.resize(tabStr.size() - 1)
#define PUSHCOMMENT commentStr += "# "
#define POPCOMMENT commentStr.resize(commentStr.size() - 2)

//========================================================================================================================
OfflineDatabaseTable::OfflineDatabaseTable(void) : TableBase("OfflineDatabaseTable")
{
	//////////////////////////////////////////////////////////////////////
	// WARNING: the names used in C++ MUST match the Table INFO  //
	//////////////////////////////////////////////////////////////////////
	__COUTS__(10) << "[OfflineDatabaseTable::OfflineDatabaseTable] Initializing the "
	                 "OfflineDatabaseTable plugin..."
	              << __E__;
	//  exit(0);
	__COUTS__(10) << StringMacros::stackTrace() << __E__;
}  // end constructor

//========================================================================================================================
OfflineDatabaseTable::~OfflineDatabaseTable(void) {}

//========================================================================================================================
void OfflineDatabaseTable::init(ConfigurationManager* configManager)
{
	isFirstAppInContext_ = configManager->isOwnerFirstAppInContext();

	__COUTV__(isFirstAppInContext_);
	if(!isFirstAppInContext_)
		return;

	// make directory just in case
	mkdir((ARTDAQ_FCL_PATH).c_str(), 0755);

	std::string trigEpilogsDir;
	std::string fcl_dir = "TriggerEpilogs";
	trigEpilogsDir      = ARTDAQ_FCL_PATH + fcl_dir;
	mkdir(trigEpilogsDir.c_str(), 0755);
	std::string outFilename = trigEpilogsDir + "/" + "OfflineDatabaseInclude.fcl";

	__COUTS__(10) << "*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*" << std::endl;
	__COUTS__(10) << configManager->__SELF_NODE__ << std::endl;

	auto childrenMap = configManager->__SELF_NODE__.getChildren();

	// Need to generate this:
	// 		services : {
	// 		  DbService : {
	// 		     purpose :  EMPTY
	// 		     version : v0   # ignored
	// 		     dbName : "mu2e_conditions_prd"  # ignored
	// 		     textFile : ["table.txt"]   # provide everything needed
	// 		     verbose : 1
	// 		  }
	// 		}

	// Process childrenMap to extract child pairs and write in required format
	std::ofstream outFile(outFilename);
	if (outFile.is_open()) {
		std::string tabStr = "";
		std::string commentStr = "";
		
		outFile << tabStr << "services : {" << std::endl;
		PUSHTAB;
		outFile << tabStr << "DbService : {" << std::endl;
		PUSHTAB;
		outFile << tabStr << "purpose :  EMPTY" << std::endl;
		outFile << tabStr << "version : v0   # ignored" << std::endl;
		outFile << tabStr << "dbName : \"mu2e_conditions_prd\"  # ignored" << std::endl;
		
		// Create textFile array with first_second.txt format
		outFile << tabStr << "textFile : [";
		bool first = true;
		for (const auto& pair : childrenMap) {
			if (!first) {
				outFile << ", ";
			}
			outFile << "\"" << pair.first << "_" << pair.second.getValue<std::string>() << ".txt\"";
			first = false;
		}
		outFile << "]   # provide everything needed" << std::endl;
		
		outFile << tabStr << "verbose : 1" << std::endl;
		POPTAB;
		outFile << tabStr << "}" << std::endl;
		POPTAB;
		outFile << tabStr << "}" << std::endl;
		outFile.close();
	}
	else 
	{
		__SS__ << "Offline Database output file could not be opened at path: " << outFilename << __E__;
		__SS_THROW__;
	}

	if(0)
	{
		// now download from MONGO-Db the trigger table to be used
		std::string getTableFromMongoDb = "otsdaq_load_json_document ";
		std::string triggerTableName =
			" " +
			childrenMap[0].second.getNode("TriggerDocName").getValue();  //" testTriggerDoc ";
		std::string triggerTableVersion =
			" " + childrenMap[0].second.getNode("TriggerConfigTag").getValue() + " ";
		std::string outputFileName = ARTDAQ_FCL_PATH + "trigger_table.json";

		__COUTS__(10) << "printing children content" << __E__;
		__COUTS__(10) << "TriggerDocName  : " << triggerTableName << __E__;
		__COUTS__(10) << "TriggerConfigTag: " << triggerTableVersion << __E__;

		getTableFromMongoDb += triggerTableName + triggerTableVersion + outputFileName;
		system(getTableFromMongoDb.c_str());

		__COUTS__(10) << StringMacros::stackTrace() << __E__;

		std::string command = "generateMenuFromJSON.py";
		std::string menuFile =
			" -mf " + outputFileName;  //"mu2e_trig_config/data/physMenu.json";
		std::string output  = " -o " + trigEpilogsDir;
		std::string evtMode = " -evtMode all";

		command += menuFile + output + evtMode;
		system(command.c_str());
	}

	// //create the fcl file
	// std::ofstream	triggerFclFile, epilogFclFile, subEpilogFclFile,
	// allPathsFile; std::string	fclFileName, skelethonName, allPathsFileName;
	// std::string	epilogName;
	// //skelethon to clone
	// //	skelethonName = Form("%s/main.fcl", (ARTDAQ_FCL_PATH).c_str());
	// skelethonName = ARTDAQ_FCL_PATH + "/main.fcl" ;
	// //file to be edited
	// //	fclFileName = Form("%s/runTriggerExample.fcl",
	// (ARTDAQ_FCL_PATH).c_str()); fclFileName =
	// ARTDAQ_FCL_PATH+"/runTriggerExample.fcl";

	// //file that will house all the includes necessary to run the trigger paths
	// allPathsFileName = trigEpilogsDir+ "/allPaths.fcl";

	// std::ifstream	mainFclFile;
	// mainFclFile   .open(skelethonName);
	// triggerFclFile.open(fclFileName);
	// allPathsFile  .open(allPathsFileName);

	// std::string	line;
	// while (std::getline(mainFclFile, line, '\n') ) triggerFclFile << line <<
	// '\n';

	// //we need to append the line where include the fcl that will contain all
	// the trigger paths triggerFclFile << "#include
	// \"Trigger_epilogs/allPaths.fcl\""<<__E__;

	// __COUTS__(10) << "*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*" << std::endl;
	// __COUTS__(10) << configManager->__SELF_NODE__ << std::endl;

	// auto childrenMap = configManager->__SELF_NODE__.getChildren();
	// __COUTS__(10) <<"printing children content"<<__E__;
	// __COUTS__(10) <<"children map size"<<childrenMap.size() << __E__;

	// for (auto &topLevelPair : childrenMap)
	//   {
	//	 __COUTS__(10)	<< "Main table name '" << topLevelPair.first << "'" <<
	//__E__; 	 auto triggerPaths =
	// topLevelPair.second.getNode("LinkToTriggerPathsTable").getChildren();
	// for (auto &triggerPathPair : triggerPaths)
	//	{
	//	  __COUTS__(10)	 << "children LOOP" << __E__;
	//	  __COUTS__(10)	 << "children Path '" << triggerPathPair.first << "'" <<
	//__E__;
	//	}
	//   }
	// __COUTS__(10) <<"children map printed..." <<__E__;

	// for (auto &topLevelPair : childrenMap)
	//   {
	//	 __COUTS__(10)	<< "top LOOP" << __E__;
	//	 __COUTS__(10)	<< "Top Level '" << topLevelPair.first << "'" << __E__;
	//	 // triggerFclFile << "Top Level '" << topLevelPair.first << "'" <<
	//__E__;
	//	 // triggerFclFile << "Node name '" << topLevelPair.second << "'" <<
	//__E__;

	//	 auto triggerPaths =
	// topLevelPair.second.getNode("LinkToTriggerPathsTable").getChildren();
	// size_t children_map_size = triggerPaths.size(); 	 size_t counter(0);
	// std::vector<int> list_of_pathIDs;

	//	 for (auto &triggerPathPair : triggerPaths)
	//	{
	//	  __COUTS__(10)	 << "internal LOOP" << __E__;
	//	  __COUTS__(10)	 << "Trigger Path '" << triggerPathPair.first << "'" <<
	//__E__;
	//	  __COUTS__(10)	 << "Trigger Name '" <<
	// triggerPathPair.second.getNode("TriggerName").getValue() << "'" << __E__;

	//	  std::string trigger_status =
	// triggerPathPair.second.getNode("Status").getValue(); 	  if
	// (trigger_status == "Off"){
	//	    __COUTS__(10)	   << "Trigger status is Off" << __E__;
	//	    continue;
	//	  }
	//	  ots::ConfigurationTree singlePath =
	// triggerPathPair.second.getNode("LinkToTriggerTable");
	//	  __COUTS__(10)	 << "singlePath : " << singlePath << __E__;
	//	  __COUTS__(10)	 << "singlePath.isDisconnected : " <<
	// singlePath.isDisconnected() << __E__;
	//	  // __COUTS__(10)	    << "singlePath.getNode : " <<
	// StringMacros::vectorToString(singlePath.getChildrenNames()) << __E__;
	//	  // __COUTS__(10)	    << "singlePath.getConfigurationManager " <<
	// singlePath.getConfigurationManager() << __E__;
	//	  // __COUTS__(10)	    <<
	//"singlePath.getConfigurationManager()->getTableByName " <<
	// singlePath.getConfigurationManager()->getTableByName("TriggerParameterTable")
	//<< __E__;

	//	  std::string  triggerType =
	// triggerPathPair.second.getNode("TriggerType").getValue<std::string>();
	// int pathID	   = triggerPathPair.second.getNode("PathID").getValue<int>();

	//	  __COUTS__(10)	 << "Trigger Type '" << triggerType << "'" << __E__;

	//	  //create the fcl housing the trigger-path configurations
	//	  epilogName = trigEpilogsDir + "/" + triggerPathPair.first + ".fcl";
	//	  allPathsFile << "#include \"Trigger_epilogs/" <<
	// triggerPathPair.first<<".fcl\"" << __E__;
	//	  //we need to append the line where we instantiate the given
	// TriggerPath
	//	  //	  allPathsFile << "art.physics." << triggerPathPair.first  <<
	//"_trigger	 : [ @sequence::Trigger.paths."<< triggerPathPair.first<< " ]\n"
	//<< __E__;
	//	  // allPathsFile << "art.physics." << triggerPathPair.first  <<
	//"_trigger  : [ makeSD, CaloDigiMaker, @sequence::Trigger.paths."<<
	// triggerPathPair.first<< " ]\n" << __E__;
	//	  //allPathsFile << "art.physics." << triggerPathPair.first  <<
	//"_trigger  : [ dtcEventVerifier, artFragFromDTCEvents, makeSD,
	//@sequence::Trigger.paths."<< triggerPathPair.first<< " ]" << __E__;
	//	  allPathsFile << "art.physics." << triggerPathPair.first  << "_trigger
	//: [ artFragFromDTCEvents, makeSD, @sequence::Trigger.paths."<<
	// triggerPathPair.first<< " ]" << __E__; 	  allPathsFile <<
	//"art.physics.trigger_paths["<< pathID <<"] : " << triggerPathPair.first  <<
	//"_trigger \n"<< __E__;

	//	  epilogFclFile.open(epilogName.c_str());

	//	  //check if the pathID is already usd by another trigger chain
	//	  for (auto & id : list_of_pathIDs){
	//	    if (id == pathID){
	//	      __SS__ << "Attempt to use twice the same PathID : "<< pathID <<
	// std::endl;
	//	      __COUT_ERR__ << ss.str() << std::endl;
	//	      __SS_THROW__;
	//	    }
	//	  }

	//	  //create the directory that will house all the epilogs of a given
	// triggerPath 	  std::string		    singlePathEpilogsDir,
	// singlePathPairFclName;
	//	  //		singlePathEpilogsDir = "%sTrigger_epilogs/%s",
	//(ARTDAQ_FCL_PATH).c_str(), triggerPathPair.first.c_str());
	//	  singlePathEpilogsDir = ARTDAQ_FCL_PATH + "Trigger_epilogs/" +
	// triggerPathPair.first; 	  mkdir(singlePathEpilogsDir.c_str(), 0755);
	//	  __COUTS__(10)	 << "single path epilogs dir " << singlePathEpilogsDir
	//<< __E__;

	//	  //set the general prescale factor at the beginning of the path
	//	  createPrescaleEpilog	       (epilogFclFile, singlePathEpilogsDir,
	// triggerPathPair.first, triggerPathPair.second); //singlePath);

	//	  if (triggerType == "TrackSeed")
	//	    {
	//	      //to set up the Tracking filters we need to loop over the children
	// of the corresponding node 	      createTrackingFiltersEpilog
	// (epilogFclFile, singlePathEpilogsDir, triggerPathPair.first, singlePath);
	//	    }
	//	  else if (triggerType == "Helix")
	//	    {
	//	      singlePath.getNode("LinkToDigiFilterParameterTable");
	//	      createHelixFiltersEpilog	(epilogFclFile, singlePathEpilogsDir,
	// triggerPathPair.first, singlePath);
	//	    }
	//	  else if (triggerType == "DigiCount")
	//	    {
	//	      createDigiCountFiltersEpilog  (epilogFclFile,
	// singlePathEpilogsDir, triggerPathPair.first, singlePath);
	//	    }

	//	  __COUTS__(10) <<" closing epilogFclFile" <<__E__;

	//	  epilogFclFile.close();
	//	  __COUTS__(10) << "epilogFclFile closed... " << __E__;

	//	  ++counter;
	//	  __COUTS__(10) <<"counter = " << counter <<", map-size = " <<
	// children_map_size <<__E__;
	//	  // if (counter == children_map_size-2) break;
	//	}//end loop over triggerPathPair

	//	 __COUTS__(10) <<" end of main LOOP" <<__E__;

	//   }//end loop over topLevelPair

	// __COUTS__(10) << "loop completed closed... " << __E__;

	// triggerFclFile.close();
	// __COUTS__(10) << "triggerFclFile closed... " << __E__;
}

DEFINE_OTS_TABLE(OfflineDatabaseTable)
