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
class OfflineDatabaseTable : public TableBase
{
  public:
	OfflineDatabaseTable(void);
	virtual ~OfflineDatabaseTable(void);

	// Methods
	void 				init							(ConfigurationManager* configManager);
	void 				createTriggerFcl				(std::ostream& fclFileOut, const ConfigurationManager* configManager) const;
	std::string	     	getFclValueForARTDAQ			(const ConfigurationManager* configManager, const std::string& field = "") const override;
};
// clang-format on
}  // namespace ots
#endif
