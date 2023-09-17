#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/http.hpp>

#include <memory>

namespace util {

class Ticker : public std::enable_shared_from_this<Ticker> {
  public:
    using Strand = boost::beast::net::strand<boost::beast::net::io_context::executor_type>;
    using Handler = std::function<void(std::chrono::milliseconds delta)>;

    // Функция handler будет вызываться внутри strand с интервалом period
    Ticker(Strand strand, std::chrono::milliseconds period, Handler handler);
    void Start();

  private:
    void ScheduleTick();
    void OnTick(boost::beast::error_code ec);

    using Clock = std::chrono::steady_clock;

    Strand strand_;
    std::chrono::milliseconds period_;
    boost::beast::net::steady_timer timer_{strand_};
    Handler handler_;
    std::chrono::steady_clock::time_point last_tick_;
};

} // namespace util