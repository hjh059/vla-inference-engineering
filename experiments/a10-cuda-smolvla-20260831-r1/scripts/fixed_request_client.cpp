#include "vla.pb.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <zmq.hpp>

namespace {

constexpr int kChunkSize = 50;
constexpr int kActionDim = 32;
constexpr int kOutputCount = kChunkSize * kActionDim;
constexpr int kStateDim = 32;

struct Args {
    std::string addr = "tcp://127.0.0.1:5555";
    std::string image;
    std::string out_dir;
    int warmup = 5;
    int reps = 30;
};

[[noreturn]] void usage(const char *prog) {
    std::cerr << "usage: " << prog
              << " --image FILE --out DIR [--addr tcp://127.0.0.1:5555]"
              << " [--warmup 5] [--reps 30]\n";
    std::exit(2);
}

Args parse_args(int argc, char **argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        const std::string opt = argv[i];
        auto value = [&]() -> std::string {
            if (i + 1 == argc) usage(argv[0]);
            return argv[++i];
        };
        if (opt == "--addr") a.addr = value();
        else if (opt == "--image") a.image = value();
        else if (opt == "--out") a.out_dir = value();
        else if (opt == "--warmup") a.warmup = std::stoi(value());
        else if (opt == "--reps") a.reps = std::stoi(value());
        else usage(argv[0]);
    }
    if (a.image.empty() || a.out_dir.empty() || a.warmup < 0 || a.reps < 1) usage(argv[0]);
    return a;
}

std::string read_file(const std::string &path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open image: " + path);
    return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
}

vla::PredictRequest make_request(const std::string &jpeg, std::uint64_t request_id) {
    vla::PredictRequest req;
    req.set_request_id(request_id);
    auto *image = req.add_images();
    image->set_encoding(vla::Image::JPEG);
    image->set_data(jpeg);
    for (const int token : {1, 100, 200, 2}) req.add_lang_tokens(token);
    for (int i = 0; i < kStateDim; ++i) req.add_state(0.0f);
    for (int i = 0; i < kOutputCount; ++i) {
        const auto v = static_cast<std::uint32_t>(i) * 2654435761u;
        req.add_noise(0.001f * static_cast<float>(v % 1000u) - 0.5f);
    }
    return req;
}

vla::PredictResponse request(zmq::socket_t &sock, const vla::PredictRequest &req,
                             double &wall_ms) {
    const std::string bytes = req.SerializeAsString();
    const auto t0 = std::chrono::steady_clock::now();
    sock.send(zmq::buffer(bytes), zmq::send_flags::none);
    zmq::message_t reply;
    const auto received = sock.recv(reply, zmq::recv_flags::none);
    const auto t1 = std::chrono::steady_clock::now();
    if (!received) throw std::runtime_error("no response from vla-server");
    wall_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    vla::PredictResponse resp;
    if (!resp.ParseFromArray(reply.data(), static_cast<int>(reply.size())))
        throw std::runtime_error("cannot parse PredictResponse");
    if (!resp.error().empty()) throw std::runtime_error("vla-server: " + resp.error());
    if (resp.chunk_size() != kChunkSize || resp.action_dim() != kActionDim ||
        resp.action_chunk_size() != kOutputCount)
        throw std::runtime_error("unexpected output shape");
    for (float v : resp.action_chunk())
        if (!std::isfinite(v)) throw std::runtime_error("non-finite action value");
    return resp;
}

void write_actions(const std::filesystem::path &path, const vla::PredictResponse &resp) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot write " + path.string());
    out << std::setprecision(9);
    for (int r = 0; r < kChunkSize; ++r) {
        for (int c = 0; c < kActionDim; ++c) {
            if (c) out << ' ';
            out << resp.action_chunk(r * kActionDim + c);
        }
        out << '\n';
    }
}

}  // namespace

int main(int argc, char **argv) {
    try {
        const Args args = parse_args(argc, argv);
        const std::string jpeg = read_file(args.image);
        std::filesystem::create_directories(args.out_dir);

        zmq::context_t context(1);
        zmq::socket_t sock(context, zmq::socket_type::req);
        sock.set(zmq::sockopt::linger, 0);
        sock.set(zmq::sockopt::rcvtimeo, 120000);
        sock.connect(args.addr);

        std::uint64_t request_id = 1;
        for (int i = 0; i < args.warmup; ++i) {
            double ignored_ms = 0.0;
            request(sock, make_request(jpeg, request_id++), ignored_ms);
        }

        std::ofstream samples(std::filesystem::path(args.out_dir) / "samples.csv");
        if (!samples) throw std::runtime_error("cannot open samples.csv");
        samples << "sample,client_wall_ms,server_total_ms,server_vision_ms,server_inference_ms,server_prefill_ms,server_denoise_ms,bitwise_equal_reference\n";
        samples << std::setprecision(9);

        std::vector<float> reference;
        bool all_equal = true;
        for (int i = 0; i < args.reps; ++i) {
            double wall_ms = 0.0;
            const vla::PredictResponse resp = request(sock, make_request(jpeg, request_id++), wall_ms);
            const std::vector<float> action(resp.action_chunk().begin(), resp.action_chunk().end());
            bool equal = true;
            if (i == 0) {
                reference = action;
                write_actions(std::filesystem::path(args.out_dir) / "reference-actions.txt", resp);
            } else {
                equal = std::memcmp(reference.data(), action.data(), kOutputCount * sizeof(float)) == 0;
                all_equal = all_equal && equal;
            }
            samples << i << ',' << wall_ms << ',' << resp.latency_ms_total() << ','
                    << resp.latency_ms_vision() << ',' << resp.latency_ms_inference() << ','
                    << resp.latency_ms_prefill() << ',' << resp.latency_ms_denoise() << ','
                    << (equal ? "true" : "false") << '\n';
        }

        std::ofstream summary(std::filesystem::path(args.out_dir) / "summary.json");
        if (!summary) throw std::runtime_error("cannot write summary.json");
        summary << "{\n"
                << "  \"warmup\": " << args.warmup << ",\n"
                << "  \"reps\": " << args.reps << ",\n"
                << "  \"output_shape\": [" << kChunkSize << ", " << kActionDim << "],\n"
                << "  \"all_finite\": true,\n"
                << "  \"all_bitwise_equal_reference\": " << (all_equal ? "true" : "false") << "\n"
                << "}\n";
        std::cout << "saved " << args.reps << " samples; all_bitwise_equal_reference="
                  << (all_equal ? "true" : "false") << '\n';
        return all_equal ? 0 : 1;
    } catch (const std::exception &e) {
        std::cerr << "fixed_request_client: " << e.what() << '\n';
        return 1;
    }
}
