#pragma once

#include <memory>
#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/gauge.h>
#include <prometheus/registry.h>
#include <string>

class PrometheusServer
{
  public:
    PrometheusServer(const std::string &address = "0.0.0.0:8082");

    // Packet counts
    void IncrementTCPReceived() noexcept;
    void IncrementTCPSent() noexcept;
    void IncrementUDPReceived() noexcept;
    void IncrementUDPSent() noexcept;

    // Bytes counters for throughput (Prometheus rate() will compute throughput)
    void AddTCPBytes(double bytes) noexcept;
    void AddUDPBytes(double bytes) noexcept;

    // Compression tracking
    void SetCompressionEnabled(bool enabled) noexcept;

    // Tick counter
    void IncrementTick() noexcept;

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
};
