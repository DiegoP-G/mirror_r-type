// PrometheusServer.hpp
#pragma once

#include <chrono>
#include <memory>
#include <mutex>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <string>

class PrometheusServer
{
  public:
    PrometheusServer(const std::string &address = "0.0.0.0:8082");

    void IncrementTCPReceived() noexcept;
    void IncrementTCPSent() noexcept;
    void IncrementUDPReceived() noexcept;
    void IncrementUDPSent() noexcept;

    void AddTCPBytes(double bytes) noexcept;
    void AddUDPBytes(double bytes) noexcept;

    void SetCompressionEnabled(bool enabled) noexcept;
    void IncrementTick() noexcept;

    // ! Added: update throughput gauge (KB/s)
    void UpdateThroughput() noexcept;

  private:
    std::unique_ptr<prometheus::Exposer> exposer_;
    std::shared_ptr<prometheus::Registry> registry_;

    prometheus::Counter *tcp_received_counter_;
    prometheus::Counter *tcp_sent_counter_;
    prometheus::Counter *udp_received_counter_;
    prometheus::Counter *udp_sent_counter_;

    prometheus::Counter *tcp_bytes_counter_;
    prometheus::Counter *udp_bytes_counter_;

    prometheus::Counter *tick_counter_;
    prometheus::Gauge *compression_enabled_;

    // ! Added: instantaneous throughput gauges
    prometheus::Gauge *tcp_kbps_;
    prometheus::Gauge *udp_kbps_;

    // ! Added: tracking state
    double last_tcp_bytes_ = 0.0;
    double last_udp_bytes_ = 0.0;
    std::chrono::steady_clock::time_point last_update_;
    std::mutex throughput_mutex_;
};