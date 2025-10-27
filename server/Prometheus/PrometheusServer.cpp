// PrometheusServer.cpp
#include "PrometheusServer.hpp"
#include <iostream>

PrometheusServer::PrometheusServer(const std::string &address)
    : exposer_(std::make_unique<prometheus::Exposer>(address)), registry_(std::make_shared<prometheus::Registry>()),

      tcp_received_counter_(&prometheus::BuildCounter()
                                 .Name("tcp_packets_received_total")
                                 .Help("Total TCP packets received")
                                 .Register(*registry_)
                                 .Add({})),
      tcp_sent_counter_(&prometheus::BuildCounter()
                             .Name("tcp_packets_sent_total")
                             .Help("Total TCP packets sent")
                             .Register(*registry_)
                             .Add({})),
      udp_received_counter_(&prometheus::BuildCounter()
                                 .Name("udp_packets_received_total")
                                 .Help("Total UDP packets received")
                                 .Register(*registry_)
                                 .Add({})),
      udp_sent_counter_(&prometheus::BuildCounter()
                             .Name("udp_packets_sent_total")
                             .Help("Total UDP packets sent")
                             .Register(*registry_)
                             .Add({})),

      tcp_bytes_counter_(&prometheus::BuildCounter()
                              .Name("tcp_bytes_total")
                              .Help("Total bytes sent/received over TCP")
                              .Register(*registry_)
                              .Add({})),
      udp_bytes_counter_(&prometheus::BuildCounter()
                              .Name("udp_bytes_total")
                              .Help("Total bytes sent/received over UDP")
                              .Register(*registry_)
                              .Add({})),

      tick_counter_(&prometheus::BuildCounter()
                         .Name("server_tick_total")
                         .Help("Total game server ticks executed")
                         .Register(*registry_)
                         .Add({})),

      compression_enabled_(&prometheus::BuildGauge()
                                .Name("compression_enabled")
                                .Help("Indicates whether packet compression is active (1 = enabled, 0 = disabled)")
                                .Register(*registry_)
                                .Add({})),

      tcp_kbps_(&prometheus::BuildGauge()
                     .Name("tcp_kbps")
                     .Help("Current TCP throughput in kilobytes per second")
                     .Register(*registry_)
                     .Add({})),
      udp_kbps_(&prometheus::BuildGauge()
                     .Name("udp_kbps")
                     .Help("Current UDP throughput in kilobytes per second")
                     .Register(*registry_)
                     .Add({}))
{
    exposer_->RegisterCollectable(registry_);
    last_update_ = std::chrono::steady_clock::now();
    std::cout << "Launched Prometheus" << std::endl;
}

void PrometheusServer::IncrementTCPReceived() noexcept
{
    tcp_received_counter_->Increment();
}
void PrometheusServer::IncrementTCPSent() noexcept
{
    tcp_sent_counter_->Increment();
}
void PrometheusServer::IncrementUDPReceived() noexcept
{
    udp_received_counter_->Increment();
}
void PrometheusServer::IncrementUDPSent() noexcept
{
    udp_sent_counter_->Increment();
}

void PrometheusServer::SetCompressionEnabled(bool enabled) noexcept
{
    compression_enabled_->Set(enabled ? 1.0 : 0.0);
}

void PrometheusServer::AddTCPBytes(double bytes) noexcept
{
    std::scoped_lock lock(throughput_mutex_);
    tcp_bytes_counter_->Increment(bytes);
}

void PrometheusServer::AddUDPBytes(double bytes) noexcept
{
    std::scoped_lock lock(throughput_mutex_);
    udp_bytes_counter_->Increment(bytes);
}

void PrometheusServer::IncrementTick() noexcept
{
    tick_counter_->Increment();
}

void PrometheusServer::UpdateThroughput() noexcept
{
    std::scoped_lock lock(throughput_mutex_);

    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - last_update_).count();
    if (elapsed < 1.0)
        return;

    double tcp_total = tcp_bytes_counter_->Value();
    double udp_total = udp_bytes_counter_->Value();

    double tcp_diff = tcp_total - last_tcp_bytes_;
    double udp_diff = udp_total - last_udp_bytes_;

    double tcp_kbps = (tcp_diff / 1024.0) / elapsed;
    double udp_kbps = (udp_diff / 1024.0) / elapsed;

    tcp_kbps_->Set(tcp_kbps);
    udp_kbps_->Set(udp_kbps);

    last_tcp_bytes_ = tcp_total;
    last_udp_bytes_ = udp_total;
    last_update_ = now;
}
