#include "otsdaq-mu2e-trigger/TablePluginDataFormats/TriggerConfigTable.h"
#include "otsdaq/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq/Macros/TablePluginMacros.h"

#include <stdio.h>
#include <sys/stat.h>  //for mkdir
#include <fstream>     // std::fstream
#include <iostream>
#include <regex>

using namespace ots;

#define ARTDAQ_FCL_PATH std::string(getenv("OTS_SCRATCH")) + "/TriggerConfigurations/"
#define ARTDAQ_FILE_PREAMBLE "boardReader"

// helpers
#define OUT out << tabStr << commentStr
#define PUSHTAB tabStr += "\t"
#define POPTAB tabStr.resize(tabStr.size() - 1)
#define PUSHCOMMENT commentStr += "# "
#define POPCOMMENT commentStr.resize(commentStr.size() - 2)

//========================================================================================================================
TriggerConfigTable::TriggerConfigTable(void) : TableBase("TriggerConfigTable")
{
	//////////////////////////////////////////////////////////////////////
	// WARNING: the names used in C++ MUST match the Table INFO  //
	//////////////////////////////////////////////////////////////////////
	__COUTS__(10) << "[TriggerConfigTable::TriggerConfigTable] Initializing the "
	                 "TriggerConfigTable plugin..."
	              << __E__;
	//  exit(0);
	__COUTS__(10) << StringMacros::stackTrace() << __E__;
}  // end constructor

//========================================================================================================================
TriggerConfigTable::~TriggerConfigTable(void) {}

//========================================================================================================================
void TriggerConfigTable::init(ConfigurationManager* configManager)
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

	__COUTS__(10) << "*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*&*" << std::endl;
	__COUTS__(10) << configManager->__SELF_NODE__ << std::endl;

	auto childrenMap = configManager->__SELF_NODE__.getChildren();

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

//----------------------------------------------------------------------------------------------------
// this function creates a string usde to set the module names in a given
// trigPath
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

		// while(name.length() >0)
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
			    std::string::npos))  // convert to upper case the first letter
			{
				token[0] = token[0] - 32;
				newName += token;
			}
			name.erase(0, pos + del.length());
			__COUTS__(10) << "[while ] newName = " << newName << ", name = " << name
			              << __E__;

		} while(name.length() > 0);  // name.find(del) != std::string::npos);
	}
	return newName;
}

DEFINE_OTS_TABLE(TriggerConfigTable)
