/** filter_pool.h                                 -*- C++ -*-
    Rémi Attab, 23 Jul 2013
    Copyright (c) 2013 Datacratic.  All rights reserved.

    blah

*/

#pragma once

#include "rtbkit/common/filter.h"
#include "soa/gc/gc_lock.h"

#include <atomic>
#include <vector>
#include <memory>
#include <string>


namespace Datacratic {

struct EventRecorder;

} // namespace Datacratic

namespace RTBKIT {

struct BidRequest;
struct ExchangeConnector;
struct AgentConfig;


/******************************************************************************/
/* FILTER POOL                                                                */
/******************************************************************************/

struct FilterPool
{
    FilterPool();
    ~FilterPool();

    void init(EventRecorder* events = nullptr);

    struct ConfigEntry
    {
        ConfigEntry(
                std::string name,
                std::shared_ptr<AgentConfig> config,
                std::shared_ptr<AgentStats> stats) :
            name(std::move(name)),
            config(std::move(config)),
            stats(std::move(stats))
        {}

        void reset()
        {
            name = "";
            config.reset();
            stats.reset();
        }

        std::string name;
        std::shared_ptr<AgentConfig> config;
        std::shared_ptr<AgentStats> stats;
        BiddableSpots biddableSpots;
    };
    typedef std::vector<ConfigEntry> ConfigList;

    ConfigList filter(
            const BidRequest& br,
            const ExchangeConnector* conn,
            const ConfigSet& mask = ConfigSet(true));


    // \todo Need batch interfaces of these to alleviate overhead.
    void addFilter(const std::string& name);
    void removeFilter(const std::string& name);

    // \todo Need batch interfaces to alleviate overhead.
    unsigned addConfig(
            const std::string& name,
            const std::shared_ptr<AgentConfig>& config,
            const std::shared_ptr<AgentStats>& stats);
    void removeConfig(const std::string& name);

    static void initWithDefaultFilters(FilterPool& pool);

private:

    struct Data
    {
        Data() {}
        Data(const Data& other);
        ~Data();

        ssize_t findConfig(const std::string& name) const;
        unsigned addConfig(
                const std::string& name,
                const std::shared_ptr<AgentConfig>& config,
                const std::shared_ptr<AgentStats>& stats);
        void removeConfig(const std::string& name);

        ssize_t findFilter(const std::string& name) const;
        void addFilter(FilterBase* filter);
        void removeFilter(const std::string& name);

        // \todo Use unique_ptr when moving to gcc 4.7
        std::vector<FilterBase*> filters;

        std::vector<ConfigEntry> configs;
        CreativeMatrix activeConfigs;
    };

    bool setData(Data*&, std::unique_ptr<Data>&);
    void recordDiff(const Data* data, const FilterBase* f, const ConfigSet& diff);
    uint64_t recordTime(uint64_t ticks, const FilterBase* filter);

    std::atomic<Data*> data;
    std::vector< std::shared_ptr<AgentConfig> > configs;
    Datacratic::GcLock gc;

    EventRecorder* events;
};

} // namespace RTBKIT
