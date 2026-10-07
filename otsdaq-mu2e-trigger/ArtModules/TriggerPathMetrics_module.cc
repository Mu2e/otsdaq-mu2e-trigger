// ======================================================================
//
// TriggerPathMetrics_module
//
// Reports the trigger path decisions of every event to the artdaq
// MetricManager so the trigger rate can be monitored per path.
// Path naming follows Offline/Mu2eUtilities TriggerResultsNavigator
// ("bit:name" entries of the trigger_paths list), but the path list is
// looked up once per TriggerResults parameter set and cached, instead of
// scanning the ParameterSetRegistry on every event.
//
// All TriggerResults products in the event are read (one per process,
// e.g. from the different event builders). Only products whose process
// name contains processNameTag (default "EvB") are used, the others are
// rejected. An empty processNameTag uses all products.
//
// For each path <name> one metric "<metricPrefix>.<name>" is sent per
// TriggerResults product with value 1 (accepted) or 0 (not accepted), in
// Rate and Accumulate mode, giving the path rate (events/s) and the
// number of accepted events per reporting interval. Sending 0 keeps
// paths that did not fire visible at 0 Hz.
//
// Additional metrics:
//   <metricPrefix>.AllEvents             every event seen
//   <metricPrefix>.AnyTriggerPath        events accepted by at least one path
//                                        not matching ignorePaths
//   <metricPrefix>.MissingTriggerResults events without a matching TriggerResults
//
// Example:
//   triggerPathMetrics : {
//     module_type  : TriggerPathMetrics
//     processNameTag : "EvB"
//   }
//
// ======================================================================

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Provenance.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Sequence.h"

#include "artdaq/DAQdata/Globals.hh"

#include <map>
#include <string>
#include <vector>

#include "trace.h"
#define TRACE_NAME "TriggerPathMetrics"

namespace mu2e
{
class TriggerPathMetrics;
}

// ======================================================================

class mu2e::TriggerPathMetrics : public art::EDAnalyzer
{
  public:
	struct Config
	{
		using Name    = fhicl::Name;
		using Comment = fhicl::Comment;
		fhicl::Atom<std::string> processNameTag{
		    Name("processNameTag"),
		    Comment("Only use TriggerResults of processes whose name contains this tag "
		            "(empty: use all processes)"),
		    "EvB"};
		fhicl::Atom<std::string> metricPrefix{
		    Name("metricPrefix"), Comment("Prefix of all metric names"), "trigger"};
		fhicl::Atom<int> metricLevel{
		    Name("metricLevel"), Comment("artdaq metric level of the metrics"), 3};
		fhicl::Sequence<std::string> ignorePaths{
		    Name("ignorePaths"),
		    Comment(
		        "Paths containing any of these strings are excluded from AnyTriggerPath"),
		    std::vector<std::string>{}};
	};

	using Parameters = art::EDAnalyzer::Table<Config>;
	explicit TriggerPathMetrics(Parameters const& config);

  private:
	// path list of one trigger menu, indexed like the TriggerResults entries
	struct PathMenu
	{
		std::vector<std::string> metricNames;
		std::vector<bool>        countsAsTrigger;  // false for paths matching ignorePaths
	};

	void            analyze(art::Event const& event) override;
	PathMenu const& pathMenu(fhicl::ParameterSetID const& parameterSetId);
	void            sendEventCount(std::string const& metricName, int value);

	std::string const              processNameTag_;
	std::string const              metricPrefix_;
	int const                      metricLevel_;
	std::vector<std::string> const ignorePaths_;

	std::map<fhicl::ParameterSetID, PathMenu> pathMenus_;
};

// ======================================================================

mu2e::TriggerPathMetrics::TriggerPathMetrics(Parameters const& config)
    : art::EDAnalyzer{config}
    , processNameTag_{config().processNameTag()}
    , metricPrefix_{config().metricPrefix()}
    , metricLevel_{config().metricLevel()}
    , ignorePaths_{config().ignorePaths()}
{
}

mu2e::TriggerPathMetrics::PathMenu const& mu2e::TriggerPathMetrics::pathMenu(
    fhicl::ParameterSetID const& parameterSetId)
{
	auto menuIterator = pathMenus_.find(parameterSetId);
	if(menuIterator != pathMenus_.end())
		return menuIterator->second;

	std::vector<std::string> pathEntries;
	fhicl::ParameterSet      triggerParameterSet;
	if(fhicl::ParameterSetRegistry::get(parameterSetId, triggerParameterSet))
		pathEntries =
		    triggerParameterSet.get<std::vector<std::string>>("trigger_paths", {});
	else
		TLOG(TLVL_WARNING) << "TriggerResults parameter set " << parameterSetId
		                   << " not found in the registry, paths are reported by index";

	PathMenu& menu = pathMenus_[parameterSetId];
	for(auto const& pathEntry : pathEntries)
	{
		// entries are "bit:name"
		auto const        delimiterPosition = pathEntry.find(':');
		std::string const pathName          = delimiterPosition == std::string::npos
		                                          ? pathEntry
		                                          : pathEntry.substr(delimiterPosition + 1);
		menu.metricNames.push_back(metricPrefix_ + "." + pathName);

		bool countsAsTrigger = true;
		for(auto const& ignoreTag : ignorePaths_)
			countsAsTrigger &= pathName.find(ignoreTag) == std::string::npos;
		menu.countsAsTrigger.push_back(countsAsTrigger);
	}
	TLOG(TLVL_INFO) << "Trigger menu " << parameterSetId << " has "
	                << menu.metricNames.size() << " paths";
	return menu;
}

void mu2e::TriggerPathMetrics::sendEventCount(std::string const& metricName, int value)
{
	metricMan->sendMetric(metricName,
	                      value,
	                      "events",
	                      metricLevel_,
	                      artdaq::MetricMode::Rate | artdaq::MetricMode::Accumulate);
}

void mu2e::TriggerPathMetrics::analyze(art::Event const& event)
{
	if(metricMan == nullptr)
		return;

	sendEventCount(metricPrefix_ + ".AllEvents", 1);

	bool foundTriggerResults = false;
	bool anyTriggerAccepted  = false;
	for(auto const& triggerResultsHandle : event.getMany<art::TriggerResults>())
	{
		if(!triggerResultsHandle.isValid())
			continue;
		if(triggerResultsHandle.provenance()->processName().find(processNameTag_) ==
		   std::string::npos)
			continue;
		foundTriggerResults = true;

		art::TriggerResults const& triggerResults = *triggerResultsHandle;
		PathMenu const&            menu = pathMenu(triggerResults.parameterSetID());
		for(unsigned pathIndex = 0; pathIndex < triggerResults.size(); ++pathIndex)
		{
			bool const accepted = triggerResults.accept(pathIndex);
			if(pathIndex < menu.metricNames.size())
			{
				sendEventCount(menu.metricNames[pathIndex], accepted ? 1 : 0);
				anyTriggerAccepted |= accepted && menu.countsAsTrigger[pathIndex];
			}
			else  // path not in the menu, report by index
			{
				sendEventCount(metricPrefix_ + ".Index" + std::to_string(pathIndex),
				               accepted ? 1 : 0);
				anyTriggerAccepted |= accepted;
			}
		}
	}

	if(!foundTriggerResults)
		TLOG(TLVL_DEBUG + 5) << "No matching TriggerResults in event " << event.id();
	sendEventCount(metricPrefix_ + ".MissingTriggerResults", foundTriggerResults ? 0 : 1);
	sendEventCount(metricPrefix_ + ".AnyTriggerPath", anyTriggerAccepted ? 1 : 0);
}

DEFINE_ART_MODULE(mu2e::TriggerPathMetrics)
