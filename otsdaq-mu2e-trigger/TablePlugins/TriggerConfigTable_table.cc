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
	__COUTS__(10) << "*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*" << __E__;
	__COUTS__(10) << configManager->__SELF_NODE__ << __E__;

	isFirstAppInContext_ = configManager->isOwnerFirstAppInContext();

	__COUTV__(isFirstAppInContext_);
	if(!isFirstAppInContext_)
		return;
} //end init()

//========================================================================================================================
void TriggerConfigTable::initPrereqsForARTDAQ(const ConfigurationManager* configManager)
{	
	__COUTS__(50) << "initPrereqsForARTDAQ()" << __E__;
	__COUTS__(51) << StringMacros::stackTrace() << __E__;
	
	bool needToGenerate = false;
	//lock for scope to check if need to generate in this thread
	std::lock_guard<std::mutex> lock(prereqsGeneratedMutex_);
	if(!prereqsGenerated_)
		needToGenerate = true;
		
	
	if(!needToGenerate) //then wait for generation
	{
		__COUTS__(10) << "initPrereqsForARTDAQ() already generated!" << __E__;
		return; //done!
	}

	__COUTS__(10) << "initPrereqsForARTDAQ() generating!" << __E__;
	__COUTS__(11) << StringMacros::stackTrace() << __E__;

	
	//make directory just in case
	mkdir((ARTDAQ_FCL_PATH).c_str(), 0755);

	std::string trigEpilogsDir;
	std::string fcl_dir = "TriggerEpilogs";
	trigEpilogsDir      = ARTDAQ_FCL_PATH + fcl_dir;
	mkdir(trigEpilogsDir.c_str(), 0755);

	auto childrenMap = configManager->__SELF_NODE__.getChildren();
	__COUTS__(10) << "children map size" << childrenMap.size() << __E__;

	if(childrenMap.empty())
	{
		__SS__ << "There is no record in the table '" << configManager->__SELF_NODE__.getTableName() 
			<< "' - the first record is required to define the Trigger Menu document and tag!" << __E__;
		__SS_THROW__;
	}

	auto& topLevelPair = childrenMap.at(0);
	__COUTS__(10) << "Main table name '" << topLevelPair.first << "'" << __E__;

	//now download from MONGO-Db the trigger table to be used
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
		     triggerTableName[c] == '_' || triggerTableName[c] == '-'))
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

	std::string getTableFromMongoDb = "otsdaq_load_json_document ";
	getTableFromMongoDb +=
	    triggerTableName + " " + triggerTableVersion + " " + outputFileName;
	__COUTS__(10) << "otsdaq_load_json command: " << getTableFromMongoDb << __E__;

	std::string laodJsonResult = StringMacros::exec(getTableFromMongoDb.c_str());
	__COUTVS__(10, laodJsonResult.size());
	__COUT_MULTI__(10, laodJsonResult);


	std::string menuFile = " -mf " + outputFileName;
	std::string output   = " -o " + trigEpilogsDir;
	std::string evtMode  = " -evtMode all";

	std::string command =
	    "python generateMenuFromJSON.py";
	command += menuFile + output + evtMode;
	__COUTS__(10) << "generateMenuFromJSON command: " << command << __E__;
	std::string genMenuResult = StringMacros::exec(command.c_str());
	__COUTVS__(10, genMenuResult.size());
	__COUT_MULTI__(10, genMenuResult);
	if(1 || genMenuResult == "")
	{
		command =
			"python $OTSDAQ_DIR/python/generateMenuFromJSON.py";
		command += menuFile + output + evtMode;
		__COUTS__(10) << "generateMenuFromJSON command tk2: " << command << __E__;
		genMenuResult = StringMacros::exec(command.c_str());
		__COUTVS__(10, genMenuResult.size());
		__COUT_MULTI__(10, genMenuResult);
	}

	prereqsGenerated_ = true;
}  //end initPrereqsForARTDAQ()

DEFINE_OTS_TABLE(TriggerConfigTable)
