#include "otsdaq-mu2e-trigger/TablePlugins/OfflineDatabaseTable.h"
#include "otsdaq/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq/Macros/TablePluginMacros.h"

#include <stdio.h>
#include <sys/stat.h>  //for mkdir
#include <fstream>     // std::fstream
#include <iostream>
#include <regex>

using namespace ots;

//Note: this could be used to inspect the path of the trigger offline db (declaration example from otsdaq-mu2e-calorimeter/otsdaq-mu2e-calorimeter/TablePlugins/SubsystemCalorimeterParametersTable_table.cc)
// const std::string SubsystemCalorimeterParametersTable::PATH_TO_TRIGGER_OFFLINE_DB = getenv("PATH_TO_TRIGGER_OFFLINE_DB") ? getenv("PATH_TO_TRIGGER_OFFLINE_DB") : "";

#define ARTDAQ_FCL_PATH std::string(getenv("OTS_SCRATCH")) + "/TriggerConfigurations/"

#define OFFLINE_DBSERVICE_VERBOSE \
	std::string(                  \
	    getenv("OFFLINE_DBSERVICE_VERBOSE") ? getenv("OFFLINE_DBSERVICE_VERBOSE") : "0")

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
	if(!outFile.is_open())
	{
		__SS__ << "Offline Database output file could not be opened at path: "
		       << outFilename << __E__;
		__SS_THROW__;
	}
	createTriggerFcl(outFile, configManager);
	outFile.close();

}  // end init()

//========================================================================================================================
void OfflineDatabaseTable::createTriggerFcl(std::ofstream&        outFile,
                                            ConfigurationManager* configManager)
{
	auto childrenMap = configManager->__SELF_NODE__.getChildren();

	std::string tabStr     = "";
	std::string commentStr = "";

	// outFile << tabStr << "services : {" << std::endl;
	// PUSHTAB;
	outFile << tabStr << "art.services.DbService : {" << std::endl;
	PUSHTAB;
	outFile << tabStr << "purpose :  EMPTY" << std::endl;
	outFile << tabStr << "version : v0   # ignored" << std::endl;
	outFile << tabStr << "dbName : \"mu2e_conditions_prd\"  # ignored" << std::endl;

	// Create textFile array with first_second.txt format
	outFile << tabStr << "textFile : [";
	bool first = true;
	for(const auto& pair : childrenMap)
	{
		if(!first)
		{
			outFile << ", ";
		}
		outFile << "\"" << pair.first << "_"
		        << pair.second.getNode("CID").getValue<std::string>() << ".txt\"";
		first = false;
	}
	outFile << "]   # provide everything needed" << std::endl;

	outFile << tabStr << "verbose : " << OFFLINE_DBSERVICE_VERBOSE << std::endl;
	POPTAB;
	outFile << tabStr << "}" << std::endl;
	// POPTAB;
	// outFile << tabStr << "}" << std::endl;
}  //end createTriggerFcl()

DEFINE_OTS_TABLE(OfflineDatabaseTable)
