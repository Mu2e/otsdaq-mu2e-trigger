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
	void        init						(ConfigurationManager* configManager);
};
// clang-format on
}  // namespace ots
#endif
