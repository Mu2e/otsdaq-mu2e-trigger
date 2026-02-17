#ifndef _ots_TriggerConfigTable_h_
#define _ots_TriggerConfigTable_h_

#include <fstream>  // std::fstream
#include <iostream>
#include <string>
#include "otsdaq/ConfigurationInterface/ConfigurationManager.h"
#include "otsdaq/TableCore/TableBase.h"

namespace ots
{

// clang-format off
class TriggerConfigTable : public TableBase
{
  public:
	TriggerConfigTable(void);
	virtual ~TriggerConfigTable(void);

	//Methods
	void        init						(ConfigurationManager* configManager)  override;
	void		initPrereqsForARTDAQ		(const ConfigurationManager* configManager) override;
	std::string getStructureAsJSON			(const ConfigurationManager* configManager) override;

private:
	void		generateTriggerEpilogs		(const std::string& triggerTableName,
											 const std::string& triggerTableVersion);
	std::string getPhysicsMenuJsonFileName	(const std::string& triggerTableName,
											 const std::string& triggerTableVersion);
	void		downloadTriggerMenuFromMongoDB	(const std::string& triggerTableName,
											 const std::string& triggerTableVersion,
											 const std::string& outputFileName);

	std::mutex			prereqsGeneratedMutex_;
	bool			 	prereqsGenerated_ = false;
};
// clang-format on
}  // namespace ots
#endif
