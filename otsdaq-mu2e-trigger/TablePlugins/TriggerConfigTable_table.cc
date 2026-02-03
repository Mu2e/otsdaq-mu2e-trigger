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
	std::string triggerTableName =
	    topLevelPair.second.getNode("TriggerDocName").getValue();  //" testTriggerDoc ";
	std::string triggerTableVersion =
	    topLevelPair.second.getNode("TriggerConfigTag").getValue();  //" 6 ";
	std::string outputFileName = ARTDAQ_FCL_PATH + "/physMenu.json";

	//sanitize values for system call
	for(size_t c = 0; c < triggerTableName.size(); ++c)
		if(!((triggerTableName[c] >= 'a' && triggerTableName[c] <= 'z') ||
		     (triggerTableName[c] >= 'A' && triggerTableName[c] <= 'Z') ||
		     (triggerTableName[c] >= '0' && triggerTableName[c] <= '9') ||
		     triggerTableName[c] >= '_' || triggerTableName[c] <= '-'))
		{
			__SS__ << "Illegal character found in triggerTableName '" << triggerTableName
			       << "' ... only alpha-numeric, dashes, and underscores allowed."
			       << __E__;
			__SS_THROW__;
		}

	for(size_t c = 0; c < triggerTableVersion.size(); ++c)
		if(!((triggerTableVersion[c] >= '0' && triggerTableVersion[c] <= '9')))
		{
			__SS__ << "Illegal character found in triggerTableVersion '"
			       << triggerTableVersion << "' ... only numeric characters allowed."
			       << __E__;
			__SS_THROW__;
		}

	getTableFromMongoDb +=
	    triggerTableName + " " + triggerTableVersion + " " + outputFileName;
	__COUTS__(10) << "otsdaq_load_json command: " << getTableFromMongoDb << __E__;

	int statusCode = system(getTableFromMongoDb.c_str());
	__COUTVS__(10, statusCode);

	__COUTS__(10) << StringMacros::stackTrace() << __E__;

	std::string command =
	    "python ${SPACK_ENV}/mu2e-trig-config/python/generateMenuFromJSON.py";
	std::string menuFile = " -mf " + outputFileName;
	std::string output   = " -o " + trigEpilogsDir;
	std::string evtMode  = " -evtMode all";

	command += menuFile + output + evtMode;
	statusCode = system(command.c_str());
	__COUTVS__(10, statusCode);

}  //end init()

DEFINE_OTS_TABLE(TriggerConfigTable)
