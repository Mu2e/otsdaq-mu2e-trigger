#ifndef _ots_TriggerConfigTable_h_
#define _ots_TriggerConfigTable_h_

#include <fstream>  // std::fstream
#include <iostream>
#include <mutex>
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
	// menu document name and tag used for the last generation; regenerate whenever
	// either changes (e.g. a new triggerMenuTag selected in the Trigger Menu Editor
	// between Configures). Empty until the first generation in this process.
	std::string			prereqsGeneratedDoc_;
	std::string			prereqsGeneratedTag_;
};
// clang-format on
}  // namespace ots
#endif
