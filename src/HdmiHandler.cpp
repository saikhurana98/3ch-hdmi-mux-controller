#include "HdmiHandler.hpp"

HdmiHandler::HdmiHandler(Config &config)
{
    for (auto channel_pinout : *(config.hdmiChannelPinouts))
    {
        String channel = channel_pinout.first.c_str();
        pair<int, int> pinout = channel_pinout.second;
        Mux *mux = new Mux(channel_pinout.first.c_str(), pinout.first, pinout.second, 500, config);
        this->channelMuxMap[channel] = mux;
    }
}

Mux *HdmiHandler::getMuxById(String channelId)
{
    if (this->channelMuxMap.count(channelId) != 1)
    {
        Serial1.printf("Mux with Channel : %s Not found", channelId);
        return NULL;
    }
    return this->channelMuxMap[channelId];
}

void HdmiHandler::setSource(String channel, HdmiSource source)
{
    Mux *mux = this->channelMuxMap[channel.c_str()];
    mux->switchSource(source);
}
void HdmiHandler::setSource(String channel, String source)
{
    Mux *mux = this->channelMuxMap[channel.c_str()];
    mux->switchSource(source);
}
String HdmiHandler::getSourceString(String channel)
{
    Mux *mux = this->getMuxById(channel);
    if (mux == NULL)
    {
        return this->appConfig->hdmiSourceStringMap[HdmiSource::INVALID];
    }
    return this->appConfig->hdmiSourceStringMap[mux->currentSource];
}

HdmiSource HdmiHandler::getSource(String channel)
{
    Mux *mux = this->channelMuxMap[channel.c_str()];
    if (mux == NULL)
        return HdmiSource::INVALID;
    return mux->currentSource;
}

void HdmiHandler::setBootRestoreMode(RestoreMode mode, HdmiChannelSourceMap map)
{
    this->appConfig->setRestoreState(mode, map);
}
void HdmiHandler::setBootRestoreMode(String mode, JsonDocument map)
{

    HdmiChannelSourceMap sourceMap;
    if (this->appConfig->RestoreModeStringToEnum[mode] == RestoreMode::CUSTOM)
    {
        for (JsonPair channel_source : map.as<JsonObject>())
        {
            sourceMap[channel_source.key().c_str()] = this->appConfig->hdmiStringSourceMap[channel_source.value().as<const char *>()];
        }
    }

    this->appConfig->setRestoreState(this->appConfig->RestoreModeStringToEnum[mode], sourceMap);
}

void HdmiHandler::init()
{
    for (auto mux : this->channelMuxMap)
    {
        mux.second->init();
    }
}
void HdmiHandler::runtime()
{
    if (this->appConfig->currentRestoreMode == RestoreMode::RESTORE_PREVIOUS)
    {
        // Track States and keep updating
    }
}

std::vector<Task *> HdmiHandler::getJobs()
{
    std::vector<Task *> jobs;

    if (this->channelMuxMap.size() == 0)
    {
        Serial.printf("No Tasks were generated");
    }

    for (auto channel_mux : this->channelMuxMap)
    {
        jobs.push_back(
            new Task(TASK_IMMEDIATE, TASK_FOREVER, [channel_mux]()
                     { channel_mux.second->runtime(); }));
    }

    return jobs;
}
