//
// Created by kostya on 26.09.2021.
//

#include "Crawler.h"

mutex mtx;
queue<string> url_queue;

set<string> filter;

Parser::Parser() {
    _http_client.setTimeOut(5);
    _http_client.setSSLVerifyPeer(0);
    _http_client.setSSLVerifyHost(0);
}

set<string> Parser::getOutBoundUrls(string url) {
    string html;
    set<string> outbound_urls;

    if (_http_client.get(url, html)) {
        // regular which takes url and anchor
        const regex regex_rule("<a\\s+(?:[^>]*?\\s+)?href=\"([^\"]+)\"[^>]*>((?:.|\\s)*?)<\\/a>");

        sregex_token_iterator pos(html.cbegin(), html.cend(), regex_rule, {1, 2});
        sregex_token_iterator end;

        auto get_node_fn = [](sregex_token_iterator &pos) -> pair<string, string> {
            return {(*pos++).str(), (*pos).str()};
        };

        // Iterate over regex matches to find {url, anchor}
        while (pos != end) {
            auto [ahref, anchor] = get_node_fn(pos);

            if (ahref[0] == '/') {
                smatch match;
                regex_search(url, match, regex(R"(((?:https?:\/\/|www)[^\/]+)(?:\/|$))"));
                ahref = match.str(1) + ahref;
            }

            outbound_urls.emplace(ahref);

            ++pos;
        }
    }

    return outbound_urls;
}

Worker::Worker() {
    _th = thread(&Worker::_run, this);
}

Worker::~Worker() {
    _th.join();
}

void Worker::_run() {
    while (_running) {
        string url;

        {
            lock_guard lg(mtx);

            if (url_queue.empty()) {
                _running = false;
                return;
            }

            url = url_queue.front();
            url_queue.pop();
        }

        if (!url.empty()) {
            cout << "th_id: " << this_thread::get_id() << ", Worker url: " << url << endl;
            auto outbound_urls = _parser.getOutBoundUrls(url);

            if (!outbound_urls.empty()) {
                lock_guard lg(mtx);
                for (auto &outbound_url: outbound_urls) {
                    if (filter.emplace(outbound_url).second) {
                        url_queue.emplace(outbound_url);
                    }
                }
            }
        }
    }
}

bool Worker::getStatus() const {
    return !_running;
}

Crawler::Crawler(const string &url) {
    filter.emplace(url);
    url_queue.emplace(url);

    // Fill first links(outbound urls)
    {
        auto outbound_urls = Parser().getOutBoundUrls(url);

        for (auto &outbound_url: outbound_urls) {
            if (filter.emplace(outbound_url).second) {
                url_queue.emplace(outbound_url);
            }
        }
    }

    uint32_t th_length = thread::hardware_concurrency();
    if (th_length <= 1) {
        th_length = 1;
    } else {
        th_length--;
    }

    for (int i = 0; i < th_length; i++) {
        _workers.emplace_back(make_unique<Worker>());
    }
}

void Crawler::process() {
    while (true) {
        this_thread::sleep_for(chrono::milliseconds (500));

        auto cnt = _workers.size();
        for (auto &worker: _workers) {
            if (worker->getStatus()) cnt--;
        }

        if (!cnt) break;
    }

    cout << "Count: " << filter.size() << endl;
    for (auto &it: filter) {
        cout << it << endl;
    }
}
