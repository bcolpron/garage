#include <iostream>
#include <string_view>
#include <thread>
#include <mutex>
#include <future>
#include <boost/regex.hpp>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/system/error_code.hpp>
#include "httpserver/httpserver.h"
#include <nlohmann/json.hpp>

class SerialPort
{
public:
    SerialPort(const std::string& name, unsigned baud_rate) 
    {
        port.open(name);
        using namespace boost::asio;
        port.set_option(serial_port_base::baud_rate(baud_rate));
        port.set_option(serial_port_base::character_size(8));
        port.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));
        port.set_option(serial_port_base::parity(serial_port_base::parity::none));
        port.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
    }

    ~SerialPort() {
        runner.join();
    }

    void start()
    {
        read_some();
        runner = std::thread([this] {ios.run();});
    }

    std::string query() {
        port.write_some(boost::asio::buffer("GET state\n"));

        std::future<std::string> fut;
        {
            std::lock_guard<std::mutex> guard(mutex);
            promise = std::make_unique<std::promise<std::string>>();
            fut = promise->get_future();
        }
        return fut.get();
    };

    template<class Callback>
    void on_update(Callback&& f) {
        callback = f;
    }

private:

    void read_some() {
        using namespace std::placeholders;
        port.async_read_some(
            boost::asio::buffer(buffer + accumulated, sizeof(buffer) - accumulated),
            std::bind(&SerialPort::handle_read, this, _1, _2));
    }

    void handle_read(boost::system::error_code ec, std::size_t bytes_transferred) {
        accumulated += bytes_transferred;
        buffer[accumulated] = '\0';
        check_message();
        read_some();
    }

    void check_message() {
        boost::match_results<const char*> m;
        if (!boost::regex_search(buffer, m, re, boost::match_partial)) {
            accumulated = 0;    
        } else if (m[0].matched) {
            on_message(std::string(m[1].first, m[1].length()),
                std::string(m[2].first, m[2].length()));
            accumulated = 0;
        }
    }

    void on_message(const std::string& message, const std::string& state) {
        std::cout << message << " state=" << state << std::endl;
        if (message == "RESPONSE")
        {
            std::lock_guard<std::mutex> guard(mutex);
            if (promise) {
                promise->set_value(state);
                promise.reset();
            }
        }
        else if (message == "UPDATE")
        {
            callback(state);
        }
    }

    boost::asio::io_service ios;
    boost::asio::serial_port port{ios};
    char buffer[256];
    std::size_t accumulated = 0;
    boost::regex re{"\\[\\d+\\] (UPDATE|RESPONSE) state=(0|1)"};
    std::thread runner;
    std::mutex mutex;
    std::unique_ptr<std::promise<std::string>> promise;
    std::function<void(std::string)> callback;
};

std::string timestamp()
{
    time_t now;
    time(&now);
    char buf[sizeof "2011-10-08 07:07:09"];
    strftime(buf, sizeof buf, "%F %T", localtime(&now));
    return buf;
}

int main(int, const char **)
{
    SerialPort port{"/dev/ttyACM0", 9600};
    port.start();

    auto history = nlohmann::json::array();
    std::string last;

    port.on_update([&] (const std::string& new_state) {
        if (new_state != last) {
            nlohmann::json entry;
            entry["timestamp"] = timestamp();
            entry["value"] = new_state;
            entry["description"] = ("door is " + new_state == "0" ? "open" : "closed");
            history.push_back(entry);
            last = new_state;
        }
    });

    HttpServer server(12001);
    server.add_http_handler(http::verb::get, "/history", [&](auto&& req)
    {
        return make_response(req, history.dump());
    });
    server.add_http_handler(http::verb::get, "/current", [&](auto&& req)
    {
        return make_response(req, port.query());
    });
    server.add_http_handler(http::verb::get, "/", [&](auto&& req)
    {
        return make_response(req, "hello");
    });
    server.run();

    for(;;) 
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << port.query() << std::endl;
    }

    return 0;
}