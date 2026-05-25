#include <boost/asio.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/as_tuple.hpp>
#include <iostream>
#include <array>
#include <locale>

namespace asio = boost::asio;
using asio::ip::tcp;

asio::awaitable<void> echo_session(tcp::socket sock) {
    std::array<char, 1024> data;

    try {
        for (;;) {
            auto [ec, n] = co_await sock.async_read_some(
                asio::buffer(data),
                asio::as_tuple(asio::use_awaitable)
            );
            if (ec == asio::error::eof) {
                std::cout << "[Сервер] Беспилотник отключился (EOF)" << std::endl;
                break;
            }
            if (ec) {
                throw boost::system::system_error(ec);
            }

            std::string received(data.data(), n);
            std::cout << "[Сервер] Получено: " << received << std::endl;
            co_await asio::async_write(
                sock,
                asio::buffer(data.data(), n),
                asio::use_awaitable
            );

            std::cout << "[Сервер] Отправлен эхо-ответ: " << received << std::endl;
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "[Сервер] Ошибка в сессии: " << ex.what() << std::endl;
    }
}

asio::awaitable<void> accept_connections(asio::io_context& io_context, short port) {
    tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), port));
    std::cout << "[Сервер] Запущен на порту " << port << std::endl;

    while (true) {
        tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
        std::cout << "[Сервер] Беспилотник подключился" << std::endl;

        asio::co_spawn(
            io_context,
            echo_session(std::move(socket)),
            asio::detached
        );
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    const short port = 12345;

    try {
        asio::io_context io_context;
        asio::co_spawn(
            io_context,
            accept_connections(io_context, port),
            asio::detached
        );

        std::cout << "[Сервер] Эхо-сервер на корутинах запущен" << std::endl;
        std::cout << "[Сервер] Ожидание подключений на порту " << port << "..." << std::endl;
        io_context.run();

    }
    catch (const std::exception& e) {
        std::cerr << "[Сервер] Фатальная ошибка: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "[Сервер] Завершение работы" << std::endl;
    return 0;
}