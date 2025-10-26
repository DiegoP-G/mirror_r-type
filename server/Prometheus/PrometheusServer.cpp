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
                              .Help("Total bytes sent/received over TCP (compressed if applicable)")
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
                                .Add({}))
{
    exposer_->RegisterCollectable(registry_);
    std::cout << "Launched Prometheus" << std::endl;
}

// Packet count
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

// Compression
void PrometheusServer::SetCompressionEnabled(bool enabled) noexcept
{
    compression_enabled_->Set(enabled ? 1.0 : 0.0);
}

// Track bytes sent/received instead of instantaneous throughput
void PrometheusServer::AddTCPBytes(double bytes) noexcept
{
    tcp_bytes_counter_->Increment(bytes);
}

void PrometheusServer::AddUDPBytes(double bytes) noexcept
{
    udp_bytes_counter_->Increment(bytes);
}

// Tick tracking
void PrometheusServer::IncrementTick() noexcept
{
    tick_counter_->Increment();
}
