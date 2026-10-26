/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright 2026 JASPAR
 */

#ifndef CONSUMER_INC_GRPC_VISS_GRPC_DATAPOINT_UTIL_H_
#define CONSUMER_INC_GRPC_VISS_GRPC_DATAPOINT_UTIL_H_

#include <string>

#include "databroker/consumer_get_datapoint.h"
#include "yoriito/viss/v1/types/value.grpc.pb.h"

namespace yoriito::databroker::consumer {

class VissGrpcDataPointUtil {
 public:
  static yoriito::databroker::ConsumerGetDataPoint ConvertDataPoint(
      const yoriito::viss::v1::DataPointCurrent& datapoint,
      const std::string& path);
};

}  // namespace yoriito::databroker::consumer

#endif  // CONSUMER_INC_GRPC_VISS_GRPC_DATAPOINT_UTIL_H_
