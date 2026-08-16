#include <evhttp_router.hpp>

#include <event2/event.h>
#include <event2/buffer.h>

#include <map>
#include <memory>
#include <random>
#include <sstream>

class LogService
{
private:
    std::map<std::string, std::string> _entries;
    static std::string generateLogId()
    {
        static const std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        static std::mt19937 _generator{std::random_device{}()};
        static std::uniform_int_distribution<> dist(0, charset.size() - 1);
        std::string result(8, ' ');
        for (auto &c : result)
        {
            c = charset[dist(_generator)];
        }
        return result;
    }
public:
    LogService() = default;
    ~LogService() = default;

    std::string addEntry(const std::string &entry)
    {
        std::string logId;
        do
        {
            // make sure log ID is unique
            logId = generateLogId();
        } while (_entries.find(logId) != _entries.end());
        _entries[logId] = entry;
        return logId;
    }

    void listEntries(std::ostringstream &oss)
    {
        for (auto it = _entries.begin(); it != _entries.end(); ++it)
        {
            oss << it->first;
            if (std::next(it) != _entries.end())
            {
                oss << ", ";
            }
        }
    }

    std::string getEntry(const std::string &logId)
    {
        std::string result;
        auto it = _entries.find(logId);
        if (it != _entries.end())
        {
            result = it->second;
        }
        return result;
    }

    bool deleteEntry(const std::string &logId)
    {
        return _entries.erase(logId) > 0;
    }
};

class EntriesHandler : public evhttprouter::Handler
{
private:
    LogService &_logService;
public:
    EntriesHandler(LogService &logService) : _logService(logService) {}

    void onGet(struct evhttp_request *req, const evhttprouter::PathVars &vars) override
    {
        std::ostringstream oss;
        _logService.listEntries(oss);
        std::string responseBody = oss.str();

        std::unique_ptr<evbuffer, decltype(&evbuffer_free)> buffer(evbuffer_new(), &evbuffer_free);
        evbuffer_add_printf(buffer.get(), "%s", responseBody.c_str());
    }

    void onPost(struct evhttp_request *req, const evhttprouter::PathVars &vars) override
    {
        struct evbuffer *inputBuffer;
        std::unique_ptr<evbuffer, decltype(&evbuffer_free)> outputBuffer(evbuffer_new(), &evbuffer_free);
        size_t inputLength;
        std::string input;
        std::string id;
        
        inputBuffer = evhttp_request_get_input_buffer(req);
        inputLength = evbuffer_get_length(inputBuffer);
        input.resize(inputLength);
        evbuffer_copyout(inputBuffer, input.data(), inputLength);
        id = _logService.addEntry(input);
        evhttp_send_reply(req, HTTP_OK, "OK", outputBuffer.get());
    }
};

class EntryHandler : public evhttprouter::Handler
{
private:
    LogService &_logService;
public:
    EntryHandler(LogService &logService) : _logService(logService) {}

    void onGet(struct evhttp_request *req, const evhttprouter::PathVars &vars) override
    {
        std::string logId = vars.get(0);
        std::string entry = _logService.getEntry(logId);
        std::unique_ptr<evbuffer, decltype(&evbuffer_free)> buffer(evbuffer_new(), &evbuffer_free);
        if (!entry.empty())
        {
            evbuffer_add_printf(buffer.get(), "%s", entry.c_str());
            evhttp_send_reply(req, HTTP_OK, "OK", buffer.get());
        }
        else
        {
            evhttp_send_reply(req, HTTP_NOTFOUND, "Not Found", nullptr);
        }
    }
    
    void onDelete(struct evhttp_request *req, const evhttprouter::PathVars &vars) override
    {
        std::string logId = vars.get(0);
        if (_logService.deleteEntry(logId))
        {
            evhttp_send_reply(req, HTTP_OK, "OK", nullptr);
        }
        else
        {
            evhttp_send_reply(req, HTTP_NOTFOUND, "Not Found", nullptr);
        }
    }
};

int main()
{
    LogService logService;
    EntriesHandler entriesHandler(logService);
    EntryHandler entryHandler(logService);

    evhttp *http;

    if(http)
    {
        evhttprouter::Router router(http);
        router.handle("/logs", &entriesHandler);
        router.handle("/logs/*", &entryHandler);

    }
    return 0;
}