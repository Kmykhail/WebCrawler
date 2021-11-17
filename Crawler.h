//
// Created by kostya on 26.09.2021.
//

#ifndef WEBCRAWLER_CRAWLER_H
#define WEBCRAWLER_CRAWLER_H

#include <set>
#include <vector>
#include <list>
#include <queue>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <regex>

#include "PrimitiveHttpClient.h"

class Parser {
    PrimitiveHttpClient _http_client;

public:
    Parser();
    set<string> getOutBoundUrls(string url);
};

class Worker {
    Parser _parser;

    thread _th;

    bool _running {true};

    void _run();
public:
    Worker();
    ~Worker();

    [[nodiscard]] bool getStatus() const;
};

class Crawler {
private:

    list<string> _nodes;

    vector<unique_ptr<Worker>> _workers;

public:
    explicit Crawler(const string &url);

    void process();
};

#endif //WEBCRAWLER_CRAWLER_H
