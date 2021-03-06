#include <iostream>
#include <memory>
using namespace std;

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class Session final : public enable_shared_from_this<Session>
{
public:
	explicit Session(tcp::socket &&socket) noexcept : socket_(move(socket))
	{
		try {
			remote_endpoint_ = socket_.remote_endpoint();
		} catch (exception &e) {
			cerr << "Exception: " << e.what() << "\n";
		}
	}

	~Session() noexcept
	{
		boost::system::error_code _ignored_ec;
		cout << "Tuple(tcp, " << socket_.local_endpoint(_ignored_ec)
		     << ", " << remote_endpoint_ << ") has been clsoed" << endl;
	}

	void Run() noexcept { read(); }

private:
	void read() noexcept
	{
		socket_.async_read_some(
		    boost::asio::buffer(data_, kMaxLength),
		    [this, self = shared_from_this()](
			boost::system::error_code ec, size_t length) {
			    if (!ec) {
				    cout << "Receive data from "
					 << remote_endpoint_ << " => ";
				    cout.write(data_,
					       static_cast<streamsize>(length));
				    if (length > 0 &&
					data_[length - 1] != '\n') {
					    cout << '\n';
				    }
				    write(length);
				    return;
			    }
			    cout << remote_endpoint_ << " leave in read"
				 << endl;
		    });
	}

	void write(size_t length)
	{
		boost::asio::async_write(
		    socket_, boost::asio::buffer(data_, length),
		    [this, self = shared_from_this()](
			boost::system::error_code ec, size_t /* length */) {
			    if (!ec) {
				    read();
				    return;
			    }
			    cout << remote_endpoint_ << " leave in write"
				 << endl;
		    });
	}

private:
	constexpr static size_t kMaxLength = 1024;
	char data_[kMaxLength];

	tcp::socket socket_;
	tcp::endpoint remote_endpoint_;
};

class Server final
{
public:
	Server(boost::asio::io_context &io_context, unsigned short port)
	    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
	{
	}

	~Server() noexcept
	{
		boost::system::error_code _ignored_ec;
		cout << "Server(" << acceptor_.local_endpoint(_ignored_ec)
		     << ") has been closed" << endl;
	}

	void Run() noexcept
	{
		boost::system::error_code _ignored_ec;
		cout << "Listen on: " << acceptor_.local_endpoint(_ignored_ec)
		     << endl;
		accept();
	}

private:
	void accept() noexcept
	{
		acceptor_.async_accept(
		    [=](boost::system::error_code ec, tcp::socket &&socket) {
			    if (!ec) {
				    make_shared<Session>(move(socket))->Run();
			    }
			    accept();
		    });
	}

private:
	tcp::acceptor acceptor_;
};

int main(int argc, char *argv[])
{
	try {
		if (argc < 2) {
			cerr << "Usage: " << argv[0] << " [port]" << endl;

			return EXIT_FAILURE;
		}

		boost::asio::io_context io_context(1);
		Server server(io_context,
			      static_cast<unsigned short>(atoi(argv[1])));
		server.Run();
		io_context.run();
	} catch (exception &e) {
		cerr << "Exception: " << e.what() << endl;
	}

	return 0;
}
