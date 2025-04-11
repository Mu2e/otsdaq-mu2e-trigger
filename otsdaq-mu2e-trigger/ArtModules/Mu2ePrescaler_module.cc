// ======================================================================
//
// Mu2ePrescaler_plugin
//
// ======================================================================

#include "art/Framework/Core/SharedFilter.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "fhiclcpp/types/Atom.h"

#include "artdaq-core-mu2e/Overlays/DTCEventFragment.hh"
#include "artdaq-core-mu2e/Overlays/FragmentType.hh"
#include "artdaq/DAQdata/Globals.hh"

#include "artdaq-core/Data/ContainerFragment.hh"
#include "artdaq-core/Data/Fragment.hh"

#include <time.h>
#include <mutex>
#include <string>
#include <vector>

#include "trace.h"
#define TRACE_NAME "Mu2ePrescaler"

namespace art
{
class Mu2ePrescaler;
}
using namespace fhicl;
using art::Mu2ePrescaler;

// ======================================================================

class art::Mu2ePrescaler : public SharedFilter
{
  public:
	struct Config
	{
		Atom<size_t> prescaleFactor{Name("prescaleFactor")};
		Atom<size_t> prescaleOffset{Name("prescaleOffset")};
	};

	using Parameters = Table<Config>;
	explicit Mu2ePrescaler(Parameters const&, ProcessingFrame const&);

  private:
	bool filter(Event& event, ProcessingFrame const&) override;

	size_t count_{};
	// Accept one in n events.
	size_t const n_;
	// An offset is allowed--i.e. sequence of events does not have to
	// start at first event.
	size_t const                    offset_;
	artdaq::Fragment::fragment_id_t min_frag_id_;
	std::mutex                      mutex_{};

};  // Mu2ePrescaler

// ======================================================================

Mu2ePrescaler::Mu2ePrescaler(Parameters const& config, ProcessingFrame const&)
    : SharedFilter{config}
    , n_{config().prescaleFactor()}
    , offset_{config().prescaleOffset()}
    , min_frag_id_(-1)
{
	async<InEvent>();
}

bool Mu2ePrescaler::filter(Event& event, ProcessingFrame const&)
{
	// The combination of incrementing, modulo dividing, and equality
	// comparing must be synchronized.  Changing count_ to the type
	// std::atomic<size_t> would not help since the entire combination
	// of operations must be atomic.  Using a mutex here is cheaper than
	// calling serialize(), since that will also serialize any of the
	// module-level service callbacks invoked before and after this
	// function is called.

	std::lock_guard lock{mutex_};

	std::vector<art::Handle<artdaq::Fragments>> fragmentHandles;
	fragmentHandles = event.getMany<std::vector<artdaq::Fragment>>();

	// First event, find the lowest fragment ID in the event
	if(count_ == 0)
	{
		for(const auto& handle : fragmentHandles)
		{
			if(!handle.isValid() || handle->empty())
			{
				continue;
			}

			if(handle->front().type() == mu2e::FragmentType::DTCEVT)
			{
				auto       frag    = handle->front();
				const auto frag_id = frag.fragmentID();
				if(min_frag_id_ > frag_id)
				{
					min_frag_id_ = frag_id;
				}
			}
		}
	}

	double evt_time(0);

	for(const auto& handle : fragmentHandles)
	{
		if(!handle.isValid() || handle->empty())
		{
			continue;
		}

		if(handle->front().type() == mu2e::FragmentType::DTCEVT &&
		   handle->front().fragmentID() == min_frag_id_)
		{
			auto frag = handle->front();
			auto time = frag.getLatency(true);
			evt_time  = time.tv_sec + static_cast<double>(time.tv_nsec) / 1.e9;
			break;
		}
	}

	std::string tag = moduleDescription().moduleLabel() + " latency";
	TLOG(TLVL_DEBUG + 25) << " Latency metric for " << tag.c_str() << ": "
	                      << evt_time * 1.e6 << " us\n";
	metricMan->sendMetric(tag, evt_time, "s", 3, artdaq::MetricMode::Average);

	++count_;
	return count_ % n_ == offset_;
}

DEFINE_ART_MODULE(Mu2ePrescaler)
