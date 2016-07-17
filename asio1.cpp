#include <iostream>
#include <boost/asio.hpp>

int main () {

        // io_service を介して OS の IO を使う
        boost::asio::io_service io_service;

        // ソケットを作る
        boost::asio::ip::tcp::socket socket(io_service);

        // 名前解決をする
        // （ホスト名とサービス名から、 IP とポートを求める）
        std::cout << "resolving hostname ..." << std::endl;
        boost::asio::ip::tcp::resolver resolver(io_service);
        boost::asio::ip::tcp::resolver::query query("time.nist.gov", "daytime");
        boost::asio::ip::tcp::endpoint endpoint(*resolver.resolve(query));

        // つなぐ
        std::cout << "connecting to " << endpoint << "..." << std::endl;
        socket.connect(endpoint);

        // バッファ
        boost::array<char, 128> buf;

        while (true) {

                // エラーコード取得用変数
                boost::system::error_code error;

                // ソケットからの入力をバッファにコピー
                // （変数 buf の参照と変数 error の参照を渡す）
                std::size_t len = socket.read_some(boost::asio::buffer(buf), error);

                // ソケットからの入力が終わったら break;
                if (error == boost::asio::error::eof) {
                        break;
                }

                // 読み込んだデータを std::cout に出力
                std::cout.write(buf.data(), len);
        }

        // 正常終了
        return 0;
}