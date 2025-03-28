/* Copyright 2019 Istio Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "source/extensions/filters/network/metadata_exchange/config.h"

#include "envoy/network/connection.h"
#include "envoy/registry/registry.h"
#include "source/extensions/filters/network/metadata_exchange/metadata_exchange.h"

namespace Envoy {
namespace Tcp {
namespace MetadataExchange {
namespace {

static constexpr char StatPrefix[] = "metadata_exchange.";

Network::FilterFactoryCb createFilterFactoryHelper(
    const envoy::tcp::metadataexchange::config::MetadataExchange& proto_config,
    Server::Configuration::ServerFactoryContext& context, FilterDirection filter_direction) {
  ASSERT(!proto_config.protocol().empty());

  absl::flat_hash_set<std::string> additional_labels;
  if (!proto_config.additional_labels().empty()) {
    for (const auto& label : proto_config.additional_labels()) {
      additional_labels.emplace(label);
    }
  }

  MetadataExchangeConfigSharedPtr filter_config(std::make_shared<MetadataExchangeConfig>(
      StatPrefix, proto_config.protocol(), filter_direction, proto_config.enable_discovery(),
      additional_labels, context, context.scope()));
  return [filter_config, &context](Network::FilterManager& filter_manager) -> void {
    filter_manager.addFilter(
        std::make_shared<MetadataExchangeFilter>(filter_config, context.localInfo()));
  };
}
} // namespace

absl::StatusOr<Network::FilterFactoryCb>
MetadataExchangeConfigFactory::createFilterFactoryFromProto(
    const Protobuf::Message& config, Server::Configuration::FactoryContext& context) {
  return createFilterFactory(
      dynamic_cast<const envoy::tcp::metadataexchange::config::MetadataExchange&>(config), context);
}

ProtobufTypes::MessagePtr MetadataExchangeConfigFactory::createEmptyConfigProto() {
  return std::make_unique<envoy::tcp::metadataexchange::config::MetadataExchange>();
}

Network::FilterFactoryCb MetadataExchangeConfigFactory::createFilterFactory(
    const envoy::tcp::metadataexchange::config::MetadataExchange& proto_config,
    Server::Configuration::FactoryContext& context) {
  return createFilterFactoryHelper(proto_config, context.serverFactoryContext(),
                                   FilterDirection::Downstream);
}

Network::FilterFactoryCb MetadataExchangeUpstreamConfigFactory::createFilterFactoryFromProto(
    const Protobuf::Message& config, Server::Configuration::UpstreamFactoryContext& context) {
  return createFilterFactory(
      dynamic_cast<const envoy::tcp::metadataexchange::config::MetadataExchange&>(config), context);
}

ProtobufTypes::MessagePtr MetadataExchangeUpstreamConfigFactory::createEmptyConfigProto() {
  return std::make_unique<envoy::tcp::metadataexchange::config::MetadataExchange>();
}

Network::FilterFactoryCb MetadataExchangeUpstreamConfigFactory::createFilterFactory(
    const envoy::tcp::metadataexchange::config::MetadataExchange& proto_config,
    Server::Configuration::UpstreamFactoryContext& context) {
  return createFilterFactoryHelper(proto_config, context.serverFactoryContext(),
                                   FilterDirection::Upstream);
}

/**
 * Static registration for the MetadataExchange Downstream filter. @see
 * RegisterFactory.
 */
static Registry::RegisterFactory<MetadataExchangeConfigFactory,
                                 Server::Configuration::NamedNetworkFilterConfigFactory>
    registered_;

/**
 * Static registration for the MetadataExchange Upstream filter. @see
 * RegisterFactory.
 */
static Registry::RegisterFactory<MetadataExchangeUpstreamConfigFactory,
                                 Server::Configuration::NamedUpstreamNetworkFilterConfigFactory>
    registered_upstream_;

} // namespace MetadataExchange
} // namespace Tcp
} // namespace Envoy
